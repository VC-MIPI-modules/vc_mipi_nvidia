#include "vc_mipi_core.h"
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/v4l2-mediabus.h>
#include "vc_mipi_modules.h"

#define MOD_REG_RESET            0x0100 // register  0 [0x0100]: reset and init register (R/W)
#define MOD_REG_STATUS           0x0101 // register  1 [0x0101]: status (R)
#define MOD_REG_MODE             0x0102 // register  2 [0x0102]: initialisation mode (R/W)
#define MOD_REG_IOCTRL           0x0103 // register  3 [0x0103]: input/output control (R/W)
#define MOD_REG_MOD_ADDR         0x0104 // register  4 [0x0104]: module i2c address (R/W, default: 0x10)
#define MOD_REG_SEN_ADDR         0x0105 // register  5 [0x0105]: sensor i2c address (R/W, default: 0x1A)
#define MOD_REG_OUTPUT           0x0106 // register  6 [0x0106]: output signal override register (R/W, default: 0x00)
#define MOD_REG_INPUT            0x0107 // register  7 [0x0107]: input signal status register (R)
#define MOD_REG_EXTTRIG          0x0108 // register  8 [0x0108]: external trigger enable (R/W, default: 0x00)
#define MOD_REG_EXPO_L           0x0109 // register  9 [0x0109]: exposure LSB (R/W, default: 0x10)
#define MOD_REG_EXPO_M           0x010A // register 10 [0x010A]: exposure 	   (R/W, default: 0x27)
#define MOD_REG_EXPO_H           0x010B // register 11 [0x010B]: exposure     (R/W, default: 0x00)
#define MOD_REG_EXPO_U           0x010C // register 12 [0x010C]: exposure MSB (R/W, default: 0x00)
#define MOD_REG_RETRIG_L         0x010D // register 13 [0x010D]: retrigger LSB (R/W, default: 0x40)
#define MOD_REG_RETRIG_M         0x010E // register 14 [0x010E]: retrigger     (R/W, default: 0x2D)
#define MOD_REG_RETRIG_H         0x010F // register 15 [0x010F]: retrigger     (R/W, default: 0x29)
#define MOD_REG_RETRIG_U         0x0110 // register 16 [0x0110]: retrigger MSB (R/W, default: 0x00)

#define REG_RESET_PWR_UP         0x00
#define REG_RESET_SENSOR         0x01   // reg0[0] = 0 sensor reset the sensor is held in reset when this bit is 1
#define REG_RESET_PWR_DOWN       0x02   // reg0[1] = 0 power down power for the sensor is switched off
#define REG_STATUS_NO_COM        0x00   // reg1[7:0] = 0x00 default, no communication with sensor possible
#define REG_STATUS_READY         0x80   // reg1[7:0] = 0x80 sensor ready after successful initialization sequence
#define REG_STATUS_ERROR         0x01   // reg1[7:0] = 0x01 internal error during initialization

#define REG_IO_DISABLE     	 0x00
#define REG_IO_FLASH_ENABLE      0x01
#define REG_IO_XTRIG_ENABLE    	 0x09

#define REG_TRIGGER_DISABLE      0x00
#define REG_TRIGGER_EXTERNAL     0x01
#define REG_TRIGGER_PULSEWIDTH   0x02
#define REG_TRIGGER_SELF         0x04
#define REG_TRIGGER_SINGLE       0x08
#define REG_TRIGGER_SYNC         0x10
#define REG_TRIGGER_STREAM_EDGE  0x20
#define REG_TRIGGER_STREAM_LEVEL 0x60


// ------------------------------------------------------------------------------------------------
//  Helper Functions for I2C Communication

#define U_BYTE(value) (__u8)((value >> 24) & 0xff)
#define H_BYTE(value) (__u8)((value >> 16) & 0xff)
#define M_BYTE(value) (__u8)((value >>  8) & 0xff)
#define L_BYTE(value) (__u8)((value >>  0) & 0xff)

static __u8 i2c_read_reg(struct i2c_client *client, const __u16 addr)
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
		vc_err(&client->dev, "%s(): Reading register 0x%04x from 0x%02x failed\n", __FUNCTION__, addr, client->addr);
		return ret;
	}

	return buf[0];
}

static int i2c_write_reg(struct device *dev, struct i2c_client *client, const __u16 addr, const __u8 value, const char* func)
{
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;
	__u8 tx[3];
	int ret;

	vc_dbg(dev, "%s():   addr: 0x%04x <= value: 0x%02x\n", func, addr, value);

	msg.addr = client->addr;
	msg.buf = tx;
	msg.len = 3;
	msg.flags = 0;
	tx[0] = addr >> 8;
	tx[1] = addr & 0xff;
	tx[2] = value;
	ret = i2c_transfer(adap, &msg, 1);

	return ret == 1 ? 0 : -EIO;
}

static __u32 i2c_read_reg2(struct device *dev, struct i2c_client *client, struct vc_csr2 *csr)
{
	__u32 reg = 0;
	__u32 value = 0;

	reg = i2c_read_reg(client, csr->l);
	if (reg)
		value |= (0x000000ff & reg);
	reg = i2c_read_reg(client, csr->m);
	if (reg)
		value |= (0x000000ff & reg) <<  8;

	return value;
}

static int i2c_write_reg2(struct device *dev, struct i2c_client *client, struct vc_csr2 *csr, const __u16 value, const char* func)
{
	int ret = 0;

	if (csr->l)		
		ret  = i2c_write_reg(dev, client, csr->l, L_BYTE(value), func);
	if (csr->m)
		ret |= i2c_write_reg(dev, client, csr->m, M_BYTE(value), func);

	return ret;
}

static __u32 i2c_read_reg4(struct device *dev, struct i2c_client *client, struct vc_csr4 *csr)
{
	__u32 reg = 0;
	__u32 value = 0;

	reg = i2c_read_reg(client, csr->l);
	if (reg)
		value |= (0x000000ff & reg);
	reg = i2c_read_reg(client, csr->m);
	if (reg)
		value |= (0x000000ff & reg) <<  8;
	reg = i2c_read_reg(client, csr->h);
	if (reg)
		value |= (0x000000ff & reg) << 16;
	reg = i2c_read_reg(client, csr->u);
	if (reg)
		value |= (0x000000ff & reg) << 24;

	return value;
}

