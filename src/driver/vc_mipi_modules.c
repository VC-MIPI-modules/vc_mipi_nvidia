#include "vc_mipi_modules.h"
#include <linux/v4l2-mediabus.h>


#define INIT_MESSAGE(camera) \
        struct device *dev = &ctrl->client_mod->dev; \
        vc_notice(dev, "%s(): Initialising module control for %s\n", __FUNCTION__, camera);

#define FRAME(_left, _top, _width, _height) \
        ctrl->frame = (vc_frame) { .left = _left, .top = _top, .width = _width, .height = _height };

#define MODE(index, lanes, format, binning, _hmax, vmax_min, vmax_max, vmax_def, \
        blacklevel_max, blacklevel_def, _retrigger_min) \
        ctrl->mode[index] = (vc_mode) { lanes, format, binning, \
                .hmax = _hmax,  \
                .vmax = {.min = vmax_min, .max = vmax_max, .def = vmax_def}, \
                .blacklevel = {.min = 0, .max = blacklevel_max,  .def = blacklevel_def }, \
                .retrigger_min = _retrigger_min };

#define BINNING_MODE_REGS(_mode, ...) \
        if (MAX_VC_MODES > _mode) { \
                do { \
                        const struct vc_reg _binning_mode_regs [] = { __VA_ARGS__ }; \
                        int vrs = 0; \
                        vrs = sizeof(_binning_mode_regs) / sizeof(vc_reg); \
                                if (MAX_BINNING_MODE_REGS > vrs) { \
                                        memcpy(&ctrl->mode[_mode].binning_mode_regs, _binning_mode_regs, sizeof(_binning_mode_regs)); \
                                } \
                } while (0); \
        }

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
        ctrl->gain                      = (vc_control) { .min =   0, .max =       255, .def =      0 };
        
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3018, .m = 0x3019, .h = 0x301A, .u = 0x0000 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x300a, .m = 0x300b };

        FRAME(0, 0, 1920, 1080)
        
        ctrl->clk_ext_trigger           = 74250000;
        ctrl->clk_pixel                 = 74250000;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
}

static void vc_init_ctrl_imx296_base(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };
        
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3010, .m = 0x3011, .h = 0x3012, .u = 0x0000 };
        ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x300A };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating	= 0x00;
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x3254, .m = 0x3255 };

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL | FLAG_TRIGGER_PULSEWIDTH | FLAG_TRIGGER_SELF_V2;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX178/IMX178C  (Rev.02)
//  6.44 MegaPixel Starvis

static void vc_init_ctrl_imx178(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX178")

        vc_init_ctrl_imx183_base(ctrl, desc);

        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x3015, .m = 0x3016 };

        FRAME(0, 0, 3072, 2048)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     680,    9,  0x1ffff,  2126,  255,   50,   2698560)
        MODE( 1, 2, FORMAT_RAW10, 0,     840,    9,  0x1ffff,  2126, 1023,   50,   2698560)
        MODE( 2, 2, FORMAT_RAW12, 0,     984,    9,  0x1ffff,  2126, 1023,  200,   2698560)
        MODE( 3, 2, FORMAT_RAW14, 0,    1156,    9,  0x1ffff,  2126, 4095,  800,   2698560)
        MODE( 4, 4, FORMAT_RAW08, 0,     600,    9,  0x1ffff,  2126,  255,   50,   2698560)
        MODE( 5, 4, FORMAT_RAW10, 0,     600,    9,  0x1ffff,  2126, 1023,   50,   2698560)
        MODE( 6, 4, FORMAT_RAW12, 0,     680,    9,  0x1ffff,  2126, 1023,  200,   2698560)
        MODE( 7, 4, FORMAT_RAW14, 0,    1156,    9,  0x1ffff,  2126, 4095,  800,   2698560)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX183/IMX183C (Rev.15)
//  20.2 MegaPixel

static void vc_init_ctrl_imx183(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX183")

        vc_init_ctrl_imx183_base(ctrl, desc);

        ctrl->gain                      = (vc_control) { .min =   0, .max =     0x7a5, .def =      0 };
        
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x0045, .m = 0x0000 };

        FRAME(0, 0, 5440, 3648)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,    1440,    5,  0x1ffff,  3728,  255,   50,   3599997)
        MODE( 1, 2, FORMAT_RAW10, 0,    1440,    5,  0x1ffff,  3728,  255,   50,   3599997)
        MODE( 2, 2, FORMAT_RAW12, 0,    1724,    5,  0x1ffff,  3728,  255,   50,   3599997)
        MODE( 3, 4, FORMAT_RAW08, 0,     720,    5,  0x1ffff,  3728,  255,   50,   3599997)
        MODE( 4, 4, FORMAT_RAW10, 0,     720,    5,  0x1ffff,  3728,  255,   50,   3599997)
        MODE( 5, 4, FORMAT_RAW12, 0,     862,    5,  0x1ffff,  3728,  255,   50,   3599997)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX226/IMX226C (Rev.16)
//  12.4 MegaPixel Starvis

