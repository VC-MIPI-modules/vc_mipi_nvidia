/*
 * imx290.c - imx290 sensor driver
 * Based on the imx219 and ov5693 drivers
 *
 * Copyright (c) 2015-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define VC_CODE             1       /* CCC - imx290.c - enable code for VC MIPI camera  */
#define IMX290_ENB_MUTEX    0       /* CCC - imx290c - mutex lock/unlock */
#define L4T_VERS            322 // 3231     /* CCC - imx290c - L4T version: 32.2, 32.3.1 */

#define STOP_STREAMING_SENSOR_RESET   1     /* CCC - reset sensor before streaming stop */

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/debugfs.h>

#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

//#include <media/tegra_v4l2_camera.h>
#include <media/tegra-v4l2-camera.h>
#include <media/tegracam_core.h>
//#include <media/camera_common.h>

//#include <media/imx290.h>

#include "../platform/tegra/camera/camera_gpio.h"
//#include "imx290.h"
//#include "imx290_mode_tbls.h"

//#define CREATE_TRACE_POINTS
//#include <trace/events/ov5693.h>



#define IMX290_TABLE_WAIT_MS    0
#define IMX290_TABLE_END        1




/* In dB*10 */
#define IMX290_DIGITAL_GAIN_MIN        0
#define IMX290_DIGITAL_GAIN_MAX        240
#define IMX290_DIGITAL_GAIN_DEFAULT    0

/* In usec */
#define IMX290_DIGITAL_EXPOSURE_MIN     29          // microseconds (us)
#define IMX290_DIGITAL_EXPOSURE_MAX     7767184
#define IMX290_DIGITAL_EXPOSURE_DEFAULT 10000

/* In dB*10 */
//#define IMX290_ANALOG_GAIN_MIN          0
//#define IMX290_ANALOG_GAIN_MAX          978
//#define IMX290_ANALOG_GAIN_DEFAULT      0

// PK: just for test, not supported yet
//#define IMX290_MIN_FRAME_LENGTH     100
//#define IMX290_MAX_FRAME_LENGTH     800000
#define IMX290_FRAME_RATE_DEFAULT   30000000   // 30 fps (*1000000)


/* VC Sensor Mode - default 10-Bit Streaming */
static int sensor_mode = 0;    /* VC sensor mode: 0=10bit_stream 1=10bit_4lanes */
//module_param(sensor_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//MODULE_PARM_DESC(sensor_mode, "VC Sensor Mode: 0=10bit_stream 1=10bit_4lanes");

static int flash_output  = 0;       // flash output enable

//enum imx_model {
//    IMX290_MODEL_MONOCHROME,
//    IMX290_MODEL_COLOR,
//    IMX297_MODEL_MONOCHROME,
//    IMX297_MODEL_COLOR,
//};


enum {
    IMX290_MODE_1920X1080 = 0,
    IMX290_MODE_START_STREAM,
    IMX290_MODE_STOP_STREAM,
};

#define imx290_reg struct reg_8

static const imx290_reg imx290_start[] = {
    {0x3000, 0x00},     /* mode select streaming on */
    {0x3002, 0x00},     /* mode select streaming on */
    {IMX290_TABLE_END, 0x00}
};

static const imx290_reg imx290_stop[] = {
    {0x3002, 0x01},     /* mode select streaming off */
    {0x3000, 0x01},     /* mode select streaming off */
    {IMX290_TABLE_END, 0x00}
};


//---------------------------- 1920 x 1080  mode -------------------------------
// 1. 10-bit: Jetson Nano requires sensor width, multiple of 32.
// 2. 8-bit: Jetson Nano requires sensor width, multiple of 64.
// Actual sensor resolution : 1948 x 1097

#define X0 0
#define Y0 0
//#define SEN_DX 1024   // 1920 // 1920, 1408 (k*64)
//#define SEN_DY 512    // 1080
#define SEN_DX 1920     // 1920 // 1920, 1408 (k*64)
#define SEN_DY 1080     // 1097 // 1080
//#define SEN_DX 1948
//#define SEN_DY 1097
#define DX_OFFS 0       // 16   // 0
#define HMAX    2200

// WINPH (X0) H,L  = 0x3041,0x3040
// WINPV (Y0) H,L  = 0x303D,0x303C
// WINWH (DX) H,L  = 0x3043,0x3042
// WINWV (DY) H,L  = 0x303F,0x303E
//
// WINMODE[2:0]    = 0x3007[6:4] : window mode: 0=Full HD 1080p, 1=HD720P, 4=window cropping from full HD 1080p
// FRSEL[1:0]      = 0x3009[1:0] : frame rate select: 1=60fps, 2=30fps
// X_OUT_SIZE[12:0] H,L = 0x3473,0x3472 = Horizontal (H) direction effective pixel width setting.
// Y_OUT_SIZE[12:0] H,L = 0x3419,0x3418 = Vertical direction effective pixels
// HMAX[15:0] H,L       = 0x301D, 0x301C = Horizontal span settings, 1 lane : 0x898 (2200)
// STANDBY[0]           = 0x3000 : Standby register: 0=Operating, 1=Standby

/* MCLK:25MHz  1920x1080  60fps   MIPI LANE1 */
static const imx290_reg imx290_mode_1920X1080[] = {
#if 1   // [[[
    { 0x3000, 0x1 },            // STANDBY[0]  = 0x3000 : Standby register: 0=Operating, 1=Standby
    { IMX290_TABLE_WAIT_MS, 5 },

//    { 0x3007, 0x40 },               // 0x3007[6:4] : window mode: 0=Full HD 1080p (1920x1080), 1=HD720P, 4=window cropping from full HD 1080p
//    { 0x3007, 0x10 },               // 0x3007[6:4] : window mode: 1=HD720P (1280x720)
//    { 0x3007, 0x0 },                // 0x3007[6:4] : window mode: 0=0=Full HD 1080p (1920x1080)

    { 0x3473, SEN_DX>>8 }, { 0x3472, SEN_DX & 0xFF },   // X_OUT_SIZE[12:0] H,L = 0x3473,0x3472 = Horizontal (H) direction effective pixel width setting.
    { 0x3419, SEN_DY>>8 }, { 0x3418, SEN_DY & 0xFF },   // Y_OUT_SIZE[12:0] H,L = 0x3419,0x3418 = Vertical direction effective pixels

#if 0  //
    { 0x3041, X0>>8 },               { 0x3040, X0 & 0xFF },                 // horizontal start H,L   0x3041,0x3040 = X0
    { 0x303D, Y0>>8 },               { 0x303C, Y0 & 0xFF },                 // vertical start H,L     0x303D,0x303C = Y0
    { 0x3043, (SEN_DX+DX_OFFS)>>8 }, { 0x3042, (SEN_DX+DX_OFFS) & 0xFF },   // hor. output width H,L  0x3043,0x3042 = DX
    { 0x303F, SEN_DY>>8 },           { 0x303E, SEN_DY & 0xFF },             // ver. output height H,L 0x303F,0x303E = DY
#endif
//    { 0x301D, HMAX>>8 },             { 0x301C, HMAX & 0xFF },               // HMAX[15:0] H,L = 0x301D, 0x301C = Horizontal span settings, 1 lane : 0x898 (2200)

    { 0x3000, 0x0 },                // STANDBY[0]  = 0x3000 : Standby register: 0=Operating, 1=Standby
    { IMX290_TABLE_WAIT_MS, 5 },

#endif  // ]]]
    { IMX290_TABLE_END, 0x00 }
};
#undef  X0
#undef  Y0
//#undef  DX
//#undef  DY

////---------------------------- 720 x 540 mode -------------------------------
//// 1. 10-bit: Jetson Nano requires sensor width, multiple of 32.
//// 2. 8-bit: Jetson Nano requires sensor width, multiple of 64.
//#define X0 0
//#define Y0 0
//#define DX 704   // multiple of 32 !
//#define DY 540
///* MCLK:25MHz  720x540   120fps   MIPI LANE1 */
//static const imx290_reg imx297_mode_720X540[] = {
//    { 0x3311, X0>>8 }, { 0x3310, X0 & 0xFF },   // horizontal start H,L   0x3311, 0x3310 = X0
//    { 0x3313, Y0>>8 }, { 0x3312, Y0 & 0xFF },   // vertical start H,L     0x3313, 0x3312 = Y0
//    { 0x3315, DX>>8 }, { 0x3314, DX & 0xFF },   // hor. output width H,L  0x3315, 0x3314 = DX
//    { 0x3317, DY>>8 }, { 0x3316, DY & 0xFF },   // ver. output height H,L 0x3317, 0x3316 = DY
//    {IMX290_TABLE_END, 0x00}
//};
//#undef  X0
//#undef  Y0
//#undef  DX
//#undef  DY

static const imx290_reg *imx290_mode_table[] = {
    [IMX290_MODE_1920X1080]    = imx290_mode_1920X1080,
    [IMX290_MODE_START_STREAM] = imx290_start,
    [IMX290_MODE_STOP_STREAM]  = imx290_stop,
};

static const int imx290_60fps[] = {
    60,
};
static const int imx290_120fps[] = {
    120,
};

static const struct camera_common_frmfmt imx290_frmfmt[] = {
    { { SEN_DX, SEN_DY }, imx290_60fps, ARRAY_SIZE(imx290_60fps), 0,
      IMX290_MODE_1920X1080 },
    { { SEN_DX, SEN_DY }, imx290_120fps, ARRAY_SIZE(imx290_120fps), 0,
      IMX290_MODE_1920X1080 },
};

struct vc_rom_table {
    char magic[12];
    char manuf[32];
    u16 manuf_id;
    char sen_manuf[8];
    char sen_type[16];
    u16 mod_id;
    u16 mod_rev;
    char regs[56];
    u16 nr_modes;
    u16 bytes_per_mode;
    char mode1[16];
    char mode2[16];
};

struct imx290 {
    struct i2c_client          *i2c_client;
    struct v4l2_subdev         *subdev;
    u16                         fine_integ_time;
    u32                         frame_length;
    u32                         frame_rate;
    u32                         digital_gain;
//                                u32 analog_gain;
    u32                         exposure_time;