static int i2c_write_reg4(struct device *dev, struct i2c_client *client, struct vc_csr4 *csr, const __u32 value, const char *func)
{
	int ret = 0;

	if (csr->l)
		ret = i2c_write_reg(dev, client, csr->l, L_BYTE(value), func);
	if (csr->m)
		ret |= i2c_write_reg(dev, client, csr->m, M_BYTE(value), func);
	if (csr->h)
		ret |= i2c_write_reg(dev, client, csr->h, H_BYTE(value), func);
	if (csr->u)
		ret |= i2c_write_reg(dev, client, csr->u, U_BYTE(value), func);

	return ret;
}

int vc_read_i2c_reg(struct i2c_client *client, const __u16 addr)
{
	return i2c_read_reg(client, addr);
}

int vc_write_i2c_reg(struct i2c_client *client, const __u16 addr, const __u8 value)
{
	return i2c_write_reg(&client->dev, client, addr, value, __FUNCTION__);
}


// ------------------------------------------------------------------------------------------------
//  Helper Functions for debugging

static void vc_core_print_desc(struct device *dev, struct vc_desc *desc)
{
	int is_color = vc_mod_is_color_sensor(desc);

	vc_notice(dev, "+--- VC MIPI Camera -----------------------------------+\n");
	vc_notice(dev, "| MANUF. | %s               MID: 0x%04x |\n", desc->manuf, desc->manuf_id);
	vc_notice(dev, "| MODULE | ID:  0x%04x                     REV:   %04u |\n", desc->mod_id, desc->mod_rev);
	vc_notice(dev, "| SENSOR | %s %s%s                                |\n", desc->sen_manuf, desc->sen_type, is_color ? "" : " ");
	vc_notice(dev, "+--------+---------------------------------------------+\n");
}

static void vc_core_print_csr(struct device *dev, struct vc_desc *desc)
{
	vc_notice(dev, "+--- Sensor Registers ------+--------+--------+--------+\n");
	vc_notice(dev, "|                           | low    | mid    | high   |\n");
	vc_notice(dev, "+---------------------------+--------+--------+--------+\n");
	vc_notice(dev, "| idle                      | 0x%04x |        |        |\n", desc->csr_mode);
	vc_notice(dev, "| horizontal start          | 0x%04x | 0x%04x |        |\n", desc->csr_h_start_l, desc->csr_h_start_h);
	vc_notice(dev, "| vertical start            | 0x%04x | 0x%04x |        |\n", desc->csr_v_start_l, desc->csr_v_start_h);
	vc_notice(dev, "| horizontal end            | 0x%04x | 0x%04x |        |\n", desc->csr_h_end_l, desc->csr_h_end_h);
	vc_notice(dev, "| vertical end              | 0x%04x | 0x%04x |        |\n", desc->csr_v_end_l, desc->csr_v_end_h);
	vc_notice(dev, "| hor. output width         | 0x%04x | 0x%04x |        |\n", desc->csr_o_width_l, desc->csr_o_width_h);
	vc_notice(dev, "| ver. output height        | 0x%04x | 0x%04x |        |\n", desc->csr_o_height_l, desc->csr_o_height_h);
	vc_notice(dev, "| exposure                  | 0x%04x | 0x%04x | 0x%04x |\n", desc->csr_exposure_l, desc->csr_exposure_m, desc->csr_exposure_h);
	vc_notice(dev, "| gain                      | 0x%04x | 0x%04x |        |\n", desc->csr_gain_l, desc->csr_gain_h);
	vc_notice(dev, "+---------------------------+--------+--------+--------+\n");
	vc_notice(dev, "| clock for ext. trigger    | %8u Hz              |\n", desc->clk_ext_trigger);
	vc_notice(dev, "| pixel clock               | %8u Hz              |\n", desc->clk_pixel);
	vc_notice(dev, "| shutter offset            | %8u us              |\n", desc->shutter_offset);
	vc_notice(dev, "+---------------------------+--------------------------+\n");
}

static void vc_core_print_modes(struct device *dev, struct vc_desc *desc)
{
	struct vc_desc_mode *mode;
	__u32 data_rate = 0;
	char format[16], type[16];
	int index = 0;

	vc_notice(dev, "+--- Module Modes -------+---------+---------+---------+\n");
	vc_notice(dev, "|  # | rate    | lanes   | format  | type    | binning |\n");
	vc_notice(dev, "+----+---------+---------+---------+---------+---------+\n");
	for (index = 0; index<desc->num_modes; index++) {
		mode = &desc->modes[index];
		data_rate = (*(__u32*)mode->data_rate)/1000000;
		switch (mode->format) {
		case FORMAT_RAW08: strcpy(format, "RAW08"); break;
		case FORMAT_RAW10: strcpy(format, "RAW10"); break;
		case FORMAT_RAW12: strcpy(format, "RAW12"); break;
		case FORMAT_RAW14: strcpy(format, "RAW14"); break;
		default: sprintf(format, "0x%02x ", mode->format); break;
		}
		switch (mode->type) {
		case 0x01: strcpy(type, "STREAM "); break;
		case 0x02: strcpy(type, "EXT.TRG"); break;
		default: sprintf(type, "0x%02x   ", mode->type); break;
		}
		vc_notice(dev, "| %2d |    %4u |       %u | %s   | %s |       %u |\n",
			index, data_rate, mode->num_lanes, format, type, mode->binning);
	}
	vc_notice(dev, "+----+---------+---------+---------+---------+---------+\n");
}

// ------------------------------------------------------------------------------------------------
//  Helper functions for internal data structures

struct device *vc_core_get_sen_device(struct vc_cam *cam)
{
	return &cam->ctrl.client_sen->dev;
}

struct device *vc_core_get_mod_device(struct vc_cam *cam)
{
	return &cam->ctrl.client_mod->dev;
}

static int vc_core_get_v4l2_fmt(__u32 code, char *buf)
{
	switch(code) {
	case MEDIA_BUS_FMT_Y8_1X8:       sprintf(buf, "GREY"); break;
	case MEDIA_BUS_FMT_Y10_1X10:     sprintf(buf, "Y10 "); break;
	case MEDIA_BUS_FMT_Y12_1X12:     sprintf(buf, "Y12 "); break;
	// case MEDIA_BUS_FMT_Y14_1X14:     sprintf(buf, "Y14 "); break; // Doesn't exist!
	case MEDIA_BUS_FMT_SRGGB8_1X8:   sprintf(buf, "RGGB"); break;
	case MEDIA_BUS_FMT_SRGGB10_1X10: sprintf(buf, "RG10"); break;
	case MEDIA_BUS_FMT_SRGGB12_1X12: sprintf(buf, "RG12"); break;
	case MEDIA_BUS_FMT_SRGGB14_1X14: sprintf(buf, "RG14"); break;
	default: return -EINVAL;
	}
	return 0;
}

