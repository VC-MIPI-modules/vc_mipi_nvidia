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
        ctrl->exposure                  = (vc_control) { .min =   1, .max = 100000000, .def =  10000 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =       255, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =       255, .def =      0 };
        ctrl->framerate                 = (vc_control) { .min =   0, .max =   1000000, .def =      0 };

        ctrl->csr.sen.mode              = (vc_csr2) { .l = desc->csr_mode, .m = 0x0000 };

        ctrl->csr.sen.mode_standby      = 0x00; 
        ctrl->csr.sen.mode_operating    = 0x01;
        
        ctrl->csr.sen.shs.l             = desc->csr_exposure_l;
        ctrl->csr.sen.shs.m             = desc->csr_exposure_m;
        ctrl->csr.sen.shs.h             = desc->csr_exposure_h;
        ctrl->csr.sen.shs.u             = 0;
        
        ctrl->csr.sen.gain.l            = desc->csr_gain_l;
        ctrl->csr.sen.gain.m            = desc->csr_gain_h;

        ctrl->csr.sen.h_start.l         = desc->csr_h_start_l;
        ctrl->csr.sen.h_start.m         = desc->csr_h_start_h;
        ctrl->csr.sen.v_start.l         = desc->csr_v_start_l;
        ctrl->csr.sen.v_start.m         = desc->csr_v_start_h;
        ctrl->csr.sen.h_end.l           = desc->csr_h_end_l;
        ctrl->csr.sen.h_end.m           = desc->csr_h_end_h;
        ctrl->csr.sen.v_end.l           = desc->csr_v_end_l;
        ctrl->csr.sen.v_end.m           = desc->csr_v_end_h;
        ctrl->csr.sen.o_width.l         = desc->csr_o_width_l;
        ctrl->csr.sen.o_width.m         = desc->csr_o_width_h;
        ctrl->csr.sen.o_height.l        = desc->csr_o_height_l;
        ctrl->csr.sen.o_height.m        = desc->csr_o_height_h;

        ctrl->frame.left                = 0;
        ctrl->frame.top                 = 0;

        ctrl->clk_ext_trigger           = desc->clk_ext_trigger;
        ctrl->clk_pixel                 = desc->clk_pixel;
}

static void vc_init_ctrl_imx183_base(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x7004, .m = 0x7005, .h = 0x7006, .u = 0x0000 };
        ctrl->csr.sen.hmax              = (vc_csr4) { .l = 0x7002, .m = 0x7003, .h = 0x0000, .u = 0x0000 };

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL | FLAG_TRIGGER_SELF |
                                          FLAG_TRIGGER_SINGLE | FLAG_TRIGGER_SYNC;
}

static void vc_init_ctrl_imx252_base(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        ctrl->gain                      = (vc_control) { .min =   0, .max =       511, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =      4095, .def =     60 };

        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x0210, .m = 0x0211, .h = 0x0212, .u = 0x0000 };
        ctrl->csr.sen.hmax              = (vc_csr4) { .l = 0x0214, .m = 0x0215, .h = 0x0000, .u = 0x0000 };
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x0454, .m = 0x0455 };

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL | FLAG_TRIGGER_PULSEWIDTH |
                                          FLAG_TRIGGER_SELF | FLAG_TRIGGER_SINGLE;
}

static void vc_init_ctrl_imx290_base(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        ctrl->vmax                      = (vc_control) { .min =   1, .max =   0x3ffff, .def =   1125 };
        ctrl->exposure                  = (vc_control) { .min =   1, .max =  15000000, .def =  10000 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =       255, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =       511, .def =  0x0f0 };
        
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3018, .m = 0x3019, .h = 0x301A, .u = 0x0000 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x300a, .m = 0x300b };

        ctrl->frame.width               = 1920;
        ctrl->frame.height              = 1080;
        
        ctrl->clk_ext_trigger           = 74250000;
        ctrl->clk_pixel                 = 74250000;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
}

