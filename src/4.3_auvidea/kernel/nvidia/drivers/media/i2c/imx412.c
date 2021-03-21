/*
 * imx412.c - imx412 sensor driver
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

#define VC_CODE             1   /* CCC - imx412.c - enable code for VC MIPI camera  */
#define VC_SENSOR_MODE      0   /* CCC - imx412.c - VC sensor mode: 0 = 10-bit, 1 = 10-bit 4 lanes */

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
//#include <media/imx412.h>

#include "../platform/tegra/camera/camera_gpio.h"
//#include "imx412.h"
//#include "imx412_mode_tbls.h"

//#define CREATE_TRACE_POINTS
//#include <trace/events/ov5693.h>


#define IMX412_TABLE_WAIT_MS    0
#define IMX412_TABLE_END        1

/* In dB*10 */
#define IMX412_ANALOG_GAIN_MIN          0
#define IMX412_ANALOG_GAIN_MAX          978
#define IMX412_ANALOG_GAIN_DEFAULT      0

/* In dB*10 */
#define IMX412_DIGITAL_GAIN_MIN         0x100
#define IMX412_DIGITAL_GAIN_MAX         0xFFF
#define IMX412_DIGITAL_GAIN_DEFAULT     0x100

/* In usec */
#define IMX412_DIGITAL_EXPOSURE_MIN     22    // microseconds (us)
#define IMX412_DIGITAL_EXPOSURE_MAX     92693
#define IMX412_DIGITAL_EXPOSURE_DEFAULT 5000

// PK: just for test, not supported yet
#define IMX412_MIN_FRAME_LENGTH     100
#define IMX412_MAX_FRAME_LENGTH     800000
#define IMX412_FRAME_RATE_DEFAULT   20000000   // 20 fps (*1000000)


#define imx412_reg struct reg_8

enum {
    IMX412_MODE_4032X3040 = 0,
    IMX412_MODE_START_STREAM,
    IMX412_MODE_STOP_STREAM,
};

static const imx412_reg imx412_start[] = {
    { 0x0100, 0x01 },
    { IMX412_TABLE_END, 0x00 }
};

static const imx412_reg imx412_stop[] = {
    { 0x0100, 0x00 },
    { IMX412_TABLE_END, 0x00 }
};

//---------------------------- 4032 x 3040 default mode -------------------------------
// ATTENTION:
// 1. 10-bit: Jetson Nano requires sensor width, multiple of 32, and changes the specified width, set by V4l2-ctl for example,
//    to the nearest number which is greater or equal to it. So we use 4032 = the largest number < 4056
//    (the width set by the VC MIPI FPGA).
// 2. 8-bit: Jetson Nano requires sensor width, multiple of 64, and changes the specified width, set by V4l2-ctl for example,
//    to the nearest number which is greater or equal to it.

#define IMX412_DX 4032
#define IMX412_DY 3040
#define X0 0
#define Y0 0
#define X1 (X0+DX-1)
#define Y1 (Y0+DY-1)
static const imx412_reg imx412_mode_4032X3040[] = {
//    { 0x0344,   X0>>8 }, { 0x0345,   X0 & 0xFF },   // horizontal start high,low  0x0344,0x0345  = X0
//    { 0x0346,   Y0>>8 }, { 0x0347,   Y0 & 0xFF },   // vertical start high,low    0x0346,0x0347  = Y0
//    { 0x0348,   X1>>8 }, { 0x0349,   X1 & 0xFF },   // horizontal end H,L         0x0348,0x0349  = X1
//    { 0x034A,   Y1>>8 }, { 0x034B,   Y1 & 0xFF },   // vertical   end H,L         0x034A,0x034B  = Y1
    { 0x034C, IMX412_DX>>8 }, { 0x034D, IMX412_DX & 0xFF },   // hor. output width H,L      0x034C,0x034D  = DX
    { 0x034E, IMX412_DY>>8 }, { 0x034F, IMX412_DY & 0xFF },   // ver. output height H,L     0x034E,0x034F  = DY
//    { 0x0340,   DY>>8 }, { 0x0341,   DY & 0xFF },   // Frame length in lines H,L  0x0340,0x0341  = DY  (see 5.3. Frame rate)
    { IMX412_TABLE_END, 0x00 }
};
//#undef  DX
//#undef  DY
#undef  X0
#undef  Y0
#undef  X1
#undef  Y1


static const imx412_reg *imx412_mode_table[] = {
    [IMX412_MODE_4032X3040]    = imx412_mode_4032X3040,
    [IMX412_MODE_START_STREAM] = imx412_start,
    [IMX412_MODE_STOP_STREAM]  = imx412_stop,
};

static const int imx412_20fps[] = {
    20,
};
static const int imx412_30fps[] = {
    30,
};

static const struct camera_common_frmfmt imx412_frmfmt[] = {
    { { IMX412_DX, IMX412_DY }, imx412_20fps, ARRAY_SIZE(imx412_20fps), 0,
      IMX412_MODE_4032X3040 },
//    { { 2016, 1520 }, imx412_30fps, ARRAY_SIZE(imx412_30fps), 0,
//    IMX412_MODE_2016X1520 },
//    { { 1280, 720 }, imx412_60fps, ARRAY_SIZE(imx412_60fps), 0,
//      IMX412_MODE_1280X720 },
//    { { 640, 400 }, imx412_60fps, ARRAY_SIZE(imx412_60fps), 0,
//      IMX412_MODE_640X400 },
};