static __u8 vc_core_v4l2_code_to_format(__u32 code)
{
	switch (code) {
	case MEDIA_BUS_FMT_Y8_1X8:
	case MEDIA_BUS_FMT_SRGGB8_1X8:
		return FORMAT_RAW08;
	case MEDIA_BUS_FMT_Y10_1X10:
	case MEDIA_BUS_FMT_SRGGB10_1X10:
		return FORMAT_RAW10;
	case MEDIA_BUS_FMT_Y12_1X12:
	case MEDIA_BUS_FMT_SRGGB12_1X12:
		return FORMAT_RAW12;
	// case MEDIA_BUS_FMT_Y14_1X14:
	case MEDIA_BUS_FMT_SRGGB14_1X14:
		return FORMAT_RAW14;
	}
	return 0;
}

static __u32 vc_core_format_to_v4l2_code(__u8 format, int is_color)
{
	switch (format) {
	case FORMAT_RAW08: return is_color ? MEDIA_BUS_FMT_SRGGB8_1X8   : MEDIA_BUS_FMT_Y8_1X8;
	case FORMAT_RAW10: return is_color ? MEDIA_BUS_FMT_SRGGB10_1X10 : MEDIA_BUS_FMT_Y10_1X10;
	case FORMAT_RAW12: return is_color ? MEDIA_BUS_FMT_SRGGB12_1X12 : MEDIA_BUS_FMT_Y12_1X12;
	case FORMAT_RAW14: return MEDIA_BUS_FMT_SRGGB14_1X14; // MEDIA_BUS_FMT_Y14_1X14
	}
	return 0;
}

static __u32 vc_core_get_default_format(struct vc_desc *desc)
{
	__u8 format = desc->modes[0].format;
	int is_color = vc_mod_is_color_sensor(desc);
	return vc_core_format_to_v4l2_code(format, is_color);
}

int vc_core_try_format(struct vc_cam *cam, __u32 code)
{
	struct vc_desc *desc = &cam->desc;
	struct device *dev = vc_core_get_sen_device(cam);
	__u8 format = vc_core_v4l2_code_to_format(code);
	char fourcc[5];
	int index;

	vc_core_get_v4l2_fmt(code, fourcc);
	vc_info(dev, "%s(): Try format 0x%04x (%s, format: 0x%02x)\n", __FUNCTION__, code, fourcc, format);

	for (index = 0; index < desc->num_modes; index++) {
		struct vc_desc_mode *mode = &desc->modes[index];
		vc_dbg(dev, "%s(): Checking mode %u (format: 0x%02x)", __FUNCTION__, index, mode->format);
		if(mode->format == format) {
			return 0;
		}
	}

	return -EINVAL;
}

int vc_core_set_format(struct vc_cam *cam, __u32 code)
{
	struct vc_desc *desc = &cam->desc;
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_sen_device(cam);
	char fourcc[5];

	vc_core_get_v4l2_fmt(code, fourcc);
	vc_info(dev, "%s(): Set format 0x%04x (%s)\n", __FUNCTION__, code, fourcc);

	if (vc_core_try_format(cam, code)) {
		state->format_code = vc_core_get_default_format(desc);
		vc_err(dev, "%s(): Format 0x%04x not supported! (Set default format: 0x%04x)\n", __FUNCTION__, code, state->format_code);
	 	return -EINVAL;
	}

	state->format_code = code;
	
	return 0;
}

__u32 vc_core_get_format(struct vc_cam *cam)
{
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_sen_device(cam);
	__u32 code = state->format_code;
	char fourcc[5];

	vc_core_get_v4l2_fmt(code, fourcc);
	vc_info(dev, "%s(): Get format 0x%04x (%s)\n", __FUNCTION__, code, fourcc);

	return code;
}

int vc_core_set_frame(struct vc_cam *cam, __u32 x, __u32 y, __u32 width, __u32 height)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct vc_frame *frame = &state->frame;
	struct device *dev = vc_core_get_sen_device(cam);

	vc_info(dev, "%s(): Set frame (x: %u, y: %u, width: %u, height: %u)\n", __FUNCTION__, x, y, width, height);

	if (width > ctrl->frame.width) {
		frame->width = ctrl->frame.width;
	} else {
		frame->width = width;
	}

	if (x > ctrl->frame.width - frame->width) {
		frame->x = ctrl->frame.width - frame->width;
	} else {
		frame->x = x;
	}

	if (height > ctrl->frame.height) {
		frame->height = ctrl->frame.height;
	} else {
		frame->height = height;
	}

	if (y > ctrl->frame.height - frame->height) {
		frame->y = ctrl->frame.height - frame->height;
	} else {
		frame->y = y;
	}

	if (frame->x != x || frame->y != y || frame->width != width || frame->height != height) {
		vc_warn(dev, "%s(): Adjusted frame (x: %u, y: %u, width: %u, height: %u)\n", __FUNCTION__, 
		frame->x, frame->y, frame->width, frame->height);
	}

	return 0;
}

struct vc_frame *vc_core_get_frame(struct vc_cam *cam)
{
	struct vc_frame* frame = &cam->state.frame;
	struct device *dev = vc_core_get_sen_device(cam);

	vc_info(dev, "%s(): Get frame (width: %u, height: %u)\n", __FUNCTION__, frame->width, frame->height);

	return frame;
}

int vc_core_set_num_lanes(struct vc_cam *cam, __u32 number)
{
	struct vc_desc *desc = &cam->desc;
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_sen_device(cam);
	__u8 index = 0;

	for (index = 0; index < desc->num_modes; index++) {
		struct vc_desc_mode *mode = &desc->modes[index];
		if (mode->num_lanes == number) {
			vc_info(dev, "%s(): Set number of lanes %u\n", __FUNCTION__, number);
			state->num_lanes = number;
			return 0;
		}
	}

	vc_err(dev, "%s(): Number of lanes %u not supported!\n", __FUNCTION__, number);
	return -EINVAL;
}

__u32 vc_core_get_num_lanes(struct vc_cam *cam)
{
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_sen_device(cam);

	vc_info(dev, "%s(): Get number of lanes: %u\n", __FUNCTION__, state->num_lanes);
	return state->num_lanes;
}

