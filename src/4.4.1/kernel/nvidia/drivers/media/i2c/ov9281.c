/*
 * ov9281.c - ov9281 sensor driver
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

#define VC_CODE             1   /* CCC - ov9281.c - enable code for VC MIPI camera  */
#define VC_SENSOR_MODE      0   /* CCC - ov9281.c - def VC sensor mode: 0=10-bit, 1=8-bit stream, 2=10-bit trig, 3=8-bit trig */

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
//#include <media/ov9281.h>

#include "../platform/tegra/camera/camera_gpio.h"
//#include "ov9281.h"
//#include "ov9281_mode_tbls.h"

//#define CREATE_TRACE_POINTS
//#include <trace/events/ov5693.h>



// Min/max values in device-tree, shown by v4l2-ctl --all  (us ?)
#define OV9281_DIGITAL_EXPOSURE_MIN           8  //     8721 ns // 0x00000010 (16)       (16 * 8721)/16     =    8721
#define OV9281_DIGITAL_EXPOSURE_MAX        7719  //  7718085 ns // 0x00003750 (14160)    (14160 * 8721)/16  = 7718085
#define OV9281_DIGITAL_EXPOSURE_DEFAULT    2233  //  2232576 ns // 0x00001000 (4096)     (4096 * 8721)/16   = 2232576


#define OV9281_DIGITAL_GAIN_MIN     0x00
#define OV9281_DIGITAL_GAIN_MAX     0xfe  // 254
#define OV9281_DIGITAL_GAIN_DEFAULT 0x10  // 16

#define OV9281_MAX_COARSE_DIFF      25
#define OV9281_MIN_FRAME_LENGTH     0x0001
#define OV9281_MAX_FRAME_LENGTH     (0xffff)
#define OV9281_DEFAULT_FRAME_LENGTH (0x0FA0)    // 4000 : increase maximum exposure time (frame_length - 25 row periods)


#define OV9281_MIN_FRAME_RATE    10
#define OV9281_MAX_FRAME_RATE    120

//#define OV9281_MIN_EXPOSURE_COARSE  (0x0002)
//#define OV9281_MAX_EXPOSURE_COARSE  (OV9281_MAX_FRAME_LENGTH-OV9281_MAX_COARSE_DIFF)
//#define OV9281_DEFAULT_EXPOSURE_COARSE  0x0A00    // 0x00002A90

/* OV9281 Registers */
#define OV9281_TIMING_VTS_HIGH_ADDR 0x380E
#define OV9281_TIMING_VTS_LOW_ADDR  0x380F

#define OV9281_EXPO_HIGH_ADDR       0x3500
#define OV9281_EXPO_MID_ADDR        0x3501
#define OV9281_EXPO_LOW_ADDR        0x3502

#define OV9281_GAIN_SHIFT_ADDR      0x3507
#define OV9281_GAIN_HIGH_ADDR       0x3508
#define OV9281_GAIN_LOW_ADDR        0x3509

#define OV9281_GROUP_HOLD_ADDR      0x3208
#define OV9281_GROUP_HOLD_START     0x00
#define OV9281_GROUP_HOLD_END       0x10

#define OV9281_GROUP_HOLD_LAUNCH_LBLANK 0x60
#define OV9281_GROUP_HOLD_LAUNCH_VBLANK 0xA0
//#define OV9281_GROUP_HOLD_LAUNCH_IMMED  0xE0
#define OV9281_GROUP_HOLD_BANK_0    0x00
#define OV9281_GROUP_HOLD_BANK_1    0x01


#define OV9281_TABLE_WAIT_MS    0
#define OV9281_TABLE_END        1

#define ov9281_reg struct reg_8

enum {
    OV9281_MODE_1280X800,
//    OV9281_MODE_1024X1000,
//    OV9281_MODE_1280X720,
//    OV9281_MODE_640X400,
    OV9281_MODE_START_STREAM,
    OV9281_MODE_STOP_STREAM,
};

//enum {
//    OV9281_FSYNC_NONE,
//    OV9281_FSYNC_MASTER,
//    OV9281_FSYNC_SLAVE,
//};

static const ov9281_reg ov9281_start[] = {
    { 0x0100, 0x01 },
    { OV9281_TABLE_END, 0x00 }
};

static const ov9281_reg ov9281_stop[] = {
    { 0x0100, 0x00 },
    { OV9281_TABLE_END, 0x00 }
};

//static const ov9281_reg ov9281_fsync_master[] = {
//    { 0x3006, 0x02 }, /* fsin pin out */
//    { 0x3823, 0x00 },
//    { OV9281_TABLE_WAIT_MS, 66 },
//    { OV9281_TABLE_END, 0x00 }
//};
//
//static const ov9281_reg ov9281_fsync_slave[] = {
//    { 0x3006, 0x00 }, /* fsin pin in */
//    { 0x3007, 0x02 },
//    { 0x38b3, 0x07 },
//    { 0x3885, 0x07 },
//    { 0x382b, 0x5a },
//    { 0x3670, 0x68 },
//    { 0x3740, 0x01 },
//    { 0x3741, 0x00 },
//    { 0x3742, 0x08 },
//    { 0x3823, 0x30 }, /* ext_vs_en, r_init_man */
//    { 0x3824, 0x00 }, /* CS reset value on fsin */
//    { 0x3825, 0x08 },
//    { OV9281_TABLE_WAIT_MS, 66 },
//    { OV9281_TABLE_END, 0x00 }
//};

static const ov9281_reg ov9281_mode_1280x800_26MhzMCLK[] = {
    { OV9281_TABLE_END, 0x00 }
};


//static const ov9281_reg ov9281_mode_1280x720_26MhzMCLK[] = {
//    { OV9281_TABLE_END, 0x00 }
//};
//
//static const ov9281_reg ov9281_mode_640x400_26MhzMCLK[] = {
//    { OV9281_TABLE_END, 0x00 }
//};

static const ov9281_reg *ov9281_mode_table[] = {
    [OV9281_MODE_1280X800] = ov9281_mode_1280x800_26MhzMCLK,
//    [OV9281_MODE_1280X720] = ov9281_mode_1280x720_26MhzMCLK,
//    [OV9281_MODE_640X400] = ov9281_mode_640x400_26MhzMCLK,
    [OV9281_MODE_START_STREAM] = ov9281_start,
    [OV9281_MODE_STOP_STREAM] = ov9281_stop,
};

//static const ov9281_reg *ov9281_fsync_slave_mode_table[] = {
//    [OV9281_MODE_1280X800] = ov9281_mode_1280x800_26MhzMCLK_fsync_slave,
//    [OV9281_MODE_1280X720] = ov9281_mode_1280x720_26MhzMCLK_fsync_slave,
//    [OV9281_MODE_640X400] = ov9281_mode_640x400_26MhzMCLK_fsync_slave,
//};
//
//static const ov9281_reg *ov9281_fsync_table[] = {
//    [OV9281_FSYNC_NONE] = NULL,
//    [OV9281_FSYNC_MASTER] = ov9281_fsync_master,
//    [OV9281_FSYNC_SLAVE] = ov9281_fsync_slave,
//};

static const int ov9281_30fps[] = {
    30,
};

static const struct camera_common_frmfmt ov9281_frmfmt[] = {
    { { 1280, 800 }, ov9281_30fps, ARRAY_SIZE(ov9281_30fps), 0,
      OV9281_MODE_1280X800 },
//    { { 1024, 1000 }, ov9281_60fps, ARRAY_SIZE(ov9281_60fps), 0,
//      OV9281_MODE_1024X1000 },

//    { { 1280, 720 }, ov9281_60fps, ARRAY_SIZE(ov9281_60fps), 0,
//      OV9281_MODE_1280X720 },
//    { { 640, 400 }, ov9281_60fps, ARRAY_SIZE(ov9281_60fps), 0,
//      OV9281_MODE_640X400 },
};


// H info:
//
// camera_common.h
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



static const struct of_device_id ov9281_of_match[] = {
    { .compatible = "nvidia,ov9281", },
    { },
};
MODULE_DEVICE_TABLE(of, ov9281_of_match);


enum ov_model {
    OV_MODEL_NONE = 0,
    OV9281_MODEL,
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
    char mode3[16];
    char mode4[16];
};

struct ov9281 {
    struct i2c_client          *i2c_client;
    struct v4l2_subdev         *subdev;
    u16                         fine_integ_time;
    u32                         frame_length;
    u32                         digital_gain;
    u32                         exposure_time;
    struct camera_common_i2c    i2c_dev;
    struct camera_common_data  *s_data;
    struct tegracam_device     *tc_dev;

    struct mutex        streaming_lock;
    bool                streaming;