static void vc_init_ctrl_imx296_base(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        ctrl->vmax                      = (vc_control) { .min =   5, .max =   0xfffff, .def =   1110 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =     0xfff, .def =     60 };
        
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3010, .m = 0x3011, .h = 0x3012, .u = 0x0000 };
        ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x300A };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating	= 0x00;
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x3254, .m = 0x3255 };

        ctrl->expo_timing[0]            = (vc_timing) { 1, FORMAT_RAW10, .hmax =  1100 };
        
        ctrl->retrigger_min             = 0x000d7940;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL | FLAG_TRIGGER_PULSEWIDTH | FLAG_TRIGGER_SELF_V2;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX178/IMX178C  (Rev.01)

static void vc_init_ctrl_imx178(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX178\n", __FUNCTION__);

        vc_init_ctrl_imx183_base(ctrl, desc);

        ctrl->vmax                      = (vc_control) { .min =   9, .max =   0x1ffff, .def =   2126 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =     0x3ff, .def =     60 };
        
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x3015, .m = 0x3016 };

        ctrl->frame.width               = 3072;
        ctrl->frame.height              = 2048;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  680 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  840 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  984 };
        ctrl->expo_timing[3]            = (vc_timing) { 2, FORMAT_RAW14, .hmax = 1156 };
        ctrl->expo_timing[4]            = (vc_timing) { 4, FORMAT_RAW08, .hmax =  600 };
        ctrl->expo_timing[5]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  600 };
        ctrl->expo_timing[6]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  680 };
        ctrl->expo_timing[7]            = (vc_timing) { 4, FORMAT_RAW14, .hmax = 1156 };

        ctrl->retrigger_min             = 0x00292d40;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX183/IMX183C (Rev.12)

static void vc_init_ctrl_imx183(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX183\n", __FUNCTION__);

        vc_init_ctrl_imx183_base(ctrl, desc);

        ctrl->vmax                      = (vc_control) { .min =   5, .max =   0x1ffff, .def =   3728 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =     0x7a5, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =      0xff, .def =   0x32 };
        
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x0045, .m = 0x0000 };

        ctrl->frame.width               = 5440;
        ctrl->frame.height              = 3648;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax = 1440 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax = 1440 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax = 1724 };
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW08, .hmax =  720 };
        ctrl->expo_timing[4]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  720 };
        ctrl->expo_timing[5]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  862 };
        
        ctrl->retrigger_min             = 0x0036ee7d;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX226/IMX226C (Rev.13)

static void vc_init_ctrl_imx226(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX226\n", __FUNCTION__);

        vc_init_ctrl_imx183_base(ctrl, desc);

        ctrl->vmax                      = (vc_control) { .min =   5, .max =   0x1ffff, .def =   3079 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =     0x7a5, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =       255, .def =     50 };
        
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x0045, .m = 0x0000 };

        ctrl->frame.width               = 3904;
        ctrl->frame.height              = 3000;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax = 1072 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax = 1072 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax = 1288 };
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW08, .hmax =  536 };
        ctrl->expo_timing[4]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  536 };
        ctrl->expo_timing[5]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  644 };

        ctrl->clk_pixel                 = 72000000;
        ctrl->retrigger_min             = 0x00292d40;

        ctrl->flags                    |= FLAG_FORMAT_GBRG;
        ctrl->flags                    |= FLAG_TRIGGER_STREAM_EDGE | FLAG_TRIGGER_STREAM_LEVEL;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX250/IMX250C (Rev.07)

static void vc_init_ctrl_imx250(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX250\n", __FUNCTION__);

        vc_init_ctrl_imx252_base(ctrl, desc);

        ctrl->vmax                      = (vc_control) { .min =  10, .max =   0xfffff, .def =   2094 };

        ctrl->frame.width               = 2432;
        ctrl->frame.height              = 2048;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  540 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  660 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  780 };
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW08, .hmax =  350 };
        ctrl->expo_timing[4]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  430 };
        ctrl->expo_timing[5]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  510 };
        
        ctrl->retrigger_min             = 0x00181c08;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX252/IMX252C (Rev.10)

static void vc_init_ctrl_imx252(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX252\n", __FUNCTION__);

        vc_init_ctrl_imx252_base(ctrl, desc);

        ctrl->vmax                      = (vc_control) { .min =  10, .max =   0xfffff, .def =   1582 };

        ctrl->frame.width               = 2048;
        ctrl->frame.height              = 1536;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  460 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  560 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  672 };
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW08, .hmax =  310 };
        ctrl->expo_timing[4]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  380 };
        ctrl->expo_timing[5]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  444 };
        
        ctrl->retrigger_min             = 0x00103b4a;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX264/IMX264C (Rev.03)

