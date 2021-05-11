#include "vc_mipi_core.h"
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/delay.h>
#include "vc_mipi_modules.h"

#define MOD_REG_RESET           0x0100 // register  0 [0x0100]: reset and init register (R/W)
#define MOD_REG_STATUS          0x0101 // register  1 [0x0101]: status (R)
#define MOD_REG_MODE            0x0102 // register  2 [0x0102]: initialisation mode (R/W)
#define MOD_REG_IOCTRL          0x0103 // register  3 [0x0103]: input/output control (R/W)
#define MOD_REG_MOD_ADDR        0x0104 // register  4 [0x0104]: module i2c address (R/W, default: 0x10)
#define MOD_REG_SEN_ADDR        0x0105 // register  5 [0x0105]: sensor i2c address (R/W, default: 0x1A)
#define MOD_REG_OUTPUT          0x0106 // register  6 [0x0106]: output signal override register (R/W, default: 0x00)
#define MOD_REG_INPUT           0x0107 // register  7 [0x0107]: input signal status register (R)
#define MOD_REG_EXTTRIG         0x0108 // register  8 [0x0108]: external trigger enable (R/W, default: 0x00)
#define MOD_REG_EXPO_L          0x0109 // register  9 [0x0109]: exposure LSB (R/W, default: 0x10)
#define MOD_REG_EXPO_M          0x010A // register 10 [0x010A]: exposure 	   (R/W, default: 0x27)
#define MOD_REG_EXPO_H          0x010B // register 11 [0x010B]: exposure     (R/W, default: 0x00)
#define MOD_REG_EXPO_U          0x010C // register 12 [0x010C]: exposure MSB (R/W, default: 0x00)
#define MOD_REG_RETRIG_L        0x010D // register 13 [0x010D]: retrigger LSB (R/W, default: 0x40)
#define MOD_REG_RETRIG_M        0x010E // register 14 [0x010E]: retrigger     (R/W, default: 0x2D)
#define MOD_REG_RETRIG_H        0x010F // register 15 [0x010F]: retrigger     (R/W, default: 0x29)
#define MOD_REG_RETRIG_U        0x0110 // register 16 [0x0110]: retrigger MSB (R/W, default: 0x00)

#define REG_RESET_PWR_UP        0x00
#define REG_RESET_SENSOR        0x01   // reg0[0] = 0 sensor reset the sensor is held in reset when this bit is 1
#define REG_RESET_PWR_DOWN      0x02   // reg0[1] = 0 power down power for the sensor is switched off
#define REG_STATUS_NO_COM       0x00   // reg1[7:0] = 0x00 default, no communication with sensor possible
#define REG_STATUS_READY        0x80   // reg1[7:0] = 0x80 sensor ready after successful initialization sequence
#define REG_STATUS_ERROR        0x01   // reg1[7:0] = 0x01 internal error during initialization
#define REG_TRIGGER_IN_ENABLE   0x01
#define REG_TRIGGER_IN_DISABLE  0x00
#define REG_FLASH_OUT_ENABLE    0x01
#define REG_FLASH_OUT_DISABLE   0x00

#define SEN_MODE_STANDBY        0x01
#define SEN_MODE_OPERATING      0x00

#define MASK_MODE_LANES         (FLAG_MODE_2_LANES | FLAG_MODE_4_LANES)
#define MASK_IO_ENABLED         (FLAG_TRIGGER_IN | FLAG_FLASH_OUT)
#define MASK_FIND_MODE          (MASK_MODE_LANES | FLAG_TRIGGER_IN)


// ------------------------------------------------------------------------------------------------
//  Helper Functions for I2C Communication

#define U_BYTE(value) (__u8)((value >> 24) & 0xff)
#define H_BYTE(value) (__u8)((value >> 16) & 0xff)
#define M_BYTE(value) (__u8)((value >> 8) & 0xff)
#define L_BYTE(value) (__u8)(value & 0xff)