    struct camera_common_i2c    i2c_dev;
    struct camera_common_data  *s_data;
    struct tegracam_device     *tc_dev;

//    struct mutex        streaming_lock;
    bool                streaming;

//    s32                 group_hold_prev;
//    bool                group_hold_en;

#if VC_CODE     // [[[ - new VC code
    struct i2c_client   *rom;
    struct vc_rom_table rom_table;
    struct mutex mutex;
    int    cam_mode;        // camera mode:
                            //   IMX290_MODE_1920X1080 (default)
                            //   IMX297_MODE_720X540
//    enum imx_model model;   // camera model

    int sensor_ext_trig;    // ext. trigger flag: 0=no, 1=yes
    int flash_output;       // flash output enable
    int sen_clk;            // sen_clk default=54Mhz imx183=72Mhz
    int sensor_mode;        // sensor mode
    int num_lanes;          // # of data lanes: 1, 2, 4
#endif  // ]]]

};

static const struct regmap_config imx290_regmap_config = {
    .reg_bits = 16,
    .val_bits = 8,
    .cache_type = REGCACHE_RBTREE,
    .use_single_rw = true,
};



// camera_common.h
// ---------------
//struct tegracam_ctrl_ops {
//    u32 numctrls;
//    u32 string_ctrl_size[TEGRA_CAM_MAX_STRING_CONTROLS];
//    const u32 *ctrl_cid_list;
//    bool is_blob_supported;
//    int (*set_gain)(struct tegracam_device *tc_dev, s64 val);
//    int (*set_exposure)(struct tegracam_device *tc_dev, s64 val);
//    int (*set_exposure_short)(struct tegracam_device *tc_dev, s64 val);
//    int (*set_frame_rate)(struct tegracam_device *tc_dev, s64 val);
//    int (*set_group_hold)(struct tegracam_device *tc_dev, bool val);
//    int (*fill_string_ctrl)(struct tegracam_device *tc_dev,
//                struct v4l2_ctrl *ctrl);
//    int (*set_gain_ex)(struct tegracam_device *tc_dev,
//            struct sensor_blob *blob, s64 val);
//    int (*set_exposure_ex)(struct tegracam_device *tc_dev,
//            struct sensor_blob *blob, s64 val);
//    int (*set_frame_rate_ex)(struct tegracam_device *tc_dev,
//            struct sensor_blob *blob, s64 val);
//    int (*set_group_hold_ex)(struct tegracam_device *tc_dev,
//            struct sensor_blob *blob, bool val);
//};
//
// tegracam_core.h
// ---------------
//struct tegracam_device {
//    struct camera_common_data   *s_data;
//    struct media_pad        pad;
//    u32                 version;
//    bool                is_streaming;
//    /* variables to be filled by the driver to register */
//    char                name[32];
//    struct i2c_client       *client;
//    struct device           *dev;
//    u32             numctrls;
//    const u32           *ctrl_cid_list;
//    const struct regmap_config  *dev_regmap_config;
//    struct camera_common_sensor_ops     *sensor_ops;
//    const struct v4l2_subdev_ops        *v4l2sd_ops;
//    const struct v4l2_subdev_internal_ops   *v4l2sd_internal_ops;
//    const struct media_entity_operations    *media_ops;
//    const struct tegracam_ctrl_ops      *tcctrl_ops;
//    void    *priv;
//};

//tegra-v4l2-camera.h
//-------------------
//struct unpackedU64 {
//    __u32 high;
//    __u32 low;
//};
//union __u64val {
//    struct unpackedU64 unpacked;
//    __u64 val;
//};
//
//struct sensor_signal_properties {
//    __u32 readout_orientation;
//    __u32 num_lanes;
//    __u32 mclk_freq;
//    union __u64val pixel_clock;
//    __u32 cil_settletime;
//    __u32 discontinuous_clk;
//    __u32 dpcm_enable;
//    __u32 tegra_sinterface;
//    __u32 phy_mode;
//    __u32 deskew_initial_enable;
//    __u32 deskew_periodic_enable;
//    union __u64val serdes_pixel_clock;
//    __u32 reserved[2];
//};
//
//struct sensor_image_properties {
//    __u32 width;
//    __u32 height;
//    __u32 line_length;
//    __u32 pixel_format;
//    __u32 embedded_metadata_height;
//    __u32 reserved[11];
//};
//
//struct sensor_dv_timings {
//    __u32 hfrontporch;
//    __u32 hsync;
//    __u32 hbackporch;
//    __u32 vfrontporch;
//    __u32 vsync;
//    __u32 vbackporch;
//    __u32 reserved[10];
//};
//
//struct sensor_control_properties {
//    __u32 gain_factor;
//    __u32 framerate_factor;
//    __u32 inherent_gain;
//    __u32 min_gain_val;
//    __u32 max_gain_val;
//    __u32 min_hdr_ratio;
//    __u32 max_hdr_ratio;
//    __u32 min_framerate;
//    __u32 max_framerate;
//    union __u64val min_exp_time;
//    union __u64val max_exp_time;
//    __u32 step_gain_val;
//    __u32 step_framerate;
//    __u32 exposure_factor;
//    union __u64val step_exp_time;
//    __u32 default_gain;
//    __u32 default_framerate;
//    union __u64val default_exp_time;
//    __u32 reserved[10];
//};
//
//struct sensor_mode_properties {
//    struct sensor_signal_properties signal_properties;
//    struct sensor_image_properties image_properties;
//    struct sensor_control_properties control_properties;
//    struct sensor_dv_timings dv_timings;
//};



#if VC_CODE     // [[[ - new VC code

/****** reg_write = Write reg = 06.2019 *********************************/
static int reg_write(struct i2c_client *client, const u16 addr, const u8 data)
{
    struct i2c_adapter *adap = client->adapter;
    struct i2c_msg msg;
    u8 tx[3];
    int ret;

    msg.addr = client->addr;
    msg.buf = tx;
    msg.len = 3;
    msg.flags = 0;
    tx[0] = addr >> 8;
    tx[1] = addr & 0xff;
    tx[2] = data;

    ret = i2c_transfer(adap, &msg, 1);
    mdelay(2);

    return ret == 1 ? 0 : -EIO;
}

/****** reg_read = Read reg = 06.2019 ***********************************/
static int reg_read(struct i2c_client *client, const u16 addr)
{
    u8 buf[2] = {addr >> 8, addr & 0xff};
    int ret;
    struct i2c_msg msgs[] = {
        {
            .addr  = client->addr,
            .flags = 0,
            .len   = 2,
            .buf   = buf,
        }, {
            .addr  = client->addr,
            .flags = I2C_M_RD,
            .len   = 1,
            .buf   = buf,
        },
    };

    ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
    if (ret < 0) {
        dev_warn(&client->dev, "Reading register %x from %x failed\n",
             addr, client->addr);
        return ret;
    }

    return buf[0];
}

///****** reg_write_table = Write reg table = 06.2019 *******************/
//static int reg_write_table(struct i2c_client *client,
//               const struct imx290_reg table[])
//{
//    const struct imx290_reg *reg;
//    int ret;
//
//    for (reg = table; reg->addr != IMX290_TABLE_END; reg++) {
//        ret = reg_write(client, reg->addr, reg->val);
//        if (ret < 0)
//            return ret;
//    }
//
//    return 0;
//}

#endif  // ]]] - VC_CODE

/****** imx290_read_reg = Read register = 09.2019 ***********************/
static int imx290_read_reg(struct camera_common_data *s_data, u16 addr, u8 *val)
{
    int err = 0;
    u32 reg_val = 0;

    err = regmap_read(s_data->regmap, addr, &reg_val);
    *val = reg_val & 0xff;

    return err;
}

/****** imx290_write_reg = Write register = 09.2019 *********************/
static int imx290_write_reg(struct camera_common_data *s_data, u16 addr, u8 val)
{
    int err = 0;

    err = regmap_write(s_data->regmap, addr, val);
    if (err)
        dev_err(s_data->dev, "%s: i2c write failed, 0x%x = %x",
            __func__, addr, val);

    return err;
}

/****** imx290_write_table = Write table = 09.2019 **********************/
static int imx290_write_table(struct imx290 *priv, const imx290_reg table[])
{
    return regmap_util_write_table_8(priv->s_data->regmap, table, NULL, 0,
        IMX290_TABLE_WAIT_MS, IMX290_TABLE_END);
}

/****** imx290_gpio_set = GPIO set = 09.2019 ****************************/
static void imx290_gpio_set(struct camera_common_data *s_data,
                unsigned int gpio, int val)
{
    struct camera_common_pdata *pdata = s_data->pdata;

    if (pdata && pdata->use_cam_gpio)
        cam_gpio_ctrl(s_data->dev, gpio, val, 1);
    else {
        if (gpio_cansleep(gpio))
            gpio_set_value_cansleep(gpio, val);
        else
            gpio_set_value(gpio, val);
    }
}