static void vc_init_ctrl_imx264(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX264\n", __FUNCTION__);

        vc_init_ctrl_imx252_base(ctrl, desc);

        ctrl->vmax                      = (vc_control) { .min =  10, .max =   0xfffff, .def =   2100 };

        ctrl->frame.width               = 2432;
        ctrl->frame.height              = 2048;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  996 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  996 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  996 };

        ctrl->retrigger_min             = 0x00181c08;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX265/IMX265C (Rev.01)

static void vc_init_ctrl_imx265(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX265\n", __FUNCTION__);

        vc_init_ctrl_imx252_base(ctrl, desc);

        ctrl->vmax                      = (vc_control) { .min =  10, .max =   0xfffff, .def =   1587 };

        ctrl->frame.width               = 2048;
        ctrl->frame.height              = 1536;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  846 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  846 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  846 };

        ctrl->retrigger_min             = 0x00181c08;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX273/IMX273C (Rev.13)

static void vc_init_ctrl_imx273(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX273\n", __FUNCTION__);

        vc_init_ctrl_imx252_base(ctrl, desc);

        ctrl->vmax                      = (vc_control) { .min =  15, .max =   0xfffff, .def =   1130 };

        ctrl->frame.width               = 1440;
        ctrl->frame.height              = 1080;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  336 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  420 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  480 };
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW08, .hmax =  238 };
        ctrl->expo_timing[4]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  290 };
        ctrl->expo_timing[5]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  396 };

        ctrl->retrigger_min             = 0x0007ec3e;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX290 (Rev.02)

static void vc_init_ctrl_imx290(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX290\n", __FUNCTION__);

        vc_init_ctrl_imx290_base(ctrl, desc);

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  550 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  550 };
        ctrl->expo_timing[2]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  550 }; 
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  550 };
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX296/IMX296C (Rev.42)

static void vc_init_ctrl_imx296(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX296\n", __FUNCTION__);

        vc_init_ctrl_imx296_base(ctrl, desc);

        ctrl->frame.width               = 1440;
        ctrl->frame.height              = 1080;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX297 (Rev.??)

static void vc_init_ctrl_imx297(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX297\n", __FUNCTION__);

        vc_init_ctrl_imx296_base(ctrl, desc);

        ctrl->frame.width               = 720;
        ctrl->frame.height              = 540;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX327C (Rev.02)
//
// NOTES:
// - For vertical flipping VREVERSE 0x3007 = 0x01 has to be set.
// - For horizontal flipping HREVERSE 0x3007 = 0x02 has to be set.
// - For cropping WINMODE 0x3007 = 0x40 has to be set. Unfortunatly cropping mode does not reduce 
//   the image size. The image is always filled up to a size of 1920x1080.
// - To increase the frame rate it is possible to reduce VMAX. In this case the image height is forced
//   to be height = VMAX - 15. This is independend of the cropped image height.
// => Cropping is not properly supported.
// => Frame rate increase by image height reduction could be implemented. 
//    But, it need an own implementation.

static void vc_init_ctrl_imx327(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX327\n", __FUNCTION__);

        vc_init_ctrl_imx290_base(ctrl, desc);

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  1100 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  1100 };
        ctrl->expo_timing[2]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  1100 }; 
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  1100 };
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX335 (Rev.00)
//
//  TODO:
//  - Max. Framerate of 19.5 fps is to low. Has to be 60 fps

static void vc_init_ctrl_imx335(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX335\n", __FUNCTION__);

        ctrl->vmax                      = (vc_control) { .min =   9, .max =   0xfffff, .def =   4500 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =      0xff, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =     0x3ff, .def =   0x32 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x3302, .m = 0x3303 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3030, .m = 0x3031, .h = 0x3032, .u = 0x0000 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        ctrl->frame.left                = 7;
        ctrl->frame.top                 = 52;
        ctrl->frame.width               = 2592;
        ctrl->frame.height              = 1944;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =   916 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =   916 };
        ctrl->expo_timing[2]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =   614 };
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =   614 };

        ctrl->flags                    |= FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_DOUBLE_HEIGHT;
        ctrl->flags                    |= FLAG_IO_ENABLED;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX392/IMX392C (Rev.06)