static void vc_init_ctrl_imx226(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX226")

        vc_init_ctrl_imx183_base(ctrl, desc);

        ctrl->gain                      = (vc_control) { .min =   0, .max =     0x7a5, .def =      0 };
        
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x0045, .m = 0x0000 };

        FRAME(0, 0, 3904, 3000)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,    1072,    5,  0x1ffff,  3079,  255,   50,   2698560)
        MODE( 1, 2, FORMAT_RAW10, 0,    1072,    5,  0x1ffff,  3079,  255,   50,   2698560)
        MODE( 2, 2, FORMAT_RAW12, 0,    1288,    5,  0x1ffff,  3079,  255,   50,   2698560)
        MODE( 3, 4, FORMAT_RAW08, 0,     536,    5,  0x1ffff,  3079,  255,   50,   2698560)
        MODE( 4, 4, FORMAT_RAW10, 0,     536,    5,  0x1ffff,  3079,  255,   50,   2698560)
        MODE( 5, 4, FORMAT_RAW12, 0,     644,    5,  0x1ffff,  3079,  255,   50,   2698560)

        ctrl->clk_pixel                 = 72000000;

        ctrl->flags                    |= FLAG_FORMAT_GBRG;
        ctrl->flags                    |= FLAG_TRIGGER_STREAM_EDGE | FLAG_TRIGGER_STREAM_LEVEL;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX250/IMX250C (Rev.09)
//  5.01 MegaPixel Pregius

static void vc_init_ctrl_imx250(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX250")

        vc_init_ctrl_imx252_base(ctrl, desc);

        FRAME(0, 0, 2432, 2048)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     540,   10,  0xfffff,  2094,  255,   15,   1580040)
        MODE( 1, 2, FORMAT_RAW10, 0,     660,   10,  0xfffff,  2094, 1023,   60,   1580040)
        MODE( 2, 2, FORMAT_RAW12, 0,     780,   10,  0xfffff,  2094, 4095,  240,   1580040)
        MODE( 3, 4, FORMAT_RAW08, 0,     350,   10,  0xfffff,  2094,  255,   15,   1580040)
        MODE( 4, 4, FORMAT_RAW10, 0,     430,   10,  0xfffff,  2094, 1023,   60,   1580040)
        MODE( 5, 4, FORMAT_RAW12, 0,     510,   10,  0xfffff,  2094, 4095,  240,   1580040)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX252/IMX252C (Rev.12)
//  3.15 MegaPixel Pregius

static void vc_init_ctrl_imx252(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX252")

        vc_init_ctrl_imx252_base(ctrl, desc);

        FRAME(0, 0, 2048, 1536)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     460,   10,  0xfffff,  1582,  255,   15,   1063754)
        MODE( 1, 2, FORMAT_RAW10, 0,     560,   10,  0xfffff,  1582, 1023,   60,   1063754)
        MODE( 2, 2, FORMAT_RAW12, 0,     672,   10,  0xfffff,  1582, 4095,  240,   1063754)
        MODE( 3, 4, FORMAT_RAW08, 0,     310,   10,  0xfffff,  1582,  255,   15,   1063754)
        MODE( 4, 4, FORMAT_RAW10, 0,     380,   10,  0xfffff,  1582, 1023,   60,   1063754)
        MODE( 5, 4, FORMAT_RAW12, 0,     444,   10,  0xfffff,  1582, 4095,  240,   1063754)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX264/IMX264C (Rev.05)
//  5.1 MegaPixel Pregius

static void vc_init_ctrl_imx264(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX264")

        vc_init_ctrl_imx252_base(ctrl, desc);

        FRAME(0, 0, 2432, 2048)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     996,   10,  0xfffff,  2100,  255,   15,   1580040)
        MODE( 1, 2, FORMAT_RAW10, 0,     996,   10,  0xfffff,  2100, 1023,   60,   1580040)
        MODE( 2, 2, FORMAT_RAW12, 0,     996,   10,  0xfffff,  2100, 4095,  240,   1580040)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX265/IMX265C (Rev.05)
//  3.2 MegaPixel Pregius

static void vc_init_ctrl_imx265(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX265")

        vc_init_ctrl_imx252_base(ctrl, desc);

        FRAME(0, 0, 2048, 1536)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     846,   10,  0xfffff,  1587,  255,  255,   1580040)
        MODE( 1, 2, FORMAT_RAW10, 0,     846,   10,  0xfffff,  1587, 1023, 1023,   1580040)
        MODE( 2, 2, FORMAT_RAW12, 0,     846,   10,  0xfffff,  1587, 4095, 4095,   1580040)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX273/IMX273C (Rev.16)
//  1.56 MegaPixel Pregius

static void vc_init_ctrl_imx273(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX273")

        vc_init_ctrl_imx252_base(ctrl, desc);

        FRAME(0, 0, 1440, 1080)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     336,   15,  0xfffff,  1130,  255,   15,    519230)
        MODE( 1, 2, FORMAT_RAW10, 0,     420,   15,  0xfffff,  1130, 1023,   60,    519230)
        MODE( 2, 2, FORMAT_RAW12, 0,     480,   15,  0xfffff,  1130, 4095,  240,    519230)
        MODE( 3, 4, FORMAT_RAW08, 0,     238,   15,  0xfffff,  1130,  255,   15,    519230)
        MODE( 4, 4, FORMAT_RAW10, 0,     290,   15,  0xfffff,  1130, 1032,   60,    519230)
        MODE( 5, 4, FORMAT_RAW12, 0,     396,   15,  0xfffff,  1130, 4095,  240,    519230)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX290 (Rev.02)
//  2.0 MegaPixel Starvis