int vc_core_set_framerate(struct vc_cam *cam, __u32 framerate)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_sen_device(cam);

	vc_info(dev, "%s(): Set framerate %u Hz\n", __FUNCTION__, framerate);

	if (framerate < ctrl->framerate.min) {
		framerate = ctrl->framerate.min;
	}
	if (framerate > ctrl->framerate.max) {
		framerate = ctrl->framerate.max;
	}
	state->framerate = framerate;

	return 0;
}

__u32 vc_core_get_framerate(struct vc_cam *cam)
{
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_sen_device(cam);

	vc_info(dev, "%s(): Get framerate %u Hz\n", __FUNCTION__, state->framerate);
	return state->framerate;
}


// ------------------------------------------------------------------------------------------------
//  Helper Functions for the VC MIPI Controller Module

static struct i2c_client *vc_mod_get_client(struct i2c_adapter *adapter, __u8 i2c_addr)
{
	// struct device *dev;
	struct i2c_client *client;
	struct i2c_board_info info = {
		I2C_BOARD_INFO("i2c", i2c_addr),
	};
	unsigned short addr_list[2] = { i2c_addr, I2C_CLIENT_END };
	client = i2c_new_probed_device(adapter, &info, addr_list, NULL);

	// How to change the drivers name.
	// i2c 6-0010
	//  ^     ^
	//  |     +--- The device name is set by 
	//  |          i2c_new_probed_device() -> i2c_new_device() -> i2c_dev_set_name()
	//  |          dev_set_name() and dev_name()
	//  +--------- 
	// dev = &client->dev;
	// vc_info(dev, "%s(): dev_name:%s\n", __FUNCTION__, dev_name(dev));	
	// if (dev->driver == 0) {
	// 	vc_err(dev, "%s(): dev->driver == 0\n", __FUNCTION__);
	// }

	return client;
}

int vc_mod_set_power(struct vc_cam *cam, int on)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client_mod = ctrl->client_mod;
	struct device *dev = &client_mod->dev;
	int ret;

	vc_info(dev, "%s(): Set module power: %s\n", __FUNCTION__, on ? "up" : "down");

	ret = i2c_write_reg(dev, client_mod, MOD_REG_RESET, on ? REG_RESET_PWR_UP : REG_RESET_PWR_DOWN, __FUNCTION__);
	if (ret) {
		vc_err(dev, "%s(): Unable to power %s the module (error: %d)\n", __FUNCTION__,
			(on == REG_RESET_PWR_UP) ? "up" : "down", ret);
		cam->state.power_on = 0;
		return ret;
	} 
	
	cam->state.power_on = on;
	return 0;
}

static int vc_mod_read_status(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	int ret;

	ret = i2c_read_reg(client, MOD_REG_STATUS);
	if (ret < 0)
		vc_err(dev, "%s(): Unable to get module status (error: %d)\n", __FUNCTION__, ret);
	else
		vc_dbg(dev, "%s(): Get module status: 0x%02x\n", __FUNCTION__, ret);

	return ret;
}

static int vc_mod_write_trigger_mode(struct i2c_client *client, int mode)
{
	struct device *dev = &client->dev;
	int ret;

	vc_dbg(dev, "%s(): Write trigger mode: 0x%02x\n", __FUNCTION__, mode);

	ret = i2c_write_reg(dev, client, MOD_REG_EXTTRIG, mode, __FUNCTION__);
	if (ret)
		vc_err(dev, "%s(): Unable to write external trigger (error: %d)\n", __FUNCTION__, ret);

	return ret;
}

static int vc_mod_write_io_mode(struct i2c_client *client, int mode)
{
	struct device *dev = &client->dev;
	int ret;

	vc_dbg(dev, "%s(): Write IO mode: %s\n", __FUNCTION__, mode ? "ON" : "OFF");

	ret = i2c_write_reg(dev, client, MOD_REG_IOCTRL, mode, __FUNCTION__);
	if (ret)
		vc_err(dev, "%s(): Unable to write IO mode (error: %d)\n", __FUNCTION__, ret);

	return ret;
}

static int vc_mod_wait_until_module_is_ready(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	int status;
	int try;

	vc_dbg(dev, "%s(): Wait until module is ready\n", __FUNCTION__);

	status = REG_STATUS_NO_COM;
	try = 0;
	while (status == REG_STATUS_NO_COM && try < 10) {
		usleep_range(200000, 200000);
		status = vc_mod_read_status(client);
		try++;
	}
	if (status == REG_STATUS_ERROR) {
		vc_err(dev, "%s(): Internal Error!", __func__);
		return -EIO;
	}

	vc_dbg(dev, "%s(): Module is ready!\n", __FUNCTION__);
	return 0;
}

static int vc_mod_setup(struct vc_ctrl *ctrl, int mod_i2c_addr, struct vc_desc *desc)
{
	struct i2c_client *client_sen = ctrl->client_sen;
	struct i2c_adapter *adapter = client_sen->adapter;
	struct device *dev_sen = &client_sen->dev;
	struct i2c_client *client_mod;
	struct device *dev_mod;
	int addr, reg;

	vc_dbg(dev_sen, "%s(): Setup the module\n", __FUNCTION__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		vc_err(dev_sen, "%s(): I2C-Adapter doesn't support I2C_FUNC_SMBUS_BYTE\n", __FUNCTION__);
		return 0;
	}

	client_mod = vc_mod_get_client(adapter, mod_i2c_addr);
	if (client_mod == 0) {
		vc_err(dev_sen, "%s(): Unable to get module I2C client for address 0x%02x\n", __FUNCTION__, mod_i2c_addr);
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
	}

	// TODO: Check if connected module is really a VC MIPI module
	vc_core_print_desc(dev_mod, desc);
	vc_core_print_csr(dev_mod, desc);
	vc_core_print_modes(dev_mod, desc);

	ctrl->client_mod = client_mod;
	ctrl->mod_i2c_addr = mod_i2c_addr;

	if (desc->num_modes == 0) {
		vc_err(dev_mod, "%s(): Could not find any module modes! Operation not possible!\n", __FUNCTION__);
		return -EIO;
	}
	
	return 0;
}