int i2c_read_reg(struct i2c_client *client, const __u16 addr)
{
	__u8 buf[2] = { addr >> 8, addr & 0xff };
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = 2,
			.buf = buf,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = buf,
		},
	};

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret < 0) {
		dev_err(&client->dev, "%s(): Reading register %x from %x failed\n", __FUNCTION__, addr, client->addr);
		return ret;
	}

	return buf[0];
}

int i2c_write_reg(struct device *dev, struct i2c_client *client, const __u16 addr, const __u8 value, const char* func)
{
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;
	__u8 tx[3];
	int ret;

	dev_dbg(dev, "%s():   addr: 0x%04x <= value: 0x%02x\n", func, addr, value);

	msg.addr = client->addr;
	msg.buf = tx;
	msg.len = 3;
	msg.flags = 0;
	tx[0] = addr >> 8;
	tx[1] = addr & 0xff;
	tx[2] = value;
	ret = i2c_transfer(adap, &msg, 1);
	mdelay(2);

	return ret == 1 ? 0 : -EIO;
}

int i2c_write_reg2(struct device *dev, struct i2c_client *client, struct vc_csr2 *csr, const __u16 value, const char* func)
{
	int ret = 0;

	if (csr->l)		
		ret  = i2c_write_reg(dev, client, csr->l, L_BYTE(value), func);
	if (csr->m)
		ret |= i2c_write_reg(dev, client, csr->m, M_BYTE(value), func);

	return ret;
}

__u32 i2c_read_reg3(struct device *dev, struct i2c_client *client, struct vc_csr3 *csr)
{
	int reg = 0;
	__u32 value = 0;

	reg = i2c_read_reg(client, csr->l);
	if (reg)
		value = (value << 8) | reg;
	reg = i2c_read_reg(client, csr->m);
	if (reg)
		value = (value << 8) | reg;
	reg = i2c_read_reg(client, csr->h);
	if (reg)
		value = reg;

	return value;
}

int i2c_write_reg3(struct device *dev, struct i2c_client *client, struct vc_csr3 *csr, const __u16 value, const char *func)
{
	int ret = 0;

	if (csr->l)
		ret = i2c_write_reg(dev, client, csr->l, L_BYTE(value), func);
	if (csr->m)
		ret |= i2c_write_reg(dev, client, csr->m, M_BYTE(value), func);
	if (csr->h)
		ret |= i2c_write_reg(dev, client, csr->h, H_BYTE(value), func);

	return ret;
}


// ------------------------------------------------------------------------------------------------
//  Helper Functions for debugging

void vc_dump_reg_value2(struct device *dev, int addr, int reg)
{
	int sval = 0; // short 2-byte value
	if (addr & 1) { // odd addr
		sval |= (int)reg << 8;
		dev_dbg(dev, "%s(): addr=0x%04x reg=0x%04x\n", __FUNCTION__, addr + 0x1000 - 1, sval);
	}
}

void vc_dump_hw_desc(struct device *dev, struct vc_desc *desc)
{
	dev_info(dev, "VC MIPI Module - Hardware Descriptor\n");
	dev_info(dev, "[ MAGIC  ] [ %s ]\n", desc->magic);
	dev_info(dev, "[ MANUF. ] [ %s ] [ MID=0x%04x ]\n", desc->manuf, desc->manuf_id);
	dev_info(dev, "[ SENSOR ] [ %s %s ]\n", desc->sen_manuf, desc->sen_type);
	dev_info(dev, "[ MODULE ] [ ID=0x%04x ] [ REV=0x%04x ]\n", desc->mod_id, desc->mod_rev);
	dev_info(dev, "[ MODES  ] [ NR=0x%04x ] [ BPM=0x%04x ]\n", desc->nr_modes, desc->bytes_per_mode);
}


// ------------------------------------------------------------------------------------------------
//  Helper functions for internal data structures