static void vc_init_ctrl_imx290(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX290")

        vc_init_ctrl_imx290_base(ctrl, desc);

        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW10, 0,     550,    1,  0x3ffff, 0x465,  511,   60,         0)
        MODE( 1, 2, FORMAT_RAW12, 0,     550,    1,  0x3ffff, 0x465,  511,  240,         0)
        MODE( 2, 4, FORMAT_RAW10, 0,     550,    1,  0x3ffff, 0x465,  511,   60,         0)
        MODE( 3, 4, FORMAT_RAW12, 0,     550,    1,  0x3ffff, 0x465,  511,  240,         0)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX296/IMX296C (Rev.43)
//  1.56 MegaPixel Pregius

static void vc_init_ctrl_imx296(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX296")

        vc_init_ctrl_imx296_base(ctrl, desc);

        FRAME(0, 0, 1440, 1080)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 1, FORMAT_RAW10, 0,    1100,    5,  0xfffff,  1110, 1023,   60,    883008)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX297 (Rev.43)
//  0.39 MegaPixel Pregius

static void vc_init_ctrl_imx297(struct vc_ctrl *ctrl, struct vc_desc* desc)
{       
        INIT_MESSAGE("IMX297")

        vc_init_ctrl_imx296_base(ctrl, desc);

        FRAME(0, 0, 704, 540) // 720 isn't divisible by 32
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 1, FORMAT_RAW10, 0,     550,    5,  0xfffff,  1110,  511,   60,    883008)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX327C (Rev.02)
//  2 MegaPixel Starvis

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
        INIT_MESSAGE("IMX327")

        vc_init_ctrl_imx290_base(ctrl, desc);

        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW10, 0,    1100,    1,  0x3ffff, 0x465,  511,   60,         0)
        MODE( 1, 2, FORMAT_RAW12, 0,    1100,    1,  0x3ffff, 0x465,  511,  240,         0)
        MODE( 2, 4, FORMAT_RAW10, 0,    1100,    1,  0x3ffff, 0x465,  511,   60,         0)
        MODE( 3, 4, FORMAT_RAW12, 0,    1100,    1,  0x3ffff, 0x465,  511,  240,         0)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX335 (Rev.02)
//  5.0 MegaPixel Starvis

