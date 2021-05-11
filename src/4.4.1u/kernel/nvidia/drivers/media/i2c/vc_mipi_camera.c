#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/debugfs.h>

#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#include <media/tegra-v4l2-camera.h>
#include <media/tegracam_core.h>
#include "../platform/tegra/camera/camera_gpio.h"

#include "vc_mipi_core.h"

struct vc_camera {
	struct tegracam_device tc_dev;
	struct vc_cam cam;
};

struct vc_cam *tegracam_to_cam(struct tegracam_device *tc_dev)
{
	struct vc_camera *camera = (struct vc_camera *)tegracam_get_privdata(tc_dev);
	return &camera->cam;
}

int vc_power_on(struct camera_common_data *s_data)
{
	// struct camera_common_power_rail *pw = s_data->power;
	// struct camera_common_pdata *pdata = s_data->pdata;
	// struct device *dev = s_data->dev;
	
	// struct vc_cam *cam = tegracam_to_cam(tc_dev);
	// return vc_mod_set_power(cam, 1);
	return 0;
}

int vc_power_off(struct camera_common_data *s_data)
{
	return 0;
}

int vc_power_get(struct tegracam_device *tc_dev)
{
	return 0;
}

int vc_power_put(struct tegracam_device *tc_dev)
{
	return 0;
}

int vc_read_reg(struct camera_common_data *s_data, u16 addr, u8 *val) 
{
	return 0;
}

int vc_write_reg(struct camera_common_data *s_data, u16 addr, u8 val) 
{
	return 0;
}

int vc_set_mode(struct tegracam_device *tc_dev)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	return vc_mod_set_mode(cam);
}

int vc_set_gain(struct tegracam_device *tc_dev, s64 val)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	return vc_sen_set_gain(cam, val);
}

int vc_set_exposure(struct tegracam_device *tc_dev, s64 val)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	return vc_sen_set_exposure_dirty(cam, val);
}

int vc_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{
	return 0;
}

int vc_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
	return 0;
}

int vc_start_streaming(struct tegracam_device *tc_dev)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	return vc_sen_start_stream(cam);
}

int vc_stop_streaming(struct tegracam_device *tc_dev)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	return vc_sen_stop_stream(cam);
}

struct camera_common_pdata *vc_parse_dt(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct device_node *node = dev->of_node;
	struct camera_common_pdata *board_priv_pdata;
	int gpio;
	int err = 0;
	struct camera_common_pdata *ret = NULL;
	// int val = 0;

	dev_info(dev, "%s(): ...\n", __func__);

	if (!node)
		return NULL;

	board_priv_pdata = devm_kzalloc(dev, sizeof(*board_priv_pdata), GFP_KERNEL);
	if (!board_priv_pdata)
		return NULL;

	// do we need reset-gpios ?
	gpio = of_get_named_gpio(node, "reset-gpios", 0);
	if (gpio < 0) {
		if (gpio == -EPROBE_DEFER) {
			ret = ERR_PTR(-EPROBE_DEFER);
		}
		dev_err(dev, "reset-gpios not found\n");
	} else {
		board_priv_pdata->reset_gpio = (unsigned int)gpio;
    	}

	// IMX219	
	err = of_property_read_string(node, "mclk", &board_priv_pdata->mclk_name);
	if (err)
		dev_err(dev, "%s(): mclk name not present, assume sensor driven externally\n", __func__);

	err  = of_property_read_string(node, "avdd-reg", &board_priv_pdata->regulators.avdd);
	err |= of_property_read_string(node, "iovdd-reg", &board_priv_pdata->regulators.iovdd);
	err |= of_property_read_string(node, "dvdd-reg", &board_priv_pdata->regulators.dvdd);
	if (err)
		dev_err(dev, "%s(): avdd, iovdd and/or dvdd reglrs. not present, assume sensor powered independently\n", __func__);

	board_priv_pdata->has_eeprom = of_property_read_bool(node, "has-eeprom");
	board_priv_pdata->v_flip     = of_property_read_bool(node, "vertical-flip");
	board_priv_pdata->h_mirror   = of_property_read_bool(node, "horizontal-mirror");

	// // Read flash output enable from DT
	// err = read_property_u32(node, "flash-output", 10, &val);
	// if (err) {
	// 	dev_err(dev, "%s(): flash-output not present in DT, def=%d\n", __func__, flash_output);
	// } else {
	// 	flash_output = val;
	// 	dev_err(dev, "%s(): flash-output=%d\n", __func__, flash_output);
	// }

	dev_err(dev, "%s(): OK\n", __func__);

	return board_priv_pdata;
}

int vc_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    return 0;
}