/****** imx290_set_gain = Set gain = 09.2019 ****************************/
static int imx290_set_gain(struct tegracam_device *tc_dev, s64 val)
{

#define TRACE_IMX290_SET_GAIN       1   /* DDD - imx290_set_gain() - trace */
//#define IMX290_SET_GAIN_GROUP_HOLD   0   /* CCC - imx290_set_gain() - enable group hold bef reg write */

//    struct camera_common_data *s_data = tc_dev->s_data;
    struct imx290 *priv = (struct imx290 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;

    imx290_reg regs[3];
    s64 gain = val;
    int err;


    if (gain < IMX290_DIGITAL_GAIN_MIN)
      gain = IMX290_DIGITAL_GAIN_MIN;
    else if (gain > IMX290_DIGITAL_GAIN_MAX)
      gain = IMX290_DIGITAL_GAIN_MAX;

//    gain = IMX290_DIGITAL_GAIN_DEFAULT; // VC: FIXED to 1.00 for now
    priv->digital_gain = gain;

#if TRACE_IMX290_SET_GAIN
    dev_err(dev, "%s: Set gain = %d\n", __func__, (int)gain);
#endif

//        ret = reg_write(client, 0x3205, (priv->digital_gain >> 8) & 0x01);
//        ret |= reg_write(client, 0x3204, priv->digital_gain & 0xff);
    regs[0].addr = 0x3014;
    regs[0].val = priv->digital_gain & 0xff;
    regs[1].addr = IMX290_TABLE_END;
    regs[1].val = 0;

//#if IMX290_SET_GAIN_GROUP_HOLD
//    if (!priv->group_hold_prev)
//        imx290_set_group_hold(tc_dev, 1);
//#endif

    err = imx290_write_table(priv, regs);
    if (err)
        goto fail;

// ???
//    /* If enabled, apply settings immediately */
//    reg = reg_read(client, 0x3000);
//    if ((reg & 0x1f) == 0x01)
//        imx290_s_stream(&priv->subdev, 1);

    return 0;

fail:
    dev_err(dev, "%s: error=%d\n", __func__, err);
    return err;
}

/****** imx290_set_exposure = Set exposure = 09.2019 ********************/
static int imx290_set_exposure(struct tegracam_device *tc_dev, s64 val)
{

#define TRACE_IMX290_SET_EXPOSURE   1   /* DDD - imx290_set_exposure() - trace */
#define DUMP_EXPOSURE_PARAMS        1   /* DDD - imx290_set_exposure() - dump DT exposure params */


//    struct camera_common_data *s_data = tc_dev->s_data;
    struct device *dev = tc_dev->dev;
    struct imx290 *priv = tc_dev->priv;
//    struct imx290 *priv = (struct imx290 *)tc_dev->priv;

//    u32 exposure_time;
    u32 exposure = 0;
    unsigned int lf = sensor_mode ? 2 : 1;
    int err = 0;

    imx290_reg regs[7];

#if IMX290_ENB_MUTEX
    mutex_lock(&priv->mutex);
#endif



//struct sensor_control_properties {
//    __u32 gain_factor;
//    __u32 framerate_factor;
//    __u32 inherent_gain;
//    __u32 min_gain_val;
//    __u32 max_gain_val;
//    __u32 min_hdr_ratio;
//    __u32 max_hdr_ratio;
//    __u32 min_framerate;
//    __u32 max_framerate;
//    union __u64val min_exp_time;
//    union __u64val max_exp_time;
//    __u32 step_gain_val;
//    __u32 step_framerate;
//    __u32 exposure_factor;
//    union __u64val step_exp_time;
//    __u32 default_gain;
//    __u32 default_framerate;
//    union __u64val default_exp_time;   // default_exp_time.val
//    __u32 reserved[10];
//};
//struct sensor_signal_properties {
//    __u32 readout_orientation;
//    __u32 num_lanes;
//    __u32 mclk_freq;
//    union __u64val pixel_clock;
//    __u32 cil_settletime;
//    __u32 discontinuous_clk;
//    __u32 dpcm_enable;
//    __u32 tegra_sinterface;
//    __u32 phy_mode;
//    __u32 deskew_initial_enable;
//    __u32 deskew_periodic_enable;
//    union __u64val serdes_pixel_clock;
//    __u32 reserved[2];
//};
//
//struct sensor_mode_properties {
//    struct sensor_signal_properties signal_properties;
//    struct sensor_image_properties image_properties;
//    struct sensor_control_properties control_properties;
//    struct sensor_dv_timings dv_timings;
//};

#if DUMP_EXPOSURE_PARAMS
{
    struct camera_common_data *s_data = tc_dev->s_data;
//    const struct sensor_mode_properties *mode =  &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];
    struct sensor_mode_properties *mode =  &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];


//    dev_err(dev, "%s: mode->control_properties:\n", __func__);
    dev_err(dev, "%s: min_exp_time,max_exp_time=%d,%d default_exp_time=%d\n", __func__,
         (int)mode->control_properties.min_exp_time.val,
         (int)mode->control_properties.max_exp_time.val,
         (int)mode->control_properties.default_exp_time.val);
}
#endif

    priv->exposure_time = (u32)val;

    if(priv->exposure_time < IMX290_DIGITAL_EXPOSURE_MIN) priv->exposure_time = IMX290_DIGITAL_EXPOSURE_MIN;
    if(priv->exposure_time > IMX290_DIGITAL_EXPOSURE_MAX) priv->exposure_time = IMX290_DIGITAL_EXPOSURE_MAX;

    if (priv->exposure_time < 38716)
    {
      // range 1..1123
      exposure = (1124 * 20000 -  (int)(priv->exposure_time) * 29 * 20 * lf) / 20000;
//      dev_info(dev, "SHS = %d \n",exposure);
#if TRACE_IMX290_SET_EXPOSURE
      dev_err(dev, "%s(): exposure_time=%d, SHS=0x%x (%d)\n", __func__, priv->exposure_time, exposure, exposure);
#endif

//            ret  = reg_write(client, 0x301A, 0x00);
//            ret |= reg_write(client, 0x3019, 0x04);
//            ret |= reg_write(client, 0x3018, 0x65);
//            ret |= reg_write(client, 0x3022, (exposure >> 16) & 0x01);
//            ret |= reg_write(client, 0x3021, (exposure >>  8) & 0xff);
//            ret |= reg_write(client, 0x3020,  exposure        & 0xff);
      regs[0].addr = 0x301A;
      regs[1].addr = 0x3019;
      regs[2].addr = 0x3018;
      regs[3].addr = 0x3022;
      regs[4].addr = 0x3021;
      regs[5].addr = 0x3020;

      regs[0].val = 0x00;
      regs[1].val = 0x04;
      regs[2].val = 0x65;
      regs[3].val = (exposure >> 16) & 0x01;
      regs[4].val = (exposure >>  8) & 0xff;
      regs[5].val =  exposure        & 0xff;

      regs[6].addr = IMX290_TABLE_END;
      regs[6].val = 0;

    }
    else
    {
        // range 1123..
      exposure = ( 1 * 20000 + (int)(priv->exposure_time) * 29 * 20 * lf ) / 20000;
//      dev_info(dev, "VMAX = %d \n",exposure);
#if TRACE_IMX290_SET_EXPOSURE
      dev_err(dev, "%s(): exposure_time=%d, VMAX=0x%x (%d)\n", __func__, priv->exposure_time, exposure, exposure);
#endif
//
//        ret  = reg_write(client, 0x3022, 0x00);
//        ret |= reg_write(client, 0x3021, 0x00);
//        ret |= reg_write(client, 0x3020, 0x01);
//        ret |= reg_write(client, 0x301A, (exposure >> 16) & 0x01);
//        ret |= reg_write(client, 0x3019, (exposure >>  8) & 0xff);
//        ret |= reg_write(client, 0x3018,  exposure        & 0xff);
      regs[0].addr = 0x3022;
      regs[1].addr = 0x3021;
      regs[2].addr = 0x3020;
      regs[3].addr = 0x301A;
      regs[4].addr = 0x3019;
      regs[5].addr = 0x3018;

      regs[0].val = 0x00;
      regs[1].val = 0x00;
      regs[2].val = 0x01;
      regs[3].val = (exposure >> 16) & 0x01;
      regs[4].val = (exposure >>  8) & 0xff;
      regs[5].val =  exposure        & 0xff;

      regs[6].addr = IMX290_TABLE_END;
      regs[6].val = 0;

    }

    err = imx290_write_table(priv, regs);
    if (err)
    {
      dev_err(dev, "%s: imx290_write_table() error=%d\n", __func__, err);
    }


// ???
//    /* If enabled, apply settings immediately */
//    reg = reg_read(client, 0x3000);
//    if ((reg & 0x1f) == 0x01)
//        imx290_s_stream(&priv->subdev, 1);

#if IMX290_ENB_MUTEX
    mutex_unlock(&priv->mutex);
#endif
    return err;
}

/****** imx290_set_frame_rate = Set frame rate = 09.2019 ****************/
static int imx290_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{

#define TRACE_IMX290_SET_FRAME_RATE     0   /* DDD - imx290_set_frame_rate() - trace */

//    struct camera_common_data *s_data = tc_dev->s_data;
//    struct device *dev = tc_dev->dev;
//    struct imx290 *priv = tc_dev->priv;
    int err = 0;

//    u32 frame_length;
//    u32 frame_rate = (int)val;
////    int i;
//
//    frame_length = (910 * 120) / frame_rate;
//
//#if TRACE_IMX290_SET_FRAME_RATE
//    dev_err(dev, "%s: frame_rate=%d: frame_length=%d\n", __func__, (int)val,frame_length);
//#endif
//
//    if(frame_length < IMX290_MIN_FRAME_LENGTH) frame_length = IMX290_MIN_FRAME_LENGTH;
//    if(frame_length > IMX290_MAX_FRAME_LENGTH) frame_length = IMX290_MAX_FRAME_LENGTH;
//
////      err = imx290_set_frame_length(tc_dev, frame_length);
////      if (err)
////         goto fail;
//
//    priv->frame_rate = frame_rate;
//    priv->frame_length = frame_length;
//    if(err)
//    {
//      dev_err(dev, "%s: error=%d\n", __func__, err);
//    }

    return err;

}

/****** imx290_set_group_hold = Set group hold = 04.2020 ****************/
static int imx290_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
    /* imx290 does not support group hold */
    return 0;
}

// CID list
static const u32 ctrl_cid_list[] = {
    TEGRA_CAMERA_CID_GAIN,
    TEGRA_CAMERA_CID_EXPOSURE,
    TEGRA_CAMERA_CID_FRAME_RATE,
    TEGRA_CAMERA_CID_SENSOR_MODE_ID,
//    TEGRA_CAMERA_CID_EXPOSURE_SHORT,
//    TEGRA_CAMERA_CID_GROUP_HOLD,
//    TEGRA_CAMERA_CID_HDR_EN,
//    TEGRA_CAMERA_CID_EEPROM_DATA,
//    TEGRA_CAMERA_CID_OTP_DATA,
//    TEGRA_CAMERA_CID_FUSE_ID,

};


static struct tegracam_ctrl_ops imx290_ctrl_ops = {
    .numctrls = ARRAY_SIZE(ctrl_cid_list),
    .ctrl_cid_list = ctrl_cid_list,
    .set_gain = imx290_set_gain,
    .set_exposure = imx290_set_exposure,
    .set_frame_rate = imx290_set_frame_rate,  // imx290_set_frame_length,
    .set_group_hold = imx290_set_group_hold,

};