    s32                 group_hold_prev;
    bool                group_hold_en;

#if VC_CODE     // [[[ - VC code
    struct i2c_client   *rom;
    struct vc_rom_table rom_table;
    enum   ov_model model;  // camera model
    int sensor_ext_trig;    // ext. trigger flag: 0=no, 1=yes
    int flash_output;       // flash output enable
    int sen_clk;            // sen_clk default=54Mhz imx183=72Mhz
    int sensor_mode;        // sensor mode
    int num_lanes;          // # of data lanes: 1, 2, 4
#endif  // ]]]
};

static const struct regmap_config ov9281_regmap_config = {
    .reg_bits = 16,
    .val_bits = 8,
//    .cache_type = REGCACHE_RBTREE,
//    .use_single_rw = true,
};

#if VC_CODE     // [[[ - VC code
  static int sensor_mode = VC_SENSOR_MODE;      // sensor mode:
                                                //      0x00 : 10bit streaming
                                                //      0x01 : 8bit  streaming
                                                //      0x02 : 10bit external trigger
                                                //      0x03 : 8bit  external trigger
                                                //      0xF0 : 10bit test image
                                                //      0xF1 : 8bit  test image
  static int flash_output  = 0;       // flash output enable

//#if OV9281_DEFAULT_DATAFMT == MEDIA_BUS_FMT_SRGGB8_1X8 || OV9281_DEFAULT_DATAFMT == MEDIA_BUS_FMT_Y8_1X8
// static int sensor_mode = 1;      // 1: 8-bit stream
// static int sensor_mode = 0;      // 0: 10-bit stream
//#else
//#endif
//module_param(sensor_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//MODULE_PARM_DESC(sensor_mode, "VC Sensor Mode: 0=10bit_stream 1=8bit_stream 2=10bit_ext_trig 3=8bit_ext_trig");


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
//               const struct ov9281_reg table[])
//{
//    const struct ov9281_reg *reg;
//    int ret;
//
//    for (reg = table; reg->addr != OV9281_TABLE_END; reg++) {
//        ret = reg_write(client, reg->addr, reg->val);
//        if (ret < 0)
//            return ret;
//    }
//
//    return 0;
//}

#endif  // ]]] - VC_CODE


/****** ov9281_read_reg = Read register = 08.2019 ***********************/
static int ov9281_read_reg(struct camera_common_data *s_data,
    u16 addr, u8 *val)
{
    int err = 0;
    u32 reg_val = 0;

    err = regmap_read(s_data->regmap, addr, &reg_val);
    *val = reg_val & 0xff;

    return err;
}

/****** ov9281_write_reg = Write register = 08.2019 *********************/
static int ov9281_write_reg(struct camera_common_data *s_data, u16 addr, u8 val)
{
    int err = 0;

    err = regmap_write(s_data->regmap, addr, val);
    if (err)
        dev_err(s_data->dev, "%s: i2c write failed, 0x%x = %x",
            __func__, addr, val);

    return err;
}

/****** ov9281_write_table = Write table = 08.2019 **********************/
static int ov9281_write_table(struct ov9281 *priv, const ov9281_reg table[])
{
    return regmap_util_write_table_8(priv->s_data->regmap, table, NULL, 0,
        OV9281_TABLE_WAIT_MS, OV9281_TABLE_END);
}