struct vc_framefmt *vc_core_find_framefmt(struct vc_cam *cam, __u32 code)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct device *dev = &ctrl->client_sen->dev; 
	struct vc_framefmt *fmts = ctrl->fmts;

	int i;

	for (i = 0; fmts[i].code != 0; i++) {
		struct vc_framefmt *fmt = &fmts[i];
		dev_dbg(dev, "%s(): Checking format (code: 0x%04x)", __FUNCTION__, fmt->code);
		if(fmt->code == code) {
			return fmt;
		}
	}
	return NULL;
}


// ------------------------------------------------------------------------------------------------
//  Helper Functions for the VC MIPI Controller Module

struct i2c_client *vc_mod_get_client(struct i2c_adapter *adapter, __u8 i2c_addr)
{
	struct i2c_board_info info = {
		I2C_BOARD_INFO("i2c", i2c_addr),
	};
	unsigned short addr_list[2] = { i2c_addr, I2C_CLIENT_END };
	return i2c_new_probed_device(adapter, &info, addr_list, NULL);
}

int vc_mod_set_power(struct vc_cam *cam, int on)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client_mod = ctrl->client_mod;
	struct device *dev = &client_mod->dev;
	int ret;

	dev_dbg(dev, "%s(): Set module power: %s\n", __FUNCTION__, on ? "up" : "down");

	ret = i2c_write_reg(dev, client_mod, MOD_REG_RESET, on ? REG_RESET_PWR_UP : REG_RESET_PWR_DOWN, __FUNCTION__);
	if (ret)
		dev_err(dev, "%s(): Unable to power %s the module (error: %d)\n", __FUNCTION__,
			(on == REG_RESET_PWR_UP) ? "up" : "down", ret);

	return ret;
}

int vc_mod_get_status(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	int ret;

	ret = i2c_read_reg(client, MOD_REG_STATUS);
	if (ret < 0)
		dev_err(dev, "%s(): Unable to get module status (error: %d)\n", __FUNCTION__, ret);
	else
		dev_dbg(dev, "%s(): Get module status: 0x%02x\n", __FUNCTION__, ret);

	return ret;
}

int vc_mod_set_trigger_in(struct i2c_client *client, int enable)
{
	struct device *dev = &client->dev;
	int ret;

	dev_dbg(dev, "%s(): Set external trigger: %s\n", __FUNCTION__, enable ? "on" : "off");

	ret = i2c_write_reg(dev, client, MOD_REG_EXTTRIG, enable, __FUNCTION__);
	if (ret)
		dev_err(dev, "%s(): Unable to set external trigger (error: %d)\n", __FUNCTION__, ret);

	return ret;
}

int vc_mod_set_flash_out(struct i2c_client *client, int enable)
{
	struct device *dev = &client->dev;
	int ret;

	dev_dbg(dev, "%s(): Set flash output: %s\n", __FUNCTION__, enable ? "on" : "off");

	ret = i2c_write_reg(dev, client, MOD_REG_IOCTRL, enable, __FUNCTION__);
	if (ret)
		dev_err(dev, "%s(): Unable to set flash output (error: %d)\n", __FUNCTION__, ret);

	return ret;
}

int vc_mod_wait_until_module_is_ready(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	int status;
	int try;

	dev_dbg(dev, "%s(): Wait until module is ready\n", __FUNCTION__);

	status = REG_STATUS_NO_COM;
	try = 0;
	while (status == REG_STATUS_NO_COM && try < 10) {
		mdelay(100);
		status = vc_mod_get_status(client);
		try++;
	}
	if (status == REG_STATUS_ERROR) {
		dev_err(dev, "%s(): Internal Error!", __func__);
		return -EIO;
	}

	dev_dbg(dev, "%s(): Module is ready!\n", __FUNCTION__);
	return 0;
}

