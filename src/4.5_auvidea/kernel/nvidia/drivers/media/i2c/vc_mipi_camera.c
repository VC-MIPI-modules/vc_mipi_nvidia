#include <linux/module.h>
#include <linux/of.h>
#include <media/tegra-v4l2-camera.h>
#include <media/tegracam_core.h>
#include "vc_mipi_core.h"
#include "vc_mipi_modules.h"


static struct vc_cam *tegracam_to_cam(struct tegracam_device *tc_dev)
{
	return (struct vc_cam *)tegracam_get_privdata(tc_dev);
}

static struct sensor_mode_properties *tegracam_to_mode0(struct tegracam_device *tc_dev) 
{
	struct sensor_properties *sensor = &tc_dev->s_data->sensor_props;

	if (sensor->sensor_modes != NULL && sensor->num_modes > 0) {
		return &sensor->sensor_modes[0];
	}
	return NULL;
}

static int vc_set_mode(struct tegracam_device *tc_dev)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	return vc_mod_set_mode(cam);
}

static int vc_set_gain(struct tegracam_device *tc_dev, s64 val)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	return vc_sen_set_gain(cam, val);
}

static int vc_set_exposure(struct tegracam_device *tc_dev, s64 val)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	return vc_sen_set_exposure_dirty(cam, val);
}

static int vc_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{
	// TODO
	return 0;
}

// Don't remove this function. It is needed by the Tegra Framework. 
static int vc_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
	return 0;
}

static int vc_start_streaming(struct tegracam_device *tc_dev)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	int ret = 0;
	
	ret  = vc_sen_set_gain(cam, cam->state.gain);
	ret |= vc_sen_set_exposure_dirty(cam, cam->state.exposure);
	ret |= vc_sen_start_stream(cam);

	return ret;
}

static int vc_stop_streaming(struct tegracam_device *tc_dev)
{
	struct vc_cam *cam = tegracam_to_cam(tc_dev);
	return vc_sen_stop_stream(cam);
}

// Don't remove this function. It is needed by the Tegra Framework. 
static int vc_power_get(struct tegracam_device *tc_dev)
{
	return 0;
}

// Don't remove this function. It is needed by the Tegra Framework. 
// (called by tegracam_device_register)
static struct camera_common_pdata *vc_parse_dt(struct tegracam_device *tc_dev)
{
	return devm_kzalloc(tc_dev->dev, sizeof(struct camera_common_pdata), GFP_KERNEL);
}

static struct camera_common_sensor_ops vc_sensor_ops = {
	.set_mode = vc_set_mode,
	.start_streaming = vc_start_streaming,
	.stop_streaming = vc_stop_streaming,
	.power_get = vc_power_get,
	.parse_dt = vc_parse_dt,
};

static void vc_adjust_cam_ctrls(struct vc_cam *cam)
{
	// TEGRA_VI_CSI_ERROR_STATUS=0x00000004 (Bits 3-0: h>,<, w>,<) => height is to high
	// TEGRA_VI_CSI_ERROR_STATUS=0x00000008 (Bits 3-0: h>,<, w>,<) => height is to low
	switch (cam->desc.mod_id) {
	case MOD_ID_IMX183:
		cam->ctrl.o_frame.height = 3647;
		break;
	case MOD_ID_IMX226:
		cam->ctrl.o_frame.height = 3040;
		break;
	case MOD_ID_IMX252:
		cam->ctrl.o_frame.height = 1535;
		break;
	}
}

static int vc_init_frmfmt(struct device *dev, struct vc_cam *cam)
{
	struct camera_common_frmfmt *frmfmt;
	int *fps;

	frmfmt = devm_kzalloc(dev, sizeof(*frmfmt), GFP_KERNEL);
	if (!frmfmt)
		return -ENOMEM;

	fps = devm_kzalloc(dev, sizeof(int), GFP_KERNEL);
	if (!fps)
		return -ENOMEM;

	vc_sensor_ops.numfrmfmts = 1;
	vc_sensor_ops.frmfmt_table = frmfmt;

	fps[0] = cam->ctrl.framerate.default_val;

	frmfmt->size.width = cam->ctrl.o_frame.width;
	frmfmt->size.height = cam->ctrl.o_frame.height;
	frmfmt->framerates = fps;
	frmfmt->num_framerates = 1;
	frmfmt->hdr_en = 0;
	frmfmt->mode = 0;

	vc_notice(dev, "%s(): Init frame (width: %d, height: %d, fps: %d)\n", __FUNCTION__,
		frmfmt->size.width, frmfmt->size.height, frmfmt->framerates[0]);

	return 0;
}