static const struct of_device_id imx412_of_match[] = {
    { .compatible = "nvidia,imx412", },
    { },
};
MODULE_DEVICE_TABLE(of, imx412_of_match);

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


struct imx412 {
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
    int    cam_mode;    // camera mode:
                        //   IMX412_MODE_4032X3040 (default)
                        //   IMX412_MODE_2016X1520
    int sensor_ext_trig;    // ext. trigger flag: 0=no, 1=yes
    int flash_output;       // flash output enable
    int sen_clk;            // sen_clk default=54Mhz imx183=72Mhz
    int sensor_mode;        // sensor mode
    int num_lanes;          // # of data lanes: 1, 2, 4
#endif  // ]]]

};

static const struct regmap_config imx412_regmap_config = {
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



#if VC_CODE     // [[[ - new VC code
static int sensor_mode = VC_SENSOR_MODE;       /* 0:10-bit 2 lanes, 1:10-bit 4 lanes */
//module_param(sensor_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//MODULE_PARM_DESC(sensor_mode, "VC Sensor Mode: 0=10bit_stream 1=10bit_4lanes");

static int flash_output  = 0;       // flash output enable

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
//               const struct imx412_reg table[])
//{
//    const struct imx412_reg *reg;
//    int ret;
//
//    for (reg = table; reg->addr != IMX412_TABLE_END; reg++) {
//        ret = reg_write(client, reg->addr, reg->val);
//        if (ret < 0)
//            return ret;
//    }
//
//    return 0;
//}

#endif  // ]]] - VC_CODE

/****** imx412_read_reg = Read register = 08.2019 ***********************/
static int imx412_read_reg(struct camera_common_data *s_data, u16 addr, u8 *val)
{
    int err = 0;
    u32 reg_val = 0;

    err = regmap_read(s_data->regmap, addr, &reg_val);
    *val = reg_val & 0xff;

    return err;
}

/****** imx412_write_reg = Write register = 08.2019 *********************/
static int imx412_write_reg(struct camera_common_data *s_data, u16 addr, u8 val)
{
    int err = 0;

    err = regmap_write(s_data->regmap, addr, val);
    if (err)
        dev_err(s_data->dev, "%s: i2c write failed, 0x%x = %x",
            __func__, addr, val);

    return err;
}

/****** imx412_write_table = Write table = 08.2019 **********************/
static int imx412_write_table(struct imx412 *priv, const imx412_reg table[])
{
    return regmap_util_write_table_8(priv->s_data->regmap, table, NULL, 0,
        IMX412_TABLE_WAIT_MS, IMX412_TABLE_END);
}

/****** imx412_gpio_set = GPIO set = 08.2019 ****************************/
static void imx412_gpio_set(struct camera_common_data *s_data,
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

/****** imx412_set_gain = Set gain = 08.2019 ****************************/
static int imx412_set_gain(struct tegracam_device *tc_dev, s64 val)
{

#define TRACE_IMX412_SET_GAIN        1   /* DDD - imx412_set_gain() - trace */
//#define IMX412_SET_GAIN_GROUP_HOLD   0   /* CCC - imx412_set_gain() - enable group hold bef reg write */

//    struct camera_common_data *s_data = tc_dev->s_data;
    struct imx412 *priv = (struct imx412 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;

    imx412_reg regs[4];
    s64 gain = val;
    int err;


    if (gain < IMX412_DIGITAL_GAIN_MIN)
      gain = IMX412_DIGITAL_GAIN_MIN;
    else if (gain > IMX412_DIGITAL_GAIN_MAX)
      gain = IMX412_DIGITAL_GAIN_MAX;

//    gain = IMX412_DIGITAL_GAIN_DEFAULT; // VC: FIXED to 1.00 for now
    priv->digital_gain = gain;

#if TRACE_IMX412_SET_GAIN
    dev_err(dev, "%s: Set gain = %d\n", __func__, (int)gain);
#endif

    regs[0].addr = 0x020e; //   // IMX412_GAIN_HIGH_ADDR
    regs[0].val = (priv->digital_gain >> 8) & 0x0f;
    regs[1].addr = 0x020f;      // IMX412_GAIN_LOW_ADDR
    regs[1].val = priv->digital_gain & 0xff;
    regs[2].addr = IMX412_TABLE_END;
    regs[2].val = 0;

//#if IMX412_SET_GAIN_GROUP_HOLD
//    if (!priv->group_hold_prev)
//        imx412_set_group_hold(tc_dev, 1);
//#endif

    err = imx412_write_table(priv, regs);
    if (err)
        goto fail;

//    priv->digital_gain = gain;

    return 0;

fail:
    dev_err(dev, "%s: error=%d\n", __func__, err);
    return err;
}

/****** imx412_set_exposure = Set exposure = 08.2019 ********************/
static int imx412_set_exposure(struct tegracam_device *tc_dev, s64 val)
{

#define TRACE_IMX412_SET_EXPOSURE   1   /* DDD - imx412_set_exposure() - trace */
#define DUMP_EXPOSURE_PARAMS        1   /* DDD - imx412_set_exposure() - dump DT exposure params */

#define IMX412_SET_EXPOSURE_MUTEX   0   /* CCC - imx412_set_exposure() - mutex lock/unlock */

//    struct camera_common_data *s_data = tc_dev->s_data;
    struct device *dev = tc_dev->dev;
    struct imx412 *priv = tc_dev->priv;
//    struct imx412 *priv = (struct imx412 *)tc_dev->priv;

    imx412_reg regs[4];
//    u32 exposure_time;
    u32 exposure = 0;
    int err = 0;

#if IMX412_SET_EXPOSURE_MUTEX
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

//         mode->control_properties.min_framerate = 10;
//         mode->control_properties.max_framerate = 120;
}
#endif


//static int imx_exposure_412(struct imx *priv)
//{
//    struct i2c_client *client = v4l2_get_subdevdata(&priv->subdev);
//    int ret=0;
//    u32 exposure = 0;
//
//    // alpha=1932/5792
//    // N={1,5792}
//    // tline=16129.2119 ns -> 16us
//    // exposure_time=(N+alpha)*tline
//    // N=(exposure_time/tline)-alpha
//    // EXP_MIN=(1+alpha)*tline=21509.34885 ns -> 22us
//    // EXP_MAX=(5792+alpha)*tline=93425775.44 ns -> 93426us
//    //
//
//
//    if (priv->exposure_time < 22)     priv->exposure_time = 22;
//    if (priv->exposure_time > 92693)   priv->exposure_time = 92693;
//
//    if (priv->exposure_time < 92694)
//    {
//        exposure = ( ((u32)(priv->exposure_time) * 20000 / 16) - (1932 * 20000 / 5792) ) / 20000;
//        dev_info(&client->dev, "EXP = %d \n",exposure);
//
//        ret |= vc_mipi_common_reg_write(client, 0x0202, (exposure >>  8) & 0xff);
//        ret |= vc_mipi_common_reg_write(client, 0x0203,  exposure        & 0xff);
//    }
//
//    return ret;
//}


    priv->exposure_time = (u32)val;

    if(priv->exposure_time < IMX412_DIGITAL_EXPOSURE_MIN) priv->exposure_time = IMX412_DIGITAL_EXPOSURE_MIN;
    if(priv->exposure_time > IMX412_DIGITAL_EXPOSURE_MAX) priv->exposure_time = IMX412_DIGITAL_EXPOSURE_MAX;

//    exposure = ((priv->exposure_time * 10*1000 / 16129) - ((19320/5792) / 10));
    exposure = ( ((u32)(priv->exposure_time) * 20000 / 16) - (1932 * 20000 / 5792) ) / 20000;
    if(exposure < 1) exposure = 1;

#if TRACE_IMX412_SET_EXPOSURE
    dev_err(dev, "%s(): exposure_time=%d, EXP=0x%x\n", __func__, priv->exposure_time, exposure);
#endif

    regs[0].addr = 0x0202;              // IMX412_EXPO_HIGH_ADDR
    regs[0].val = (exposure >> 8) & 0xff;
    regs[1].addr = 0x0203;              // IMX412_EXPO_LOW_ADDR
    regs[1].val =  exposure  & 0xff;
    regs[2].addr = IMX412_TABLE_END;
    regs[2].val = 0;

    err = imx412_write_table(priv, regs);
    if (err)
    {
      dev_err(dev, "%s: imx412_write_table() error=%d\n", __func__, err);
    }

#if IMX412_SET_EXPOSURE_MUTEX
    mutex_unlock(&priv->mutex);
#endif
    return err;
}

/****** imx412_set_frame_rate = Set frame rate = 08.2019 ****************/
static int imx412_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{

#define TRACE_IMX412_SET_FRAME_RATE     0   /* DDD - imx412_set_frame_rate() - trace */

//    struct camera_common_data *s_data = tc_dev->s_data;
//    struct device *dev = tc_dev->dev;
//    struct imx412 *priv = tc_dev->priv;
    int err = 0;

//    u32 frame_length;
//    u32 frame_rate = (int)val;
////    int i;
//
//    frame_length = (910 * 120) / frame_rate;
//
//#if TRACE_IMX412_SET_FRAME_RATE
//    dev_err(dev, "%s: frame_rate=%d: frame_length=%d\n", __func__, (int)val,frame_length);
//#endif
//
//    if(frame_length < IMX412_MIN_FRAME_LENGTH) frame_length = IMX412_MIN_FRAME_LENGTH;
//    if(frame_length > IMX412_MAX_FRAME_LENGTH) frame_length = IMX412_MAX_FRAME_LENGTH;
//
////      err = imx412_set_frame_length(tc_dev, frame_length);
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

/****** imx412_set_group_hold = Set group hold = 04.2020 ****************/
static int imx412_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
    /* imx412 does not support group hold */
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


static struct tegracam_ctrl_ops imx412_ctrl_ops = {
    .numctrls = ARRAY_SIZE(ctrl_cid_list),
    .ctrl_cid_list = ctrl_cid_list,
    .set_gain = imx412_set_gain,
    .set_exposure = imx412_set_exposure,
    .set_frame_rate = imx412_set_frame_rate,  // imx412_set_frame_length,
    .set_group_hold = imx412_set_group_hold,

};

/****** imx412_power_on = Power on = 08.2019 ****************************/
static int imx412_power_on(struct camera_common_data *s_data)
{
#define TRACE_IMX412_POWER_ON   0   /* DDD - imx412_power_on() - trace */

    int err = 0;
    struct camera_common_power_rail *pw = s_data->power;
    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = s_data->dev;


#if TRACE_IMX412_POWER_ON
    dev_info(dev, "%s: power on\n", __func__);
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
            goto imx412_avdd_fail;
    }

    if (pw->iovdd) {
        err = regulator_enable(pw->iovdd);
        if (err)
            goto imx412_iovdd_fail;
    }

// IMX
//    if (pw->dvdd) {
//        err = regulator_enable(pw->dvdd);
//        if (err)
//            goto imx412_dvdd_fail;
//    }

    usleep_range(10, 20);

skip_power_seqn:
    usleep_range(1, 2);
    if (gpio_is_valid(pw->pwdn_gpio))
        imx412_gpio_set(s_data, pw->pwdn_gpio, 1);

    /*
     * datasheet 2.9: reset requires ~2ms settling time
     * a power on reset is generated after core power becomes stable
     */
//    usleep_range(2000, 2010);
    usleep_range(23000, 23100);

//    msleep(20);

    if (gpio_is_valid(pw->reset_gpio))
        imx412_gpio_set(s_data, pw->reset_gpio, 1);

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

//imx412_dvdd_fail:
//    regulator_disable(pw->iovdd);

imx412_iovdd_fail:
    regulator_disable(pw->avdd);

imx412_avdd_fail:
    dev_err(dev, "%s failed.\n", __func__);

    return -ENODEV;
}

/****** imx412_power_off = Power off = 08.2019 **************************/
static int imx412_power_off(struct camera_common_data *s_data)
{

#define TRACE_IMX412_POWER_OFF  0   /* DDD - imx412_power_off() - trace */

    int err = 0;
    struct camera_common_power_rail *pw = s_data->power;
    struct device *dev = s_data->dev;
    struct camera_common_pdata *pdata = s_data->pdata;

#if TRACE_IMX412_POWER_OFF
    dev_info(dev, "%s: power off\n", __func__);
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
        imx412_gpio_set(s_data, pw->pwdn_gpio, 0);
    usleep_range(1, 2);
    if (gpio_is_valid(pw->reset_gpio))
        imx412_gpio_set(s_data, pw->reset_gpio, 0);

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

/****** imx412_power_put = Power put = 08.2019 **************************/
static int imx412_power_put(struct tegracam_device *tc_dev)
{
    struct camera_common_data *s_data = tc_dev->s_data;
    struct camera_common_power_rail *pw = s_data->power;
//    struct camera_common_pdata *pdata = s_data->pdata;
//    struct device *dev = tc_dev->dev;

    if (unlikely(!pw))
        return -EFAULT;


// IMX
    if (likely(pw->dvdd))
        regulator_disable(pw->dvdd);

    if (likely(pw->avdd))
        regulator_put(pw->avdd);

    if (likely(pw->iovdd))
        regulator_put(pw->iovdd);

    pw->dvdd = NULL;
    pw->avdd = NULL;
    pw->iovdd = NULL;

    if (likely(pw->reset_gpio))
        gpio_free(pw->reset_gpio);

    return 0;
}

/****** imx412_power_get = Power get = 08.2019 **************************/
static int imx412_power_get(struct tegracam_device *tc_dev)
{

#define TRACE_IMX412_POWER_GET  0   /* DDD - imx412_power_get() - trace */

    struct camera_common_data *s_data = tc_dev->s_data;
    struct camera_common_power_rail *pw = s_data->power;
    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = tc_dev->dev;
    struct clk *parent;
//    const char *mclk_name;
//    const char *parentclk_name;
    int err = 0;
//    int ret = 0;

#if TRACE_IMX412_POWER_GET
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
#if TRACE_IMX412_POWER_GET
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

/****** imx412_parse_dt = Parse DT = 08.2019 ****************************/
static struct camera_common_pdata *imx412_parse_dt(
    struct tegracam_device *tc_dev)
{

#define TRACE_IMX412_PARSE_DT   1   /* DDD - imx412_parse_dt() - trace */

// IMX219 : no parse clocks
//#define PARSE_CLOCKS        0   /* CCC - imx412_parse_dt() - parse clocks */
//#define PARSE_GPIOS         0   /* CCC - imx412_parse_dt() - parse GPIOss */

    struct device *dev = tc_dev->dev;
    struct device_node *node = dev->of_node;
    struct camera_common_pdata *board_priv_pdata;
//    const struct of_device_id *match;
    int gpio;
    int err = 0;
    struct camera_common_pdata *ret = NULL;
    int val = 0;

#if TRACE_IMX412_PARSE_DT
    dev_info(dev, "%s(): ...\n", __func__);
#endif

    if (!node)
        return NULL;

//    match = of_match_device(imx412_of_match, dev);
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
        goto error;
    }
    board_priv_pdata->reset_gpio = (unsigned int)gpio;

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

#if TRACE_IMX412_PARSE_DT
    dev_err(dev, "%s(): OK\n", __func__);
#endif

    return board_priv_pdata;

error:
    devm_kfree(dev, board_priv_pdata);
#if TRACE_IMX412_PARSE_DT
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

    struct imx412 *priv = (struct imx412 *)tegracam_get_privdata(tc_dev);
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

/****** imx412_set_mode = Set mode = 08.2019 ****************************/
static int imx412_set_mode(struct tegracam_device *tc_dev)
{

#define TRACE_IMX412_SET_MODE   1   /* DDD - imx412_set_mode() - trace */
// #define SKIP_IMX412_SET_MODE    1   /* DDD - imx412_set_mode() - skip setting mode */

//#if !SKIP_IMX412_SET_MODE
    struct imx412 *priv = (struct imx412 *)tegracam_get_privdata(tc_dev);
//#endif
    struct camera_common_data *s_data = tc_dev->s_data;
    struct device *dev = tc_dev->dev;
    const struct camera_common_colorfmt *colorfmt = s_data->colorfmt;
    int pix_fmt = colorfmt->pix_fmt;
    int err = 0;

/*............. Set new sensor mode */
// IMX412 sensor modes:
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
#if TRACE_IMX412_SET_MODE
      dev_err(dev, "%s(): New sensor_mode=%d (0=2-lanes, 1=4-lanes)d\n", __func__, sensor_mode);
#endif
      err = vc_mipi_reset(tc_dev, sensor_mode);
      if(err)
      {
          dev_err(dev, "%s(): vc_mipi_set_mode() error=%d\n", __func__, err);
      }
    } /* if(priv->sensor_mode != sensor_mode) */


// Get camera mode:
    if(s_data->fmt_width >= IMX412_DX && s_data->fmt_height >= IMX412_DY)
    {
      priv->cam_mode = IMX412_MODE_4032X3040;   // def. mode
    }

#if TRACE_IMX412_SET_MODE
{
    dev_err(dev, "%s(): fmt_width,fmt_height=%d,%d pix_fmt=0x%x '%c%c%c%c', cam_mode=%d\n", __func__,
                        s_data->fmt_width, s_data->fmt_height,
                        pix_fmt,
                        (char)((pix_fmt      ) & 0xFF),
                        (char)((pix_fmt >>  8) & 0xFF),
                        (char)((pix_fmt >> 16) & 0xFF),
                        (char)((pix_fmt >> 24) & 0xFF),
                        priv->cam_mode);
}
#endif

//#if TRACE_IMX412_SET_MODE
//    dev_err(dev, "%s(): cam_mode=%d\n", __func__, priv->cam_mode);
//#endif


// Set camera mode:
// Note: After each streaming stop, the sensor is initialized to default mode:
//       by vc_mipi_reset(), but this is not our default mode
//    if(priv->cam_mode != IMX412_MODE_4032X3040)
    {
      err = imx412_write_table(priv, imx412_mode_table[priv->cam_mode]);
      if (err)
      {
        dev_err(dev, "%s(): imx412_write_table() error=%d\n", __func__, err);
//        goto exit;
      }
      else
      {
#if TRACE_IMX412_SET_MODE
//        dev_err(dev, "%s(): imx412_write_table() OK, cam_mode=%d\n", __func__, priv->cam_mode);
#endif
      }
    }

    return err;
}

#if STOP_STREAMING_SENSOR_RESET     // [[[
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
#endif  // ]]]

/****** imx412_start_streaming = Start streaming = 08.2019 **************/
static int imx412_start_streaming(struct tegracam_device *tc_dev)
{

#define TRACE_IMX412_START_STREAMING    1   /* DDD - imx412_start_streaming() - trace */

    struct imx412 *priv = (struct imx412 *)tegracam_get_privdata(tc_dev);
//    struct imx412 *priv = (struct imx412 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;
    int err = 0;
    int ret = 0;

#if TRACE_IMX412_START_STREAMING
//    dev_err(dev, "%s():...\n", __func__);
#endif


// Set gain and exposure before streaming start
      imx412_set_gain(tc_dev, priv->digital_gain);
      imx412_set_exposure(tc_dev, priv->exposure_time);
      imx412_set_frame_rate(tc_dev, priv->frame_rate);
//      imx412_set_frame_length(tc_dev, priv->frame_length);

#if TRACE_IMX412_START_STREAMING
//    dev_info(dev, "%s(): Set frame_length=%d\n", __func__, IMX412_DEFAULT_FRAME_LENGTH);
#endif

//    imx412_set_gain(tc_dev, priv->digital_gain);
#if TRACE_IMX412_START_STREAMING
//    dev_info(dev, "%s(): Set gain=%d\n", __func__, IMX412_DEFAULT_GAIN);
#endif

//............... Set trigger mode: on/off
#if VC_CODE // [[[
    if(priv->sensor_ext_trig)
    {
//        u64 exposure = (priv->exposure_time * 10000) / 185185;
        u64 exposure = (priv->exposure_time * (priv->sen_clk/1000000)); // sen_clk default=54Mhz imx183=72Mhz

        int addr = 0x0108; // ext trig enable
        // data =      1; // external trigger enable
        // data =      4; // TEST external self trigger enable
        int data = priv->sensor_ext_trig; // external trigger enable
        ret = reg_write(priv->rom, addr, data);

        addr = 0x0103; // flash output enable
//        data =      1; // flash output enable
        data = priv->flash_output; // flash output enable
        ret = reg_write(priv->rom, addr, data);

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
    err = imx412_write_table(priv, imx412_mode_table[IMX412_MODE_START_STREAM]);
    if (err) {
//        mutex_unlock(&priv->streaming_lock);
        goto exit;
    } else {
        priv->streaming = true;
//        mutex_unlock(&priv->streaming_lock);
    }

exit:
#if TRACE_IMX412_START_STREAMING
    dev_err(dev, "%s(): err=%d\n", __func__, err);
#endif

    return err;

}

/****** imx412_stop_streaming = Stop streaming = 08.2019 ****************/
static int imx412_stop_streaming(struct tegracam_device *tc_dev)
{

#define TRACE_IMX412_STOP_STREAMING                1    /* DDD - imx412_stop_streaming() - trace */
#define TRACE_IMX412_STOP_STREAMING_DISABLE_RESET  0    /* DDD - imx412_stop_streaming() - disable VC MIPI reset */
//#define TRACE_IMX412_STOP_STREAMING_VC             0    /* DDD - imx412_stop_streaming() - trace VC stuff */

//    struct camera_common_data *s_data = tc_dev->s_data;
    struct imx412 *priv = (struct imx412 *)tegracam_get_privdata(tc_dev);
//    struct imx412 *priv = (struct imx412 *)tc_dev->priv;
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

#if TRACE_IMX412_STOP_STREAMING
//    dev_err(dev, "%s():\n", __func__);
#endif

    err = imx412_write_table(priv, imx412_mode_table[IMX412_MODE_STOP_STREAM]);
    if (err)
    {
//#if TRACE_IMX412_STOP_STREAMING
      dev_err(dev, "%s(): imx412_write_table() err=%d\n", __func__, err);
//#endif
//        mutex_unlock(&priv->streaming_lock);
        goto exit;
    }
    else
    {
#if TRACE_IMX412_STOP_STREAMING_DISABLE_RESET  // [[[
        dev_err(dev, "%s(): Disabled VC sensor reset\n", __func__);
#else  // ]]] [[[
        priv->streaming = false;
        err = vc_mipi_reset(tc_dev, sensor_mode);
        if(err)
        {
          dev_err(dev, "%s(): VC MIPI sensor reset: err=%d\n", __func__, err);
        }

#endif // ]]] - VC reset
    }

    usleep_range(50000, 51000);

exit:
#if TRACE_IMX412_STOP_STREAMING
    dev_err(dev, "%s(): err=%d\n\n", __func__, err);
#endif

//    mutex_unlock(&priv->mutex);
    return err;

}

static struct camera_common_sensor_ops imx412_common_ops = {
    .numfrmfmts = ARRAY_SIZE(imx412_frmfmt),
    .frmfmt_table = imx412_frmfmt,
    .power_on = imx412_power_on,
    .power_off = imx412_power_off,
    .write_reg = imx412_write_reg,
    .read_reg = imx412_read_reg,
    .parse_dt = imx412_parse_dt,
    .power_get = imx412_power_get,
    .power_put = imx412_power_put,
    .set_mode = imx412_set_mode,
    .start_streaming = imx412_start_streaming,
    .stop_streaming = imx412_stop_streaming,
};

/****** imx412_probe_vc_rom = Probe VC ROM = 08.2019 ********************/
static struct i2c_client * imx412_probe_vc_rom(struct i2c_adapter *adapter, u8 addr)
{
    struct i2c_board_info info = {
        I2C_BOARD_INFO("dummy", addr),
    };
    unsigned short addr_list[2] = { addr, I2C_CLIENT_END };

    return i2c_new_probed_device(adapter, &info, addr_list, NULL);
}

/****** imx412_board_setup = Board setup = 08.2019 **********************/
static int imx412_board_setup(struct imx412 *priv)
{

#define DUMP_CTL_REG_DATA       0   /* DDD - imx412_board_setup() - dump module control registers 0x100-0x108 (I2C addr=0x10) */
#define DUMP_HWD_DESC_ROM_DATA  0   /* DDD - imx412_board_setup() - dump Hardware Desriptor ROM data (I2C addr=0x10) */
#define DUMP_V4L_PARAMS         1   /* DDD - imx412_board_setup() - dump V4L params */
#define DUMP_IMX412_REGS        0   /* DDD - imx412_board_setup() - dump IMX412 regs */

    struct camera_common_data *s_data = priv->s_data;
    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = s_data->dev;
    struct tegracam_device  *tc_dev = priv->tc_dev;
//    u8 reg_val[2];
//    bool eeprom_ctrl = 0;
    int err = 0;

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
            dev_err(dev, "error turning on mclk (%d)\n", err);
            goto done;
        }
    }

    err = imx412_power_on(s_data);
    if (err) {
        dev_err(dev, "error during power on sensor (%d)\n", err);
        goto err_power_on;
    }

/*----------------------------------------------------------------------*/
#if VC_CODE    // [[[ - new VC code
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
    priv->rom = imx412_probe_vc_rom(adapter,0x10);

    if ( priv->rom )
    {
//        static int i=1;
//        int addr,reg,data;
        int addr,reg;

#if DUMP_HWD_DESC_ROM_DATA      /* [[[ - dump Hardware Desriptor ROM data */
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
//            dev_err(&client->dev, "addr=0x%04x reg=0x%02x\n",addr+0x1000,reg);
            dev_err(&client->dev, "addr=0x%04x reg=0x%04x\n",addr+0x1000-1,sval);
          }
          else
          {
            sval = reg;
          }
}
#endif
        } /* for (addr=0; addr<sizeof(priv->rom_table); addr++) */

        dev_info(&client->dev, "%s(): VC FPGA found!\n", __func__);

        dev_info(&client->dev, "[ MAGIC  ] [ %s ]\n",
                priv->rom_table.magic);

        dev_info(&client->dev, "[ MANUF. ] [ %s ] [ MID=0x%04x ]\n",
                priv->rom_table.manuf,
                priv->rom_table.manuf_id);

        dev_info(&client->dev, "[ SENSOR ] [ %s %s ]\n",
                priv->rom_table.sen_manuf,
                priv->rom_table.sen_type);

        dev_info(&client->dev, "[ MODULE ] [ ID=0x%04x ] [ REV=0x%04x ]\n",
                priv->rom_table.mod_id,
                priv->rom_table.mod_rev);

        dev_info(&client->dev, "[ MODES  ] [ NR=0x%04x ] [ BPM=0x%04x ]\n",
                priv->rom_table.nr_modes,
                priv->rom_table.bytes_per_mode);

// Reset VC MIPI sensor: This initializes IMX registers with def. values (shown by above test)
        priv->sensor_mode = sensor_mode;
        err = vc_mipi_reset(tc_dev, sensor_mode);
        if(err)
        {
          dev_err(dev, "%s(): vc_mipi_reset() error=%d\n", __func__, err);
        }

#if DUMP_CTL_REG_DATA
{
        int i;
        int reg_val[10];

        dev_info(&client->dev, "%s(): Module controller registers (0x10):\n", __func__);

//        addr = 0x100;
        i = 0;
        for(addr=0x100; addr<=0x108; addr++)
        {
          reg_val[i] = reg_read(priv->rom, addr);
//          dev_info(&client->dev, "0x%04x: %02x\n", addr, reg_val[i]);
          i++;
        }

        dev_info(&client->dev, "0x100-103: %02x %02x %02x %02x\n", reg_val[0],reg_val[1],reg_val[2],reg_val[3]);
        dev_info(&client->dev, "0x104-108: %02x %02x %02x %02x %02x\n", reg_val[4],reg_val[5],reg_val[6],reg_val[7],reg_val[8]);

}
#endif

    }
    else
    {
        dev_err(&client->dev, "%s(): Error !!! VC FPGA not found !!!\n", __func__);
        return -EIO;
    }
