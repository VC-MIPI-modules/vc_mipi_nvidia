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

// ------------------------------------------------------------------------------------------------
//  Settings for 226/226C

static struct vc_mode imx226_mono_modes[] = {
	{ MEDIA_BUS_FMT_Y8_1X8,       FLAG_MODE_2_LANES,                   0x00 },
	{ MEDIA_BUS_FMT_Y10_1X10,     FLAG_MODE_2_LANES,                   0x01 },
	{ MEDIA_BUS_FMT_Y12_1X12,     FLAG_MODE_2_LANES,                   0x02 },
	{ MEDIA_BUS_FMT_Y8_1X8,       FLAG_MODE_2_LANES | FLAG_TRIGGER_IN, 0x03 },
	{ MEDIA_BUS_FMT_Y10_1X10,     FLAG_MODE_2_LANES | FLAG_TRIGGER_IN, 0x04 },
	{ MEDIA_BUS_FMT_Y12_1X12,     FLAG_MODE_2_LANES | FLAG_TRIGGER_IN, 0x05 },
	{ MEDIA_BUS_FMT_Y8_1X8,       FLAG_MODE_4_LANES,                   0x06 },
	{ MEDIA_BUS_FMT_Y10_1X10,     FLAG_MODE_4_LANES,                   0x07 },
	{ MEDIA_BUS_FMT_Y12_1X12,     FLAG_MODE_4_LANES,                   0x08 },
	{ MEDIA_BUS_FMT_Y8_1X8,       FLAG_MODE_4_LANES | FLAG_TRIGGER_IN, 0x09 },
	{ MEDIA_BUS_FMT_Y10_1X10,     FLAG_MODE_4_LANES | FLAG_TRIGGER_IN, 0x0a },
	{ MEDIA_BUS_FMT_Y12_1X12,     FLAG_MODE_4_LANES | FLAG_TRIGGER_IN, 0x0b },
	{ 0, 0, 0 },
};

static struct vc_mode imx226_color_modes[] = {
	{ MEDIA_BUS_FMT_SRGGB8_1X8,   FLAG_MODE_2_LANES,                   0x00 },
	{ MEDIA_BUS_FMT_SRGGB10_1X10, FLAG_MODE_2_LANES,                   0x01 },
	{ MEDIA_BUS_FMT_SRGGB12_1X12, FLAG_MODE_2_LANES,                   0x02 },
	{ MEDIA_BUS_FMT_SRGGB8_1X8,   FLAG_MODE_2_LANES | FLAG_TRIGGER_IN, 0x03 },
	{ MEDIA_BUS_FMT_SRGGB10_1X10, FLAG_MODE_2_LANES | FLAG_TRIGGER_IN, 0x04 },
	{ MEDIA_BUS_FMT_SRGGB12_1X12, FLAG_MODE_2_LANES | FLAG_TRIGGER_IN, 0x05 },
	{ MEDIA_BUS_FMT_SRGGB8_1X8,   FLAG_MODE_4_LANES,                   0x06 },
	{ MEDIA_BUS_FMT_SRGGB10_1X10, FLAG_MODE_4_LANES,                   0x07 },
	{ MEDIA_BUS_FMT_SRGGB12_1X12, FLAG_MODE_4_LANES,                   0x08 },
	{ MEDIA_BUS_FMT_SRGGB8_1X8,   FLAG_MODE_4_LANES | FLAG_TRIGGER_IN, 0x09 },
	{ MEDIA_BUS_FMT_SRGGB10_1X10, FLAG_MODE_4_LANES | FLAG_TRIGGER_IN, 0x0a },
	{ MEDIA_BUS_FMT_SRGGB12_1X12, FLAG_MODE_4_LANES | FLAG_TRIGGER_IN, 0x0b },
	{ 0, 0, 0 }
};

