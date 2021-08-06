#include "vc_mipi_modules.h"
#include <linux/v4l2-mediabus.h>


int vc_mod_is_color_sensor(struct vc_desc *desc)
{
	if (desc->sen_type) {
		__u32 len = strnlen(desc->sen_type, 16);
		if (len > 0 && len < 17) {
			return *(desc->sen_type + len - 1) == 'C';
		}
	}
	return 0;
}

static void vc_init_ctrl(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	ctrl->mod_i2c_addr		= 0x10;

	ctrl->exposure			= (vc_control) { .min =   0, .max = 10000000, .def =  10000 };
	ctrl->gain			= (vc_control) { .min =   0, .max =     2000, .def =     10 };
	ctrl->framerate 		= (vc_control) { .min =   0, .max =       60, .def =     60 };

	ctrl->csr.sen.mode.l 		= desc->csr_mode;
	ctrl->csr.sen.mode.m 		= 0;
	ctrl->csr.sen.mode_standby	= 0x00; // FPGA Variante?
	ctrl->csr.sen.mode_operating	= 0x01;
	
	ctrl->csr.sen.expo.l 		= desc->csr_exposure_l;
	ctrl->csr.sen.expo.m 		= desc->csr_exposure_m;
	ctrl->csr.sen.expo.h 		= desc->csr_exposure_h;
	ctrl->csr.sen.expo.u 		= 0;
	
	ctrl->csr.sen.gain.l 		= desc->csr_gain_l;
	ctrl->csr.sen.gain.m 		= desc->csr_gain_h;

	ctrl->csr.sen.o_width.l		= desc->csr_o_width_l;
	ctrl->csr.sen.o_width.m		= desc->csr_o_width_h;
	ctrl->csr.sen.o_height.l	= desc->csr_o_height_l;
	ctrl->csr.sen.o_height.m	= desc->csr_o_height_h;

	ctrl->sen_clk			= desc->clk_ext_trigger;
	ctrl->flags			= FLAG_FLASH_ENABLED;
}


// ------------------------------------------------------------------------------------------------
//  Settings for IMX178/IMX178C

static void vc_init_ctrl_imx178(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_mod->dev;

	vc_notice(dev, "%s(): Initialising module control for IMX178\n", __FUNCTION__);

	ctrl->mod_id			= MOD_ID_IMX178;

	ctrl->framesize 		= (vc_size) { .width =  5440, .height = 3694 };

	ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x7004, .m = 0x7005, .h = 0x7006, .u = 0x0000 };
	
	ctrl->expo_period_1H 		= 327680;	// 20.000 µs * 16384
	ctrl->expo_toffset 		= 47563;	//  2.903 µs * 16384
	ctrl->expo_shs_min		= 5;
	ctrl->expo_vmax 		= 3648;
	// ctrl->expo_time_min2 	= 74480;

	ctrl->flags			= FLAG_FLASH_ENABLED | FLAG_READ_VMAX;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX183/IMX183C

static void vc_init_ctrl_imx183(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_mod->dev;

	vc_notice(dev, "%s(): Initialising module control for IMX183\n", __FUNCTION__);

	// vc_init_ctrl_3_fmts(ctrl, desc);

	ctrl->mod_id			= MOD_ID_IMX183;

	ctrl->framesize 		= (vc_size) { .width =  5440, .height = 3694 };

	ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x7004, .m = 0x7005, .h = 0x7006, .u = 0x0000 };
	
	ctrl->expo_period_1H 		= 327680;	// 20.000 µs * 16384
	ctrl->expo_toffset 		= 47563;	//  2.903 µs * 16384
	ctrl->expo_shs_min		= 5;
	ctrl->expo_vmax 		= 3648;
	// ctrl->expo_time_min2 	= 74480;

	// With FLAG_READ_VMAX frame rate is extremely slow.
	// ctrl->flags			= FLAG_FLASH_ENABLED | FLAG_READ_VMAX;
	ctrl->flags			= FLAG_FLASH_ENABLED;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX226/IMX226C

static void vc_init_ctrl_imx226(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_mod->dev;

	vc_notice(dev, "%s(): Initialising module control for IMX226\n", __FUNCTION__);

	ctrl->mod_id			= MOD_ID_IMX226;

	ctrl->framesize 		= (vc_size) { .width =  3840, .height = 3046 };

	ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x7004, .m = 0x7005, .h = 0x7006, .u = 0x0000 };
	
	ctrl->expo_period_1H 		= 327680;	// 20.000 µs * 16384
	ctrl->expo_toffset 		= 47563;	//  2.903 µs * 16384
	ctrl->expo_shs_min		= 5;
	ctrl->expo_vmax 		= 3079;		// Read 1804
	// ctrl->expo_time_min2 	= 74480;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX252/IMX252C