#endif  // ]]] - end of VC_CODE


#if DUMP_IMX412_REGS
{
    u16 addr;
    u8  low;
    u8  high;
    int sval;

// horizontal start high = 0x0344
// horizontal start low  = 0x0345
    addr = 0x0344;
    imx412_read_reg(s_data, addr, &high);
    addr = 0x0345;
    imx412_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX412: horizontal start = %d\n", __func__, sval);

// vertical start high   = 0x0346
// vertical start low    = 0x0347
    addr = 0x0346;
    imx412_read_reg(s_data, addr, &high);
    addr = 0x0347;
    imx412_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX412: vertical start = %d\n", __func__, sval);

// horizontal end H      = 0x0348
// horizontal end L      = 0x0349
    addr = 0x0348;
    imx412_read_reg(s_data, addr, &high);
    addr = 0x0349;
    imx412_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX412: horizontal end   = %d\n", __func__, sval);

// vertical   end H      = 0x034A
// vertical   end L      = 0x034B
    addr = 0x034A;
    imx412_read_reg(s_data, addr, &high);
    addr = 0x034B;
    imx412_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX412: vertical end   = %d\n", __func__, sval);

// hor. output width H   = 0x034C
// hor. output width L   = 0x034D
    addr = 0x034C;
    imx412_read_reg(s_data, addr, &high);
    addr = 0x034D;
    imx412_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX412: output width   = %d\n", __func__, sval);

// ver. output height H  = 0x034E
// ver. output height L  = 0x034F
    addr = 0x034E;
    imx412_read_reg(s_data, addr, &high);
    addr = 0x034F;
    imx412_read_reg(s_data, addr, &low);
    sval = (high << 8) | (low & 0xFF);
    dev_err(dev, "%s(): IMX412: output height  = %d\n", __func__, sval);


// embedded data line control register
    addr = 0xBCF1;

/*
    low = 2;
    imx412_write_reg(s_data, addr, low);
    dev_err(dev, "%s(): IMX412: Set: embedded data line control = 2\n", __func__);
*/

    imx412_read_reg(s_data, addr, &high);
    dev_err(dev, "%s(): IMX412: embedded data line control = 0x%x\n", __func__, (int)high);




// exposure H            = 0x0202
// exposure M            = 0x0203
// exposure L            = 0x0200
// gain high             = 0x0204
// gain low              = 0x0205
}
#endif