/****** imx290_power_on = Power on = 09.2019 ****************************/
static int imx290_power_on(struct camera_common_data *s_data)
{
#define TRACE_IMX290_POWER_ON   0   /* DDD - imx290_power_on() - trace */

    int err = 0;
    struct camera_common_power_rail *pw = s_data->power;
    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = s_data->dev;


#if TRACE_IMX290_POWER_ON
    dev_err(dev, "%s: power on\n", __func__);
#endif

    if (pdata && pdata->power_on) {
        err = pdata->power_on(pw);
        if (err)
            dev_err(dev, "%s failed.\n", __func__);
        else
            pw->state = SWITCH_ON;
        return err;
    }

// IMX219
//    if (pw->reset_gpio) {
//        if (gpio_cansleep(pw->reset_gpio))
//            gpio_set_value_cansleep(pw->reset_gpio, 0);
//        else
//            gpio_set_value(pw->reset_gpio, 0);
//    }

    if (unlikely(!(pw->avdd || pw->iovdd || pw->dvdd)))
        goto skip_power_seqn;

    usleep_range(10, 20);

    if (pw->avdd) {
        err = regulator_enable(pw->avdd);
        if (err)
            goto imx290_avdd_fail;
    }

    if (pw->iovdd) {
        err = regulator_enable(pw->iovdd);
        if (err)
            goto imx290_iovdd_fail;
    }

// IMX
//    if (pw->dvdd) {
//        err = regulator_enable(pw->dvdd);
//        if (err)
//            goto imx290_dvdd_fail;
//    }

    usleep_range(10, 20);

skip_power_seqn:
    usleep_range(1, 2);
    if (gpio_is_valid(pw->pwdn_gpio))
        imx290_gpio_set(s_data, pw->pwdn_gpio, 1);

    /*
     * datasheet 2.9: reset requires ~2ms settling time
     * a power on reset is generated after core power becomes stable
     */
    usleep_range(2000, 2010);
//    usleep_range(23000, 23100);

//    msleep(20);

    if (gpio_is_valid(pw->reset_gpio))
        imx290_gpio_set(s_data, pw->reset_gpio, 1);

//    msleep(20);

// IMX
//    if (pw->reset_gpio) {
//        if (gpio_cansleep(pw->reset_gpio))
//            gpio_set_value_cansleep(pw->reset_gpio, 1);
//        else
//            gpio_set_value(pw->reset_gpio, 1);
//    }
//
//    /* Need to wait for t4 + t5 + t9 time as per the data sheet */
//    /* t4 - 200us, t5 - 21.2ms, t9 - 1.2ms */
//    usleep_range(23000, 23100);

    pw->state = SWITCH_ON;

    return 0;

//imx290_dvdd_fail:
//    regulator_disable(pw->iovdd);

imx290_iovdd_fail:
    regulator_disable(pw->avdd);

imx290_avdd_fail:
    dev_err(dev, "%s failed.\n", __func__);

    return -ENODEV;
}

/****** imx290_power_off = Power off = 09.2019 **************************/
static int imx290_power_off(struct camera_common_data *s_data)
{

#define TRACE_IMX290_POWER_OFF  0   /* DDD - imx290_power_off() - trace */

    int err = 0;
    struct camera_common_power_rail *pw = s_data->power;
    struct device *dev = s_data->dev;
    struct camera_common_pdata *pdata = s_data->pdata;

#if TRACE_IMX290_POWER_OFF
    dev_err(dev, "%s: power off\n", __func__);
#endif

    if (pdata && pdata->power_off) {
        err = pdata->power_off(pw);
        if (!err) {
            goto power_off_done;
        } else {
            dev_err(dev, "%s failed.\n", __func__);
            return err;
        }
    }

    /* sleeps calls in the sequence below are for internal device
     * signal propagation as specified by sensor vendor
     */
    usleep_range(21, 25);
    if (gpio_is_valid(pw->pwdn_gpio))
        imx290_gpio_set(s_data, pw->pwdn_gpio, 0);
    usleep_range(1, 2);
    if (gpio_is_valid(pw->reset_gpio))
        imx290_gpio_set(s_data, pw->reset_gpio, 0);

    /* datasheet 2.9: reset requires ~2ms settling time*/
    usleep_range(2000, 2010);

    if (pw->iovdd)
        regulator_disable(pw->iovdd);
    if (pw->avdd)
        regulator_disable(pw->avdd);

power_off_done:
    pw->state = SWITCH_OFF;
    return 0;

}

/****** imx290_power_put = Power put = 09.2019 **************************/
static int imx290_power_put(struct tegracam_device *tc_dev)
{
    struct camera_common_data *s_data = tc_dev->s_data;
    struct camera_common_power_rail *pw = s_data->power;
//    struct camera_common_pdata *pdata = s_data->pdata;
//    struct device *dev = tc_dev->dev;

    if (unlikely(!pw))
        return -EFAULT;


// IMX
// R32.2
#if L4T_VERS == 322
    if (likely(pw->dvdd))
        regulator_disable(pw->dvdd);

    if (likely(pw->avdd))
        regulator_put(pw->avdd);

    if (likely(pw->iovdd))
        regulator_put(pw->iovdd);

// R32.3.1
#else
    if (likely(pw->dvdd))
        devm_regulator_put(pw->dvdd);

    if (likely(pw->avdd))
        devm_regulator_put(pw->avdd);

    if (likely(pw->iovdd))
        devm_regulator_put(pw->iovdd);
#endif


    pw->dvdd = NULL;
    pw->avdd = NULL;
    pw->iovdd = NULL;

    if (likely(pw->reset_gpio))
        gpio_free(pw->reset_gpio);

    return 0;
}

/****** imx290_power_get = Power get = 09.2019 **************************/
static int imx290_power_get(struct tegracam_device *tc_dev)
{

#define TRACE_IMX290_POWER_GET  0   /* DDD - imx290_power_get() - trace */

    struct camera_common_data *s_data = tc_dev->s_data;
    struct camera_common_power_rail *pw = s_data->power;
    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = tc_dev->dev;
    struct clk *parent;
//    const char *mclk_name;
//    const char *parentclk_name;
    int err = 0;
//    int ret = 0;

#if TRACE_IMX290_POWER_GET
    dev_info(dev, "%s(): ...\n", __func__);
#endif

    if (!pdata) {
        dev_err(dev, "pdata missing\n");
        return -EFAULT;
    }


// IMX219
    /* Sensor MCLK (aka. INCK) */
    if (pdata->mclk_name) {
        pw->mclk = devm_clk_get(dev, pdata->mclk_name);
        if (IS_ERR(pw->mclk)) {
            dev_err(dev, "unable to get clock %s\n",
                pdata->mclk_name);
            return PTR_ERR(pw->mclk);
        }

        if (pdata->parentclk_name) {
            parent = devm_clk_get(dev, pdata->parentclk_name);
            if (IS_ERR(parent)) {
                dev_err(dev, "unable to get parent clock %s",
                    pdata->parentclk_name);
            } else
                clk_set_parent(pw->mclk, parent);
        }
    }

    /* analog 2.8v */
    if (pdata->regulators.avdd)
    {
        dev_info(dev, "%s: Get regulator avdd\n", __func__);
        err |= camera_common_regulator_get(dev,
                &pw->avdd, pdata->regulators.avdd);
    }
    /* IO 1.8v */
    if (pdata->regulators.iovdd)
    {
        dev_info(dev, "%s: Get regulator iovdd\n", __func__);
        err |= camera_common_regulator_get(dev,
                &pw->iovdd, pdata->regulators.iovdd);
    }
    /* dig 1.2v */
    if (pdata->regulators.dvdd)
    {
        dev_info(dev, "%s: Get regulator dvdd\n", __func__);
        err |= camera_common_regulator_get(dev,
                &pw->dvdd, pdata->regulators.dvdd);
    }
    if (err) {
        dev_err(dev, "%s: unable to get regulator(s)\n", __func__);
        goto done;
    }

    /* Reset or ENABLE GPIO */
    pw->reset_gpio = pdata->reset_gpio;
    err = gpio_request(pw->reset_gpio, "cam_reset_gpio");
    if (err < 0) {
        dev_err(dev, "%s: unable to request reset_gpio (%d)\n",
            __func__, err);
        goto done;
    }

done:
    pw->state = SWITCH_OFF;
#if TRACE_IMX290_POWER_GET
    dev_info(dev, "%s(): err=%d\n", __func__, err);
#endif
    return err;
}

/****** read_property_u32 = Read U32 property = 06.2020 *****************/
static int read_property_u32(
    struct device_node *node, const char *name, int radix, u32 *value)
{
    const char *str;
    int err = 0;
    if(radix != 16) radix = 10;

    err = of_property_read_string(node, name, &str);
    if (err)
        return -ENODATA;

//    err = kstrtou32(str, 10, value);
    err = kstrtou32(str, radix, value);
    if (err)
        return -EFAULT;

    return 0;
}

/****** imx290_parse_dt = Parse DT = 09.2019 ****************************/
static struct camera_common_pdata *imx290_parse_dt(
    struct tegracam_device *tc_dev)
{

#define TRACE_IMX290_PARSE_DT   1   /* DDD - imx290_parse_dt() - trace */

// IMX219 : no parse clocks
//#define PARSE_CLOCKS        0   /* CCC - imx290_parse_dt() - parse clocks */
//#define PARSE_GPIOS         0   /* CCC - imx290_parse_dt() - parse GPIOss */

    struct device *dev = tc_dev->dev;
    struct device_node *node = dev->of_node;
    struct camera_common_pdata *board_priv_pdata;
//    const struct of_device_id *match;
//    struct imx290 *priv = (struct imx290 *)tegracam_get_privdata(tc_dev);

    int gpio;
    int err = 0;
    struct camera_common_pdata *ret = NULL;
    int val = 0;

#if TRACE_IMX290_PARSE_DT
    dev_info(dev, "%s(): ...\n", __func__);
#endif

    if (!node)
        return NULL;

//    match = of_match_device(imx290_of_match, dev);
//    if (!match) {
//        dev_err(dev, "Failed to find matching dt id\n");
//        return NULL;
//    }

    board_priv_pdata = devm_kzalloc(dev,
        sizeof(*board_priv_pdata), GFP_KERNEL);
    if (!board_priv_pdata)
        return NULL;

// do we need reset-gpios ?
    gpio = of_get_named_gpio(node, "reset-gpios", 0);
    if (gpio < 0) {
        if (gpio == -EPROBE_DEFER)
            ret = ERR_PTR(-EPROBE_DEFER);
        dev_err(dev, "reset-gpios not found\n");
//        goto error;
    }
    else
    {
      board_priv_pdata->reset_gpio = (unsigned int)gpio;
    }

// IMX219
    err = of_property_read_string(node, "mclk", &board_priv_pdata->mclk_name);
    if (err)
        dev_err(dev, "%s(): mclk name not present, assume sensor driven externally\n", __func__);

