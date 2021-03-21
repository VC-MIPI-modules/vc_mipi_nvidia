/*
 * vc-camera.c - VC MIPI CSI-2 camera driver
 * Based on ... drivers
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
#include <media/tegracam_core.h>

static const struct of_device_id vc_camera_of_match[] = {
    { .compatible = "vc,camera", },
    { },
};
MODULE_DEVICE_TABLE(of, vc_camera_of_match);



static int vc_camera_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{
    struct device *dev = &client->dev;

    dev_info(dev, "TEST %s", __func__);
    return 0;
}

static int vc_camera_remove(struct i2c_client *client) 
{
    struct device *dev = &client->dev;
    
    dev_info(dev, "TEST %s", __func__);
    return 0;
}

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