int vc_mod_setup(struct vc_ctrl *ctrl, int mod_i2c_addr, struct vc_desc *desc)
{
	struct i2c_client *client_sen = ctrl->client_sen;
	struct i2c_adapter *adapter = client_sen->adapter;
	struct device *dev_sen = &client_sen->dev;
	struct i2c_client *client_mod;
	struct device *dev_mod;
	int addr, reg;

	dev_dbg(dev_sen, "%s(): Setup the module\n", __FUNCTION__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev_sen, "%s(): I2C-Adapter doesn't support I2C_FUNC_SMBUS_BYTE\n", __FUNCTION__);
		return 0;
	}

	client_mod = vc_mod_get_client(adapter, mod_i2c_addr);
	if (client_mod == 0) {
		dev_err(dev_sen, "%s(): Unable to get module I2C client for address 0x%02x\n", __FUNCTION__, mod_i2c_addr);
		return -EIO;
	}

	dev_mod = &client_mod->dev;
	for (addr = 0; addr < sizeof(*desc); addr++) {
		reg = i2c_read_reg(client_mod, addr + 0x1000);
		if (reg < 0) {
			i2c_unregister_device(client_mod);
			return -EIO;
		}
		*((char *)(desc) + addr) = (char)reg;

		//vc_dump_reg_value2(dev, addr, reg);
	}

	vc_dump_hw_desc(dev_mod, desc);
	ctrl->client_mod = client_mod;
	ctrl->mod_i2c_addr = mod_i2c_addr;

	dev_info(dev_mod, "VC MIPI Sensor succesfully initialized.");
	return 0;
}

void vc_core_state_init(struct vc_ctrl *ctrl, struct vc_state *state)
{
	state->fmt = &ctrl->fmts[ctrl->default_fmt];
	state->width = ctrl->o_width;
	state->height = ctrl->o_height;	
	state->streaming = 0;
	state->flags = 0x00;
}

int vc_core_init(struct vc_cam *cam, struct i2c_client *client) 
{
	int ret;

	cam->ctrl.client_sen = client;
	ret = vc_mod_setup(&cam->ctrl, 0x10, &cam->desc);
	if (ret) {
		return -EIO;
	}
	ret = vc_mod_ctrl_init(&cam->ctrl, &cam->desc);
	if (ret) {
		return -EIO;
	}
	vc_core_state_init(&cam->ctrl, &cam->state);
	return 0;
}

int vc_mod_set_exposure(struct i2c_client *client, __u32 value, __u32 sen_clk)
{
	struct device *dev = &client->dev;
	int ret;

	__u32 exposure = (value * (sen_clk / 1000000));

	dev_dbg(dev, "%s(): Set module exposure = 0x%08x (%u)\n", __FUNCTION__, exposure, exposure);

	ret  = i2c_write_reg(dev, client, MOD_REG_EXPO_L, L_BYTE(exposure), __FUNCTION__);
	ret |= i2c_write_reg(dev, client, MOD_REG_EXPO_M, M_BYTE(exposure), __FUNCTION__);
	ret |= i2c_write_reg(dev, client, MOD_REG_EXPO_H, H_BYTE(exposure), __FUNCTION__);
	ret |= i2c_write_reg(dev, client, MOD_REG_EXPO_U, U_BYTE(exposure), __FUNCTION__);
	
	return ret;
}

struct vc_mode *vc_mod_find_mode(struct device *dev, struct vc_mode *modes, __u32 code, __u8 flags)
{
	__u8 mode_flags = flags & MASK_FIND_MODE;
	int i;

	for (i = 0; modes[i].code != 0; i++) {
		struct vc_mode *mode = &modes[i];
		dev_dbg(dev, "%s(): Checking mode (code: 0x%04x, flags: 0x%02x, value: 0x%02x)", __FUNCTION__, 
			mode->code, mode->flags, mode->value);
		if(mode->code == code && mode->flags == mode_flags) {
			return mode;
		}
	}
	return NULL;
}