static void vc_init_ctrl_imx252(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_mod->dev;

	vc_notice(dev, "%s(): Initialising module control for IMX252\n", __FUNCTION__);

	ctrl->mod_id			= MOD_ID_IMX252;

	ctrl->framesize 		= (vc_size) { .width =  2048, .height = 1536 };

	ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x7004, .m = 0x7005, .h = 0x7006, .u = 0x0000 };
	ctrl->csr.sen.hmax              = (vc_csr4) { .l = 0x7007, .m = 0x7008, .h = 0x0000, .u = 0x0000 };

	ctrl->expo_period_1H 		= 327680;	// 20.000 µs * 16384 (Zeit die eine Zeile benötigt)
	ctrl->expo_toffset 		= 47563;	//  2.903 µs * 16384
	ctrl->expo_shs_min		= 40;
	ctrl->expo_vmax 		= 3079;
	// ctrl->expo_time_min2 	= 74480; 	// µs

	ctrl->flags			= FLAG_FLASH_ENABLED | FLAG_READ_VMAX;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX296/IMX296C

static void vc_init_ctrl_imx296(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_mod->dev;

	vc_notice(dev, "%s(): Initialising module control for IMX296\n", __FUNCTION__);
	
	ctrl->mod_id			= MOD_ID_IMX296;
	
	ctrl->framesize 		= (vc_size) { .width =  1440, .height = 1080 };

	ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3010, .m = 0x3011, .h = 0x3012, .u = 0x0000 };
	ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x300A };
	ctrl->csr.sen.mode_standby	= 0x01;
	ctrl->csr.sen.mode_operating	= 0x00;

	ctrl->sen_clk			= 54000000;	// Hz
	ctrl->expo_period_1H 		= 242726;	// 14.815 µs * 16384
	ctrl->expo_toffset 		= 233636;	// 14.260 µs * 16384
	ctrl->expo_shs_min              = 5;
	ctrl->expo_vmax 		= 1118;
	// ctrl->expo_time_min2 	= 16504;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX327C

static void vc_init_ctrl_imx327(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_mod->dev;

	vc_notice(dev, "%s(): Initialising module control for IMX327\n", __FUNCTION__);
	
	ctrl->mod_id			= MOD_ID_IMX327;

	ctrl->framesize 		= (vc_size) { .width =  1920, .height = 1080 };

	ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3018, .m = 0x3019, .h = 0x301A, .u = 0x0000 };
	ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x3002 };
	ctrl->csr.sen.mode_standby	= 0x01;
	ctrl->csr.sen.mode_operating	= 0x00;
	
	ctrl->sen_clk			= 54000000;
	ctrl->expo_period_1H 		= 327680;
	ctrl->expo_toffset 		= 47563;
	ctrl->expo_vmax 		= 3728;
	// ctrl->expo_time_min2 	= 38716;

	ctrl->flags			= 0; // <= Disable flash out 
}

// ------------------------------------------------------------------------------------------------
//  Settings for OV9281

static void vc_init_ctrl_ov9281(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_mod->dev;

	vc_notice(dev, "%s(): Initialising module control for OV9281 (NOT CONFIGURED!!!)\n", __FUNCTION__);
	
	ctrl->mod_id			= MOD_ID_OV9281;

	// ctrl->framesize 		= (vc_size) { .width =  1920, .height = 1080 };

	// ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3018, .m = 0x3019, .h = 0x301A, .u = 0x0000 };
	// ctrl->csr.sen.mode              = (vc_csr4) { .l = 0x3000, .m = 0x3002, .h = 0x0000, .u = 0x0000 };
	// ctrl->csr.sen.mode_standby	= 0x01;
	// ctrl->csr.sen.mode_operating	= 0x00;
	
	// ctrl->sen_clk			= 54000000;
	// ctrl->expo_period_1H 		= 327680;
	// ctrl->expo_toffset 		= 47563;
	// ctrl->expo_vmax 		= 3728;
	// // ctrl->expo_time_min2 	= 38716;

	// ctrl->flags			= 0; // <= Disable flash out 
}


int vc_mod_ctrl_init(struct vc_ctrl* ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_mod->dev;

	vc_init_ctrl(ctrl, desc);

	switch(desc->mod_id) {
	case MOD_ID_IMX178: vc_init_ctrl_imx178(ctrl, desc); break;
	case MOD_ID_IMX183: vc_init_ctrl_imx183(ctrl, desc); break;
	case MOD_ID_IMX226: vc_init_ctrl_imx226(ctrl, desc); break;
	case MOD_ID_IMX252: vc_init_ctrl_imx252(ctrl, desc); break;
	case MOD_ID_IMX296: vc_init_ctrl_imx296(ctrl, desc); break;
	case MOD_ID_IMX327: vc_init_ctrl_imx327(ctrl, desc); break;
	case MOD_ID_OV9281: vc_init_ctrl_ov9281(ctrl, desc); break;
	default:
		vc_err(dev, "%s(): Detected Module not supported!\n", __FUNCTION__);
		return 1;
	}
	return 0;
}