static void vc_init_ctrl_imx335(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX335")

        ctrl->gain                      = (vc_control) { .min =   0, .max =      0xff, .def =      0 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x3302, .m = 0x3303 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3030, .m = 0x3031, .h = 0x3032, .u = 0x0000 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        FRAME(7, 52, 2592, 1944)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW10, 0,   0x113,    9,  0xfffff,  4500, 1023,   50,         0)
        MODE( 1, 2, FORMAT_RAW12, 0,   0x226,    9,  0xfffff,  4500, 1023,   50,         0)
        MODE( 2, 4, FORMAT_RAW10, 0,   0x113,    9,  0xfffff,  4500, 1023,   50,         0)
        MODE( 3, 4, FORMAT_RAW12, 0,   0x226,    9,  0xfffff,  4500, 1023,   50,         0)

        ctrl->flags                    |= FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_DOUBLE_HEIGHT;
        ctrl->flags                    |= FLAG_IO_ENABLED;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX392/IMX392C (Rev.08)
//  2.3 MegaPixel Pregius

static void vc_init_ctrl_imx392(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX392")

        vc_init_ctrl_imx252_base(ctrl, desc);

        FRAME(0, 0, 1920, 1200)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     448,   10,  0xfffff,  1252,  255,   15,   1063754)
        MODE( 1, 2, FORMAT_RAW10, 0,     530,   10,  0xfffff,  1252, 1023,   60,   1063754)
        MODE( 2, 2, FORMAT_RAW12, 0,     624,   10,  0xfffff,  1252, 4095,  240,   1063754)
        MODE( 3, 4, FORMAT_RAW08, 0,     294,   10,  0xfffff,  1252,  255,   15,   1063754)
        MODE( 4, 4, FORMAT_RAW10, 0,     355,   10,  0xfffff,  1252, 1023,   60,   1063754)
        MODE( 5, 4, FORMAT_RAW12, 0,     441,   10,  0xfffff,  1252, 4095,  240,   1063754)
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX412C (Rev.05)
//  12.3 MegaPixel Starvis
//  
//  TODO: 
//  - No black level (0x0B04 <= 1 - Black level correction enable)

#define IMX412_BINNING_MODE            0x0900
#define IMX412_BINNING_MODE_DISABLE    0x00
#define IMX412_BINNING_MODE_ENABLE     0x01
#define IMX412_BINNING_TYPE            0x0901
#define IMX412_BINNING_WEIGHTING       0x0902
#define IMX412_BINNING_WEIGHTING_AVG   0x00
#define IMX412_BINNING_TYPE_EXT_EN     0x3f42
#define IMX412_BINNING_TYPE_H_EXT      0x3f43

static void vc_init_ctrl_imx412(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX412")
        
        ctrl->gain                      = (vc_control) { .min =   0, .max =      1023, .def =      0 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x0009, .m = 0x0008 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x0341, .m = 0x0340, .h = 0x0000, .u = 0x0000 };
        ctrl->csr.sen.shs               = (vc_csr4) { .l = 0x0203, .m = 0x0202, .h = 0x0000, .u = 0x0000 };

        FRAME(0, 0, 4032, 3040)
        //all read out         binning  hmax  vmax      vmax    vmax  blkl  blkl  retrigger
        //                      mode           min       max     def   max   def
        MODE( 0, 2, FORMAT_RAW10, 0,     436,   10,   0xffff, 0x0c14, 1023,   40,         0)
        MODE( 1, 4, FORMAT_RAW10, 0,     218,   10,   0xffff, 0x0c14, 1023,   40,         0)

        ctrl->clk_ext_trigger           = 27000000;
        ctrl->clk_pixel                 = 27000000;

        BINNING_START(ctrl->binnings[0], 0, 0)
                { IMX412_BINNING_MODE, IMX412_BINNING_MODE_DISABLE },
                { IMX412_BINNING_TYPE, 0x11 }
        BINNING_END(ctrl->binnings[0])
        BINNING_START(ctrl->binnings[1], 1, 2)
                { IMX412_BINNING_MODE, IMX412_BINNING_MODE_ENABLE },
                { IMX412_BINNING_WEIGHTING, IMX412_BINNING_WEIGHTING_AVG },
                { IMX412_BINNING_TYPE, 0x12 }
        BINNING_END(ctrl->binnings[1])
        BINNING_START(ctrl->binnings[2], 2, 2)
                { IMX412_BINNING_MODE, IMX412_BINNING_MODE_ENABLE },
                { IMX412_BINNING_WEIGHTING, IMX412_BINNING_WEIGHTING_AVG },
                { IMX412_BINNING_TYPE, 0x22 }
        BINNING_END(ctrl->binnings[2])
        BINNING_START(ctrl->binnings[3], 4, 2)
                { IMX412_BINNING_MODE, IMX412_BINNING_MODE_ENABLE },
                { IMX412_BINNING_WEIGHTING, IMX412_BINNING_WEIGHTING_AVG },
                { IMX412_BINNING_TYPE, 0x42 }
        BINNING_END(ctrl->binnings[3])
        BINNING_START(ctrl->binnings[4], 8, 2)
                { IMX412_BINNING_MODE, IMX412_BINNING_MODE_ENABLE },
                { IMX412_BINNING_WEIGHTING, IMX412_BINNING_WEIGHTING_AVG },
                { IMX412_BINNING_TYPE, 0x82 }
        BINNING_END(ctrl->binnings[4])

        BINNING_START(ctrl->binnings[5], 16, 2)
                { IMX412_BINNING_MODE, IMX412_BINNING_MODE_ENABLE },
                { IMX412_BINNING_WEIGHTING, IMX412_BINNING_WEIGHTING_AVG },
                { IMX412_BINNING_TYPE, 0x02 },
                { IMX412_BINNING_TYPE_EXT_EN, 0x01 },
                { IMX412_BINNING_TYPE_H_EXT, 0x01 }
        BINNING_END(ctrl->binnings[5])

        ctrl->max_binning_modes_used = 5;

        ctrl->flags                     = FLAG_RESET_ALWAYS;
        ctrl->flags                    |= FLAG_EXPOSURE_NORMAL;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_SLAVE;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX415C (Rev.02)
//  8.3 MegaPixel Starvis

static void vc_init_ctrl_imx415(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX415")
        
        ctrl->gain                      = (vc_control) { .min =   0, .max =       240, .def =      0 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x30e2, .m = 0x30e3 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x3024, .m = 0x3025, .h = 0x3026, .u = 0x0000 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        FRAME(0, 0, 3840, 2160)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW10, 0,    1042,    8,  0xfffff, 0x8ca, 1023,   50,         0)
        MODE( 1, 4, FORMAT_RAW10, 0,     551,    8,  0xfffff, 0x8ca, 1023,   50,         0)

        ctrl->clk_pixel                 = 74250000;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_DOUBLE_HEIGHT;
        ctrl->flags                    |= FLAG_FORMAT_GBRG;
        ctrl->flags                    |= FLAG_IO_ENABLED;
}

// -------------------------------------------------------------
//  Settings for IMX462 (Rev.01)
//  2.0 MegaPixel Starvis

static void vc_init_ctrl_imx462(struct vc_ctrl *ctrl, struct vc_desc *desc)
{
        INIT_MESSAGE("IMX462")

        vc_init_ctrl_imx290_base(ctrl, desc);

        ctrl->gain    = (vc_control) { .min = 0, .max = 238,     .def = 0 };

        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW10, 0,    1100,    1,  0x3ffff, 0x465,  511,   60,         0)
        MODE( 1, 4, FORMAT_RAW10, 0,     550,    1,  0x3ffff, 0x465,  511,   60,         0)
}

#define IMX56X_HV_MODE                  0x303c
#define IMX56X_BINNING_MODE_DISABLE     0x00
#define IMX56X_BINNING_MODE_ENABLE      0x10
#define IMX56X_VBLK_HWIDTH_UPPER        0x30d1
#define IMX56X_VBLK_HWIDTH_LOWER        0x30d0
#define IMX56X_FINFO_HWIDTH_UPPER       0x30d3
#define IMX56X_FINFO_HWIDTH_LOWER       0x30d2
#define IMX56X_EAV_SELECT               0x3942
#define IMX56X_EAV_SELECT_VALUE         0x03

#define IMX56X_GMRWT                    0x30e2
#define IMX56X_GMTWT                    0x30e3
#define IMX56X_GAINDLY                  0x30e5
#define IMX56X_GSDLY                    0x30e6

// ------------------------------------------------------------------------------------------------
//  Settings for IMX565 (Rev.03)
//  12.4 MegaPixel Pregius S

static void vc_init_ctrl_imx565(struct vc_ctrl *ctrl, struct vc_desc *desc)
{
        INIT_MESSAGE("IMX565")

        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };
        
        ctrl->csr.sen.gain              = (vc_csr2) { .l = 0x3514, .m = 0x3515 };
        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x35b4, .m = 0x35b5 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x30d4, .m = 0x30d5, .h = 0x30d6, .u = 0x0000 };
        ctrl->csr.sen.hmax              = (vc_csr4) { .l = 0x30d8, .m = 0x30d9, .h = 0x0000, .u = 0x0000 };
        ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x3010 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        FRAME(0, 0, 4128, 3000)

        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,    1070,   18, 0xffffff, 0xc2c,  255,   15,   2410776)
        MODE( 1, 2, FORMAT_RAW10, 0,    1328,   16, 0xffffff, 0xc2a, 1023,   60,   2990142)
        MODE( 2, 2, FORMAT_RAW12, 0,    1586,   14, 0xffffff, 0xc26, 4095,  240,   3568752)
        MODE( 3, 4, FORMAT_RAW08, 0,     555,   30, 0xffffff, 0xc40,  255,   15,   1256094)
        MODE( 4, 4, FORMAT_RAW10, 0,     684,   26, 0xffffff, 0xc3a, 1023,   60,   1546074)
        MODE( 5, 4, FORMAT_RAW12, 0,     812,   22, 0xffffff, 0xc34, 4095,  240,   1833030)

        //binning
        MODE( 6, 2, FORMAT_RAW08, 1,     554,  32, 0xffffff, 0x644,  255,   15,    638982)
        MODE( 7, 2, FORMAT_RAW10, 1,     683,  28, 0xffffff, 0x640, 1023,   60,    785808)
        MODE( 8, 2, FORMAT_RAW12, 1,     812,  24, 0xffffff, 0x638, 4095,  240,    931878)
        MODE( 9, 4, FORMAT_RAW08, 1,     297,  52, 0xffffff, 0x668,  255,   15,    347760)
        MODE(10, 4, FORMAT_RAW10, 1,     361,  44, 0xffffff, 0x65c, 1023,   60,    420552)
        MODE(11, 4, FORMAT_RAW12, 1,     425,  40, 0xffffff, 0x654, 4095,  240,    493884)
        
        // special registers for binning mode
        BINNING_MODE_REGS(  6, { IMX56X_GMRWT, 0x04 }, { IMX56X_GMTWT, 0x1c }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x0c } );
        BINNING_MODE_REGS(  7, { IMX56X_GMRWT, 0x04 }, { IMX56X_GMTWT, 0x18 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x0c } );
        BINNING_MODE_REGS(  8, { IMX56X_GMRWT, 0x04 }, { IMX56X_GMTWT, 0x14 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x08 } );
        BINNING_MODE_REGS(  9, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x30 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x18 } );
        BINNING_MODE_REGS( 10, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x28 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x14 } );
        BINNING_MODE_REGS( 11, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x24 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x10 } );

        BINNING_START(ctrl->binnings[0], 0, 0)
                { IMX56X_HV_MODE, IMX56X_BINNING_MODE_DISABLE },
                { IMX56X_EAV_SELECT, IMX56X_EAV_SELECT_VALUE }
        BINNING_END(ctrl->binnings[0])

        BINNING_START(ctrl->binnings[1], 2, 2)
                { IMX56X_HV_MODE, IMX56X_BINNING_MODE_ENABLE },
                { IMX56X_EAV_SELECT, IMX56X_EAV_SELECT_VALUE }
        BINNING_END(ctrl->binnings[1])

        ctrl->max_binning_modes_used = 1;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_PREGIUS_S;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_PULSEWIDTH;
        ctrl->flags                    |= FLAG_TRIGGER_SELF;
        ctrl->flags                    |= FLAG_TRIGGER_SINGLE;
        ctrl->flags                    |= FLAG_USE_BINNING_INDEX;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX566 (Rev.03)