static void vc_core_state_init(struct vc_cam *cam)
{
	struct vc_desc *desc = &cam->desc;
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;

	state->mode = 0xff;
	state->exposure = ctrl->exposure.def;
	state->gain = ctrl->gain.def;
	state->shs = 0;
	state->vmax = 0;
	state->exposure_cnt = 0;
	state->retrigger_cnt = 0;
	state->framerate = ctrl->framerate.def;
	state->format_code = vc_core_get_default_format(desc);
	state->frame.x = 0;
	state->frame.y = 0;
	state->frame.width = ctrl->frame.width;
	state->frame.height = ctrl->frame.height;	
	state->streaming = 0;
	state->flags = 0x00;
}

static int vc_sen_read_image_size(struct vc_ctrl *ctrl, struct vc_frame *size);

int vc_core_init(struct vc_cam *cam, struct i2c_client *client) 
{
	struct vc_desc *desc = &cam->desc;
	struct vc_ctrl *ctrl = &cam->ctrl;	
	int ret;

	ctrl->client_sen = client;
	ret = vc_mod_setup(ctrl, 0x10, desc);
	if (ret) {
		return -EIO;
	}
	ret = vc_mod_ctrl_init(ctrl, desc);
	if (ret) {
		return -EIO;
	}
	if (ctrl->frame.width == 0 || ctrl->frame.height == 0) {
		vc_sen_read_image_size(ctrl, &ctrl->frame);
	}
	vc_core_state_init(cam);

	vc_notice(&ctrl->client_mod->dev, "VC MIPI Core succesfully initialized");
	return 0;
}

static int vc_mod_write_exposure(struct i2c_client *client, __u32 value)
{
	struct device *dev = &client->dev;
	int ret;

	vc_dbg(dev, "%s(): Write module exposure = 0x%08x (%u)\n", __FUNCTION__, value, value);

	ret  = i2c_write_reg(dev, client, MOD_REG_EXPO_L, L_BYTE(value), __FUNCTION__);
	ret |= i2c_write_reg(dev, client, MOD_REG_EXPO_M, M_BYTE(value), __FUNCTION__);
	ret |= i2c_write_reg(dev, client, MOD_REG_EXPO_H, H_BYTE(value), __FUNCTION__);
	ret |= i2c_write_reg(dev, client, MOD_REG_EXPO_U, U_BYTE(value), __FUNCTION__);
	
	return ret;
}

static int vc_mod_write_retrigger(struct i2c_client *client, __u32 value)
{
	struct device *dev = &client->dev;
	int ret;

	vc_dbg(dev, "%s(): Write module retrigger = 0x%08x (%u)\n", __FUNCTION__, value, value);

	ret  = i2c_write_reg(dev, client, MOD_REG_RETRIG_L, L_BYTE(value), __FUNCTION__);
	ret |= i2c_write_reg(dev, client, MOD_REG_RETRIG_M, M_BYTE(value), __FUNCTION__);
	ret |= i2c_write_reg(dev, client, MOD_REG_RETRIG_H, H_BYTE(value), __FUNCTION__);
	ret |= i2c_write_reg(dev, client, MOD_REG_RETRIG_U, U_BYTE(value), __FUNCTION__);
	
	return ret;
}

static __u8 vc_mod_find_mode(struct vc_cam *cam, __u8 num_lanes, __u8 format, __u8 type, __u8 binning)
{
	struct vc_desc *desc = &cam->desc;
	struct device *dev = vc_core_get_mod_device(cam);
	__u8 index = 0;

	for (index = 0; index < desc->num_modes; index++) {
		struct vc_desc_mode *mode = &desc->modes[index];
		vc_dbg(dev, "%s(): Checking mode (#%02u, lanes: %u, format: 0x%02x, type: 0x%02x, binning: 0x%02x)", __FUNCTION__, 
			index, mode->num_lanes, mode->format, mode->type, mode->binning);
		if(mode->num_lanes == num_lanes && mode->format == format && mode->type == type && mode->binning == binning) {
			return index;
		}
	}
	return 0;
}

static int vc_mod_write_mode(struct i2c_client *client, __u8 mode)
{
	struct device *dev = &client->dev;
	int ret;

	vc_dbg(dev, "%s(): Write module mode: 0x%02x\n", __FUNCTION__, mode);

	ret = i2c_write_reg(dev, client, MOD_REG_MODE, mode, __FUNCTION__);
	if (ret)
		vc_err(dev, "%s(): Unable to write module mode: 0x%02x (error: %d)\n", __FUNCTION__, mode, ret);

	return ret;
}

static int vc_mod_reset_module(struct vc_cam *cam, __u8 mode)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client = ctrl->client_mod;
	struct device *dev = &client->dev;
	int ret;

	vc_dbg(dev, "%s(): Reset the module!\n", __FUNCTION__);

	ret = vc_mod_set_power(cam, 0);
	ret |= vc_mod_write_mode(client, mode);
	ret |= vc_mod_set_power(cam, 1);
	ret |= vc_mod_wait_until_module_is_ready(client);

	return ret;
}

int vc_mod_set_mode(struct vc_cam *cam, int *reset)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_mod_device(cam);
	__u8 num_lanes = state->num_lanes;
	__u8 format = vc_core_v4l2_code_to_format(state->format_code);
	char fourcc[5];
	char *stype;
	__u8 type = 0;
	__u8 binning = 0; // TODO: Not implemented yet
	__u8 mode = 0;
	int ret = 0;

	switch (cam->state.trigger_mode) {
	case REG_TRIGGER_DISABLE:
	case REG_TRIGGER_SYNC:
	case REG_TRIGGER_STREAM_EDGE:
	case REG_TRIGGER_STREAM_LEVEL:
	default:
		type = 0x01;
		stype = "STREAM";
		break;
	case REG_TRIGGER_EXTERNAL:
	case REG_TRIGGER_PULSEWIDTH:
	case REG_TRIGGER_SELF:
	case REG_TRIGGER_SINGLE:
		type = 0x02;
		stype = "EXT.TRG";
		break;
	}
	
	mode = vc_mod_find_mode(cam, num_lanes, format, type, binning);
	if (mode == state->mode && !(ctrl->flags & FLAG_RESET_ALWAYS)) {
		vc_dbg(dev, "%s(): Module mode %u already set!\n", __FUNCTION__, mode);
		*reset = 0;
		return 0;
	}

	vc_core_get_v4l2_fmt(state->format_code, fourcc);
	vc_info(dev, "%s(): Set module mode: %u (lanes: %u, format: %s, type: %s)\n", __FUNCTION__, 
		mode, num_lanes, fourcc, stype);

	ret = vc_mod_reset_module(cam, mode);
	if (ret) {
		vc_err(dev, "%s(): Unable to set module mode: %u (lanes: %u, format: %s, type: %s) (error: %d)\n", __func__, 
			mode, num_lanes, fourcc, stype, ret);
		return ret;
	}

	state->mode = mode;
	*reset = 1;

	return ret;
}