int vc_mod_write_mode(struct i2c_client *client, int mode)
{
	struct device *dev = &client->dev;
	int ret;

	dev_dbg(dev, "%s(): Write module mode: 0x%02x\n", __FUNCTION__, mode);

	ret = i2c_write_reg(dev, client, MOD_REG_MODE, mode, __FUNCTION__);
	if (ret)
		dev_err(dev, "%s(): Unable to write module mode: 0x%02x (error: %d)\n", __FUNCTION__, mode, ret);

	return ret;
}

int vc_mod_reset_module(struct vc_cam *cam, int mode)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client = ctrl->client_mod;
	struct device *dev = &client->dev;
	int ret;

	dev_dbg(dev, "%s(): Reset the module!\n", __FUNCTION__);

	// TODO: Check if it really necessary to set mode in power down state.
	ret = vc_mod_set_power(cam, 0);
	ret |= vc_mod_write_mode(client, mode);
	ret |= vc_mod_set_power(cam, 1);
	ret |= vc_mod_wait_until_module_is_ready(client);

	return ret;
}

int vc_mod_set_mode(struct vc_cam *cam)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct i2c_client *client_mod = ctrl->client_mod;
	struct device *dev = &client_mod->dev;
	struct vc_mode *mode;
	__u8 flags = 0;
	int ret = 0;
	
	flags = ctrl->flags & (state->flags | MASK_MODE_LANES);

	dev_dbg(dev, "%s(): Set module mode: (code: 0x%04x, flags: 0x%02x)\n", __FUNCTION__, state->fmt->code, flags);

	mode = vc_mod_find_mode(dev, ctrl->modes, state->fmt->code, flags);
	if (mode == NULL) {
		mode = &ctrl->modes[ctrl->default_mode];
		dev_err(dev, "%s(): Mode (code: 0x%04x, flags: 0x%02x) not supported! (Set default mode (code: 0x%04x, flags: 0x%02x, value: 0x%02x))\n", 
			__FUNCTION__, state->fmt->code, flags, mode->code, mode->flags, mode->value);
	}

	ret  = vc_mod_reset_module(cam, mode->value);
	if (ret) {
		dev_err(dev, "%s(): Unable to set mode: (code: 0x%04x, flags: 0x%02x, value: 0x%02x) (error=%d)\n", __func__, 
			mode->code, mode->flags, mode->value, ret);
		return ret;
	}

	return ret;
}


// ------------------------------------------------------------------------------------------------
//  Helper Functions for the VC MIPI Sensors

int vc_sen_write_mode(struct vc_ctrl *ctrl, int mode) 
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;
	__u8 value;
	int ret = 0;

	dev_dbg(dev, "%s(): Write sensor mode: %s\n", __FUNCTION__, (mode == SEN_MODE_STANDBY)? "standby" : "operating");

	// TODO: Check if it is realy nessesary to swap order of write opertations.
	if(mode == SEN_MODE_STANDBY) {
		value = SEN_MODE_STANDBY;
		if(ctrl->csr.sen.mode.l) {
			ret = i2c_write_reg(dev, client, ctrl->csr.sen.mode.l, value, __FUNCTION__);
		}
		if(ctrl->csr.sen.mode.m) {
			ret |= i2c_write_reg(dev, client, ctrl->csr.sen.mode.m, value, __FUNCTION__);
		}
	} else {
		value = SEN_MODE_OPERATING;
		if(ctrl->csr.sen.mode.m) {
			ret |= i2c_write_reg(dev, client, ctrl->csr.sen.mode.m, value, __FUNCTION__);
		}
		if(ctrl->csr.sen.mode.l) {
			ret = i2c_write_reg(dev, client, ctrl->csr.sen.mode.l, value, __FUNCTION__);
		}
	}
	if (ret) 
		dev_err(dev, "%s(): Couldn't write sensor mode: 0x%02x (error=%d)\n", __FUNCTION__, mode, ret);

	return ret;
}