/****** ov9281_gpio_set = GPIO set = 08.2019 ****************************/
static void ov9281_gpio_set(struct camera_common_data *s_data,
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

/****** ov9281_set_group_hold = Set group hold = 08.2019 ****************/
static int ov9281_set_group_hold(struct tegracam_device *tc_dev, bool val)
{

    struct device *dev = tc_dev->dev;
    struct ov9281 *priv = tc_dev->priv;
    int gh_prev = switch_ctrl_qmenu[priv->group_hold_prev];
    int err = 0;

    if (priv->group_hold_en == true && gh_prev == SWITCH_OFF) {
        camera_common_i2c_aggregate(&priv->i2c_dev, true);
        /* enter group hold */
        /* group hold start */
        err = ov9281_write_reg(priv->s_data, OV9281_GROUP_HOLD_ADDR,
                               (OV9281_GROUP_HOLD_START | OV9281_GROUP_HOLD_BANK_0));
//        val);
        if (err)
            goto fail;
        priv->group_hold_prev = 1;

        dev_dbg(dev, "%s: enter group hold\n", __func__);
    } else if (priv->group_hold_en == false && gh_prev == SWITCH_ON) {
        /* leave group hold */
        err = ov9281_write_reg(priv->s_data, OV9281_GROUP_HOLD_ADDR,
                               (OV9281_GROUP_HOLD_END | OV9281_GROUP_HOLD_BANK_0));
//                       0x11);
        if (err)
            goto fail;

        err = ov9281_write_reg(priv->s_data, OV9281_GROUP_HOLD_ADDR,
                               (OV9281_GROUP_HOLD_LAUNCH_VBLANK | OV9281_GROUP_HOLD_BANK_0));
//                              0x61);
        if (err)
            goto fail;

        camera_common_i2c_aggregate(&priv->i2c_dev, false);

        priv->group_hold_prev = 0;

        dev_dbg(dev, "%s: leave group hold\n", __func__);
    }

    return 0;

fail:
    dev_dbg(dev, "%s: Group hold control error\n", __func__);
    return err;
}

/****** ov9281_set_frame_length = Set frame length = 08.2019 ************/
static int ov9281_set_frame_length(struct tegracam_device *tc_dev, s64 val)
//static int ov9281_set_frame_length(struct ov9281 *priv, s64 val)
{

#define TRACE_OV9281_SET_FRAME_LENGTH   1   /* DDD - ov9281_set_frame_length() - trace */
#define OV9281_SET_FRAME_GROUP_HOLD     1   /* CCC - ov9281_set_frame_length() - set group hold */

//    struct camera_common_data *s_data = tc_dev->s_data;
    struct ov9281 *priv = (struct ov9281 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;
//    struct device *dev = &priv->i2c_client->dev;

    ov9281_reg regs[5];
    u16 frame_length;
    int err = 0;

    /*
     * This is a workaround for nvbug 1865041, where setting the VTS
     * timing registers when the sensor is set up for fsync master or
     * slave leads to streaming instability.
     */
//    if (priv->fsync != OV9281_FSYNC_NONE)
//        return 0;

    frame_length = (u16)val;

#if TRACE_OV9281_SET_FRAME_LENGTH
    dev_err(dev, "%s(): frame_length = %d\n", __func__, (int)frame_length);
#endif

    regs[0].addr = OV9281_TIMING_VTS_HIGH_ADDR;         // OV9281_TIMING_VTS_HIGH_ADDR = 0x380E
    regs[0].val = (frame_length >> 8) & 0xff;
    regs[1].addr = OV9281_TIMING_VTS_LOW_ADDR;          // OV9281_TIMING_VTS_LOW_ADDR  = 0x380F
    regs[1].val = (frame_length) & 0xff;
    regs[2].addr = OV9281_TABLE_END;
    regs[2].val = 0;

//    if (priv->fsync == OV9281_FSYNC_SLAVE) {
//        regs[2].addr = OV9281_TIMING_RST_FSIN_HIGH_ADDR;
//        regs[2].val = ((frame_length - 4) >> 8) & 0xff;
//        regs[3].addr = OV9281_TIMING_RST_FSIN_LOW_ADDR;
//        regs[3].val = (frame_length - 4) & 0xff;
//        regs[4].addr = OV9281_TABLE_END;
//        regs[4].val = 0;
//    }

#if OV9281_SET_FRAME_GROUP_HOLD
    if (!priv->group_hold_prev)
        ov9281_set_group_hold(tc_dev, 1);
//    ov9281_set_group_hold(priv);
#endif

    err = ov9281_write_table(priv, regs);
    if (err)
        goto fail;

//    priv->frame_period_ms = (frame_length * 1000) /
//                OV9281_FRAME_LENGTH_1SEC;
//    dev_info(dev, "%s: frame_period_ms: %d\n",  __func__, priv->frame_period_ms);

    return 0;

fail:
//    dev_dbg(dev, "%s: FRAME_LENGTH control error=%d\n", __func__, err);
    dev_err(dev, "%s: error=%d\n", __func__, err);
    return err;
}

/****** ov9281_set_frame_rate = Set frame rate = 08.2019 ****************/
static int ov9281_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{

#define TRACE_OV9281_SET_FRAME_RATE     1   /* DDD - ov9281_set_frame_rate() - trace */

//    struct camera_common_data *s_data = tc_dev->s_data;
    struct device *dev = tc_dev->dev;
    struct ov9281 *priv = tc_dev->priv;
//    const struct sensor_mode_properties *mode =  &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];
//    struct sensor_mode_properties *mode =  &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];

//    ov9281_reg reg_list[2];
    int err = 0;
    u32 frame_length;
    u32 frame_rate = (int)val/1000000;   // in frames/sec
//    int i;


// Get params from device-tree
//   struct sensor_mode_properties *mode =  &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];
//    dev_err(dev, "%s: mode->control_properties:\n", __func__);
//    dev_err(dev, "%s: min_gain_val,max_gain_val =%d,%d\n", __func__,
//         mode->control_properties.min_gain_val,
//         mode->control_properties.max_gain_val);
//         mode->control_properties.default_gain);
//
//         mode->control_properties.min_framerate,
//         mode->control_properties.max_framerate,
//         mode->control_properties.default_framerate);
//
//         mode->control_properties.min_exp_time.val,
//         mode->control_properties.max_exp_time.val,
//         mode->control_properties.default_exp_time.val);

// Done by ov9281_set_frame_length
//    if (!priv->group_hold_prev)
//    {
//#if TRACE_OV9281_SET_FRAME_RATE
//      dev_err(dev, "%s: ov9281_set_group_hold()\n", __func__);
//#endif
//        ov9281_set_group_hold(tc_dev, 1);
//    }

//    frame_length = mode->signal_properties.pixel_clock.val *
//                   mode->control_properties.framerate_factor /
//                   mode->image_properties.line_length / val;


    if(frame_rate < OV9281_MIN_FRAME_RATE) frame_rate = OV9281_MIN_FRAME_RATE;
    if(frame_rate > OV9281_MAX_FRAME_RATE) frame_rate = OV9281_MAX_FRAME_RATE;

/*
* frame_length=4000 : 28 fps (frames/sec)
* frame_length=2000 : 58 fps
* frame_length=1000 : 115 fps
* frame_length=910  : 120 fps
*/
    frame_length = (910 * 120) / frame_rate;

#if TRACE_OV9281_SET_FRAME_RATE
{
    struct camera_common_data   *s_data = tc_dev->s_data;
    struct sensor_mode_properties *mode   = &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];
//    dev_err(dev, "%s: frame_length = (pixel_clock=%d * framerate_factor=%d) / line_length=%d / val=%d\n", __func__,
//                  (int)mode->signal_properties.pixel_clock.val,
//                  (int)mode->control_properties.framerate_factor,
//                  (int)mode->image_properties.line_length,
//                  (int)val);

    dev_err(dev, "%s: frame_rate=%d fps: frame_length=%d\n", __func__, (int)frame_rate,frame_length);
    dev_err(dev, "%s: DT min,max,default_framerate=%d,%d,%d\n", __func__,
         (int)mode->control_properties.min_framerate,
         (int)mode->control_properties.max_framerate,
         (int)mode->control_properties.default_framerate);
// ???
//    mode->control_properties.default_framerate = 33;
}
#endif

    if(frame_length < OV9281_MIN_FRAME_LENGTH) frame_length = OV9281_MIN_FRAME_LENGTH;
    if(frame_length > OV9281_MAX_FRAME_LENGTH) frame_length = OV9281_MAX_FRAME_LENGTH;

// ???
      err = ov9281_set_frame_length(tc_dev, frame_length);
      if (err)
         goto fail;

//    ov9281_get_frame_length_regs(reg_list, frame_length);
//    for (i = 0; i < 2; i++) {
//        err = ov9281_write_reg(s_data, reg_list[i].addr,
//             reg_list[i].val);
//        if (err)
//            goto fail;
//    }

    priv->frame_length = frame_length;

    return 0;

fail:
    dev_err(dev, "%s: error=%d\n", __func__, err);
    return err;
}

/****** ov9281_set_gain = Set gain = 08.2019 ****************************/
static int ov9281_set_gain(struct tegracam_device *tc_dev, s64 val)
{

#define TRACE_OV9281_SET_GAIN        1   /* DDD - ov9281_set_gain() - trace */
#define OV9281_SET_GAIN_GROUP_HOLD   1   /* CCC - ov9281_set_gain() - enable group hold bef reg write */

//    struct camera_common_data *s_data = tc_dev->s_data;
    struct ov9281 *priv = (struct ov9281 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;

//    const struct sensor_mode_properties *mode =
//        &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];
//    ov9281_reg reg_list[2];
//    int err;
//    u16 gain;
//    int i;



    ov9281_reg regs[4];
    s64 gain = val;
    int err;



    if (gain < OV9281_DIGITAL_GAIN_MIN) gain = OV9281_DIGITAL_GAIN_MIN;
    if (gain > OV9281_DIGITAL_GAIN_MAX) gain = OV9281_DIGITAL_GAIN_MAX;

//    if (gainl < OV9281_MIN_GAIN)
//        gain = OV9281_MIN_GAIN;
//    else if (val > OV9281_MAX_GAIN)
//        gain = OV9281_MAX_GAIN;


    priv->digital_gain = gain;

#if TRACE_OV9281_SET_GAIN
{
/*
    struct camera_common_data   *s_data = tc_dev->s_data;
    struct sensor_mode_properties *mode = &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];

//    dev_err(dev, "%s: mode->control_properties:\n", __func__);
    dev_err(dev, "%s: min,max,default gain=%d,%d,%d\n", __func__,
         mode->control_properties.min_gain_val,
         mode->control_properties.max_gain_val,
         mode->control_properties.default_gain);
//         mode->control_properties.gain_factor,
//         mode->control_properties.inherent_gain);

//    mode->control_properties.default_gain = 17;
*/
    dev_err(dev, "%s: Gain = %d\n", __func__, (int)gain);
}
#endif


// VC
/*
        ret  = reg_write(client, 0x3507, 0x03);
        ret |= reg_write(client, 0x3509, (priv->digital_gain) & 0xfe);
*/
//    regs[0].addr = OV9281_GAIN_SHIFT_ADDR;      // OV9281_GAIN_SHIFT_ADDR = 0x3507
//    regs[0].val = 0x03;
//    regs[1].addr = OV9281_GAIN_LOW_ADDR;        // OV9281_GAIN_LOW_ADDR   = 0x3509
//    regs[1].val = gain & 0xfe;
//    regs[2].addr = OV9281_TABLE_END;
//    regs[3].val = 0;


// NVIDIA
    regs[0].addr = 0x3507;                      // OV9281_GAIN_SHIFT_ADDR = 0x3507
    regs[0].val = 0x03;
#if 1   // VC MIPI UNI driver 06.2020
    regs[1].addr = 0x3509;                      // OV9281_GAIN_HIGH_ADDR  = 0x3508
    regs[1].val = gain & 0xfe;
    regs[2].addr = OV9281_TABLE_END;
    regs[2].val = 0;
#else   // old code
    regs[1].addr = OV9281_GAIN_HIGH_ADDR;       // OV9281_GAIN_HIGH_ADDR  = 0x3508
    regs[1].val = gain >> 8;
    regs[2].addr = OV9281_GAIN_LOW_ADDR;        // OV9281_GAIN_LOW_ADDR   = 0x3509
    regs[2].val = gain & 0xff;
    regs[3].addr = OV9281_TABLE_END;
    regs[3].val = 0;
#endif


//            ret = vc_mipi_common_reg_write(client, 0x3507,  0x03);
//            ret |= vc_mipi_common_reg_write(client, sen_reg(priv, GAIN_HIGH) ,
//                        priv->digital_gain       & 0xfe);



#if OV9281_SET_GAIN_GROUP_HOLD
    if (!priv->group_hold_prev)
        ov9281_set_group_hold(tc_dev, 1);
#endif

    err = ov9281_write_table(priv, regs);
    if (err)
        goto fail;


    return 0;

fail:
    dev_err(dev, "%s: error=%d\n", __func__, err);
    return err;
}

/****** ov9281_set_exposure = Set exposure = 08.2019 ********************/
static int ov9281_set_exposure(struct tegracam_device *tc_dev, s64 val)
{

#define TRACE_OV9281_OV9281_SET_EXPOSURE        1   /* DDD - ov9281_set_exposure() - trace */
#define OV9281_OV9281_SET_EXPOSURE_GROUP_HOLD   1   /* CCC - ov9281_set_exposure() - enable group hold bef reg write */

//    struct camera_common_data *s_data = tc_dev->s_data;
    struct ov9281 *priv = (struct ov9281 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;
//    struct device *dev = &priv->i2c_client->dev;

    ov9281_reg regs[4];
    u32 exposure_time;
    u32 exposure = 0;
    int err = 0;
    int ret = 0;

    exposure_time = (u32)val;

    if(exposure_time < OV9281_DIGITAL_EXPOSURE_MIN) exposure_time = OV9281_DIGITAL_EXPOSURE_MIN;
    if(exposure_time > OV9281_DIGITAL_EXPOSURE_MAX) exposure_time = OV9281_DIGITAL_EXPOSURE_MAX;

    priv->exposure_time = exposure_time;

/*----------------------------------------------------------------------*/
/*                   Set exposure: Ext. trigger mode                    */
/*----------------------------------------------------------------------*/
#if 0   // [[[
    if(priv->sensor_ext_trig)
    {
        u64 exposure = (priv->exposure_time * (priv->sen_clk/1000000)); // sen_clk default=54Mhz, imx183=72Mhz
        int addr;
        int data;

#if TRACE_OV9281_OV9281_SET_EXPOSURE
        dev_err(dev, "%s(): exposure_time=%d: trig exposure=%llu (0x%llx)\n", __func__, priv->exposure_time, exposure, exposure);
#endif

//        addr = 0x0108; // ext trig enable
//        // data =      1; // external trigger enable
//        // data =      4; // TEST external self trigger enable
//        data = priv->sensor_ext_trig; // external trigger enable
//        ret = reg_write(priv->rom, addr, data);
//
//        addr = 0x0103; // flash output enable
//        data =      1; // flash output enable
//        ret = reg_write(priv->rom, addr, data);

        addr = 0x0109; // shutter lsb
        data = exposure & 0xff;
        ret += reg_write(priv->rom, addr, data);

        addr = 0x010a;
        data = (exposure >> 8) & 0xff;
        ret += reg_write(priv->rom, addr, data);

        addr = 0x010b;
        data = (exposure >> 16) & 0xff;
        ret += reg_write(priv->rom, addr, data);

        addr = 0x010c; // shutter msb
        data = (exposure >> 24) & 0xff;
        ret += reg_write(priv->rom, addr, data);
        if(ret)
        {
          dev_err(dev, "%s: reg_write() error=%d\n", __func__, ret);
        }


    } /* if(priv->sensor_ext_trig) */

/*----------------------------------------------------------------------*/
/*                       Set exposure: Free-run mode                    */
/*----------------------------------------------------------------------*/
    else
#endif  // ]]]
    {

#if TRACE_OV9281_OV9281_SET_EXPOSURE
{
/*
    struct camera_common_data   *s_data = tc_dev->s_data;
    struct sensor_mode_properties *mode =  &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];

    dev_err(dev, "%s: min,max,default exposure_time=%d,%d,%d\n", __func__,
                (int)mode->control_properties.min_exp_time.val,
                (int)mode->control_properties.max_exp_time.val,
                (int)mode->control_properties.default_exp_time.val);
//                (int)mode->control_properties.exposure_factor);
*/

      dev_err(dev, "%s(): exposure_time = %d, EXP=%d\n", __func__, exposure_time, exposure);
}
#endif

//      exposure = (exposure_time*1000 / 8721) * 16; // value in ns - 4 bit shift
      exposure = (((u32)(exposure_time*1000) / 9100)) << 4; // value in ns - 4 bit shift

      regs[0].addr = OV9281_EXPO_HIGH_ADDR;       // OV9281_EXPO_HIGH_ADDR = 0x3500 : exposure bits [19:16]
      regs[0].val = (exposure >> 16) & 0x0f;
      regs[1].addr = OV9281_EXPO_MID_ADDR;        // OV9281_EXPO_MID_ADDR = 0x3501
      regs[1].val = (exposure >> 8) & 0xff;
      regs[2].addr = OV9281_EXPO_LOW_ADDR;        // OV9281_EXPO_LOW_ADDR = 0x3502
      regs[2].val = (exposure & 0xff);
      regs[3].addr = OV9281_TABLE_END;
      regs[3].val = 0;

//    exposure = (((int)(priv->exposure_time * 1000) / 9100)) << 4; // calculate in ns - 4 bit shift
//
//    dev_info(&client->dev, "EXPOSURE = %d \n",exposure);
//
//    ret  = reg_write(priv->rom, 0x3500, (exposure >> 16) & 0x0f);
//    ret |= reg_write(priv->rom, 0x3501, (exposure >>  8) & 0xff);
//    ret |= reg_write(priv->rom, 0x3502,  exposure        & 0xff);
//


#if OV9281_OV9281_SET_EXPOSURE_GROUP_HOLD
      if (!priv->group_hold_prev)
          ov9281_set_group_hold(tc_dev, 1);
#endif

      err = ov9281_write_table(priv, regs);
      if (err)
      {
        dev_err(dev, "%s: error=%d\n", __func__, err);
      }


      exposure = (((int)(priv->exposure_time * 1000) / 9100)) ; // calculate in ns
      ret = 0;

      /* flash duration for flashout signal*/
      ret |= reg_write(priv->rom, 0x3926, (exposure >> 16) & 0x0f);
      ret |= reg_write(priv->rom, 0x3927, (exposure >>  8) & 0xff);
      ret |= reg_write(priv->rom, 0x3928,  exposure        & 0xff);

      /* flash offset for flashout signal , not necessary for >= Rev3 of sensor module */
      ret |= reg_write(priv->rom, 0x3922, 0);
      ret |= reg_write(priv->rom, 0x3923, 0);
      ret |= reg_write(priv->rom, 0x3924, 4);

      if(ret)
      {
        dev_err(dev, "%s: reg_write() error=%d\n", __func__, ret);
      }

    } /* else: free-run mode */

    return err;
}


// ???
// CID list
static const u32 ctrl_cid_list[] = {
    TEGRA_CAMERA_CID_GAIN,
    TEGRA_CAMERA_CID_EXPOSURE,
//    TEGRA_CAMERA_CID_EXPOSURE_SHORT,
    TEGRA_CAMERA_CID_FRAME_RATE,
    TEGRA_CAMERA_CID_GROUP_HOLD,
    TEGRA_CAMERA_CID_SENSOR_MODE_ID,
//    TEGRA_CAMERA_CID_HDR_EN,
//    TEGRA_CAMERA_CID_EEPROM_DATA,
//    TEGRA_CAMERA_CID_OTP_DATA,
//    TEGRA_CAMERA_CID_FUSE_ID,

};


static struct tegracam_ctrl_ops ov9281_ctrl_ops = {
    .numctrls = ARRAY_SIZE(ctrl_cid_list),
    .ctrl_cid_list = ctrl_cid_list,
    .set_gain = ov9281_set_gain,
    .set_exposure = ov9281_set_exposure,
//    .set_exposure = ov9281_set_coarse_time,   // ov9281_set_exposure,
    .set_frame_rate = ov9281_set_frame_rate,  // ov9281_set_frame_length,
    .set_group_hold = ov9281_set_group_hold,

// ov5693
//    .numctrls = ARRAY_SIZE(ctrl_cid_list),
//    .ctrl_cid_list = ctrl_cid_list,
//    .string_ctrl_size = {OV5693_EEPROM_STR_SIZE,
//                OV5693_FUSE_ID_STR_SIZE,
//                OV5693_OTP_STR_SIZE},
//    .set_gain = ov5693_set_gain,
//    .set_exposure = ov5693_set_exposure,
//    .set_exposure_short = ov5693_set_exposure_short,
//    .set_frame_rate = ov5693_set_frame_rate,
//    .set_group_hold = ov5693_set_group_hold,
//    .fill_string_ctrl = ov5693_fill_string_ctrl,

};

/****** ov9281_power_on = Power on = 08.2019 ****************************/
static int ov9281_power_on(struct camera_common_data *s_data)
{
    int err = 0;
    struct camera_common_power_rail *pw = s_data->power;
    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = s_data->dev;


    dev_info(dev, "%s: power on\n", __func__);

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
            goto ov9281_avdd_fail;
    }

    if (pw->iovdd) {
        err = regulator_enable(pw->iovdd);
        if (err)
            goto ov9281_iovdd_fail;
    }

// IMX
//    if (pw->dvdd) {
//        err = regulator_enable(pw->dvdd);
//        if (err)
//            goto ov9281_dvdd_fail;
//    }

    usleep_range(10, 20);

skip_power_seqn:
    usleep_range(1, 2);
    if (gpio_is_valid(pw->pwdn_gpio))
        ov9281_gpio_set(s_data, pw->pwdn_gpio, 1);

    /*
     * datasheet 2.9: reset requires ~2ms settling time
     * a power on reset is generated after core power becomes stable
     */
//    usleep_range(2000, 2010);
    usleep_range(23000, 23100);

//    msleep(20);

    if (gpio_is_valid(pw->reset_gpio))
        ov9281_gpio_set(s_data, pw->reset_gpio, 1);

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

//ov9281_dvdd_fail:
//    regulator_disable(pw->iovdd);

ov9281_iovdd_fail:
    regulator_disable(pw->avdd);

ov9281_avdd_fail:
    dev_err(dev, "%s failed.\n", __func__);

    return -ENODEV;
}

/****** ov9281_power_off = Power off = 08.2019 **************************/
static int ov9281_power_off(struct camera_common_data *s_data)
{

    int err = 0;
    struct camera_common_power_rail *pw = s_data->power;
    struct device *dev = s_data->dev;
    struct camera_common_pdata *pdata = s_data->pdata;

    dev_info(dev, "%s: power off\n", __func__);

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
        ov9281_gpio_set(s_data, pw->pwdn_gpio, 0);
    usleep_range(1, 2);
    if (gpio_is_valid(pw->reset_gpio))
        ov9281_gpio_set(s_data, pw->reset_gpio, 0);

    /* datasheet 2.9: reset requires ~2ms settling time*/
    usleep_range(2000, 2010);

    if (pw->iovdd)
        regulator_disable(pw->iovdd);
    if (pw->avdd)
        regulator_disable(pw->avdd);

power_off_done:
    pw->state = SWITCH_OFF;
    return 0;

// IMX
//    int err = 0;
//    struct camera_common_power_rail *pw = s_data->power;
//    struct device *dev = s_data->dev;
//    struct camera_common_pdata *pdata = s_data->pdata;
//
//    dev_dbg(dev, "%s: power off\n", __func__);
//
//    if (pdata && pdata->power_off) {
//        err = pdata->power_off(pw);
//        if (err) {
//            dev_err(dev, "%s failed.\n", __func__);
//            return err;
//        }
//    } else {
//        if (pw->reset_gpio) {
//            if (gpio_cansleep(pw->reset_gpio))
//                gpio_set_value_cansleep(pw->reset_gpio, 0);
//            else
//                gpio_set_value(pw->reset_gpio, 0);
//        }
//
//        usleep_range(10, 10);
//
//        if (pw->dvdd)
//            regulator_disable(pw->dvdd);
//        if (pw->iovdd)
//            regulator_disable(pw->iovdd);
//        if (pw->avdd)
//            regulator_disable(pw->avdd);
//    }
//
//    pw->state = SWITCH_OFF;
//
//    return 0;
}

/****** ov9281_power_put = Power put = 08.2019 **************************/
static int ov9281_power_put(struct tegracam_device *tc_dev)
{
    struct camera_common_data *s_data = tc_dev->s_data;
    struct camera_common_power_rail *pw = s_data->power;
//    struct camera_common_pdata *pdata = s_data->pdata;
//    struct device *dev = tc_dev->dev;

    if (unlikely(!pw))
        return -EFAULT;

//    if (pdata && pdata->use_cam_gpio)
//        cam_gpio_deregister(dev, pw->pwdn_gpio);
//    else {
//        if (gpio_is_valid(pw->pwdn_gpio))
//            gpio_free(pw->pwdn_gpio);
//        if (gpio_is_valid(pw->reset_gpio))
//            gpio_free(pw->reset_gpio);
//    }

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

/****** ov9281_power_get = Power get = 08.2019 **************************/
static int ov9281_power_get(struct tegracam_device *tc_dev)
{

#define TRACE_OV9281_POWER_GET  0   /* DDD - ov9281_power_get() - trace */
#define RESET_GPIO_ENB          0  /* CCC - ov9281_power_get() - enable reset_gpio code */

    struct camera_common_data *s_data = tc_dev->s_data;
    struct camera_common_power_rail *pw = s_data->power;
    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = tc_dev->dev;
    struct clk *parent;
//    const char *mclk_name;
//    const char *parentclk_name;
    int err = 0;
//    int ret = 0;

#if TRACE_OV9281_POWER_GET
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
#if RESET_GPIO_ENB
    pw->reset_gpio = pdata->reset_gpio;
    err = gpio_request(pw->reset_gpio, "cam_reset_gpio");
    if (err < 0) {
        dev_err(dev, "%s: unable to request reset_gpio (%d)\n",
            __func__, err);
        goto done;
    }
#endif

done:
    pw->state = SWITCH_OFF;
#if TRACE_OV9281_POWER_GET
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

/****** ov9281_parse_dt = Parse DT = 08.2019 ****************************/
static struct camera_common_pdata *ov9281_parse_dt(
    struct tegracam_device *tc_dev)
{

#define TRACE_OV9281_PARSE_DT   1   /* DDD - ov9281_parse_dt() - trace */

// IMX219 : no parse clocks
#define PARSE_CLOCKS        0   /* CCC - ov9281_parse_dt() - parse clocks */
#define PARSE_GPIOS         0   /* CCC - ov9281_parse_dt() - parse GPIOss */

    struct device *dev = tc_dev->dev;
    struct device_node *node = dev->of_node;
    struct camera_common_pdata *board_priv_pdata;
//    const struct of_device_id *match;
    int gpio;
    int err = 0;
    struct camera_common_pdata *ret = NULL;
    int val = 0;

#if TRACE_OV9281_PARSE_DT
    dev_info(dev, "%s(): ...\n", __func__);
#endif

    if (!node)
        return NULL;

//    match = of_match_device(ov9281_of_match, dev);
//    if (!match) {
//        dev_err(dev, "Failed to find matching dt id\n");
//        return NULL;
//    }

    board_priv_pdata = devm_kzalloc(dev,
        sizeof(*board_priv_pdata), GFP_KERNEL);
    if (!board_priv_pdata)
        return NULL;

#if PARSE_CLOCKS
    err = camera_common_parse_clocks(dev,
                     board_priv_pdata);
    if (err) {
        dev_err(dev, "Failed to find clocks\n");
        goto error;
    }
#endif

#if PARSE_GPIOS
    gpio = of_get_named_gpio(node, "pwdn-gpios", 0);
    if (gpio < 0) {
        if (gpio == -EPROBE_DEFER) {
            ret = ERR_PTR(-EPROBE_DEFER);
            dev_err(dev, "reset-gpios not found\n");
            goto error;
        }
        gpio = 0;
    }
    board_priv_pdata->pwdn_gpio = (unsigned int)gpio;

    board_priv_pdata->use_cam_gpio =
        of_property_read_bool(node, "cam, use-cam-gpio");
#endif

// ??? do we need reset-gpios !
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
        dev_dbg(dev, "mclk name not present, "
            "assume sensor driven externally\n");

    err = of_property_read_string(node, "avdd-reg",
        &board_priv_pdata->regulators.avdd);
    err |= of_property_read_string(node, "iovdd-reg",
        &board_priv_pdata->regulators.iovdd);
    err |= of_property_read_string(node, "dvdd-reg",
        &board_priv_pdata->regulators.dvdd);
    if (err)
        dev_dbg(dev, "avdd, iovdd and/or dvdd reglrs. not present, "
            "assume sensor powered independently\n");


// OV5693
//    err = of_property_read_string(node, "avdd-reg",
//            &board_priv_pdata->regulators.avdd);
//    if (err) {
//        dev_err(dev, "avdd-reg not in DT\n");
//        goto error;
//    }
//    err = of_property_read_string(node, "iovdd-reg",
//            &board_priv_pdata->regulators.iovdd);
//    if (err) {
//        dev_err(dev, "iovdd-reg not in DT\n");
//        goto error;
//    }

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


#if TRACE_OV9281_PARSE_DT
    dev_err(dev, "%s(): OK\n", __func__);
#endif

    return board_priv_pdata;

// IMX
//    err = of_property_read_string(node, "mclk", &board_priv_pdata->mclk_name);
//    if (err)
//        dev_dbg(dev, "mclk name not present, "
//            "assume sensor driven externally\n");
//
//    err = of_property_read_string(node, "avdd-reg",
//        &board_priv_pdata->regulators.avdd);
//    err |= of_property_read_string(node, "iovdd-reg",
//        &board_priv_pdata->regulators.iovdd);
//    err |= of_property_read_string(node, "dvdd-reg",
//        &board_priv_pdata->regulators.dvdd);
//    if (err)
//        dev_dbg(dev, "avdd, iovdd and/or dvdd reglrs. not present, "
//            "assume sensor powered independently\n");
//
//    board_priv_pdata->has_eeprom =
//        of_property_read_bool(node, "has-eeprom");
//
//    return board_priv_pdata;

error:
    devm_kfree(dev, board_priv_pdata);
#if TRACE_OV9281_PARSE_DT
    dev_err(dev, "%s(): ERROR\n", __func__);
#endif
    return ret;
}

#if VC_CODE    // [[[ - VC MIPI code

/****** ov9281_probe_vc_rom = Probe VC ROM = 08.2019 ********************/
static struct i2c_client * ov9281_probe_vc_rom(struct i2c_adapter *adapter, u8 addr)
{
    struct i2c_board_info info = {
        I2C_BOARD_INFO("dummy", addr),
    };
    unsigned short addr_list[2] = { addr, I2C_CLIENT_END };

    return i2c_new_probed_device(adapter, &info, addr_list, NULL);
}

/****** vc_mipi_reset = Reset VC MIPI sensor = 08.2019 ******************/
// vc_mipi_common_rom_init
static int vc_mipi_reset (
    struct tegracam_device *tc_dev, /* [i/o] tegra camera device        */
    int  sen_mode )                 /* [in] VC sensor mode              */
{

#define TRACE_VC_MIPI_RESET  1   /* DDD - vc_mipi_reset() - trace */

    struct ov9281 *priv = (struct ov9281 *)tegracam_get_privdata(tc_dev);
    struct device *dev = tc_dev->dev;
    int err = 0;

//    struct i2c_client *client = priv->i2c_client;
//    struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);

//    struct camera_common_data *s_data = priv->s_data;
//    struct camera_common_pdata *pdata = s_data->pdata;
//    struct device *dev = s_data->dev;
//    u8 reg_val[2];
//    bool eeprom_ctrl = 0;

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
//                dev_err(&client->dev, "%s(): !!! ERROR !!! setting VC Sensor MODE=%d STATUS=0x%02x i=%d\n", __func__, sen_mode,reg,i);
                dev_err(dev, "%s(): !!! ERROR !!! setting VC Sensor MODE=%d STATUS=0x%02x i=%d\n", __func__, sen_mode,reg,i);
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

    mdelay(300); // wait 300ms

//done:
#if TRACE_VC_MIPI_RESET
//    if(err)
      dev_err(dev, "%s(): sensor_mode=%d err=%d\n", __func__, sen_mode, err);
#endif
    return err;
}

/****** vc_mipi_setup = VC MIPI setup = 08.2019 *************************/
static int vc_mipi_setup(struct ov9281 *priv)
{

#define DUMP_CTL_REG_DATA       0   /* DDD - vc_mipi_setup() - dump module control registers 0x100-0x108 (I2C addr=0x10) */
#define DUMP_HWD_DESC_ROM_DATA  0   /* DDD - vc_mipi_setup() - dump Hardware Desriptor ROM data (I2C addr=0x10) */

//    struct camera_common_data *s_data = priv->s_data;
//    struct camera_common_pdata *pdata = s_data->pdata;
//    struct device *dev = s_data->dev;
    struct tegracam_device *tc_dev = priv->tc_dev;

//    u8 reg_val[2];
//    bool eeprom_ctrl = 0;
    int err = 0;

    struct i2c_client *client = priv->i2c_client;
    struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);


    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&client->dev, "%s(): I2C-Adapter doesn't support I2C_FUNC_SMBUS_BYTE\n", __func__);
        return -EIO;
    }

//    priv->rom = i2c_new_dummy(adapter,0x10);
    priv->rom = ov9281_probe_vc_rom(adapter,0x10);

    if ( priv->rom )
    {
//        static int i=1;
        int addr,reg;
//        int data;

#if DUMP_HWD_DESC_ROM_DATA      /* [[[ - dump Hardware Desriptor ROM data */
        dev_err(&client->dev, "%s(): Dump Hardware Descriptor ROM data:\n", __func__);
#endif  /* ]]] */

        for (addr=0; addr<sizeof(priv->rom_table); addr++)
        {
          reg = reg_read(priv->rom, addr+0x1000);
          if(reg < 0)
          {
              i2c_unregister_device(priv->rom);
              return -EIO;
          }
          *((char *)(&(priv->rom_table))+addr)=(char)reg;
#if DUMP_HWD_DESC_ROM_DATA
           dev_err(&client->dev, "addr=0x%04x reg=0x%02x\n",addr+0x1000,reg);
#endif
        }

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

        priv->model = OV_MODEL_NONE;
        if(priv->rom_table.mod_id == 0x9281)
        {
          priv->model = OV9281_MODEL;
          dev_err(&client->dev, "%s(): Sensor model=OV9281_MODEL\n", __func__);
        }

        if(priv->model == OV_MODEL_NONE)
        {
          dev_err(&client->dev, "%s(): Invalid sensor model=0x%04x, err=%d\n", __func__, priv->rom_table.mod_id, -EIO);
          return -EIO;
        }


//#if DUMP_HWD_DESC_ROM_DATA      /* [[[ - dump Hardware Desriptor ROM data */
//{
////#error
//        int i;
//        int reg_buf[2];
//        int step = 4;
//
//        dev_err(&client->dev, "%s(): Dump Hardware Descriptor ROM data:\n", __func__);
//
//        addr = 0x1000;
//        while(addr < 0x10d0)
//        {
//          for(i=0; i<step; i++)
//          {
//            reg_buf[i] = reg_read(priv->rom, addr+i);
//          }
//
//          if(step == 4)
//            dev_err(&client->dev, "0x%04x: %02x %02x %02x %02x\n",addr,reg_buf[0],reg_buf[1],reg_buf[2],reg_buf[3]);
//          else
//            dev_err(&client->dev, "0x%04x: %02x %02x\n",addr,reg_buf[0],reg_buf[1]);
//          addr += step;
//        }
//
////        for (addr=0x1000; addr<0x10d0; addr++)
////        {
////          reg = reg_read(priv->rom, addr);
////          dev_err(&client->dev, "[IMX412]: imx412_probe(): addr=0x%04x reg=0x%02x\n",addr,reg);
////        }
//}
//#endif      /* ]]] */

// Done by vc_mipi_reset()
//       addr = 0x0100; // reset
//           data =      2; // powerdown sensor
//           reg = reg_write(priv->rom, addr, data);
//
//       addr = 0x0102; // mode
//           data = sensor_mode; // default 10-bit streaming
//           reg = reg_write(priv->rom, addr, data);
//
//       addr = 0x0100; // reset
//           data =      0; // powerup sensor
//           reg = reg_write(priv->rom, addr, data);
//
//       while(1)
//       {
//           mdelay(100); // wait 100ms
//
//           addr = 0x0101; // status
//           reg = reg_read(priv->rom, addr);
//
//           if(reg & 0x80)
//                   break;
//
//           if(reg & 0x01)
//           {
//               dev_err(&client->dev, "%s(): !!! ERROR !!! setting VC Sensor MODE=%d STATUS=0x%02x i=%d\n", __func__, sensor_mode,reg,i);
//           }
//
//           if(i++ >  4)
//               break;
//       }
//
//       dev_err(&client->dev, "%s(): VC Sensor MODE=%d PowerOn STATUS=0x%02x i=%d\n",__func__, sensor_mode,reg,i);


#if DUMP_CTL_REG_DATA
{
        int i;
        int reg_val[10];

        dev_err(&client->dev, "%s(): Module controller registers (0x10):\n", __func__);
        i = 0;
        for(addr=0x100; addr<=0x108; addr++)
        {
          reg_val[i] = reg_read(priv->rom, addr);
          i++;
        }
        dev_err(&client->dev, "0x100-103: %02x %02x %02x %02x\n", reg_val[0],reg_val[1],reg_val[2],reg_val[3]);
        dev_err(&client->dev, "0x104-108: %02x %02x %02x %02x %02x\n", reg_val[4],reg_val[5],reg_val[6],reg_val[7],reg_val[8]);
}
#endif

    } /* if ( priv->rom ) */
    else
    {
        dev_err(&client->dev, "%s(): Error !!! VC FPGA not found !!!\n", __func__);
        return -EIO;
    }

// Set VC sensor mode: 0=10-bit, 1=8-bit
    priv->sensor_mode = sensor_mode;
    err = vc_mipi_reset(tc_dev, sensor_mode);
    if(err)
    {
        dev_err(&client->dev, "%s(): vc_mipi_reset() error=%d\n", __func__, err);
    }

//done:
    return err;
}
#endif  // ]]] - end of VC_CODE

/****** ov9281_set_mode = Set mode = 08.2019 ****************************/
static int ov9281_set_mode(struct tegracam_device *tc_dev)
{

#define TRACE_OV9281_SET_MODE   1   /* DDD - ov9281_set_mode() - trace */
//#define SKIP_OV9281_SET_MODE    1   /* DDD - ov9281_set_mode() - skip setting mode */

//#if !SKIP_OV9281_SET_MODE
    struct ov9281 *priv = (struct ov9281 *)tegracam_get_privdata(tc_dev);
//#endif
    struct camera_common_data *s_data = tc_dev->s_data;
    struct device *dev = tc_dev->dev;
    const struct camera_common_colorfmt *colorfmt = s_data->colorfmt;
    int pix_fmt = colorfmt->pix_fmt;
    int err = 0;
    int sensor_mode_id;

#if TRACE_OV9281_SET_MODE
{
//    dev_err(dev, "%s(): s_data->mode=%d s_data->mode_prop_idx=%d\n", __func__, s_data->mode, s_data->mode_prop_idx);
//    dev_err(dev, "%s(): sensor_mode_id=%d use_sensor_mode_id=%d\n", __func__, s_data->sensor_mode_id, s_data->use_sensor_mode_id);
//    dev_err(dev, "%s(): numfmts=%d def_mode,def_width,def_height=%d,%d,%d\n", __func__,
//                        s_data->numfmts, s_data->def_mode, s_data->def_width, s_data->def_height);
    dev_err(dev, "%s(): fmt_width,fmt_height=%d,%d pix_fmt=0x%x '%c%c%c%c'\n", __func__,
                        s_data->fmt_width, s_data->fmt_height,
                        pix_fmt,
                        (char)((pix_fmt      ) & 0xFF),
                        (char)((pix_fmt >>  8) & 0xFF),
                        (char)((pix_fmt >> 16) & 0xFF),
                        (char)((pix_fmt >> 24) & 0xFF));
}
#endif

/*............. Set new sensor mode */
// OV9281 sensor modes:
//  0x00 : 10bit free-run streaming
//  0x01 : 8bit  free-run streaming
//  0x02 : 10bit external trigger
//  0x03 : 8bit  external trigger
    if(pix_fmt == V4L2_PIX_FMT_Y10)
    {
      sensor_mode = 0;      // 10-bit
    }
    else if(pix_fmt == V4L2_PIX_FMT_GREY)
    {
      sensor_mode = 1;      // 8-bit
    }

    sensor_mode_id = s_data->sensor_mode_id;
    if(sensor_mode_id)      // 0=free run, 1=ext. trigger, 2=trigger self test
    {
      sensor_mode += 2;
    }


// Change VC MIPI sensor mode
    if(priv->sensor_mode != sensor_mode)
    {
      priv->sensor_mode = sensor_mode;

      if(sensor_mode_id == 0)
        priv->sensor_ext_trig = 0;      // 0=trig off, 1=trig on, 4=trig test
      else if(sensor_mode_id == 1)
        priv->sensor_ext_trig = 1;      // 0=trig off, 1=trig on, 4=trig test
      else if(sensor_mode_id == 2)
        priv->sensor_ext_trig = 4;      // 0=trig off, 1=trig on, 4=trig test

#if TRACE_OV9281_SET_MODE
      dev_err(dev, "%s(): New sensor_mode=%d (0=10bit, 1=8bit, 2=10bit trig, 3=8bit trig), sensor_ext_trig=%d\n", __func__,
                    sensor_mode, priv->sensor_ext_trig);
#endif

      err = vc_mipi_reset(tc_dev, sensor_mode);
// ??? Xavier: switch between 8-bit and 10-bit modes
//      mdelay(300); // wait 300ms : added in vc_mipi_reset()
      if(err)
      {
          dev_err(dev, "%s(): vc_mipi_reset() error=%d\n", __func__, err);
      }

    }

//#if SKIP_OV9281_SET_MODE
//    dev_err(dev, "%s(): mode=%d: SKIPPED!\n", __func__, s_data->mode_prop_idx);
//    return 0;
//#else
//    err = ov9281_write_table(priv, ov9281_mode_table[s_data->mode_prop_idx]);
//    if (err) goto exit;
//#endif

// IMX
//    err = ov9281_write_table(priv, ov9281_mode_table[OV9281_MODE_COMMON]);
//    if (err)
//        return err;
//
//    err = ov9281_write_table(priv, ov9281_mode_table[s_data->mode]);
//    if (err)
//        return err;

// exit:

#if TRACE_OV9281_SET_MODE
//    dev_err(dev, "%s(): mode_prop_idx=%d sensor_mode=%d err=%d\n", __func__, s_data->mode_prop_idx, sensor_mode, err);
//    dev_err(dev, "%s(): sensor_mode=%d (0=10,1=8-bit) err=%d\n", __func__, sensor_mode, err);
#endif
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

/****** ov9281_start_streaming = Start streaming = 08.2019 **************/
static int ov9281_start_streaming(struct tegracam_device *tc_dev)
{

#define TRACE_OV9281_START_STREAMING    1   /* DDD - ov9281_start_streaming() - trace */
#define VC_EXT_TRIG_SET_EXPOSURE        1   /* CCC - ov9281_start_streaming() - set exposure in ext. trigger code */

    struct ov9281 *priv = (struct ov9281 *)tegracam_get_privdata(tc_dev);
//    struct ov9281 *priv = (struct ov9281 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;
    int err = 0;



#if TRACE_OV9281_START_STREAMING
//    dev_err(dev, "%s():...\n", __func__);
#endif

// ??? - before start streaming
// set mode (write mode table)
// set exposure, gain, frame_length
///

//#if SET_FRAME_LEN
//    ov9281_set_frame_length(tc_dev, OV9281_DEFAULT_FRAME_LENGTH);
//#endif

#if TRACE_OV9281_START_STREAMING
//    dev_info(dev, "%s(): Set frame_length=%d\n", __func__, OV9281_DEFAULT_FRAME_LENGTH);
#endif
      ov9281_set_frame_length(tc_dev, priv->frame_length);
      ov9281_set_gain(tc_dev, priv->digital_gain);
      ov9281_set_exposure(tc_dev, priv->exposure_time);

#if TRACE_OV9281_START_STREAMING
//    dev_info(dev, "%s(): Set gain=%d\n", __func__, OV9281_DEFAULT_GAIN);
#endif

//............... Set trigger mode: on/off
#if VC_CODE // [[[
{
    int ret = 0;

    if(priv->sensor_ext_trig)
    {
//        u64 exposure = (priv->exposure_time * 10000) / 185185;
        u64 exposure = (priv->exposure_time * (priv->sen_clk/1000000)); // sen_clk default=54Mhz imx183=72Mhz
        int addr;
        int data;
        int flash_output_enb = priv->flash_output + 8;

        // OV9281 uses this setting for external triggering only
//        ret += reg_write_table(->client, priv->trait->stop);
        ret += ov9281_write_table(priv, ov9281_mode_table[OV9281_MODE_STOP_STREAM]);
//        ov9281_stop_streaming(tc_dev);

#if TRACE_OV9281_START_STREAMING
        dev_err(dev, "%s(): sensor_ext_trig=%d, exposure=%llu (0x%llx)\n", __func__, priv->sensor_ext_trig, exposure, exposure);
#endif

        addr = 0x0108; // ext trig enable
        // data =      1; // external trigger enable
        // data =      4; // TEST external self trigger enable
        data = priv->sensor_ext_trig; // external trigger enable
        ret += reg_write(priv->rom, addr, data);

        addr = 0x0103; // flash output enable
        data = flash_output_enb; //     8; // flash output enable
        ret += reg_write(priv->rom, addr, data);
#if TRACE_OV9281_START_STREAMING
        dev_err(dev, "%s(): flash-output=%d\n", __func__, (int)data);
#endif

#if VC_EXT_TRIG_SET_EXPOSURE  /* [[[ */
        addr = 0x0109; // shutter lsb
        data = exposure & 0xff;
        ret += reg_write(priv->rom, addr, data);

        addr = 0x010a;
        data = (exposure >> 8) & 0xff;
        ret += reg_write(priv->rom, addr, data);

        addr = 0x010b;
        data = (exposure >> 16) & 0xff;
        ret += reg_write(priv->rom, addr, data);

        addr = 0x010c; // shutter msb
        data = (exposure >> 24) & 0xff;
        ret += reg_write(priv->rom, addr, data);
#endif  /* ]]] */
        if(ret)
        {
          dev_err(dev, "%s(): reg_write() error=%d\n", __func__, ret);
//          goto exit;
        }
        else
        {
          priv->streaming = true;
        }
        goto exit;
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
    mdelay(10);    //  ms
}
#endif  // ]]]

//............... Start streaming
//    mutex_lock(&priv->streaming_lock);
    err = ov9281_write_table(priv, ov9281_mode_table[OV9281_MODE_START_STREAM]);
    if (err) {
//        mutex_unlock(&priv->streaming_lock);
        goto exit;
    } else {
        priv->streaming = true;
//        mutex_unlock(&priv->streaming_lock);
    }

//............... Exit
exit:
#if TRACE_OV9281_START_STREAMING
    dev_err(dev, "%s(): err=%d\n", __func__, err);
#endif

    return err;
}

/****** ov9281_stop_streaming = Stop streaming = 08.2019 ****************/
static int ov9281_stop_streaming(struct tegracam_device *tc_dev)
{

#define TRACE_OV9281_STOP_STREAMING    1   /* DDD - ov9281_stop_streaming() - trace */

    struct ov9281 *priv = (struct ov9281 *)tegracam_get_privdata(tc_dev);
//    struct ov9281 *priv = (struct ov9281 *)tc_dev->priv;
    struct device *dev = tc_dev->dev;
    int err = 0;

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

//    mutex_lock(&priv->streaming_lock);
    err = ov9281_write_table(priv, ov9281_mode_table[OV9281_MODE_STOP_STREAM]);
    if (err) {
//        mutex_unlock(&priv->streaming_lock);
        goto exit;
    } else {
        priv->streaming = false;
//        mutex_unlock(&priv->streaming_lock);
    }

    usleep_range(50000, 51000);

exit:
#if TRACE_OV9281_STOP_STREAMING
    dev_err(dev, "%s(): err=%d\n\n", __func__, err);
#endif

    return err;

}

static struct camera_common_sensor_ops ov9281_common_ops = {
    .numfrmfmts = ARRAY_SIZE(ov9281_frmfmt),
    .frmfmt_table = ov9281_frmfmt,
    .power_on = ov9281_power_on,
    .power_off = ov9281_power_off,
    .write_reg = ov9281_write_reg,
    .read_reg = ov9281_read_reg,
    .parse_dt = ov9281_parse_dt,
    .power_get = ov9281_power_get,
    .power_put = ov9281_power_put,
    .set_mode = ov9281_set_mode,
    .start_streaming = ov9281_start_streaming,
    .stop_streaming = ov9281_stop_streaming,
};

/****** ov9281_board_setup = Board setup = 08.2019 **********************/
static int ov9281_board_setup(struct ov9281 *priv)
{

//#define DUMP_CTL_REG_DATA       1   /* DDD - ov9281_board_setup() - dump module control registers 0x100-0x108 (I2C addr=0x10) */
//#define DUMP_HWD_DESC_ROM_DATA  0   /* DDD - ov9281_board_setup() - dump Hardware Desriptor ROM data (I2C addr=0x10) */

    struct camera_common_data *s_data = priv->s_data;
    struct camera_common_pdata *pdata = s_data->pdata;
    struct device *dev = s_data->dev;
//    u8 reg_val[2];
//    bool eeprom_ctrl = 0;
    int err = 0;

//#if VC_CODE    // [[[ - new VC code
//    struct i2c_client *client = priv->i2c_client;
//    struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
//#endif  // ]]] - end of VC_CODE


// 5693
//    dev_dbg(dev, "%s++\n", __func__);
//
//    /* eeprom interface */
//    err = ov5693_eeprom_device_init(priv);
//    if (err && s_data->pdata->has_eeprom)
//        dev_err(dev,
//            "Failed to allocate eeprom reg map: %d\n", err);
//    eeprom_ctrl = !err;

    if (pdata->mclk_name) {
        err = camera_common_mclk_enable(s_data);
        if (err) {
            dev_err(dev, "error turning on mclk (%d)\n", err);
            goto done;
        }
    }

    err = ov9281_power_on(s_data);
    if (err) {
        dev_err(dev, "error during power on sensor (%d)\n", err);
        goto err_power_on;
    }

#if VC_CODE    // [[[ - VC code
    err = vc_mipi_setup(priv);
    if(err)
    {
      dev_err(dev, "%s(): error turning on mclk (%d)\n", __func__, err);
      return err;
    }
#endif  // ]]] - end of VC_CODE


// IMX
//    /* Probe sensor model id registers */
//    err = ov9281_read_reg(s_data, OV9281_MODEL_ID_ADDR_MSB, &reg_val[0]);
//    if (err) {
//        dev_err(dev, "%s: error during i2c read probe (%d)\n",
//            __func__, err);
//        goto err_reg_probe;
//    }
//    err = ov9281_read_reg(s_data, OV9281_MODEL_ID_ADDR_LSB, &reg_val[1]);
//    if (err) {
//        dev_err(dev, "%s: error during i2c read probe (%d)\n",
//            __func__, err);
//        goto err_reg_probe;
//    }
//    if (!((reg_val[0] == 0x02) && reg_val[1] == 0x19))
//        dev_err(dev, "%s: invalid sensor model id: %x%x\n",
//            __func__, reg_val[0], reg_val[1]);
//
//    /* Sensor fine integration time */
//    err = ov9281_get_fine_integ_time(priv, &priv->fine_integ_time);
//    if (err)
//        dev_err(dev, "%s: error querying sensor fine integ. time\n",
//            __func__);


//err_reg_probe:
    ov9281_power_off(s_data);

err_power_on:
    if (pdata->mclk_name)
        camera_common_mclk_disable(s_data);

done:
    return err;
}

/****** ov9281_open = Open device = 08.2019 *****************************/
static int ov9281_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    dev_dbg(&client->dev, "%s:\n", __func__);
    return 0;
}

static const struct v4l2_subdev_internal_ops ov9281_subdev_internal_ops = {
    .open = ov9281_open,
};

/****** ov9281_probe = Probe = 08.2019 **********************************/
static int ov9281_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
    struct device *dev = &client->dev;
//    struct device_node *node = client->dev.of_node;
    struct tegracam_device *tc_dev;
    struct ov9281 *priv;
    int err;
    const struct of_device_id *match;

    dev_err(dev, "%s(): Probing v4l2 sensor at addr 0x%0x - %s/%s\n", __func__, client->addr, __DATE__, __TIME__);

    match = of_match_device(ov9281_of_match, dev);
    if (!match) {
        dev_err(dev, "No device match found\n");
        return -ENODEV;
    }
    dev_err(dev, "%s(): of_match_device() OK\n", __func__);

    if (!IS_ENABLED(CONFIG_OF) || !client->dev.of_node)
        return -EINVAL;

    priv = devm_kzalloc(dev,
                sizeof(struct ov9281), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    tc_dev = devm_kzalloc(dev,
                sizeof(struct tegracam_device), GFP_KERNEL);
    if (!tc_dev)
        return -ENOMEM;

    dev_info(dev, "%s(): devm_kzalloc() OK\n", __func__);

    priv->i2c_client = tc_dev->client = client;
    tc_dev->dev = dev;
    strncpy(tc_dev->name, "ov9281", sizeof(tc_dev->name));
    tc_dev->dev_regmap_config = &ov9281_regmap_config;
    tc_dev->sensor_ops = &ov9281_common_ops;
    tc_dev->v4l2sd_internal_ops = &ov9281_subdev_internal_ops;
    tc_dev->tcctrl_ops = &ov9281_ctrl_ops;

    err = tegracam_device_register(tc_dev);
    if (err) {
        dev_err(dev, "tegra camera driver registration failed\n");
        return err;
    }
    dev_err(dev, "%s(): tegracam_device_register() OK\n", __func__);

    priv->tc_dev = tc_dev;
    priv->s_data = tc_dev->s_data;
    priv->subdev = &tc_dev->s_data->subdev;
    tegracam_set_privdata(tc_dev, (void *)priv);
// 5693    mutex_init(&priv->streaming_lock);

    priv->frame_length  = OV9281_DEFAULT_FRAME_LENGTH;
    priv->digital_gain  = OV9281_DIGITAL_GAIN_DEFAULT;
    priv->exposure_time = OV9281_DIGITAL_EXPOSURE_DEFAULT;

    priv->sensor_ext_trig = 0;    // ext. trigger flag: 0=no, 1=yes
    priv->sen_clk = 54000000;     // clock-frequency: default=54Mhz imx183=72Mhz
    priv->flash_output = flash_output;

//colorfmt
// camera_common.c:
//   camera_common_s_fmt()
//   camera_common_g_fmt()

    err = ov9281_board_setup(priv);
    if (err) {
        dev_err(dev, "board setup failed\n");
        return err;
    }
    dev_err(dev, "%s(): ov9281_board_setup() OK\n", __func__);

    err = tegracam_v4l2subdev_register(tc_dev, true);
    if (err) {
        dev_err(dev, "tegra camera subdev registration failed\n");
        return err;
    }
    dev_err(dev, "%s(): tegracam_v4l2subdev_register() OK\n", __func__);

//    err = ov9281_debugfs_create(priv);
//    if (err) {
//        dev_err(dev, "error creating debugfs interface");
//        ov9281_debugfs_remove(priv);
//        return err;
//    }

{
    struct camera_common_data   *s_data = tc_dev->s_data;
    struct sensor_mode_properties *mode = &s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];

    mode->control_properties.default_gain         = OV9281_DIGITAL_GAIN_DEFAULT;
    mode->control_properties.default_exp_time.val = OV9281_DIGITAL_EXPOSURE_DEFAULT;
}


// Note: dev_err() dumps messages on the serial console.
// Very convenient to trace driver.
    dev_err(dev, "%s(): Detected ov9281 sensor - %s/%s\n", __func__, __DATE__, __TIME__);

    return 0;
}

/****** ov9281_remove = Remove = 08.2019 ********************************/
static int ov9281_remove(struct i2c_client *client)
{
    struct camera_common_data *s_data = to_camera_common_data(&client->dev);
    struct ov9281 *priv = (struct ov9281 *)s_data->priv;

// 5693
//    ov5693_debugfs_remove(priv);

    tegracam_v4l2subdev_unregister(priv->tc_dev);
//    ov5693_power_put(priv->tc_dev);
    tegracam_device_unregister(priv->tc_dev);

// 5693
//    mutex_destroy(&priv->streaming_lock);

    return 0;
}

static const struct i2c_device_id ov9281_id[] = {
    { "ov9281", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, ov9281_id);

/****** i2c_driver = I2C driver = 08.2019 *******************************/
static struct i2c_driver ov9281_i2c_driver = {
    .driver = {
        .name = "ov9281",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(ov9281_of_match),
    },
    .probe = ov9281_probe,
    .remove = ov9281_remove,
    .id_table = ov9281_id,
};
module_i2c_driver(ov9281_i2c_driver);

MODULE_DESCRIPTION("Media Controller driver for OV9281");
//MODULE_AUTHOR("NVIDIA Corporation");
MODULE_AUTHOR("Vision Components");
MODULE_LICENSE("GPL v2");