const struct regmap_config vc_regmap_config = {
    .reg_bits = 16,
    .val_bits = 8,
    .cache_type = REGCACHE_RBTREE,
    .use_single_rw = true,
};

const struct v4l2_subdev_internal_ops vc_subdev_internal_ops = {
    .open = vc_open,
};

#define SEN_DX 1920     // 1920 // 1920, 1408 (k*64)
#define SEN_DY 1080     // 1097 // 1080

enum {
    IMX327C_MODE_1920X1080 = 0,
    IMX327C_MODE_START_STREAM,
    IMX327C_MODE_STOP_STREAM,
};

const int vc_60fps[] = {
    60,
};

const struct camera_common_frmfmt vc_frmfmt[] = {
    { { SEN_DX, SEN_DY }, vc_60fps, ARRAY_SIZE(vc_60fps), 0,
      IMX327C_MODE_1920X1080 },
};

struct camera_common_sensor_ops vc_common_ops = {
	.numfrmfmts = ARRAY_SIZE(vc_frmfmt),
	.frmfmt_table = vc_frmfmt,
	.power_on = vc_power_on,
	.power_off = vc_power_off,
	.power_get = vc_power_get,
	.power_put = vc_power_put,
	.write_reg = vc_write_reg,
	.read_reg = vc_read_reg,
	.set_mode = vc_set_mode,
	.start_streaming = vc_start_streaming,
	.stop_streaming = vc_stop_streaming,
	.parse_dt = vc_parse_dt,
};

const __u32 ctrl_cid_list[] = {
    TEGRA_CAMERA_CID_GAIN,
    TEGRA_CAMERA_CID_EXPOSURE,
    TEGRA_CAMERA_CID_FRAME_RATE,
    TEGRA_CAMERA_CID_SENSOR_MODE_ID,
};

struct tegracam_ctrl_ops vc_ctrl_ops = {
    .numctrls = ARRAY_SIZE(ctrl_cid_list),
    .ctrl_cid_list = ctrl_cid_list,
    .set_gain = vc_set_gain,
    .set_exposure = vc_set_exposure,
    .set_frame_rate = vc_set_frame_rate,
    .set_group_hold = vc_set_group_hold,
};

int vc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct tegracam_device *tc_dev;
	struct vc_camera *camera;
	int ret;

	camera = devm_kzalloc(dev, sizeof(*camera), GFP_KERNEL);
	if (!camera)
		return -ENOMEM;

	tc_dev = &camera->tc_dev;
	tc_dev->client = client;
	tc_dev->dev = dev;
	strncpy(tc_dev->name, "vc-mipi-cam", sizeof(tc_dev->name));
	tc_dev->dev_regmap_config = &vc_regmap_config;
	tc_dev->sensor_ops = &vc_common_ops;
	tc_dev->v4l2sd_internal_ops = &vc_subdev_internal_ops;
	tc_dev->tcctrl_ops = &vc_ctrl_ops;

	ret = tegracam_device_register(tc_dev);
	if (ret) {
		dev_err(dev, "%s(): tegra camera driver registration failed\n", __FUNCTION__);
		return ret;
	}
	tegracam_set_privdata(tc_dev, (void *)camera);

	ret = vc_core_init(&camera->cam, client);
	if (ret)
		return ret;
	// TODO: Achtung es gibt einen Absturz, wenn zwei Kameras im DT 
	//       konfiguriert sind, aber nur eine Kamera angeschlossen ist.

	ret = tegracam_v4l2subdev_register(tc_dev, true);
	if (ret) {
       		dev_err(dev, "%s(): tegra camera subdev registration failed\n", __FUNCTION__);
       		return ret;
    	}

	return 0;
}

int vc_remove(struct i2c_client *client)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct vc_camera *camera = (struct vc_camera *)s_data->priv;
	struct tegracam_device *tc_dev = &camera->tc_dev;

	tegracam_v4l2subdev_unregister(tc_dev);
	tegracam_device_unregister(tc_dev);

	return 0;
}

const struct i2c_device_id vc_id[] = {
    { "vc-mipi-cam", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, vc_id);

const struct of_device_id vc_dt_ids[] = {
    { .compatible = "nvidia,imx327c", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, vc_dt_ids);

/****** i2c_driver = I2C driver = 09.2019 *******************************/
struct i2c_driver vc_i2c_driver = {
    .driver = {
        .name = "vc-mipi-cam",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(vc_dt_ids),
    },
    .id_table = vc_id,
    .probe = vc_probe,
    .remove = vc_remove,
};
module_i2c_driver(vc_i2c_driver);

MODULE_DESCRIPTION("Media Controller driver for IMX327C");
MODULE_AUTHOR("Vision Components GmbH <mipi-tech@vision-components.com>");
MODULE_LICENSE("GPL v2");