int vc_sen_set_roi(struct vc_cam *cam, int width, int height)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;
	int ret;

	dev_dbg(dev, "%s(): Set sensor roi: (width: %u, height: %u)\n", __FUNCTION__, width, height);

	ret  = vc_sen_write_mode(ctrl, SEN_MODE_STANDBY);
	ret |= i2c_write_reg2(dev, client, &ctrl->csr.sen.o_width, width, __FUNCTION__);
	ret |= i2c_write_reg2(dev, client, &ctrl->csr.sen.o_height, height, __FUNCTION__);
	ret |= vc_sen_write_mode(ctrl, SEN_MODE_OPERATING);
	if (ret) 
		dev_err(dev, "%s(): Couldn't set sensor roi: (width: %u, height: %u) (error=%d)\n", __FUNCTION__, width, height, ret);

	return ret;
}

__u32 vc_sen_read_vmax(struct vc_ctrl *ctrl)
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;
	__u32 vmax = i2c_read_reg3(dev, client, &ctrl->csr.sen.vmax);

	dev_dbg(dev, "%s(): Read sensor VMAX: 0x%08x (%d)\n", __FUNCTION__, vmax, vmax);

	return vmax;
}

int vc_sen_write_vmax(struct vc_ctrl *ctrl, __u32 vmax)
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;

	dev_dbg(dev, "%s(): Write sensor VMAX: 0x%08x (%d)\n", __FUNCTION__, vmax, vmax);

	return i2c_write_reg3(dev, client, &ctrl->csr.sen.vmax, vmax, __FUNCTION__);
}

int vc_sen_write_exposure(struct vc_ctrl *ctrl, __u32 exposure)
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;

	dev_dbg(dev, "%s(): Write sensor exposure: 0x%08x (%d)\n", __FUNCTION__, exposure, exposure);

	return i2c_write_reg3(dev, client, &ctrl->csr.sen.expo, exposure, __FUNCTION__);
}

int vc_sen_set_gain(struct vc_cam *cam, int value)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;
	int ret = 0;

	dev_dbg(dev, "%s(): Set sensor gain: %u\n", __FUNCTION__, value);

	ret = i2c_write_reg2(dev, client, &ctrl->csr.sen.gain, value, __FUNCTION__);;
	if (ret)
		dev_err(dev, "%s(): Couldn't set 'Gain' (error=%d)\n", __FUNCTION__, ret);

	return ret;
}

int vc_sen_start_stream(struct vc_cam *cam)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct i2c_client *client_mod = ctrl->client_mod;
	struct device *dev = &ctrl->client_sen->dev;
	int ret = 0;

	dev_dbg(dev, "%s(): Start streaming\n", __FUNCTION__);

	if(ctrl->flags & MASK_IO_ENABLED) {
		ret  = vc_mod_set_trigger_in(client_mod, 
			(state->flags & FLAG_TRIGGER_IN) ? REG_TRIGGER_IN_ENABLE : REG_TRIGGER_IN_DISABLE);
		ret |= vc_mod_set_flash_out(client_mod, 
			(state->flags & FLAG_FLASH_OUT) ? REG_FLASH_OUT_ENABLE : REG_FLASH_OUT_DISABLE);
	}

	ret |= vc_sen_write_mode(ctrl, SEN_MODE_OPERATING);
	if (ret)
		dev_err(dev, "%s(): Unable to start streaming (error=%d)\n", __FUNCTION__, ret);

	return ret;
}

int vc_sen_stop_stream(struct vc_cam *cam)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client_mod = ctrl->client_mod;
	struct device *dev = &ctrl->client_sen->dev;
	int ret = 0;

	dev_dbg(dev, "%s(): Stop streaming\n", __FUNCTION__);

	if (ctrl->flags & MASK_IO_ENABLED) {
		ret |= vc_mod_set_trigger_in(client_mod, REG_TRIGGER_IN_DISABLE);
		ret |= vc_mod_set_flash_out(client_mod, REG_FLASH_OUT_DISABLE);
	}

	ret |= vc_sen_write_mode(ctrl, SEN_MODE_STANDBY);
	if (ret)
		dev_err(dev, "%s(): Unable to stop streaming (error=%d)\n", __FUNCTION__, ret);

	return ret;
}