//err_reg_probe:
    imx412_power_off(s_data);

err_power_on:
    if (pdata->mclk_name)
        camera_common_mclk_disable(s_data);

done:
    return err;
}

/****** imx412_open = Open device = 08.2019 *****************************/
static int imx412_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    dev_dbg(&client->dev, "%s:\n", __func__);
    return 0;
}

static const struct v4l2_subdev_internal_ops imx412_subdev_internal_ops = {
    .open = imx412_open,
};

/****** imx412_probe = Probe = 08.2019 **********************************/
static int imx412_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
    struct device *dev = &client->dev;
//    struct device_node *node = client->dev.of_node;
    struct tegracam_device *tc_dev;
    struct imx412 *priv;
    int err;
    const struct of_device_id *match;

//    dev_info(dev, "%s(): Probing v4l2 sensor at addr 0x%0x\n", __func__, client->addr); // , __DATE__, __TIME__);
    dev_err(dev, "%s(): Probing v4l2 sensor at addr 0x%0x - %s/%s\n", __func__, client->addr, __DATE__, __TIME__);

    match = of_match_device(imx412_of_match, dev);
    if (!match) {
        dev_err(dev, "No device match found\n");
        return -ENODEV;
    }
    dev_info(dev, "%s(): of_match_device() OK\n", __func__);

    if (!IS_ENABLED(CONFIG_OF) || !client->dev.of_node)
        return -EINVAL;

    priv = devm_kzalloc(dev,
                sizeof(struct imx412), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    tc_dev = devm_kzalloc(dev,
                sizeof(struct tegracam_device), GFP_KERNEL);
    if (!tc_dev)
        return -ENOMEM;

    dev_info(dev, "%s(): devm_kzalloc() OK\n", __func__);

    priv->i2c_client = tc_dev->client = client;
    tc_dev->dev = dev;
    strncpy(tc_dev->name, "imx412", sizeof(tc_dev->name));
    tc_dev->dev_regmap_config = &imx412_regmap_config;
    tc_dev->sensor_ops = &imx412_common_ops;
    tc_dev->v4l2sd_internal_ops = &imx412_subdev_internal_ops;
    tc_dev->tcctrl_ops = &imx412_ctrl_ops;

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
// 5693    mutex_init(&priv->streaming_lock);

//    priv->frame_length = IMX412_DEFAULT_FRAME_LENGTH;
    priv->digital_gain = IMX412_DIGITAL_GAIN_DEFAULT;
    priv->exposure_time = IMX412_DIGITAL_EXPOSURE_DEFAULT;
    priv->frame_rate = IMX412_FRAME_RATE_DEFAULT;
    priv->cam_mode = IMX412_MODE_4032X3040;   // default camera mode

    priv->sensor_ext_trig = 0;    // ext. trigger flag: 0=no, 1=yes
    priv->sen_clk = 54000000;     // clock-frequency: default=54Mhz imx183=72Mhz
    priv->flash_output = flash_output;

//colorfmt
// camera_common.c:
//   camera_common_s_fmt()
//   camera_common_g_fmt()

    err = imx412_board_setup(priv);
    if (err) {
        dev_err(dev, "board setup failed\n");
        return err;
    }
    dev_info(dev, "%s(): imx412_board_setup() OK\n", __func__);

    err = tegracam_v4l2subdev_register(tc_dev, true);
    if (err) {
        dev_err(dev, "tegra camera subdev registration failed\n");
        return err;
    }
    dev_info(dev, "%s(): tegracam_v4l2subdev_register() OK\n", __func__);

    dev_err(dev, "%s(): Detected imx412 sensor - %s/%s\n", __func__, __DATE__, __TIME__);

    return 0;
}

/****** imx412_remove = Remove = 08.2019 ********************************/
static int imx412_remove(struct i2c_client *client)
{
    struct camera_common_data *s_data = to_camera_common_data(&client->dev);
    struct imx412 *priv = (struct imx412 *)s_data->priv;

    tegracam_v4l2subdev_unregister(priv->tc_dev);
//    ov5693_power_put(priv->tc_dev);
    tegracam_device_unregister(priv->tc_dev);

//    mutex_destroy(&priv->streaming_lock);

    return 0;
}

static const struct i2c_device_id imx412_id[] = {
    { "imx412", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, imx412_id);

/****** i2c_driver = I2C driver = 08.2019 *******************************/
static struct i2c_driver imx412_i2c_driver = {
    .driver = {
        .name = "imx412",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(imx412_of_match),
    },
    .probe = imx412_probe,
    .remove = imx412_remove,
    .id_table = imx412_id,
};
module_i2c_driver(imx412_i2c_driver);

MODULE_DESCRIPTION("Media Controller driver for IMX412");
//MODULE_AUTHOR("NVIDIA Corporation");
MODULE_AUTHOR("Vision Components GmbH <mipi-tech@vision-components.com>");
MODULE_LICENSE("GPL v2");