static void vc_init_image(struct tegracam_device *tc_dev, struct vc_cam *cam)
{
	struct device *dev = tc_dev->dev;
	struct sensor_mode_properties *mode = tegracam_to_mode0(tc_dev);
	struct sensor_image_properties *image;

	if (mode != NULL) {
		image = &mode->image_properties;
		image->width = cam->ctrl.o_frame.width;
		image->height = cam->ctrl.o_frame.height;
		image->line_length = cam->ctrl.o_frame.width;
		// image->pixel_format = ... TODO!
		vc_core_set_format(cam, MEDIA_BUS_FMT_Y10_1X10);
		// vc_core_set_format(cam, MEDIA_BUS_FMT_SRGGB10_1X10);

		vc_notice(dev, "%s(): Init image (width: %d, height: %d, line_length: %d)\n", __FUNCTION__,
			image->width, image->height, image->line_length);
	}
}

static void vc_init_lanes(struct tegracam_device *tc_dev, struct vc_cam *cam) 
{
	struct sensor_mode_properties *mode = tegracam_to_mode0(tc_dev);
	struct sensor_signal_properties *signal;

	if (mode != NULL) {
		signal = &mode->signal_properties;

		vc_core_set_num_lanes(cam, signal->num_lanes);
	}
}

static int read_property_u32(struct device_node *node, const char *name, int radix, u32 *value)
{
	const char *str;
	int ret = 0;

    	ret = of_property_read_string(node, name, &str);
    	if (ret)
        	return -ENODATA;

	ret = kstrtou32(str, radix, value);
	if (ret)
		return -EFAULT;

    	return 0;
}

static void vc_init_trigger(struct tegracam_device *tc_dev, struct vc_cam *cam)
{
	struct device *dev = tc_dev->dev;
	struct device_node *node = dev->of_node;
	int value = 1;
	int ret = 0;

	if (node != NULL) {
		ret = read_property_u32(node, "external-trigger-mode", 10, &value);
		if (ret) {
			vc_err(dev, "%s(): Unable to read external-trigger-mode from device tree!\n", __FUNCTION__);
		} else {
			vc_notice(dev, "%s(): Init external-trigger-mode (enable: %d)\n", __FUNCTION__, value);
			vc_mod_set_trigger_in(cam, value);
		}

		ret = read_property_u32(node, "flash-output", 10, &value);
		if (ret) {
			vc_err(dev, "%s(): Unable to read flash-output from device tree!\n", __FUNCTION__);
		} else {
			vc_notice(dev, "%s(): Init flash-output (enable: %d)\n", __FUNCTION__, value);
			vc_mod_set_flash_out(cam, value);
		}
	}
}

static void vc_init_controls(struct tegracam_device *tc_dev, struct vc_cam *cam) 
{
	struct device *dev = tc_dev->dev;
	struct sensor_mode_properties *mode = tegracam_to_mode0(tc_dev);
	struct sensor_control_properties *control;

	if (mode != NULL) {
		control = &mode->control_properties;

		if (cam->ctrl.gain.enabled) {
			vc_notice(dev, "%s(): Init control gain (min: %d, max: %d, default: %d)\n", __FUNCTION__, 
				cam->ctrl.gain.min, cam->ctrl.gain.max, cam->ctrl.gain.default_val);
			control->gain_factor = 1;
			control->min_gain_val = cam->ctrl.gain.min;
			control->max_gain_val = cam->ctrl.gain.max;
			control->default_gain = cam->ctrl.gain.default_val;
			control->step_gain_val = 1;
		}		

		if (cam->ctrl.exposure.enabled) {
			vc_notice(dev, "%s(): Init control exposure (min: %d, max: %d, default: %d)\n", __FUNCTION__, 
				cam->ctrl.exposure.min, cam->ctrl.exposure.max, cam->ctrl.exposure.default_val);
			control->exposure_factor = 1;
			control->min_exp_time.val = cam->ctrl.exposure.min;
			control->max_exp_time.val = cam->ctrl.exposure.max;
			control->default_exp_time.val = cam->ctrl.exposure.default_val;
			control->step_exp_time.val = 1;
		}

		if(cam->ctrl.framerate.enabled) {
			vc_notice(dev, "%s(): Init control framerate (min: %d, max: %d, default: %d)\n", __FUNCTION__, 
				cam->ctrl.framerate.min, cam->ctrl.framerate.max, cam->ctrl.framerate.default_val);
			control->framerate_factor = 1;
			control->min_framerate = cam->ctrl.framerate.min;
			control->max_framerate = cam->ctrl.framerate.max;
			control->default_framerate = cam->ctrl.framerate.default_val;
			control->step_framerate = 1;
		}
	}

	// Workaround: Set this state to enable controls in tegracam_ctrls.c -> tegracam_set_ctrls
	tc_dev->s_data->power->state = SWITCH_ON;
}