    err = of_property_read_string(node, "avdd-reg",
        &board_priv_pdata->regulators.avdd);
    err |= of_property_read_string(node, "iovdd-reg",
        &board_priv_pdata->regulators.iovdd);
    err |= of_property_read_string(node, "dvdd-reg",
        &board_priv_pdata->regulators.dvdd);
    if (err)
        dev_err(dev, "%s(): avdd, iovdd and/or dvdd reglrs. not present, assume sensor powered independently\n", __func__);

    board_priv_pdata->has_eeprom = of_property_read_bool(node, "has-eeprom");
    board_priv_pdata->v_flip     = of_property_read_bool(node, "vertical-flip");
    board_priv_pdata->h_mirror   = of_property_read_bool(node, "horizontal-mirror");

//    err = of_property_read_string(node, "num-lanes",  &priv->num_lanes);


//............. Read flash output enable from DT
    err = read_property_u32(node, "flash-output", 10, &val);
    if (err)
    {
        dev_err(dev, "%s(): flash-output not present in DT, def=%d\n", __func__, flash_output);
    }
    else
    {
      flash_output = val;
      dev_err(dev, "%s(): flash-output=%d\n", __func__, flash_output);
    }


#if TRACE_IMX290_PARSE_DT
    dev_err(dev, "%s(): OK\n", __func__);
#endif

    return board_priv_pdata;

//error:
    devm_kfree(dev, board_priv_pdata);
#if TRACE_IMX290_PARSE_DT
    dev_err(dev, "%s(): ERROR\n", __func__);
#endif
    return ret;
}

/****** vc_mipi_reset = Reset VC MIPI sensor = 09.2019 ******************/
static int vc_mipi_reset (
    struct tegracam_device *tc_dev, /* [i/o] tegra camera device        */
    int  sen_mode )                 /* [in] VC sensor mode              */
{

#define TRACE_VC_MIPI_RESET     1   /* DDD - vc_mipi_reset() - trace */

    struct imx290 *priv = (struct imx290 *)tegracam_get_privdata(tc_dev);
    struct device *dev = tc_dev->dev;
    int err = 0;

    if(priv->rom)
    {
        static int i=1;
        int addr,reg,data;

        addr = 0x0100; // reset
            data =      2; // powerdown sensor
            reg = reg_write(priv->rom, addr, data);

        addr = 0x0102; // mode
            data = sen_mode; // default 10-bit streaming
            reg = reg_write(priv->rom, addr, data);

        addr = 0x0100; // reset
            data =      0; // powerup sensor
            reg = reg_write(priv->rom, addr, data);

        while(1)
        {
            mdelay(100); // wait 100ms

            addr = 0x0101; // status
            reg = reg_read(priv->rom, addr);

            if(reg & 0x80)
                    break;

            if(reg & 0x01)
            {
               dev_err(dev, "%s(): !!! ERROR !!! setting VC Sensor MODE=%d STATUS=0x%02x i=%d\n", __func__, sen_mode,reg,i);
               err = -EIO;
            }

            if(i++ >  4)
                break;
        }

#if TRACE_VC_MIPI_RESET
//        dev_info(dev, "%s(): VC Sensor MODE=%d PowerOn STATUS=0x%02x i=%d\n",__func__, sen_mode,reg,i);
#endif
    }
    else
    {
        dev_err(dev, "%s(): ERROR: VC FPGA not present !!!\n", __func__);
        err = -EIO;
//        return -EIO;
    }

//done:
#if TRACE_VC_MIPI_RESET
    dev_err(dev, "%s(): sensor_mode=%d err=%d\n", __func__, sen_mode, err);
#endif
    return err;
}