//  8.3 MegaPixel Pregius S

static void vc_init_ctrl_imx566(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX566")

        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x35b4, .m = 0x35b5 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x30d4, .m = 0x30d5, .h = 0x30d6, .u = 0x0000 };
        ctrl->csr.sen.hmax              = (vc_csr4) { .l = 0x30d8, .m = 0x30d9, .h = 0x0000, .u = 0x0000 };
        ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x3010 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        FRAME(0, 0, 2848, 2848)

        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     752,   24, 0xffffff, 0xb98,  255,   15,   1612278)
        MODE( 1, 2, FORMAT_RAW10, 0,     930,   20, 0xffffff, 0xb92, 1023,   60,   1991196)
        MODE( 2, 2, FORMAT_RAW12, 0,    1109,   18, 0xffffff, 0xb8c, 4095,  240,   2371194)
        MODE( 3, 4, FORMAT_RAW08, 0,     396,   40, 0xffffff, 0xbb0,  255,   15,    854172)
        MODE( 4, 4, FORMAT_RAW10, 0,     485,   34, 0xffffff, 0xba6, 1023,   60,   1043334)
        MODE( 5, 4, FORMAT_RAW12, 0,     574,   30, 0xffffff, 0xba0, 4095,  240,   1233144)

        //binning
        MODE( 6, 2, FORMAT_RAW08, 1,     396,   40, 0xffffff, 0x604,  255,   15,    430812)
        MODE( 7, 2, FORMAT_RAW10, 1,     485,   36, 0xffffff, 0x5fc, 1023,   60,    524826)
        MODE( 8, 2, FORMAT_RAW12, 1,     575,   32, 0xffffff, 0x5f4, 4095,  240,    620568)
        MODE( 9, 4, FORMAT_RAW08, 1,     218,   72, 0xffffff, 0x634,  255,   15,    242892)
        MODE(10, 4, FORMAT_RAW10, 1,     262,   60, 0xffffff, 0x620, 1023,   60,    288846)
        MODE(11, 4, FORMAT_RAW12, 1,     307,   52, 0xffffff, 0x614, 4095,  240,    336690)

        // special registers for binning mode
        BINNING_MODE_REGS(  6, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x24 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x10 } );
        BINNING_MODE_REGS(  7, { IMX56X_GMRWT, 0x04 }, { IMX56X_GMTWT, 0x20 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x10 } );
        BINNING_MODE_REGS(  8, { IMX56X_GMRWT, 0x04 }, { IMX56X_GMTWT, 0x1c }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x0c } );
        BINNING_MODE_REGS(  9, { IMX56X_GMRWT, 0x0c }, { IMX56X_GMTWT, 0x44 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x1c } );
        BINNING_MODE_REGS( 10, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x38 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x18 } );
        BINNING_MODE_REGS( 11, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x30 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x14 } );

        BINNING_START(ctrl->binnings[0], 0, 0)
                { IMX56X_HV_MODE, IMX56X_BINNING_MODE_DISABLE },
                { IMX56X_EAV_SELECT, IMX56X_EAV_SELECT_VALUE }
        BINNING_END(ctrl->binnings[0])

        BINNING_START(ctrl->binnings[1], 2, 2)
                { IMX56X_HV_MODE, IMX56X_BINNING_MODE_ENABLE },
                { IMX56X_EAV_SELECT, IMX56X_EAV_SELECT_VALUE }
        BINNING_END(ctrl->binnings[1])

        ctrl->max_binning_modes_used = 1;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_PREGIUS_S;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL | FLAG_TRIGGER_PULSEWIDTH |
                                          FLAG_TRIGGER_SELF | FLAG_TRIGGER_SINGLE;
        ctrl->flags                    |= FLAG_USE_BINNING_INDEX;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX567 (Rev.03)
