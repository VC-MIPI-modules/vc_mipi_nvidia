

static int vc_camera_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    dev_info(dev, "%s", __func__);
    return 0;
}

static int vc_camera_remove(struct i2c_client *client) {
    dev_info(dev, "%s", __func__);
    return 0;
}

static const struct of_device_id vc_camera_of_match[] = {
    { .compatible = "vc,camera", },
    { },
};
MODULE_DEVICE_TABLE(of, vc_camera_of_match);

static const struct i2c_device_id vc_camera_id[] = {
    { "vc_camera", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, vc_camera_id);

static struct i2c_driver vc_camera_i2c_driver = {
    .driver = {
        .name = "vc_camera",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(vc_camera_of_match),
    },
    .probe = vc_camera_probe,
    .remove = vc_camera_remove,
    .id_table = vc_camera_id,
};
module_i2c_driver(vc_camera_i2c_driver);

MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Media Controller driver for VC MIPI Cameras");
MODULE_AUTHOR("Vision Components GmbH <mipi-tech@vision-components.com>");
MODULE_LICENSE("GPL v2");