/****** imx290_dump_regs = Dump IMX registers = 10.2019 *****************/
int imx290_dump_regs(struct imx290 *priv)
{

#define REG_WRITE_TEST  0   /* DDD - imx290_dump_regs - reg write test */

    struct camera_common_data *s_data = priv->s_data;

//    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = s_data->dev;
//    struct tegracam_device  *tc_dev = priv->tc_dev;
//    u8 reg_val[2];
//    bool eeprom_ctrl = 0;
    int err = 0;
//    int ret;

    u16 addr;
    u8  low;
    u8  mid;
    u8  high;
    int sval;

// WINMODE[2:0]  = 0x3007[6:4] : window mode: 0=Full HD 1080p, 1=HD720P, 4=window cropping from full HD 1080p
    addr = 0x3007;
#if REG_WRITE_TEST
    low = 0x40;
    dev_err(dev, "%s(): IMX290: Write WINMODE = 0x%x\n", __func__, (int) low);
    imx290_write_reg(s_data, addr, low);
#endif

    imx290_read_reg(s_data, addr, &low);
    sval = (low & 0xFF);
    dev_err(dev, "%s(): IMX290: WINMODE = 0x%x\n", __func__, sval);



// FRSEL[1:0]      = 0x3009[1:0] : frame rate select: 1=60fps, 2=30fps
    addr = 0x3009;
    imx290_read_reg(s_data, addr, &low);
    sval = (low & 0xFF);
    dev_err(dev, "%s(): IMX290: FRSEL   = 0x%x\n", __func__, sval);

// WINPH (X0) H,L  = 0x3041,0x3040
// WINPV (Y0) H,L  = 0x303D,0x303C

// WINWH (DX) H,L  = 0x3043,0x3042
    addr = 0x3043;
    imx290_read_reg(s_data, addr, &high);
    addr = 0x3042;
    imx290_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX290: sensor width = 0x%x (%d)\n", __func__, sval,sval);



// WINWV (DY) H,L  = 0x303F,0x303E
    addr = 0x303F;
    imx290_read_reg(s_data, addr, &high);
    addr = 0x303E;
    imx290_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX290: sensor height = 0x%x (%d)\n", __func__, sval,sval);

// WINWV_OB[3:0]   = 0x303A
    addr = 0x303A;
    imx290_read_reg(s_data, addr, &low);
    sval = (low & 0xFF);
    dev_err(dev, "%s(): IMX290: WINWV_OB = 0x%x (%d)\n", __func__, sval,sval);

// X_OUT_SIZE[12:0] H,L = 0x3473,0x3472 = Horizontal (H) direction effective pixel width setting.
    addr = 0x3473;
    imx290_read_reg(s_data, addr, &high);
    addr = 0x3472;
    imx290_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX290: X_OUT_SIZE = 0x%x (%d)\n", __func__, sval,sval);

// Y_OUT_SIZE H,L [12:0] = 0x3419,0x3418 = Vertical direction effective pixels
    addr = 0x3419;
    imx290_read_reg(s_data, addr, &high);
    addr = 0x3418;
    imx290_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX290: Y_OUT_SIZE = 0x%x (%d)\n", __func__, sval,sval);

// HMAX[15:0] H,L =  0x301D,0x301C
    addr = 0x301D;
    imx290_read_reg(s_data, addr, &high);
    addr = 0x301C;
    imx290_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX290: HMAX = 0x%x (%d)\n", __func__, sval,sval);

// VMAX[17:0] H,M,L =  0x301A,0x3019,0x3018
    addr = 0x301A;
    imx290_read_reg(s_data, addr, &high);
    addr = 0x3019;
    imx290_read_reg(s_data, addr, &mid);
    addr = 0x3018;
    imx290_read_reg(s_data, addr, &low);
    sval = (high << 16) | (mid << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX290: VMAX = 0x%x (%d)\n", __func__, sval,sval);

//// embedded data line control register
//    addr = 0xBCF1;
////    low = 2;
////    imx290_write_reg(s_data, addr, low);
////    dev_err(dev, "%s(): IMX290: Set: embedded data line control = 2\n", __func__);
//    imx290_read_reg(s_data, addr, &high);
//    dev_err(dev, "%s(): IMX290: embedded data line control = 0x%x\n", __func__, (int)high);

//done:
    return err;
}

/****** imx290_set_mode = Set mode = 09.2019 ****************************/
static int imx290_set_mode(struct tegracam_device *tc_dev)
{

#define TRACE_IMX290_SET_MODE   1   /* DDD - imx290_set_mode() - trace */
#define DUMP_IMX290_REGS1       0   /* DDD - imx290_set_mode() - dump sensor regs aft set mode */



//#if !SKIP_IMX290_SET_MODE
    struct imx290 *priv = (struct imx290 *)tegracam_get_privdata(tc_dev);
//#endif
    struct camera_common_data *s_data = tc_dev->s_data;
    struct device *dev = tc_dev->dev;
    const struct camera_common_colorfmt *colorfmt = s_data->colorfmt;
    int pix_fmt = colorfmt->pix_fmt;
    int err = 0;

/*............. Set new sensor mode */
// IMX290 sensor modes:
//   0x00 : 10bit streaming, 2 lanes
//   0x01 : 10bit streaming, 4 lanes (22_22 cable required)

    sensor_mode = 0;        // 10-bit, 2 lanes
    if(priv->num_lanes == 4)
    {
      sensor_mode += 1;     // 10-bit, 4 lanes
    }

/*----------------------------------------------------------------------*/
/*                       Change VC MIPI sensor mode                     */
/*----------------------------------------------------------------------*/
    if(priv->sensor_mode != sensor_mode)
    {
      priv->sensor_mode = sensor_mode;
#if TRACE_IMX290_SET_MODE
      dev_err(dev, "%s(): New sensor_mode=%d (0=2-lanes, 1=4-lanes)d\n", __func__, sensor_mode);
#endif
      err = vc_mipi_reset(tc_dev, sensor_mode);
      if(err)
      {
          dev_err(dev, "%s(): vc_mipi_set_mode() error=%d\n", __func__, err);
      }
    } /* if(priv->sensor_mode != sensor_mode) */


// Get camera mode:
    if(s_data->fmt_width >= 1920 && s_data->fmt_height >= 1080)
    {
      priv->cam_mode = IMX290_MODE_1920X1080;
    }
//    else
//    {
//      priv->cam_mode = IMX297_MODE_720X540;
//    }


//#if TRACE_IMX290_SET_MODE
//    dev_err(dev, "%s(): cam_mode=%d\n", __func__, priv->cam_mode);
//#endif


// Set camera mode:
// Note: After each streaming stop, the sensor is initialized to default mode:
//       by vc_mipi_reset(), but this is not our default mode
//    if(priv->cam_mode != IMX290_MODE_1920X1080)
    {
      err = imx290_write_table(priv, imx290_mode_table[priv->cam_mode]);
      if (err)
      {
        dev_err(dev, "%s(): imx290_write_table() error=%d\n", __func__, err);
//        goto exit;
      }
      else
      {
#if TRACE_IMX290_SET_MODE
//        dev_err(dev, "%s(): imx290_write_table() OK, cam_mode=%d\n", __func__, priv->cam_mode);
#endif
      }
    }

#if TRACE_IMX290_SET_MODE
{
    dev_err(dev, "%s(): fmt_width,fmt_height=%d,%d pix_fmt=0x%x '%c%c%c%c', cam_mode=%d, err=%d\n", __func__,
                        s_data->fmt_width, s_data->fmt_height,
                        pix_fmt,
                        (char)((pix_fmt      ) & 0xFF),
                        (char)((pix_fmt >>  8) & 0xFF),
                        (char)((pix_fmt >> 16) & 0xFF),
                        (char)((pix_fmt >> 24) & 0xFF),
                        priv->cam_mode, err);

//    imx290_dump_regs(priv);
}
#endif

#if DUMP_IMX290_REGS1
{
    dev_err(dev, "%s(): Dump sensor regs\n", __func__);
    imx290_dump_regs(priv);
}
#endif

    return err;
}

#if STOP_STREAMING_SENSOR_RESET
/****** vc_mipi_common_trigmode_write = Trigger mode setup = 10.2020 ****/
static int vc_mipi_common_trigmode_write(struct i2c_client *rom, u32 sensor_ext_trig, u32 exposure_time, u32 io_config, u32 enable_extrig, u32 sen_clk)
{
    int ret;

    if(sensor_ext_trig)
    {
        u64 exposure = (exposure_time * (sen_clk/1000000)); // sen_clk default=54Mhz imx183=72Mhz

        //exposure = (exposure_time * 24 or 25?); //TODO OV9281
        //exposure = (exposure_time * 54); //default
        //exposure = (exposure_time * 72); //TEST IMX183
        //printk("ext_trig exposure=%lld",exposure);

        int addr = 0x0108; // ext trig enable
        //int data =      1; // external trigger enable
        //int data =      2; // external static trigger variable shutter enable
        //int data =      4; // TEST external self trigger enable
        int data = enable_extrig; // external trigger enable
        ret = reg_write(rom, addr, data);

        addr = 0x0103; // io configuration
        data = io_config;
        ret |= reg_write(rom, addr, data);

        addr = 0x0109; // shutter lsb
        data = exposure & 0xff;
        ret |= reg_write(rom, addr, data);

        addr = 0x010a;
        data = (exposure >> 8) & 0xff;
        ret |= reg_write(rom, addr, data);

        addr = 0x010b;
        data = (exposure >> 16) & 0xff;
        ret |= reg_write(rom, addr, data);

        addr = 0x010c; // shutter msb
        data = (exposure >> 24) & 0xff;
        ret |= reg_write(rom, addr, data);

    }
    else
    {
        int addr = 0x0108; // ext trig enable
        int data =      0; // external trigger disable
        ret = reg_write(rom, addr, data);

        addr = 0x0103; // io configuration
        data = io_config;
        ret |= reg_write(rom, addr, data);
    }
    return ret;
}
#endif

/****** imx290_start_streaming = Start streaming = 09.2019 **************/
static int imx290_start_streaming(struct tegracam_device *tc_dev)
{

#define TRACE_IMX290_START_STREAMING    1   /* DDD - imx290_start_streaming() - trace */
#define IMX290_START_STREAMING_SET_CTRLS   1   /* CCC - imx290_start_streaming() - set controls */

    struct imx290 *priv = (struct imx290 *)tegracam_get_privdata(tc_dev);
//    struct imx290 *priv = (struct imx290 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;
    int err = 0;
    int ret = 0;

#if TRACE_IMX290_START_STREAMING
//    dev_err(dev, "%s():...\n", __func__);
#endif


// Set gain and exposure before streaming start
#if IMX290_START_STREAMING_SET_CTRLS
      imx290_set_gain(tc_dev, priv->digital_gain);
      imx290_set_exposure(tc_dev, priv->exposure_time);
      imx290_set_frame_rate(tc_dev, priv->frame_rate);
//      imx290_set_frame_length(tc_dev, priv->frame_length);
#endif

#if TRACE_IMX290_START_STREAMING
//    dev_info(dev, "%s(): Set frame_length=%d\n", __func__, IMX290_DEFAULT_FRAME_LENGTH);
#endif

#if TRACE_IMX290_START_STREAMING
//    dev_info(dev, "%s(): Set gain=%d\n", __func__, IMX290_DEFAULT_GAIN);
#endif

//............... Set trigger mode: on/off
#if VC_CODE // [[[
    if(priv->sensor_ext_trig)
    {
        u64 exposure = (priv->exposure_time * 10000) / 185185;

        int addr = 0x0108; // ext trig enable
        int data =      1; // external trigger enable
        ret = reg_write(priv->rom, addr, data);
#if 0
        // TODO
        addr = 0x0103; // flash output enable
        data =      1; // flash output enable
        ret = reg_write(priv->rom, addr, data);
#endif
        addr = 0x0109; // shutter lsb
        data = exposure & 0xff;
        ret = reg_write(priv->rom, addr, data);

        addr = 0x010a;
        data = (exposure >> 8) & 0xff;
        ret = reg_write(priv->rom, addr, data);

        addr = 0x010b;
        data = (exposure >> 16) & 0xff;
        ret = reg_write(priv->rom, addr, data);

        addr = 0x010c; // shutter msb
        data = (exposure >> 24) & 0xff;
        ret = reg_write(priv->rom, addr, data);

//        return reg_write_table(client, start);
    }
    else
    {
        int addr = 0x0108; // ext trig enable
        int data =      0; // external trigger disable
        ret = reg_write(priv->rom, addr, data);

        addr = 0x0103; // flash output enable
        data = priv->flash_output; // flash output enable
        ret += reg_write(priv->rom, addr, data);

        if(ret)
        {
          dev_err(dev, "%s(): reg_write() error=%d\n", __func__, ret);
          goto exit;
        }

//        return reg_write_table(client, start);
    }
#endif  // ]]]


//............... Start streaming
//    mutex_lock(&priv->streaming_lock);
    err = imx290_write_table(priv, imx290_mode_table[IMX290_MODE_START_STREAM]);
    if (err) {
//        mutex_unlock(&priv->streaming_lock);
        goto exit;
    } else {
        priv->streaming = true;
//        mutex_unlock(&priv->streaming_lock);
    }

    usleep_range(50000, 51000);

exit:
#if TRACE_IMX290_START_STREAMING
    dev_err(dev, "%s(): err=%d\n", __func__, err);
#endif

    return err;

}

/****** imx290_stop_streaming = Stop streaming = 09.2019 ****************/
static int imx290_stop_streaming(struct tegracam_device *tc_dev)
{

#define TRACE_IMX290_STOP_STREAMING     1   /* DDD - imx290_stop_streaming() - trace */
#define IMX290_STOP_STREAMING_ENB_RESET    0    /* CCC - imx290_stop_streaming() - enable VC MIPI reset */

//    struct camera_common_data *s_data = tc_dev->s_data;
    struct imx290 *priv = (struct imx290 *)tegracam_get_privdata(tc_dev);
//    struct imx290 *priv = (struct imx290 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;
    int err = 0;

//    mutex_lock(&priv->mutex);

#if STOP_STREAMING_SENSOR_RESET
{
// VC RPI driver:
//  /* reinit sensor */
//    ret = vc_mipi_common_rom_init(client, priv->rom, -1);
    err = vc_mipi_reset (tc_dev, -1);
    if (err)
      return err;

//    ret = vc_mipi_common_rom_init(client, priv->rom, priv->cur_mode->sensor_mode);
    err = vc_mipi_reset (tc_dev, sensor_mode);
    if (err)
      return err;

    err = vc_mipi_common_trigmode_write(priv->rom, 0, 0, 0, 0, 0); /* disable external trigger counter */
    if (err)
        dev_err(dev, "%s: REINIT: Error %d disabling trigger counter\n", __func__, err);
//
//              ret = v4l2_ctrl_handler_setup(&priv->ctrl_handler);
//              if (ret < 0)
//                  dev_err(&client->dev, "REINIT: Error %d setting controls\n", ret);
//                return vc_mipi_common_reg_write_table(client, priv->trait->stop);
//                break;
}
#endif

#if TRACE_IMX290_STOP_STREAMING
//    dev_err(dev, "%s():\n", __func__);
#endif

    err = imx290_write_table(priv, imx290_mode_table[IMX290_MODE_STOP_STREAM]);
    if (err)
    {
//#if TRACE_IMX290_STOP_STREAMING
      dev_err(dev, "%s(): imx290_write_table() err=%d\n", __func__, err);
//#endif
//        mutex_unlock(&priv->streaming_lock);
        goto exit;
    }
    else
    {
        priv->streaming = false;

#if IMX290_STOP_STREAMING_ENB_RESET  // [[[
        err = vc_mipi_reset(tc_dev, sensor_mode);
        if(err)
        {
          dev_err(dev, "%s(): VC MIPI sensor reset: err=%d\n", __func__, err);
        }
        else
        {
          dev_err(dev, "%s(): VC sensor reset\n", __func__);
        }

#endif // ]]] - VC reset
    }

//    usleep_range(10000, 11000);
    usleep_range(50000, 51000);

exit:
#if TRACE_IMX290_STOP_STREAMING
    dev_err(dev, "%s(): err=%d\n\n", __func__, err);
#endif

//    mutex_unlock(&priv->mutex);
    return err;

}

static struct camera_common_sensor_ops imx290_common_ops = {
    .numfrmfmts = ARRAY_SIZE(imx290_frmfmt),
    .frmfmt_table = imx290_frmfmt,
    .power_on = imx290_power_on,
    .power_off = imx290_power_off,
    .write_reg = imx290_write_reg,
    .read_reg = imx290_read_reg,
    .parse_dt = imx290_parse_dt,
    .power_get = imx290_power_get,
    .power_put = imx290_power_put,
    .set_mode = imx290_set_mode,
    .start_streaming = imx290_start_streaming,
    .stop_streaming = imx290_stop_streaming,
};

/****** imx290_video_probe = Video probe = 09.2019 **********************/
static int imx290_video_probe(struct i2c_client *client)
{

#define TRACE_IMX290_VIDEO_PROBE    0   /* DDD - imx290_video_probe - trace */

    u16 model_id;
#if TRACE_IMX290_VIDEO_PROBE
    u32 lot_id=0;
    u16 chip_id=0;
#endif
    int ret;

    /* Check and show model, lot, and chip ID. */
    ret = reg_read(client, 0x359b);
    if (ret < 0) {
        dev_err(&client->dev, "Failure to read Model ID (high byte)\n");
        goto done;
    }
    model_id = ret << 8;

    ret = reg_read(client, 0x359a);
    if (ret < 0) {
        dev_err(&client->dev, "Failure to read Model ID (low byte)\n");
        goto done;
    }
    model_id |= ret;
#if 0
    if ( ! ((model_id == 296) || (model_id == 202)) ) {
        dev_err(&client->dev, "Model ID: %x not supported!\n",
            model_id);
        ret = -ENODEV;
        goto done;
    }
#endif
#if TRACE_IMX290_VIDEO_PROBE
    dev_err(&client->dev,
         "Model ID 0x%04x, Lot ID 0x%06x, Chip ID 0x%04x\n",
         model_id, lot_id, chip_id);
#endif
done:
    return ret;
}

/****** imx290_probe_vc_rom = Probe VC ROM = 09.2019 ********************/
static struct i2c_client * imx290_probe_vc_rom(struct i2c_adapter *adapter, u8 addr)
{
    struct i2c_board_info info = {
        I2C_BOARD_INFO("dummy", addr),
    };
    unsigned short addr_list[2] = { addr, I2C_CLIENT_END };

    return i2c_new_probed_device(adapter, &info, addr_list, NULL);
}


/****** imx290_board_setup = Board setup = 09.2019 **********************/
static int imx290_board_setup(struct imx290 *priv)
{

#define TRACE_IMX290_BOARD_SETUP   1   /* DDD - imx290_board_setup() - trace */
#define DUMP_CTL_REG_DATA          0   /* DDD - imx290_board_setup() - dump module control registers 0x100-0x108 (I2C addr=0x10) */
#define DUMP_HWD_DESC_ROM_DATA     0   /* DDD - imx290_board_setup() - dump Hardware Desriptor ROM data (I2C addr=0x10) */
#define DUMP_IMX290_REGS           0   /* DDD - imx290_board_setup() - dump IMX290 regs */
#define DUMP_V4L_PARAMS            1   /* DDD - imx290_board_setup() - dump V4L params */
#define MOD_ID                0x0290   /* CCC - imx290_board_setup() - module id., 0=off: don't check */
#define ERR_MOD_ID                 0   /* CCC - imx290_board_setup() - test invalid module id */

    struct camera_common_data *s_data = priv->s_data;
    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = s_data->dev;
    struct tegracam_device  *tc_dev = priv->tc_dev;
//    u8 reg_val[2];
//    bool eeprom_ctrl = 0;
    int err = 0;
    int ret;

#if VC_CODE    // [[[ - new VC code
    struct i2c_client *client = priv->i2c_client;
    struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
    struct sensor_mode_properties *mode =  &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];
//    int num_lanes;

#if DUMP_V4L_PARAMS
    int mclk_freq      = (int)mode->signal_properties.mclk_freq;
    int pixel_clock    = (int)mode->signal_properties.pixel_clock.val;
    int cil_settletime = (int)mode->signal_properties.cil_settletime;
    int discontinuous_clk = (int)mode->signal_properties.discontinuous_clk;

//struct sensor_image_properties {
//    __u32 width;
//    __u32 height;
//    __u32 line_length;
//    __u32 pixel_format;
//    __u32 embedded_metadata_height;
//};

    dev_err(dev, "%s: mclk_freq=%d pixel_clock=%d cil_settletime=%d discontinuous_clk=%d\n", __func__,
         mclk_freq, pixel_clock, cil_settletime, discontinuous_clk);
    dev_err(dev, "%s: width,height,line_length=%d,%d,%d pixel_format=0x%x embedded_metadata_height=%d\n", __func__,
         (int)mode->image_properties.width,
         (int)mode->image_properties.height,
         (int)mode->image_properties.line_length,
         (int)mode->image_properties.pixel_format,
         (int)mode->image_properties.embedded_metadata_height);
#endif

#endif  // ]]] - end of VC_CODE

    if (pdata->mclk_name) {
        err = camera_common_mclk_enable(s_data);
        if (err) {
            dev_err(dev, "%s: error turning on mclk (%d)\n", __func__, err);
            goto done;
        }
    }

    err = imx290_power_on(s_data);
    if (err) {
        dev_err(dev, "%s: error during power on sensor (%d)\n", __func__, err);
        goto err_power_on;
    }

/*----------------------------------------------------------------------*/
#if VC_CODE    // [[[ - VC code
/*----------------------------------------------------------------------*/

    priv->num_lanes = (int)mode->signal_properties.num_lanes;
    if(priv->num_lanes == 2)
    {
        sensor_mode = 0;   // autoswitch to sensor_mode=0 if 2 lanes are given
    }
    else if(priv->num_lanes == 4)
    {
        sensor_mode = 1;    // autoswitch to sensor_mode=1 if 4 lanes are given
    }
    else
    {
        dev_err(dev, "%s: VC Sensor device-tree: Invalid number of data-lanes: %d\n",__func__, priv->num_lanes);
        return -EINVAL;
    }
    dev_err(dev, "%s: VC Sensor device-tree has configured %d data-lanes: sensor_mode=%d\n",__func__, priv->num_lanes, sensor_mode);

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&client->dev, "%s(): I2C-Adapter doesn't support I2C_FUNC_SMBUS_BYTE\n", __func__);
        return -EIO;
    }