//  5.1 MegaPixel Pregius S

static void vc_init_ctrl_imx567(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX567")

        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x35b4, .m = 0x35b5 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x30d4, .m = 0x30d5, .h = 0x30d6, .u = 0x0000 };
        ctrl->csr.sen.hmax              = (vc_csr4) { .l = 0x30d8, .m = 0x30d9, .h = 0x0000, .u = 0x0000 };
        ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x3010 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        FRAME(0, 0, 2464, 2064)

        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     656,   26, 0xffffff, 0x88a,  255,   15,   1058562)
        MODE( 1, 2, FORMAT_RAW10, 0,     810,   22, 0xffffff, 0x884, 1023,   60,   1273590)
        MODE( 2, 2, FORMAT_RAW12, 0,     965,   20, 0xffffff, 0x880, 4095,  240,   1514484)
        MODE( 3, 4, FORMAT_RAW08, 0,     348,   46, 0xffffff, 0x8a8,  255,   15,    553716)
        MODE( 4, 4, FORMAT_RAW10, 0,     425,   38, 0xffffff, 0x89e, 1023,   60,    673812)
        MODE( 5, 4, FORMAT_RAW12, 0,     502,   34, 0xffffff, 0x896, 4095,  240,    793692)

        //binning
        MODE( 6, 2, FORMAT_RAW08, 1,     348,   48, 0xffffff, 0x488,  255,   15,    285444)
        MODE( 7, 2, FORMAT_RAW10, 1,     425,   40, 0xffffff, 0x47c, 1023,   60,    346140)
        MODE( 8, 2, FORMAT_RAW12, 1,     503,   36, 0xffffff, 0x474, 4095,  240,    406782)
        MODE( 9, 4, FORMAT_RAW08, 1,     194,   80, 0xffffff, 0x4b8,  255,   15,    164214)
        MODE(10, 4, FORMAT_RAW10, 1,     232,   68, 0xffffff, 0x4a8, 1023,   60,    194346)
        MODE(11, 4, FORMAT_RAW12, 1,     271,   60, 0xffffff, 0x498, 4095,  240,    224640)

        // special registers for binning mode
        BINNING_MODE_REGS(  6, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x2c }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x14 } );
        BINNING_MODE_REGS(  7, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x24 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x10 } );
        BINNING_MODE_REGS(  8, { IMX56X_GMRWT, 0x04 }, { IMX56X_GMTWT, 0x20 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x10 } );
        BINNING_MODE_REGS(  9, { IMX56X_GMRWT, 0x0c }, { IMX56X_GMTWT, 0x4c }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x20 } );
        BINNING_MODE_REGS( 10, { IMX56X_GMRWT, 0x0c }, { IMX56X_GMTWT, 0x40 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x1c } );
        BINNING_MODE_REGS( 11, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x38 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x18 } );

        BINNING_START(ctrl->binnings[0], 0, 0)
                { IMX56X_HV_MODE, IMX56X_BINNING_MODE_DISABLE },
                { IMX56X_EAV_SELECT, IMX56X_EAV_SELECT_VALUE }
        BINNING_END(ctrl->binnings[0])

        BINNING_START(ctrl->binnings[1], 2, 2)
                { IMX56X_HV_MODE, IMX56X_BINNING_MODE_ENABLE },
                { IMX56X_EAV_SELECT, IMX56X_EAV_SELECT_VALUE }
        BINNING_END(ctrl->binnings[1])

        ctrl->max_binning_modes_used = 1;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_PREGIUS_S;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL | FLAG_TRIGGER_PULSEWIDTH |
                                          FLAG_TRIGGER_SELF | FLAG_TRIGGER_SINGLE;
        ctrl->flags                    |= FLAG_USE_BINNING_INDEX;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX568 (Rev.04)