static void vc_init_ctrl_imx392(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX392\n", __FUNCTION__);

        vc_init_ctrl_imx252_base(ctrl, desc);

        ctrl->vmax                      = (vc_control) { .min =  10, .max =   0xfffff, .def =   1252 };

        ctrl->frame.width               = 1920;
        ctrl->frame.height              = 1200;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  448 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  530 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  624 };
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW08, .hmax =  294 };
        ctrl->expo_timing[4]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  355 };
        ctrl->expo_timing[5]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  441 };
        
        ctrl->retrigger_min             = 0x00103b4a;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX412C (Rev.02)
//
//  NOTES: 
//  - No TRIGGER and FLASH capability.
//  TODO:
//  - Slave Mode not implemented.

static void vc_init_ctrl_imx412(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX412\n", __FUNCTION__);
        
        ctrl->vmax                      = (vc_control) { .min =  10, .max =    0xffff, .def = 0x0c14 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =      1023, .def =      0 };

        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x0341, .m = 0x0340, .h = 0x0000, .u = 0x0000 };
        ctrl->csr.sen.shs               = (vc_csr4) { .l = 0x0203, .m = 0x0202, .h = 0x0000, .u = 0x0000 };

        ctrl->frame.width               = 4032;
        ctrl->frame.height              = 3040;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  436 };
        ctrl->expo_timing[1]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  218 };

        ctrl->clk_ext_trigger           = 27000000;
        ctrl->clk_pixel                 = 27000000;

        ctrl->flags                     = FLAG_RESET_ALWAYS;
        ctrl->flags                    |= FLAG_EXPOSURE_NORMAL;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_SLAVE;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX415C (Rev.01)

static void vc_init_ctrl_imx415(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX415\n", __FUNCTION__);
        
        ctrl->vmax                      = (vc_control) { .min =   8, .max =   0xfffff, .def =  0x8ca };
        ctrl->gain                      = (vc_control) { .min =   0, .max =       240, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =     0x3ff, .def =   0x32 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x30e2, .m = 0x30e3 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3024, .m = 0x3025, .h = 0x3026, .u = 0x0000 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        ctrl->frame.width               = 3840;
        ctrl->frame.height              = 2160;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW10, .hmax = 1042 };
        ctrl->expo_timing[1]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  551 };

        ctrl->clk_pixel                 = 74250000;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_DOUBLE_HEIGHT;
        ctrl->flags                    |= FLAG_FORMAT_GBRG;
        ctrl->flags                    |= FLAG_IO_ENABLED;
}

// -------------------------------------------------------------
//  Settings for IMX462 (Rev.01)

static void vc_init_ctrl_imx462(struct vc_ctrl *ctrl, struct vc_desc *desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX462\n", __FUNCTION__);

        vc_init_ctrl_imx290_base(ctrl, desc);

        ctrl->gain    = (vc_control) { .min = 0, .max = 238,     .def = 0 };
        ctrl->vmax    = (vc_control) { .min = 1, .max = 0x3ffff, .def = 0x465 };

        ctrl->expo_timing[0] = (vc_timing){2, FORMAT_RAW10, .hmax = 1100};
        ctrl->expo_timing[1] = (vc_timing){4, FORMAT_RAW10, .hmax = 550};
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX565 (Rev.01)

static void vc_init_ctrl_imx565(struct vc_ctrl *ctrl, struct vc_desc *desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX565 \n", __FUNCTION__);

        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =     0xfff, .def =   0x3c };
        ctrl->vmax                      = (vc_control) { .min =  27, .max =   0xfffff, .def =  0xcd4 };
        
        ctrl->csr.sen.gain              = (vc_csr2) { .l = 0x3514, .m = 0x3515 };
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x35b4, .m = 0x35b5 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x30d4, .m = 0x30d5, .h = 0x30d6, .u = 0x0000 };
        ctrl->csr.sen.hmax              = (vc_csr4) { .l = 0x30d8, .m = 0x30d9, .h = 0x0000, .u = 0x0000 };
        ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x3010 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        ctrl->frame.width               = 4128;
        ctrl->frame.height              = 3000;
        ctrl->frame.left                = 0;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  1071 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  1329 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  1587 };
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW08, .hmax =  555 };
        // ---------------------------------------------------------------
        // Workaround for Rev.01. This limits the fps to 18.8 fps!
        // The theoretically correct value for Rev.02 is .hmax = 684
        ctrl->expo_timing[4]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  1197 };
        // ---------------------------------------------------------------
        ctrl->expo_timing[5]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  813 };

        ctrl->retrigger_min             = 0x0011A7F8;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_PULSEWIDTH;
        ctrl->flags                    |= FLAG_TRIGGER_SELF;
        ctrl->flags                    |= FLAG_TRIGGER_SINGLE;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX568 (Rev.01)