// ------------------------------------------------------------------------------------------------

int vc_sen_set_exposure_dirty(struct vc_cam *cam, int value)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct i2c_client *client_sen = ctrl->client_sen;
	struct i2c_client *client_mod = ctrl->client_mod;
	struct device *dev = &client_sen->dev;
	__u32 exposure = 0;
	__u32 vmax = 0;
	int ret = 0;

	if (state->flags & FLAG_TRIGGER_IN) {
		return vc_mod_set_exposure(client_mod, value, ctrl->sen_clk);
	}

	dev_dbg(dev, "%s(): Set sensor exposure: %u us\n", __FUNCTION__, value);
	dev_dbg(dev, "%s(): Checking Limits Min1: %u, Min2: %u, Max: %u us\n", __FUNCTION__,
		ctrl->set.exposure.min, ctrl->expo_time_min2, ctrl->set.exposure.max);

	// TODO: It is assumed, that the exposure value is valid => remove clamping.
	if (value < ctrl->set.exposure.min)
		value = ctrl->set.exposure.min;
	if (value > ctrl->set.exposure.max)
		value = ctrl->set.exposure.max;

	if (value < ctrl->expo_time_min2) {
		dev_dbg(dev, "%s(): Set exposure by method 1 (%u < Min2 %u)\n", __FUNCTION__, value, ctrl->expo_time_min2);
		switch (ctrl->mod_id) {
		case MOD_ID_IMX226:
			{
				// TODO: Find out which version is correct.
				// __u32 vmax = vc_sen_read_vmax(client);
				vmax = ctrl->expo_vmax;

				// exposure = (NumberOfLines - exp_time / 1Hperiod + toffset / 1Hperiod )
				// shutter = {VMAX - SHR}*HMAX + 209(157) clocks
				exposure = (vmax - ((__u32)(value)*16384 - ctrl->expo_toffset) / ctrl->expo_h1period);
			}
			break;

		case MOD_ID_IMX327:
			{
				vmax = 0x0465; // 1125
			}
			{
				// range 1..1123
				__u32 mode = 0; // Vorläufig 0 setzen bis set mode richtig implementiert ist.
				__u32 lf = mode ? 2 : 1;
				exposure = (1124 * 20000 - (__u32)(value) * 29 * 20 * lf) / 20000;
			}
			break;
		}

		// TODO: Is it realy nessecary to write the same vmax value back?


	} else {
		dev_dbg(dev, "%s(): Set exposure by method 2 (%u >= Min2 %u)\n", __FUNCTION__, value, ctrl->expo_time_min2);
		switch (ctrl->mod_id) {
		case MOD_ID_IMX226:
			{
				// exposure = 5 + ((unsigned long long)(value * 16384) - tOffset)/h1Period;
				// __u64 divresult = ((__u64)value * 16384) - ctrl->expo_toffset;
				// __u32 divisor   = IMX226_EXPO_H1PERIOD;
				// __u32 remainder = (__u32)(do_div(divresult, divisor)); // caution: division result value at dividend!
				// vmax = 5 + (__u32)divresult;
				vmax = ((__u64)value * 16384) - ctrl->expo_toffset;
				exposure = 0x0004;
			}
			break;

		case MOD_ID_IMX327:
			{
				// range 1123..
				__u32 mode = 0; // Vorläufig 0 setzen bis set mode richtig implementiert ist.
				__u32 lf = mode ? 2 : 1;
				vmax = ( 1 * 20000 + (__u32)(value) * 29 * 20 * lf ) / 20000;
				exposure = 0x0001;
			}
			break;
		}
	}

	switch (ctrl->mod_id) {
	case MOD_ID_IMX226:
	case MOD_ID_IMX327:
		ret  = vc_sen_write_vmax(ctrl, vmax);
		ret |= vc_sen_write_exposure(ctrl, exposure);
		break;
	}

	return ret;
}