int vc_mod_is_trigger_enabled(struct vc_cam *cam)
{
	return cam->state.trigger_mode != REG_TRIGGER_DISABLE;
}

int vc_mod_set_trigger_mode(struct vc_cam *cam, int mode)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_mod_device(cam);
	char *mode_desc;

	if (mode == 0) {
		mode_desc = "DISABLED";		
		state->trigger_mode = REG_TRIGGER_DISABLE;

	} else if (mode == 1 && ctrl->flags & FLAG_TRIGGER_EXTERNAL) {
		mode_desc = "EXTERNAL";
		state->trigger_mode = REG_TRIGGER_EXTERNAL;
	
	} else if (mode == 2 && ctrl->flags & FLAG_TRIGGER_PULSEWIDTH) {
		mode_desc = "PULSEWIDTH";
		state->trigger_mode = REG_TRIGGER_PULSEWIDTH;
	
	} else if (mode == 3 && ctrl->flags & FLAG_TRIGGER_SELF) {
		mode_desc = "SELF";
		state->trigger_mode = REG_TRIGGER_SELF;
		
	} else if (mode == 4 && ctrl->flags & FLAG_TRIGGER_SINGLE) {
		mode_desc = "SINGLE";
		state->trigger_mode = REG_TRIGGER_SINGLE;
		
	} else if (mode == 5 && ctrl->flags & FLAG_TRIGGER_SYNC) {
		mode_desc = "SYNC";
		state->trigger_mode = REG_TRIGGER_SYNC;
		
	} else if (mode == 6 && ctrl->flags & FLAG_TRIGGER_STREAM_EDGE) {
		mode_desc = "STREAM_EDGE";
		state->trigger_mode = REG_TRIGGER_STREAM_EDGE;
		
	} else if (mode == 7 && ctrl->flags & FLAG_TRIGGER_STREAM_LEVEL) {
		mode_desc = "STREAM_EDGE";
		state->trigger_mode = REG_TRIGGER_STREAM_LEVEL;
		
	} else {
		vc_err(dev, "%s(): Trigger mode %d not supported!\n", __FUNCTION__, mode);
		return -EINVAL;
	}

	vc_notice(dev, "%s(): Set trigger mode: %s\n", __FUNCTION__, mode_desc);

	return 0;
}

int vc_mod_get_trigger_mode(struct vc_cam *cam)
{
	switch (cam->state.trigger_mode)  {
	case REG_TRIGGER_DISABLE: 	return 0;
	case REG_TRIGGER_EXTERNAL: 	return 1;
	case REG_TRIGGER_PULSEWIDTH: 	return 2;
	case REG_TRIGGER_SELF:		return 3;
	case REG_TRIGGER_SINGLE:	return 4;
	case REG_TRIGGER_SYNC:		return 5;
	case REG_TRIGGER_STREAM_EDGE:	return 6;
	case REG_TRIGGER_STREAM_LEVEL:	return 7;
	}
	return 0;
}

int vc_mod_is_io_enabled(struct vc_cam *cam)
{
	return cam->state.io_mode != REG_IO_DISABLE;
}

int vc_mod_set_io_mode(struct vc_cam *cam, int mode)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_mod_device(cam);
	char *mode_desc;

	if (mode == 0) {
		mode_desc = "DISABLED";		
		state->io_mode = REG_IO_DISABLE;
	} else if (mode == 1 && ctrl->flags & FLAG_IO_FLASH_ENABLED) {
		mode_desc = "FLASH";
		state->io_mode = REG_IO_FLASH_ENABLE;
	} else if (mode == 2 && ctrl->flags & FLAG_IO_XTRIG_ENABLED) {
		mode_desc = "XTRIG";
		state->io_mode = REG_IO_XTRIG_ENABLE;
	} else {
		vc_err(dev, "%s(): IO mode %d not supported!\n", __FUNCTION__, mode);
		return -EINVAL;
	}

	vc_notice(dev, "%s(): Set IO mode: %s\n", __FUNCTION__, mode_desc);

	return 0;
}

int vc_mod_get_io_mode(struct vc_cam *cam)
{
	switch (cam->state.io_mode)  {
	case REG_IO_DISABLE: 		return 0;
	case REG_IO_FLASH_ENABLE: 	return 1;
	case REG_IO_XTRIG_ENABLE: 	return 2;
	}
	return 0;
}


// ------------------------------------------------------------------------------------------------
//  Helper Functions for the VC MIPI Sensors

static int vc_sen_write_mode(struct vc_ctrl *ctrl, int mode) 
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;
	__u8 value;
	int ret = 0;

	vc_dbg(dev, "%s(): Write sensor mode: %s\n", __FUNCTION__, (mode == ctrl->csr.sen.mode_standby)? "standby" : "operating");

	// TODO: Check if it is realy nessesary to swap order of write opertations.
	if(mode == ctrl->csr.sen.mode_standby) {
		value = ctrl->csr.sen.mode_standby;
		if(ctrl->csr.sen.mode.l) {
			ret = i2c_write_reg(dev, client, ctrl->csr.sen.mode.l, value, __FUNCTION__);
		}
		if(ctrl->csr.sen.mode.m) {
			ret |= i2c_write_reg(dev, client, ctrl->csr.sen.mode.m, value, __FUNCTION__);
		}
	} else {
		value = ctrl->csr.sen.mode_operating;
		if(ctrl->csr.sen.mode.m) {
			ret |= i2c_write_reg(dev, client, ctrl->csr.sen.mode.m, value, __FUNCTION__);
		}
		if(ctrl->csr.sen.mode.l) {
			ret = i2c_write_reg(dev, client, ctrl->csr.sen.mode.l, value, __FUNCTION__);
		}
	}
	if (ret) 
		vc_err(dev, "%s(): Couldn't write sensor mode: 0x%02x (error: %d)\n", __FUNCTION__, mode, ret);

	return ret;
}