static struct vc_framefmt imx226_mono_fmts[] = {
	{ MEDIA_BUS_FMT_Y8_1X8,       V4L2_COLORSPACE_SRGB },   /* 8-bit grayscale pixel format  : V4L2_PIX_FMT_GREY 'GREY'     */
	{ MEDIA_BUS_FMT_Y10_1X10,     V4L2_COLORSPACE_SRGB },   /* 10-bit grayscale pixel format : V4L2_PIX_FMT_Y10  'Y10 '     */
	{ MEDIA_BUS_FMT_Y12_1X12,     V4L2_COLORSPACE_SRGB },   /* 12-bit grayscale pixel format : V4L2_PIX_FMT_Y12  'Y12 '     */
	{ MEDIA_BUS_FMT_SRGGB8_1X8,   V4L2_COLORSPACE_SRGB },   /* 8-bit color pixel format      : V4L2_PIX_FMT_SRGGB8  'RGGB'  */
	{ 0, 0 }
	// use 8-bit 'RGGB' instead GREY format to save 8-bit frame(s) to raw file by v4l2-ctl
};

static struct vc_framefmt imx226_color_fmts[] = {
	{ MEDIA_BUS_FMT_SRGGB8_1X8,   V4L2_COLORSPACE_SRGB },   /* 8-bit color pixel format      : V4L2_PIX_FMT_SRGGB8  'RGGB'  */
	{ MEDIA_BUS_FMT_SRGGB10_1X10, V4L2_COLORSPACE_SRGB },   /* 10-bit color pixel format     : V4L2_PIX_FMT_SRGGB10 'RG10'  */
	{ MEDIA_BUS_FMT_SRGGB12_1X12, V4L2_COLORSPACE_SRGB },   /* 12-bit color pixel format     : V4L2_PIX_FMT_SRGGB12 'RG12'  */
	{ 0, 0 }
};

void vc_init_imx226_ctrl(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_sen->dev;

	dev_dbg(dev, "%s(): Initialising module control for IMX226\n", __FUNCTION__);

	ctrl->mod_id			= MOD_ID_IMX226;
	ctrl->mod_i2c_addr		= 0x10;

	ctrl->set.exposure.min		= 160;
	ctrl->set.exposure.max		= 10000000;
	ctrl->set.exposure.default_val	= 10000;
	ctrl->set.gain.min		= 0x0000;
	ctrl->set.gain.max		= 0x07A5;		// 1957
	ctrl->set.gain.default_val	= 0x0010;

	if (vc_mod_is_color_sensor(desc)) {
		ctrl->modes		= imx226_color_modes;
		ctrl->fmts 		= imx226_color_fmts;
	} else {
		ctrl->modes		= imx226_mono_modes;
		ctrl->fmts 		= imx226_mono_fmts;
	}
	ctrl->default_mode		= 0;
	ctrl->default_fmt		= 0;
	ctrl->o_width 			= 3840;
	ctrl->o_height 			= 3040;

	ctrl->csr.sen.mode.l 		= 0x7000;		// Standby register: 0=Operating, 1=Standby
	ctrl->csr.sen.mode.m 		= 0;
	ctrl->csr.sen.vmax.l 		= 0x7004;
	ctrl->csr.sen.vmax.m 		= 0x7005;
	ctrl->csr.sen.vmax.h 		= 0x7006;	
	ctrl->csr.sen.expo.l 		= 0x000B;
	ctrl->csr.sen.expo.m 		= 0x000C;
	ctrl->csr.sen.expo.h 		= 0;
	ctrl->csr.sen.gain.l 		= 0x0009;
	ctrl->csr.sen.gain.m 		= 0x000A;
	ctrl->csr.sen.o_width.l		= 0x6015;
	ctrl->csr.sen.o_width.m		= 0x6016;
	ctrl->csr.sen.o_height.l	= 0x6010;
	ctrl->csr.sen.o_height.m	= 0x6011;

	ctrl->sen_clk			= 54000000; 		// sen_clk default=54Mhz, imx183=72Mhz
	ctrl->expo_time_min2 		= 74480;
	ctrl->expo_vmax 		= 3728;
	ctrl->expo_toffset 		= 47563; 		// tOffset (U32)(2.903 * 16384.0)
	ctrl->expo_h1period 		= 327680;		// h1Period 20.00us => (U32)(20.000 * 16384.0)

	ctrl->flags			= FLAG_MODE_2_LANES | FLAG_TRIGGER_IN;
}


// ------------------------------------------------------------------------------------------------
//  Settings for 327/327C