//    priv->rom = i2c_new_dummy(adapter,0x10);
    priv->rom = imx290_probe_vc_rom(adapter,0x10);

    if ( priv->rom )
    {
//        static int i=1;
        int addr,reg;

#if DUMP_HWD_DESC_ROM_DATA      /* dump Hardware Desriptor ROM data */
        dev_err(&client->dev, "%s(): Dump Hardware Descriptor ROM data:\n", __func__);
#endif

        for (addr=0; addr<sizeof(priv->rom_table); addr++)
        {
          reg = reg_read(priv->rom, addr+0x1000);
          if(reg < 0)
          {
              i2c_unregister_device(priv->rom);
              return -EIO;
          }
          *((char *)(&(priv->rom_table))+addr)=(char)reg;
#if DUMP_HWD_DESC_ROM_DATA      /* [[[ - dump Hardware Desriptor ROM data */
{
          static int sval = 0;   // short 2-byte value

          if(addr & 1)  // odd addr
          {
            sval |= (int)reg << 8;
            dev_err(&client->dev, "addr=0x%04x reg=0x%04x\n",addr+0x1000-1,sval);
          }
          else
          {
            sval = reg;
          }
}
#endif  // ]]]

        } /* for (addr=0; addr<sizeof(priv->rom_table); addr++) */

        dev_err(&client->dev, "%s(): VC FPGA found!\n", __func__);

        dev_err(&client->dev, "[ MAGIC  ] [ %s ]\n",
                priv->rom_table.magic);

        dev_err(&client->dev, "[ MANUF. ] [ %s ] [ MID=0x%04x ]\n",
                priv->rom_table.manuf,
                priv->rom_table.manuf_id);

        dev_err(&client->dev, "[ SENSOR ] [ %s %s ]\n",
                priv->rom_table.sen_manuf,
                priv->rom_table.sen_type);

        dev_err(&client->dev, "[ MODULE ] [ ID=0x%04x ] [ REV=0x%04x ]\n",
                priv->rom_table.mod_id,
                priv->rom_table.mod_rev);

        dev_err(&client->dev, "[ MODES  ] [ NR=0x%04x ] [ BPM=0x%04x ]\n",
                priv->rom_table.nr_modes,
                priv->rom_table.bytes_per_mode);

// Reset VC MIPI sensor: Initialize FPGA: reset sensor registers to def. values
        priv->sensor_mode = sensor_mode;
        err = vc_mipi_reset(tc_dev, sensor_mode);
        if(err)
        {
          dev_err(dev, "%s(): vc_mipi_reset() error=%d\n", __func__, err);
          goto done;
        }

#if DUMP_CTL_REG_DATA
{
        int i;
        int reg_val[10];

        dev_err(&client->dev, "%s(): Module controller registers (0x10):\n", __func__);

//        addr = 0x100;
        i = 0;
        for(addr=0x100; addr<=0x108; addr++)
        {
          reg_val[i] = reg_read(priv->rom, addr);
//          dev_err(&client->dev, "0x%04x: %02x\n", addr, reg_val[i]);
          i++;
        }

        dev_err(&client->dev, "0x100-103: %02x %02x %02x %02x\n", reg_val[0],reg_val[1],reg_val[2],reg_val[3]);
        dev_err(&client->dev, "0x104-108: %02x %02x %02x %02x %02x\n", reg_val[4],reg_val[5],reg_val[6],reg_val[7],reg_val[8]);

}
#endif

    }
    else
    {
        dev_err(&client->dev, "%s(): Error !!! VC FPGA not found !!!\n", __func__);
        return -EIO;
    }

    ret = imx290_video_probe(client);
    if (ret < 0)
    {
      dev_err(dev, "%s(): imx290_video_probe() error=%d\n", __func__, ret);
//      err = -EIO;
//      goto done;
    }

#if MOD_ID
    if(MOD_ID != priv->rom_table.mod_id)
    {
      dev_err(dev, "%s(): Invalid module id: E=0x%04x A=0x%04x\n", __func__, MOD_ID, priv->rom_table.mod_id);
      err = -EIO;
      goto done;
    }
    dev_err(dev, "%s(): Valid module id: 0x%04x\n", __func__, priv->rom_table.mod_id);
#endif

#if ERR_MOD_ID
      dev_err(dev, "===> %s(): Invalid module id test !!\n", __func__);
      err = -EIO;
      goto done;
#endif