static int vc_sen_read_image_size(struct vc_ctrl *ctrl, struct vc_frame *size)
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;

	size->width = i2c_read_reg2(dev, client, &ctrl->csr.sen.o_width);
	size->height = i2c_read_reg2(dev, client, &ctrl->csr.sen.o_height);

	vc_dbg(dev, "%s(): Read image size (width: %u, height: %u)\n", __FUNCTION__, size->width, size->height);
	
	return 0;
}

int vc_sen_set_roi(struct vc_cam *cam, int x, int y, int width, int height)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;
	int ret = 0;

	vc_info(dev, "%s(): Set sensor roi: (x: %u, y: %u, width: %u, height: %u)\n", __FUNCTION__, x, y, width, height);

	ret |= i2c_write_reg2(dev, client, &ctrl->csr.sen.h_start, x, __FUNCTION__);
	ret |= i2c_write_reg2(dev, client, &ctrl->csr.sen.v_start, y, __FUNCTION__);
	ret |= i2c_write_reg2(dev, client, &ctrl->csr.sen.o_width, width, __FUNCTION__);
	ret |= i2c_write_reg2(dev, client, &ctrl->csr.sen.o_height, height, __FUNCTION__);
	if (ret) {
		vc_err(dev, "%s(): Couldn't set sensor roi: (x: %u, y: %u, width: %u, height: %u) (error: %d)\n", __FUNCTION__, 
			x, y, width, height, ret);
		return ret;
	}

	cam->state.frame.x = x;
	cam->state.frame.y = y;
	cam->state.frame.width = width;
	cam->state.frame.height = height;
	return 0;
}

static __u32 vc_sen_read_vmax(struct vc_ctrl *ctrl)
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;
	__u32 vmax = i2c_read_reg4(dev, client, &ctrl->csr.sen.vmax);

	vc_dbg(dev, "%s(): Read sensor VMAX: 0x%08x (%u)\n", __FUNCTION__, vmax, vmax);

	return vmax;
}

// static __u32 vc_sen_read_hmax(struct vc_ctrl *ctrl)
// {
// 	struct i2c_client *client = ctrl->client_sen;
// 	struct device *dev = &client->dev;
// 	__u32 hmax = i2c_read_reg4(dev, client, &ctrl->csr.sen.hmax);

// 	vc_dbg(dev, "%s(): Read sensor HMAX: 0x%08x (%u)\n", __FUNCTION__, hmax, hmax);

// 	return hmax;
// }

static int vc_sen_write_vmax(struct vc_ctrl *ctrl, __u32 vmax)
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;

	vc_dbg(dev, "%s(): Write sensor VMAX: 0x%08x (%u)\n", __FUNCTION__, vmax, vmax);

	return i2c_write_reg4(dev, client, &ctrl->csr.sen.vmax, vmax, __FUNCTION__);
}

static int vc_sen_write_shs(struct vc_ctrl *ctrl, __u32 shs)
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;

	vc_dbg(dev, "%s(): Write sensor SHS: 0x%08x (%u)\n", __FUNCTION__, shs, shs);

	return i2c_write_reg4(dev, client, &ctrl->csr.sen.shs, shs, __FUNCTION__);
}

static int vc_sen_write_flash_duration(struct vc_ctrl *ctrl, __u32 duration)
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;

	vc_dbg(dev, "%s(): Write sensor flash duration: 0x%08x (%u)\n", __FUNCTION__, duration, duration);

	return i2c_write_reg4(dev, client, &ctrl->csr.sen.flash_duration, duration, __FUNCTION__);
}

static int vc_sen_write_flash_offset(struct vc_ctrl *ctrl, __u32 offset)
{
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;

	vc_dbg(dev, "%s(): Write sensor flash offset: 0x%08x (%u)\n", __FUNCTION__, offset, offset);

	return i2c_write_reg4(dev, client, &ctrl->csr.sen.flash_offset, offset, __FUNCTION__);
}

int vc_sen_set_gain(struct vc_cam *cam, int gain)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client = ctrl->client_sen;
	struct device *dev = &client->dev;
	int ret = 0;

	if (gain < ctrl->gain.min) 
		gain = ctrl->gain.min;
	if (gain > ctrl->gain.max)
		gain = ctrl->gain.max;

	vc_notice(dev, "%s(): Set sensor gain: %u\n", __FUNCTION__, gain);

	ret |= i2c_write_reg2(dev, client, &ctrl->csr.sen.gain, gain, __FUNCTION__);
	if (ret) {
		vc_err(dev, "%s(): Couldn't set 'Gain' (error: %d)\n", __FUNCTION__, ret);
		return ret;
	}

	cam->state.gain = gain;
	return 0;
}

int vc_sen_start_stream(struct vc_cam *cam)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct i2c_client *client_mod = ctrl->client_mod;
	struct device *dev = &ctrl->client_sen->dev;
	int ret = 0;
	
	vc_info(dev, "%s(): Start streaming\n", __FUNCTION__);
	vc_dbg(dev, "%s(): MM: 0x%02x, TM: 0x%02x, IO: 0x%02x\n",
		__FUNCTION__, state->mode, state->trigger_mode, state->io_mode);

	ret  = vc_mod_write_trigger_mode(client_mod, state->trigger_mode);
	ret |= vc_mod_write_io_mode(client_mod, state->io_mode);

	ret |= vc_sen_write_mode(ctrl, ctrl->csr.sen.mode_operating);
	if (ret)
		vc_err(dev, "%s(): Unable to start streaming (error: %d)\n", __FUNCTION__, ret);

	return ret;
}

int vc_sen_stop_stream(struct vc_cam *cam)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct i2c_client *client_mod = ctrl->client_mod;
	struct device *dev = &ctrl->client_sen->dev;
	int ret = 0;

	vc_info(dev, "%s(): Stop streaming\n", __FUNCTION__);

	ret |= vc_mod_write_trigger_mode(client_mod, REG_TRIGGER_DISABLE);
	ret |= vc_mod_write_io_mode(client_mod, REG_IO_DISABLE);

	ret |= vc_sen_write_mode(ctrl, ctrl->csr.sen.mode_standby);
	if (ret)
		vc_err(dev, "%s(): Unable to stop streaming (error: %d)\n", __FUNCTION__, ret);

	vc_dbg(dev, "%s(): ----------------------------------------------------------\n", __FUNCTION__);

	return ret;
}


// ------------------------------------------------------------------------------------------------