static struct vc_mode imx327_mono_modes[] = {
	{ MEDIA_BUS_FMT_Y10_1X10,     FLAG_MODE_2_LANES, 0x00 },
	{ MEDIA_BUS_FMT_Y10_1X10,     FLAG_MODE_4_LANES, 0x01 },
	{ 0, 0, 0 }
};

static struct vc_mode imx327_color_modes[] = {
	{ MEDIA_BUS_FMT_SRGGB10_1X10, FLAG_MODE_2_LANES, 0x00 },
	{ MEDIA_BUS_FMT_SRGGB10_1X10, FLAG_MODE_4_LANES, 0x01 },
	{ 0, 0, 0 }
};

static struct vc_framefmt imx327_mono_fmts[] = {
	{ MEDIA_BUS_FMT_Y10_1X10,     V4L2_COLORSPACE_SRGB },
	{ 0, 0 }
};

static struct vc_framefmt imx327_color_fmts[] = {
	{ MEDIA_BUS_FMT_SRGGB10_1X10, V4L2_COLORSPACE_SRGB },
	{ 0, 0 }
};

void vc_init_imx327_ctrl(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_sen->dev;

	dev_dbg(dev, "%s(): Initialising module control for IMX327\n", __FUNCTION__);
	
	ctrl->mod_id			= MOD_ID_IMX327;
	ctrl->mod_i2c_addr		= 0x10;

	ctrl->set.exposure.min		= 29;
	ctrl->set.exposure.max		= 7767184;
	ctrl->set.exposure.default_val	= 10000;
	ctrl->set.gain.min		= 0;
	ctrl->set.gain.max		= 240;
	ctrl->set.gain.default_val	= 0;

	if (vc_mod_is_color_sensor(desc)) {
		ctrl->modes		= imx327_color_modes;
		ctrl->fmts 		= imx327_color_fmts;
	} else {
		ctrl->modes		= imx327_mono_modes;
		ctrl->fmts 		= imx327_mono_fmts;
	}
	ctrl->default_mode		= 0;
	ctrl->default_fmt		= 0;
	ctrl->o_width 			= 1920;
	ctrl->o_height 			= 1080;

	ctrl->csr.sen.mode.l 		= 0x3000;			// Standby register: 0=Operating, 1=Standby
	ctrl->csr.sen.mode.m 		= 0x3002;
	ctrl->csr.sen.vmax.l 		= 0x3018;
	ctrl->csr.sen.vmax.m 		= 0x3019;
	ctrl->csr.sen.vmax.h 		= 0x301A;	
	ctrl->csr.sen.expo.l 		= 0x3020;
	ctrl->csr.sen.expo.m 		= 0x3021;
	ctrl->csr.sen.expo.h 		= 0x3022;
	ctrl->csr.sen.gain.l 		= 0x3014;
	ctrl->csr.sen.gain.m 		= 0;
	ctrl->csr.sen.o_width.l		= 0x3472;
	ctrl->csr.sen.o_width.m		= 0x3473;
	ctrl->csr.sen.o_height.l	= 0x3418;
	ctrl->csr.sen.o_height.m	= 0x3419;
	
	ctrl->sen_clk			= 54000000; 			// sen_clk default=54Mhz, imx183=72Mhz
	ctrl->expo_time_min2 		= 38716;
	ctrl->expo_vmax 		= 3728;
	ctrl->expo_toffset 		= 47563;			// tOffset (U32)(2.903 * 16384.0)
	ctrl->expo_h1period 		= 327680;			// h1Period 20.00us => (U32)(20.000 * 16384.0)
	
	ctrl->flags			= FLAG_MODE_2_LANES;
}

int vc_mod_ctrl_init(struct vc_ctrl* ctrl, struct vc_desc* desc)
{
	struct device *dev = &ctrl->client_sen->dev;

	switch(desc->mod_id) {
	case MOD_ID_IMX226: 
		vc_init_imx226_ctrl(ctrl, desc); 
		break;
	case MOD_ID_IMX327: 
		vc_init_imx327_ctrl(ctrl, desc); 
		break;
	default:
		dev_err(dev, "%s(): Detected Module not supported!\n", __FUNCTION__);
		return 1;
	}
	return 0;
}