//  5.1 MegaPixel Pregius S

static void vc_init_ctrl_imx568(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX568")

        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x35b4, .m = 0x35b5 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x30d4, .m = 0x30d5, .h = 0x30d6, .u = 0x0000 };
        ctrl->csr.sen.hmax              = (vc_csr4) { .l = 0x30d8, .m = 0x30d9, .h = 0x0000, .u = 0x0000 };
        ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x3010 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        FRAME(0, 0, 2464, 2064)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     656,   26, 0xffffff, 0x88a,  255,   15,   1058562)
        MODE( 1, 2, FORMAT_RAW10, 0,     810,   22, 0xffffff, 0x884, 1023,   60,   1273590)
        MODE( 2, 2, FORMAT_RAW12, 0,     965,   20, 0xffffff, 0x880, 4095,  240,   1514484)
        MODE( 3, 4, FORMAT_RAW08, 0,     348,   46, 0xffffff, 0x8a8,  255,   15,    553716)
        MODE( 4, 4, FORMAT_RAW10, 0,     425,   38, 0xffffff, 0x89e, 1023,   60,    673812)
        MODE( 5, 4, FORMAT_RAW12, 0,     502,   34, 0xffffff, 0x896, 4095,  240,    793692)

        //binning
        MODE( 6, 2, FORMAT_RAW08, 1,     348,   48, 0xffffff, 0x488,  255,   15,    285444)
        MODE( 7, 2, FORMAT_RAW10, 1,     425,   40, 0xffffff, 0x47c, 1023,   60,    346140)
        MODE( 8, 2, FORMAT_RAW12, 1,     503,   36, 0xffffff, 0x474, 4095,  240,    406782)
        MODE( 9, 4, FORMAT_RAW08, 1,     194,   80, 0xffffff, 0x4b8,  255,   15,    164214)
        MODE(10, 4, FORMAT_RAW10, 1,     232,   68, 0xffffff, 0x4a8, 1023,   60,    194346)
        MODE(11, 4, FORMAT_RAW12, 1,     271,   60, 0xffffff, 0x498, 4095,  240,    224640)

        // special registers for binning mode
        BINNING_MODE_REGS(  6, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x2c }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x14 } );
        BINNING_MODE_REGS(  7, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x24 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x10 } );
        BINNING_MODE_REGS(  8, { IMX56X_GMRWT, 0x04 }, { IMX56X_GMTWT, 0x20 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x10 } );
        BINNING_MODE_REGS(  9, { IMX56X_GMRWT, 0x0c }, { IMX56X_GMTWT, 0x4c }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x20 } );
        BINNING_MODE_REGS( 10, { IMX56X_GMRWT, 0x0c }, { IMX56X_GMTWT, 0x40 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x1c } );
        BINNING_MODE_REGS( 11, { IMX56X_GMRWT, 0x08 }, { IMX56X_GMTWT, 0x38 }, { IMX56X_GAINDLY, 0x04 }, { IMX56X_GSDLY, 0x18 } );

        BINNING_START(ctrl->binnings[0], 0, 0)
                { IMX56X_HV_MODE, IMX56X_BINNING_MODE_DISABLE },
                { IMX56X_EAV_SELECT, IMX56X_EAV_SELECT_VALUE }
        BINNING_END(ctrl->binnings[0])

        BINNING_START(ctrl->binnings[1], 2, 2)
                { IMX56X_HV_MODE, IMX56X_BINNING_MODE_ENABLE },
                { IMX56X_EAV_SELECT, IMX56X_EAV_SELECT_VALUE }
        BINNING_END(ctrl->binnings[1])

        ctrl->max_binning_modes_used = 1;

        ctrl->flags                     = FLAG_EXPOSURE_SONY;
        ctrl->flags                    |= FLAG_PREGIUS_S;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL | FLAG_TRIGGER_PULSEWIDTH |
                                          FLAG_TRIGGER_SELF | FLAG_TRIGGER_SINGLE;
        ctrl->flags                    |= FLAG_USE_BINNING_INDEX;
}

// ------------------------------------------------------------------------------------------------
//  Settings for IMX900 (Rev.00)
//  3.2 MegaPixel Pregius S