static void vc_calculate_exposure_simple(struct vc_cam *cam, __u32 exposure)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	__u32 factor = ctrl->expo_factor;
	__u32 toffset = ctrl->expo_toffset;

	state->shs = (((__u64)exposure)*factor)/1000000 - toffset;
}

static void vc_core_get_timing(struct vc_cam *cam, __u32 *period_1H)
{
	struct vc_desc *desc = &cam->desc;
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	__u8 num_lanes = state->num_lanes;
	__u8 format = vc_core_v4l2_code_to_format(state->format_code);
	__u8 index = 0;

	for (index = 0; index < 6; index++) {
		struct vc_timing *timing = &ctrl->expo_timing[index];
		if (timing->num_lanes == num_lanes && timing->format == format) {
			*period_1H = ((__u64)timing->clk * 1000000000) / desc->clk_pixel;
			return;
		}
	}

	*period_1H = ctrl->expo_period_1H;
}

static void vc_calculate_exposure_vmax(struct vc_cam *cam, __u32 exposure)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct device *dev = &ctrl->client_sen->dev;
	__u32 period_1H = 0;
	__u32 shs_min = ctrl->expo_shs_min;
	__u64 exposure_ns;
	__u64 exposure_1H;
	// __u64 hmax;

	vc_core_get_timing(cam, &period_1H);
	
	vc_dbg(dev, "%s(): flags: 0x%04x, period_1H: %u\n", __FUNCTION__, ctrl->flags, period_1H);

	if (ctrl->flags & FLAG_EXPOSURE_READ_VMAX) {	
		state->vmax = vc_sen_read_vmax(&cam->ctrl);
		if (state->vmax == 0) {
			vc_err(dev, "%s(): VMAX should not be zero! Using default value.\n", __FUNCTION__);
			state->vmax = ctrl->expo_vmax;
		}
	} else {
		state->vmax = ctrl->expo_vmax;	
	}

	// hmax = vc_sen_read_hmax(&cam->ctrl);
	// h1period = (hmax*1000000)/ctrl->sen_clk;

	// Exposure time [s] = (1 H period) × (Number of lines per frame - SHS) 
	//                     + Exposure time error (t OFFSET ) [µs]
	// 
	// | <---               VMAX (frame time)               ---> |
	// +---------+----------------------------+------------------+
	// | SHS_MIN | SHS (exposure delay) --->  | exposure time    | 
	//

	// Convert exposure time from µs to ns.
	exposure_ns = (__u64)(exposure)*1000;
	// Calculate number of lines equivalent to the exposure time without shs_min.
	exposure_1H = exposure_ns / period_1H - shs_min;

	// Is exposure time less than frame time?
	if (exposure_1H + shs_min <= state->vmax) {
		// Yes then calculate exposure delay (shs) in between frame time.
		state->shs = state->vmax - exposure_1H;
	
	} else {
		// No, then increase frame time and set exposure delay to the minimal value.
		state->vmax = shs_min + exposure_1H;
		state->shs = shs_min;
	}

	// Special case: Framerate of slave module has to be a little bit faster (Tested with IMX183)
	if (state->trigger_mode == REG_TRIGGER_SYNC) {
		state->vmax--;
	}
}

int vc_sen_set_exposure(struct vc_cam *cam, int exposure)
{
	struct vc_ctrl *ctrl = &cam->ctrl;
	struct vc_state *state = &cam->state;
	struct device *dev = vc_core_get_sen_device(cam);
	struct i2c_client *client_mod = ctrl->client_mod;
	__u32 frametime;
	__u32 retrigger;
	int ret = 0;

	vc_notice(dev, "%s(): Set sensor exposure: %u us\n", __FUNCTION__, exposure);

	if (exposure < ctrl->exposure.min)
		exposure = ctrl->exposure.min;
	if (exposure > ctrl->exposure.max)
		exposure = ctrl->exposure.max;

	state->vmax = 0;
	state->shs = 0;
	state->exposure_cnt = 0;
	state->retrigger_cnt = 0;

	// TODO: For testing read shs and vmax when using vc_mod_write_exposure.

	switch (state->trigger_mode) {
	case REG_TRIGGER_EXTERNAL:
	case REG_TRIGGER_SINGLE:
		state->exposure_cnt = ((__u64)exposure * cam->ctrl.sen_clk) / 1000000;
		ret  = vc_mod_write_exposure(client_mod, state->exposure_cnt);
		break;
	case REG_TRIGGER_PULSEWIDTH:
		break;
	case REG_TRIGGER_SELF:
		frametime = 1000000 / state->framerate;
		if (frametime >= exposure) {
			retrigger = frametime - exposure;
		} else {
			retrigger = 0;
		}
		state->exposure_cnt = ((__u64)exposure * cam->ctrl.sen_clk) / 1000000;
		state->retrigger_cnt = ((__u64)retrigger * cam->ctrl.sen_clk) / 1000000;
		ret  = vc_mod_write_exposure(client_mod, state->exposure_cnt);
		ret |= vc_mod_write_retrigger(client_mod, state->retrigger_cnt);
		break;
	case REG_TRIGGER_DISABLE:
	case REG_TRIGGER_SYNC:
	case REG_TRIGGER_STREAM_EDGE:
	case REG_TRIGGER_STREAM_LEVEL:
		if (ctrl->flags & FLAG_EXPOSURE_SIMPLE) {
			vc_calculate_exposure_simple(cam, exposure);

		} else if (ctrl->flags & (FLAG_EXPOSURE_READ_VMAX | FLAG_EXPOSURE_WRITE_VMAX)) {
			vc_calculate_exposure_vmax(cam, exposure);
		} 
		ret = vc_sen_write_shs(ctrl, state->shs);
		if (ctrl->flags & FLAG_EXPOSURE_WRITE_VMAX) {
			ret |= vc_sen_write_vmax(ctrl, state->vmax);
		}
	}

	if (ctrl->flags & FLAG_IO_FLASH_DURATION) {
		__u32 duration = (((__u64)exposure)*ctrl->flash_factor)/1000000;
		ret |= vc_sen_write_flash_duration(ctrl, duration);
		ret |= vc_sen_write_flash_offset(ctrl, ctrl->flash_toffset);
	}

	if (ret == 0) {
		cam->state.exposure = exposure;
	}

	vc_dbg(dev, "%s(): VMAX: %5u, SHS: %5u, EXPC: %6u, RETC: %6u\n",
		__FUNCTION__, state->vmax, state->shs, state->exposure_cnt, state->retrigger_cnt);

	return ret;
}