static void vc_init_ctrl_imx568(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for IMX568\n", __FUNCTION__);

        ctrl->vmax                      = (vc_control) { .min =  42, .max =  0xffffff, .def =  0x8aa };
        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };
        ctrl->blacklevel                = (vc_control) { .min =   0, .max =      4095, .def =   0x3c };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x35b4, .m = 0x35b5 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x30d4, .m = 0x30d5, .h = 0x30d6, .u = 0x0000 };
        ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x3010 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        ctrl->frame.width               = 2472;
        ctrl->frame.height              = 2048;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  489 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  590 };
        ctrl->expo_timing[2]            = (vc_timing) { 2, FORMAT_RAW12, .hmax =  703 };
        ctrl->expo_timing[3]            = (vc_timing) { 4, FORMAT_RAW08, .hmax =  253 };
        ctrl->expo_timing[4]            = (vc_timing) { 4, FORMAT_RAW10, .hmax =  309 };
        ctrl->expo_timing[5]            = (vc_timing) { 4, FORMAT_RAW12, .hmax =  365 };

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL | FLAG_TRIGGER_PULSEWIDTH |
                                          FLAG_TRIGGER_SELF | FLAG_TRIGGER_SINGLE;
}

// ------------------------------------------------------------------------------------------------
//  Settings for OV7251 (Rev.01)
//
//  TODO: 
//  - No flash out

static void vc_init_ctrl_ov7251(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for OV7251\n", __FUNCTION__);

        ctrl->vmax                      = (vc_control) { .min =   0, .max =    0xffff, .def =    598 };
        ctrl->exposure                  = (vc_control) { .min =   1, .max =   1000000, .def =  10000 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =      1023, .def =      0 };

        ctrl->csr.sen.h_end             = (vc_csr2) { .l = 0x0000, .m = 0x0000 };
        ctrl->csr.sen.v_end             = (vc_csr2) { .l = 0x0000, .m = 0x0000 };
        ctrl->csr.sen.flash_duration    = (vc_csr4) { .l = 0x3b8f, .m = 0x3b8e, .h = 0x3b8d, .u = 0x3b8c };
        ctrl->csr.sen.flash_offset      = (vc_csr4) { .l = 0x3b8b, .m = 0x3b8a, .h = 0x3b89, .u = 0x3b88 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x380f, .m = 0x380e, .h = 0x0000, .u = 0x0000 };
        // NOTE: Modules rom table contains swapped address assigment.
        ctrl->csr.sen.gain              = (vc_csr2) { .l = 0x350b, .m = 0x350a };
        
        ctrl->frame.width               = 640;
        ctrl->frame.height              = 480;

        ctrl->expo_timing[0]            = (vc_timing) { 1, FORMAT_RAW08, .hmax =  772 };
        ctrl->expo_timing[1]            = (vc_timing) { 1, FORMAT_RAW10, .hmax =  772 };

        ctrl->flash_factor              = 1758241 >> 4; // (1000 << 4)/9100 >> 4
        ctrl->flash_toffset             = 4;

        ctrl->flags                     = FLAG_EXPOSURE_OMNIVISION;
        ctrl->flags                    |= FLAG_IO_ENABLED;
}

// ------------------------------------------------------------------------------------------------
//  Settings for OV9281 (Rev.02)