static void vc_init_ctrl_imx900(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("IMX900")

        ctrl->gain                      = (vc_control) { .min =   0, .max =       480, .def =      0 };

        ctrl->csr.sen.blacklevel        = (vc_csr2) { .l = 0x35b4, .m = 0x35b5 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x30d4, .m = 0x30d5, .h = 0x30d6, .u = 0x0000 };
        ctrl->csr.sen.mode              = (vc_csr2) { .l = 0x3000, .m = 0x3010 };
        ctrl->csr.sen.mode_standby      = 0x01;
        ctrl->csr.sen.mode_operating    = 0x00;

        FRAME(0, 0, 2048, 1536)

        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     460,   68, 0xffffff, 1716,  255,   15,   1058562)
        MODE( 1, 2, FORMAT_RAW10, 0,     564,   56, 0xffffff, 1697, 1023,   60,   1273590)
        MODE( 2, 2, FORMAT_RAW12, 0,     667,   46, 0xffffff, 1681, 4095,  240,   1514484)
        MODE( 3, 4, FORMAT_RAW08, 0,     338,   92, 0xffffff, 1755,  255,   15,    553716)
        MODE( 4, 4, FORMAT_RAW10, 0,     364,   85, 0xffffff, 1743, 1023,   60,    673812)
        MODE( 5, 4, FORMAT_RAW12, 0,     610,   51, 0xffffff, 1689, 4095,  240,    793692)

        ctrl->flags                     = FLAG_EXPOSURE_SONY;

        ctrl->flags                    |= FLAG_PREGIUS_S;
        ctrl->flags                    |= FLAG_INCREASE_FRAME_RATE;
        ctrl->flags                    |= FLAG_IO_ENABLED;
        ctrl->flags                    |= FLAG_TRIGGER_EXTERNAL | FLAG_TRIGGER_PULSEWIDTH |
                                          FLAG_TRIGGER_SELF | FLAG_TRIGGER_SINGLE;
}

// ------------------------------------------------------------------------------------------------
//  Settings for OV7251 (Rev.01)
//  0.3 MegaPixel OmniPixel3-GS
//
//  TODO: 
//  - No flash out

static void vc_init_ctrl_ov7251(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("OV7251")

        ctrl->exposure                  = (vc_control) { .min =   1, .max =   1000000, .def =  10000 };
        ctrl->gain                      = (vc_control) { .min =   0, .max =      1023, .def =      0 };

        ctrl->csr.sen.h_end             = (vc_csr2) { .l = 0x0000, .m = 0x0000 };
        ctrl->csr.sen.v_end             = (vc_csr2) { .l = 0x0000, .m = 0x0000 };
        ctrl->csr.sen.flash_duration    = (vc_csr4) { .l = 0x3b8f, .m = 0x3b8e, .h = 0x3b8d, .u = 0x3b8c };
        ctrl->csr.sen.flash_offset      = (vc_csr4) { .l = 0x3b8b, .m = 0x3b8a, .h = 0x3b89, .u = 0x3b88 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x380f, .m = 0x380e, .h = 0x0000, .u = 0x0000 };
        // NOTE: Modules rom table contains swapped address assigment.
        ctrl->csr.sen.gain              = (vc_csr2) { .l = 0x350b, .m = 0x350a };
        
        FRAME(0, 0, 640, 480)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 1, FORMAT_RAW08, 0,     772,    0,   0xffff,   598,    0,    0,         0)
        MODE( 1, 1, FORMAT_RAW10, 0,     772,    0,   0xffff,   598,    0,    0,         0)

        ctrl->flash_factor              = 1758241 >> 4; // (1000 << 4)/9100 >> 4
        ctrl->flash_toffset             = 4;

        ctrl->flags                     = FLAG_EXPOSURE_OMNIVISION;
        ctrl->flags                    |= FLAG_IO_ENABLED;
}

// ------------------------------------------------------------------------------------------------
//  Settings for OV9281 (Rev.03)
//  1.02 MegaPixel OmniPixel3-GS

static void vc_init_ctrl_ov9281(struct vc_ctrl *ctrl, struct vc_desc* desc)
{
        INIT_MESSAGE("OV9281")
        
        ctrl->exposure                  = (vc_control) { .min = 146, .max =    595000, .def =  10000 };
        ctrl->gain                      = (vc_control) { .min =  16, .max =       255, .def =     16 };

        ctrl->csr.sen.h_end             = (vc_csr2) { .l = 0x0000, .m = 0x0000 };
        ctrl->csr.sen.v_end             = (vc_csr2) { .l = 0x0000, .m = 0x0000 };
        ctrl->csr.sen.flash_duration	= (vc_csr4) { .l = 0x3928, .m = 0x3927, .h = 0x3926, .u = 0x3925 };
        ctrl->csr.sen.flash_offset      = (vc_csr4) { .l = 0x3924, .m = 0x3923, .h = 0x3922, .u = 0x0000 };
        ctrl->csr.sen.vmax              = (vc_csr4) { .l = 0x380f, .m = 0x380e, .h = 0x0000, .u = 0x0000 };
        // NOTE: Modules rom table contains swapped address assigment.
        ctrl->csr.sen.gain              = (vc_csr2) { .l = 0x3509, .m = 0x0000 };
        
        FRAME(0, 0, 1280, 800)
        //all read out         binning  hmax  vmax      vmax   vmax  blkl  blkl  retrigger
        //                      mode           min       max    def   max   def
        MODE( 0, 2, FORMAT_RAW08, 0,     227,   16,   0xffff,   910,    0,    0,         0)
        MODE( 1, 2, FORMAT_RAW10, 0,     227,   16,   0xffff,   910,    0,    0,         0)

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
        case MOD_ID_IMX566: vc_init_ctrl_imx566(ctrl, desc); break;
        case MOD_ID_IMX567: vc_init_ctrl_imx567(ctrl, desc); break;
        case MOD_ID_IMX568: vc_init_ctrl_imx568(ctrl, desc); break;
        case MOD_ID_IMX900: vc_init_ctrl_imx900(ctrl, desc); break;
        case MOD_ID_OV7251: vc_init_ctrl_ov7251(ctrl, desc); break;
        case MOD_ID_OV9281: vc_init_ctrl_ov9281(ctrl, desc); break;
        default:
                vc_err(dev, "%s(): Detected module not supported!\n", __FUNCTION__);
                return 1;
        }

        return 0;
}