static const struct regmap_config vc_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
	.use_single_rw = true,
};

static const __u32 ctrl_cid_list[] = {
	TEGRA_CAMERA_CID_GAIN,
	TEGRA_CAMERA_CID_EXPOSURE,
	TEGRA_CAMERA_CID_FRAME_RATE,
};

static struct tegracam_ctrl_ops vc_ctrl_ops = {
	.numctrls = ARRAY_SIZE(ctrl_cid_list),
	.ctrl_cid_list = ctrl_cid_list,
	.set_gain = vc_set_gain,
	.set_exposure = vc_set_exposure,
	.set_frame_rate = vc_set_frame_rate,
	.set_group_hold = vc_set_group_hold,
};

static int vc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct vc_cam *cam;
	struct tegracam_device *tc_dev;
	int ret;

	// Don't remove this line of code. It will cause a kernel crash.
	// "Unable to handle kernel paging request at virtual address 7dabb7951ec6f65c"
	vc_notice(dev, "%s(): Probing UNIVERSAL VC MIPI Driver (%s/%s)\n", __func__, __DATE__, __TIME__);

	cam = devm_kzalloc(dev, sizeof(struct vc_cam), GFP_KERNEL);
	if (!cam)
		return -ENOMEM;

	tc_dev = devm_kzalloc(dev, sizeof(struct tegracam_device), GFP_KERNEL);
	if (!tc_dev)
		return -ENOMEM;

	ret = vc_core_init(cam, client);
	if (ret) {
		vc_err(dev, "%s(): Error in vc_core_init!\n", __func__);
		return ret;
	}
	vc_adjust_cam_ctrls(cam);
	vc_init_frmfmt(dev, cam);

	// Defined in tegracam_core.c
	// Initializes 
	//   * tc_dev->s_data
	// Calls 
	//   * camera_common_initialize
	tc_dev->client = client;
	tc_dev->dev = dev;
	strncpy(tc_dev->name, "vc-mipi-cam", sizeof(tc_dev->name));
	tc_dev->dev_regmap_config = &vc_regmap_config;
	tc_dev->sensor_ops = &vc_sensor_ops;
	tc_dev->tcctrl_ops = &vc_ctrl_ops;
	ret = tegracam_device_register(tc_dev);
	if (ret) {
		vc_err(dev, "%s(): Tegra camera device registration failed\n", __FUNCTION__);
		return ret;
	}

	// Defined in tegracam_core.c
	// Initializes
	//   * tc_dev->priv
	//   * tc_dev->s_data->priv
	tegracam_set_privdata(tc_dev, (void *)cam);
	
	// Defined in tegracam_v4l2.c
	// Initializes
	//   * tc_dev->s_data->tegracam_ctrl_hdl
	ret = tegracam_v4l2subdev_register(tc_dev, true);
	if (ret) {
       		vc_err(dev, "%s(): Tegra camera subdev registration failed\n", __FUNCTION__);
       		return ret;
    	}

	// This functions need tc_dev->s_data to be initialized.
	vc_init_image(tc_dev, cam);
	vc_init_controls(tc_dev, cam);
	vc_init_lanes(tc_dev, cam);
	vc_init_trigger(tc_dev, cam);

	return 0;
}

static int vc_remove(struct i2c_client *client)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct tegracam_device *tc_dev = s_data->tegracam_ctrl_hdl->tc_dev;

	tegracam_v4l2subdev_unregister(tc_dev);
	tegracam_device_unregister(tc_dev);

	return 0;
}

static const struct i2c_device_id vc_id[] = {
	{ "vc-mipi-cam", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, vc_id);

static const struct of_device_id vc_dt_ids[] = {
	{ .compatible = "vc,vc_mipi", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, vc_dt_ids);

static struct i2c_driver vc_i2c_driver = {
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