static void vc_init_ctrl_ov9281(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_notice(dev, "%s(): Initialising module control for OV9281\n", __FUNCTION__);
        
        ctrl->vmax                      = (vc_control) { .min =  16, .max =    0xffff, .def =    910 };
        ctrl->exposure                  = (vc_control) { .min = 146, .max =    595000, .def =  10000 };
        ctrl->gain                      = (vc_control) { .min =  16, .max =       255, .def =     16 };

        ctrl->csr.sen.h_end             = (vc_csr2) { .l = 0x0000, .m = 0x0000 };
        ctrl->csr.sen.v_end             = (vc_csr2) { .l = 0x0000, .m = 0x0000 };
        ctrl->csr.sen.flash_duration	= (vc_csr4) { .l = 0x3928, .m = 0x3927, .h = 0x3926, .u = 0x3925 };
        ctrl->csr.sen.flash_offset      = (vc_csr4) { .l = 0x3924, .m = 0x3923, .h = 0x3922, .u = 0x0000 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x380f, .m = 0x380e, .h = 0x0000, .u = 0x0000 };
        // NOTE: Modules rom table contains swapped address assigment.
        ctrl->csr.sen.gain              = (vc_csr2) { .l = 0x3509, .m = 0x0000 };
        
        ctrl->frame.width               = 1280;
        ctrl->frame.height              = 800;

        ctrl->expo_timing[0]            = (vc_timing) { 2, FORMAT_RAW08, .hmax =  227 };
        ctrl->expo_timing[1]            = (vc_timing) { 2, FORMAT_RAW10, .hmax =  227 };

        ctrl->clk_ext_trigger           = 25000000;
        ctrl->clk_pixel                 = 25000000;

        ctrl->flash_factor              = 1758241 >> 4; // (1000 << 4)/9100 >> 4
        ctrl->flash_toffset             = 4;

        ctrl->flags                     = FLAG_EXPOSURE_OMNIVISION;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL;
}


int vc_mod_ctrl_init(struct vc_ctrl* ctrl, struct vc_desc* desc)
{
        struct device *dev = &ctrl->client_mod->dev;

        vc_init_ctrl(ctrl, desc);

        switch(desc->mod_id) {
        case MOD_ID_IMX178: vc_init_ctrl_imx178(ctrl, desc); break;
        case MOD_ID_IMX183: vc_init_ctrl_imx183(ctrl, desc); break;
        case MOD_ID_IMX226: vc_init_ctrl_imx226(ctrl, desc); break;
        case MOD_ID_IMX250: vc_init_ctrl_imx250(ctrl, desc); break;
        case MOD_ID_IMX252: vc_init_ctrl_imx252(ctrl, desc); break;
        case MOD_ID_IMX264: vc_init_ctrl_imx264(ctrl, desc); break;
        case MOD_ID_IMX265: vc_init_ctrl_imx265(ctrl, desc); break;
        case MOD_ID_IMX273: vc_init_ctrl_imx273(ctrl, desc); break;
        case MOD_ID_IMX290: vc_init_ctrl_imx290(ctrl, desc); break;
        case MOD_ID_IMX296: vc_init_ctrl_imx296(ctrl, desc); break;
        case MOD_ID_IMX297: vc_init_ctrl_imx297(ctrl, desc); break;
        case MOD_ID_IMX327: vc_init_ctrl_imx327(ctrl, desc); break;
        case MOD_ID_IMX335: vc_init_ctrl_imx335(ctrl, desc); break;
        case MOD_ID_IMX392: vc_init_ctrl_imx392(ctrl, desc); break;
        case MOD_ID_IMX412: vc_init_ctrl_imx412(ctrl, desc); break;
        case MOD_ID_IMX415: vc_init_ctrl_imx415(ctrl, desc); break;
        case MOD_ID_IMX462: vc_init_ctrl_imx462(ctrl, desc); break;
        case MOD_ID_IMX565: vc_init_ctrl_imx565(ctrl, desc); break;
        case MOD_ID_IMX568: vc_init_ctrl_imx568(ctrl, desc); break;
        case MOD_ID_OV7251: vc_init_ctrl_ov7251(ctrl, desc); break;
        case MOD_ID_OV9281: vc_init_ctrl_ov9281(ctrl, desc); break;
        default:
                vc_err(dev, "%s(): Detected module not supported!\n", __FUNCTION__);
                return 1;
        }

        return 0;
}