#if TRACE_IMX290_BOARD_SETUP
{
//    char *sen_models[4] =
//    {
//      "IMX290_MODEL_MONOCHROME",
//      "IMX290_MODEL_COLOR",
//      "IMX297_MODEL_MONOCHROME",
//      "IMX297_MODEL_COLOR",
//    };
//    dev_err(&client->dev, "%s(): Success: sensor model=%s, err=%d\n", __func__, sen_models[priv->model], err);
}
#endif

#endif  // ]]] - end of VC_CODE

#if DUMP_IMX290_REGS
{
    imx290_dump_regs(priv);

//    u16 addr;
//    u8  low;
//    u8  mid;
//    u8  high;
//    int sval;
//
//
//// WINWH (win dx) H,L   = 0x3043,0x3042
//    addr = 0x3043;
//    imx290_read_reg(s_data, addr, &high);
//    addr = 0x3042;
//    imx290_read_reg(s_data, addr, &low);
//    sval = (high << 8) | (low & 0xFF);
//    dev_err(dev, "%s(): IMX290: sensor width = 0x%x (%d)\n", __func__, sval,sval);
//
//// WINWV (win dy) H,L   = 0x303F,0x303E
//    addr = 0x303F;
//    imx290_read_reg(s_data, addr, &high);
//    addr = 0x303E;
//    imx290_read_reg(s_data, addr, &low);
//    sval = (high << 8) | (low & 0xFF);
//    dev_err(dev, "%s(): IMX290: sensor height = 0x%x (%d)\n", __func__, sval,sval);
//
//// WINWV_OB[3:0]   = 0x303A
//    addr = 0x303A;
//    imx290_read_reg(s_data, addr, &low);
//    sval = (low & 0xFF);
//    dev_err(dev, "%s(): IMX290: WINWV_OB = 0x%x (%d)\n", __func__, sval,sval);
//
//// Y_OUT_SIZE (V effective pixels) H,L   = 0x3419,0x3418
//    addr = 0x3419;
//    imx290_read_reg(s_data, addr, &high);
//    addr = 0x3418;
//    imx290_read_reg(s_data, addr, &low);
//    sval = (high << 8) | (low & 0xFF);
//    dev_err(dev, "%s(): IMX290: Y_OUT_SIZE = 0x%x (%d)\n", __func__, sval,sval);
//
//// HMAX[15:0] H,L =  0x301D,0x301C
//    addr = 0x301D;
//    imx290_read_reg(s_data, addr, &high);
//    addr = 0x301C;
//    imx290_read_reg(s_data, addr, &low);
//    sval = (high << 8) | (low & 0xFF);
//    dev_err(dev, "%s(): IMX290: HMAX = 0x%x (%d)\n", __func__, sval,sval);
//
//// VMAX[17:0] H,M,L =  0x301A,0x3019,0x3018
//    addr = 0x301A;
//    imx290_read_reg(s_data, addr, &high);
//    addr = 0x3019;
//    imx290_read_reg(s_data, addr, &mid);
//    addr = 0x3018;
//    imx290_read_reg(s_data, addr, &low);
//    sval = (high << 16) | (mid << 8) | (low & 0xFF);
//    dev_err(dev, "%s(): IMX290: VMAX = 0x%x (%d)\n", __func__, sval,sval);

//// embedded data line control register
//    addr = 0xBCF1;
////    low = 2;
////    imx290_write_reg(s_data, addr, low);
////    dev_err(dev, "%s(): IMX290: Set: embedded data line control = 2\n", __func__);
//    imx290_read_reg(s_data, addr, &high);
//    dev_err(dev, "%s(): IMX290: embedded data line control = 0x%x\n", __func__, (int)high);
}
#endif


//err_reg_probe:
    imx290_power_off(s_data);

err_power_on:
    if (pdata->mclk_name)
        camera_common_mclk_disable(s_data);

done:
    return err;
}

/****** imx290_open = Open device = 09.2019 *****************************/
static int imx290_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    dev_dbg(&client->dev, "%s:\n", __func__);
    return 0;
}

static const struct v4l2_subdev_internal_ops imx290_subdev_internal_ops = {
    .open = imx290_open,
};

static const struct of_device_id imx290_of_match[] = {
    { .compatible = "nvidia,imx290", },
    { },
};
MODULE_DEVICE_TABLE(of, imx290_of_match);

/****** imx290_probe = Probe = 09.2019 **********************************/
static int imx290_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
    struct device *dev = &client->dev;
//    struct device_node *node = client->dev.of_node;
    struct tegracam_device *tc_dev;
    struct imx290 *priv;
    int err;
    const struct of_device_id *match;

//    dev_info(dev, "%s(): Probing v4l2 sensor at addr 0x%0x\n", __func__, client->addr); // , __DATE__, __TIME__);
    dev_err(dev, "%s(): Probing v4l2 sensor at addr 0x%0x - %s/%s\n", __func__, client->addr, __DATE__, __TIME__);

    match = of_match_device(imx290_of_match, dev);
    if (!match) {
        dev_err(dev, "No device match found\n");
        return -ENODEV;
    }
    dev_err(dev, "%s(): of_match_device() OK\n", __func__);

    if (!IS_ENABLED(CONFIG_OF) || !client->dev.of_node)
    {
        dev_err(dev, "%s(): !IS_ENABLED(CONFIG_OF) || !client->dev.of_node\n", __func__);
        return -EINVAL;
    }

    priv = devm_kzalloc(dev,
                sizeof(struct imx290), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    tc_dev = devm_kzalloc(dev,
                sizeof(struct tegracam_device), GFP_KERNEL);
    if (!tc_dev)
        return -ENOMEM;

    dev_info(dev, "%s(): devm_kzalloc() OK\n", __func__);

    priv->i2c_client = tc_dev->client = client;
    tc_dev->dev = dev;
    strncpy(tc_dev->name, "imx290", sizeof(tc_dev->name));
    tc_dev->dev_regmap_config = &imx290_regmap_config;
    tc_dev->sensor_ops = &imx290_common_ops;
    tc_dev->v4l2sd_internal_ops = &imx290_subdev_internal_ops;
    tc_dev->tcctrl_ops = &imx290_ctrl_ops;

    err = tegracam_device_register(tc_dev);
    if (err) {
        dev_err(dev, "tegra camera driver registration failed\n");
        return err;
    }
    dev_info(dev, "%s(): tegracam_device_register() OK\n", __func__);

    priv->tc_dev = tc_dev;
    priv->s_data = tc_dev->s_data;
    priv->subdev = &tc_dev->s_data->subdev;
    tegracam_set_privdata(tc_dev, (void *)priv);

#if IMX290_ENB_MUTEX
    mutex_init(&priv->mutex);
#endif


    err = imx290_board_setup(priv);
    if (err) {
        dev_err(dev, "%s: imx290_board_setup() error=%d\n", __func__, err);
        return err;
    }
    dev_info(dev, "%s(): imx290_board_setup() OK\n", __func__);

    err = tegracam_v4l2subdev_register(tc_dev, true);
    if (err) {
        dev_err(dev, "tegra camera subdev registration failed\n");
        return err;
    }
    dev_info(dev, "%s(): tegracam_v4l2subdev_register() OK\n", __func__);

    priv->digital_gain  = IMX290_DIGITAL_GAIN_DEFAULT;
    priv->exposure_time = IMX290_DIGITAL_EXPOSURE_DEFAULT;
    priv->frame_rate    = IMX290_FRAME_RATE_DEFAULT;
//    priv->frame_length = IMX290_DEFAULT_FRAME_LENGTH;
//    priv->sensor_ext_trig = 0;    // ext. trigger flag: 0=no, 1=yes

    priv->sensor_ext_trig = 0;    // ext. trigger flag: 0=no, 1=yes
    priv->sen_clk = 54000000;     // clock-frequency: default=54Mhz imx183=72Mhz
    priv->flash_output = flash_output;


    dev_err(dev, "%s(): Detected imx290 sensor - %s/%s\n", __func__, __DATE__, __TIME__);

//    switch(priv->model)
//    {
//      case IMX290_MODEL_MONOCHROME:
//        priv->cam_mode = IMX290_MODE_1920X1080;
//        dev_err(dev, "%s(): Detected imx290 sensor\n", __func__); // , __DATE__, __TIME__);
//        break;
//
//      case IMX290_MODEL_COLOR:
//        priv->cam_mode = IMX290_MODE_1920X1080;
//        dev_err(dev, "%s(): Detected imx290c sensor\n", __func__); // , __DATE__, __TIME__);
//        break;
//
//      case IMX297_MODEL_MONOCHROME:
//        priv->cam_mode = IMX297_MODE_720X540;
//        dev_err(dev, "%s(): Detected imx297 sensor\n", __func__); // , __DATE__, __TIME__);
//        break;
//
//      case IMX297_MODEL_COLOR:
//        priv->cam_mode = IMX297_MODE_720X540;
//        dev_err(dev, "%s(): Detected imx297c sensor\n", __func__); // , __DATE__, __TIME__);
//        break;
//
//      default:
//        dev_err(dev, "%s(): Detected inknown imx290/297 sensor\n", __func__); // , __DATE__, __TIME__);
//        break;
//    }

    return 0;
}

/****** imx290_remove = Remove = 09.2019 ********************************/
static int imx290_remove(struct i2c_client *client)
{
    struct camera_common_data *s_data = to_camera_common_data(&client->dev);
    struct imx290 *priv = (struct imx290 *)s_data->priv;

    tegracam_v4l2subdev_unregister(priv->tc_dev);
//    ov5693_power_put(priv->tc_dev);
    tegracam_device_unregister(priv->tc_dev);

#if IMX290_ENB_MUTEX
    mutex_destroy(&priv->mutex);
#endif

    return 0;
}

static const struct i2c_device_id imx290_id[] = {
    { "imx290", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, imx290_id);



/****** i2c_driver = I2C driver = 09.2019 *******************************/
static struct i2c_driver imx290_i2c_driver = {
    .driver = {
        .name = "imx290",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(imx290_of_match),
    },
    .probe = imx290_probe,
    .remove = imx290_remove,
    .id_table = imx290_id,
};
module_i2c_driver(imx290_i2c_driver);

MODULE_DESCRIPTION("Media Controller driver for IMX290");
//MODULE_AUTHOR("NVIDIA Corporation");
MODULE_AUTHOR("Vision Components GmbH <mipi-tech@vision-components.com>");
MODULE_LICENSE("GPL v2");

#if 0   // [[[
#endif   // ]]]
