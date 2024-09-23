#include <linux/version.h>
#include <linux/module.h>
#include <linux/of.h>
#include <media/tegra-v4l2-camera.h>
#include <media/mc_common.h>
#include <media/tegracam_core.h>
#include "vc_mipi_core.h"
#include "vc_mipi_modules.h"

#define VERSION "0.18.1"
// #define VC_CTRL_VALUE

static struct vc_cam *tegracam_to_cam(struct tegracam_device *tc_dev)
{
        return (struct vc_cam *)tegracam_get_privdata(tc_dev);
}

static struct sensor_mode_properties *tegracam_to_mode(struct tegracam_device *tc_dev, int mode_idx) 
{
        struct sensor_properties *sensor = &tc_dev->s_data->sensor_props;

        if (sensor->sensor_modes != NULL && sensor->num_modes > 0 && mode_idx < sensor->num_modes) {
                return &sensor->sensor_modes[mode_idx];
        }
        return NULL;
}

struct tegra_channel *get_tegra_channel(struct tegracam_device *tc_dev)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        struct device *dev = vc_core_get_sen_device(cam);
        struct vc_ctrl *ctrl = &cam->ctrl;
        struct i2c_client *client_sen = ctrl->client_sen;
        struct camera_common_data *s_data = NULL;
        struct v4l2_subdev *sd = NULL;
        struct media_pad *pad_csi = NULL;
        struct media_pad *pad_vi = NULL;
        struct v4l2_subdev *sd_csi = NULL;
        struct v4l2_subdev *sd_vi = NULL;
        struct video_device *vdev_vi = NULL;

        s_data = to_camera_common_data(&client_sen->dev);
        if (NULL == s_data) {
                vc_err(dev, "%s(): s_data is NULL!\n", __FUNCTION__);
                return NULL;
        }

        sd = &s_data->subdev;
        if (NULL == sd) {
                vc_err(dev, "%s(): sd is NULL!\n", __FUNCTION__);
                return NULL;
        }

        pad_csi = media_entity_remote_pad(&sd->entity.pads[0]);
        if (NULL == pad_csi) {
                vc_err(dev, "%s(): pad_csi is NULL!\n", __FUNCTION__);
                return NULL;
        }

        sd_csi = media_entity_to_v4l2_subdev(pad_csi->entity);
        if (NULL == sd_csi) {
                vc_err(dev, "%s(): sd_csi is NULL!\n", __FUNCTION__);
                return NULL;
        }

        pad_vi = media_entity_remote_pad(&sd_csi->entity.pads[1]);
        if (NULL == pad_vi) {
                vc_err(dev, "%s(): pad_vi is NULL!\n", __FUNCTION__);
                return NULL;
        }

        sd_vi = media_entity_to_v4l2_subdev(pad_vi->entity);
        if (NULL == sd_vi) {
                vc_err(dev, "%s(): sd_vi is NULL!\n", __FUNCTION__);
                return NULL;
        }
        
        vdev_vi = media_entity_to_video_device(pad_vi->entity);
        if (NULL == vdev_vi) {
                vc_err(dev, "%s(): vdev_vi is NULL!\n", __FUNCTION__);
                return NULL;
        }

        return video_get_drvdata(vdev_vi);
}

void vc_update_image_size_from_mode(struct tegracam_device *tc_dev,  __u32 *left, __u32 *top, __u32 *width, __u32 *height)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        struct vc_ctrl *ctrl = NULL;
        
        struct device *dev = vc_core_get_sen_device(cam);
        struct sensor_mode_properties *mode = NULL;
        struct sensor_image_properties *image = NULL;
        struct tegra_channel *chan = NULL;
        int mode_idx = 0;
        bool bypass_mode = false;

        chan = get_tegra_channel(tc_dev);
        if (NULL == chan) {
                vc_err(dev, "%s(): Could not get tegra channel!\n", __FUNCTION__);
                return;
        }

        bypass_mode = chan->bypass;

        if (bypass_mode) {
                mode_idx = tc_dev->s_data->sensor_mode_id;
        }

        mode = tegracam_to_mode(tc_dev, mode_idx);
        if (mode == NULL)
                return;

        image = &mode->image_properties;
        if (image->width != 0 && image->height != 0) {
                *left = image->left;
                *top = image->top;
                *width = image->width;
                *height = image->height;

                vc_notice(dev, "%s(): Update image size from mode%u (l: %u, t: %u, w: %u, h: %u)\n",
                        __FUNCTION__, mode_idx, *left, *top, *width, *height);

                if (NULL == cam) {
                        vc_err(dev, "%s(): Could not get cam device!\n", __FUNCTION__);
                        return;
                }

                ctrl = &cam->ctrl;
                if (NULL == ctrl) {
                        vc_err(dev, "%s(): Could not get control!\n", __FUNCTION__);
                        return;
                }

                if (bypass_mode) {
                        if (ctrl->dt_binning_modes[mode_idx].mode_set) {
                                vc_notice(dev, "%s(): Using binning_mode=%d from device tree mode%u \n",
                                __FUNCTION__, ctrl->dt_binning_modes[mode_idx].binning_mode, mode_idx);
                                vc_sen_set_binning_mode(cam, ctrl->dt_binning_modes[mode_idx].binning_mode);
                        }
                }
        }
}

#if defined(VC_MIPI_JETSON_NANO) && defined(VC_MIPI_L4T_32_7_4)
int vc_set_channel_trigger_mode(struct tegracam_device *tc_dev, __u8 trigger_mode)
{
        struct tegra_channel *chan = NULL;
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        struct device *dev = vc_core_get_sen_device(cam);

        chan = get_tegra_channel(tc_dev);
        if (NULL == chan) {
                vc_err(dev, "%s(): channel is NULL!\n", __FUNCTION__);
                return -EINVAL;
        }

        chan->trigger_mode = trigger_mode;

        return 0;
}
#endif

#ifdef VC_MIPI_JETSON_NANO
void vc_fix_image_size(struct tegracam_device *tc_dev, __u32 *width, __u32 *height, 
        __u32 *tegra_width, __u32 *tegra_height, __u32 *tegra_line_length)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        __u8 num_lanes = vc_core_get_num_lanes(cam);
        int trigger_mode = vc_mod_get_trigger_mode(cam);
        int trigger_enabled = 0;

        __u32 code = vc_core_get_format(cam);

        // Triggermodi
        // 0: FLAG_TRIGGER_DISABLE
        // 1: FLAG_TRIGGER_EXTERNAL
        // 2: FLAG_TRIGGER_PULSEWIDTH
        // 3: FLAG_TRIGGER_SELF
        // 4: FLAG_TRIGGER_SINGLE
        // 5: FLAG_TRIGGER_SYNC
        // 6: FLAG_TRIGGER_STREAM_EDGE
        // 7: FLAG_TRIGGER_STREAM_LEVEL

        switch (trigger_mode) {
                case 0: case 5: case 6: case 7: default:
                        trigger_enabled = 0;
                        break;
                case 1: case 2: case 3: case 4:
                        trigger_enabled = 1;
                        break;
        }

        // This error is dependent on VMAX, SHS and frame.height
        // TEGRA_VI_CSI_ERROR_STATUS=0x00000001 (Bits 3-0: h>,<, w>,<) => width is to high
        // TEGRA_VI_CSI_ERROR_STATUS=0x00000002 (Bits 3-0: h>,<, w>,<) => width is to low
        // TEGRA_VI_CSI_ERROR_STATUS=0x00000004 (Bits 3-0: h>,<, w>,<) => height is to high
        // TEGRA_VI_CSI_ERROR_STATUS=0x00000008 (Bits 3-0: h>,<, w>,<) => height is to low
        switch (cam->desc.mod_id) {
        case MOD_ID_IMX178: // Active pixels 3072 x 2048
                if (cam->desc.mod_rev >  1) break;
                if (!trigger_enabled) {
                        *height += 1;
                }
                break;
        case MOD_ID_IMX183: // Active pixels 5440 x 3648
                if (cam->desc.mod_rev > 12) break;
                if (trigger_enabled) {
                        switch (code) {
                        case MEDIA_BUS_FMT_Y8_1X8: 	case MEDIA_BUS_FMT_SRGGB8_1X8:
                        case MEDIA_BUS_FMT_Y10_1X10: 	case MEDIA_BUS_FMT_SRGGB10_1X10:
                                *height += 12;
                                break;
                        }
                }
                break;

        case MOD_ID_IMX264: // Active pixels 2432 x 2048
                if (cam->desc.mod_rev <=  3 && num_lanes == 2) { *height += 1; } break;
        case MOD_ID_IMX265: // Active pixels 2048 x 1536
                if (cam->desc.mod_rev <=  1 && num_lanes == 2) { *height += 1; } break;
        case MOD_ID_IMX250: // Active pixels 2432 x 2048
                if (cam->desc.mod_rev <=  7 && num_lanes == 4) { *height += 1; } break;
        case MOD_ID_IMX252: // Active pixels 2048 x 1536
                if (cam->desc.mod_rev <= 10 && num_lanes == 4) { *height += 1; } break;
        case MOD_ID_IMX273: // Active pixels 1440 x 1080
                if (cam->desc.mod_rev <= 13 && num_lanes == 4) { *height += 1; } break;
        case MOD_ID_IMX392: // Active pixels 1920 x 1200
                if (cam->desc.mod_rev <=  6 && num_lanes == 4) { *height += 1; } break;
                break;
        }

        if (*tegra_height != *height) {
                vc_dbg(tc_dev->s_data->dev, "%s(): Fix image size (h: %u, hc: %u)\n",
                                __FUNCTION__, *tegra_height, *height);
        }
}
#endif

__u32 g_width = 0;
__u32 g_height = 0;
__u32 g_tegra_width = 0;
__u32 g_tegra_height = 0;
__u32 g_tegra_line_length = 0;

void vc_overwrite_image_size(struct tegracam_device *tc_dev, __u32 *width, __u32 *height, 
        __u32 *tegra_width, __u32 *tegra_height, __u32 *tegra_line_length)
{
        if (g_width != 0) {
                *width = g_width;
        }
        if (g_height != 0) {
                *height = g_height;
        }
        if (g_tegra_width != 0) {
                *tegra_width = g_tegra_width;
        }
        if (g_tegra_height != 0) {
                *tegra_height = g_tegra_height;
        }
        if (g_tegra_line_length != 0) {
                *tegra_line_length = g_tegra_line_length;
        }
}

void vc_update_tegra_image_size(struct tegracam_device *tc_dev, __u32 width, __u32 height, __u32 line_length)
{
        struct camera_common_frmfmt *frmfmt1 = (struct camera_common_frmfmt *)tc_dev->sensor_ops->frmfmt_table;
        struct camera_common_frmfmt *frmfmt2 = (struct camera_common_frmfmt *)tc_dev->s_data->frmfmt;
        struct sensor_image_properties *image = &tegracam_to_mode(tc_dev, 0)->image_properties;

        // TODO: Problem! When the format is changed set_mode is called to late in s_stream 
         //       to make the change active. Currently it is necessary to start streaming twice!
        tc_dev->s_data->def_width = width;
        tc_dev->s_data->def_height = height;
        tc_dev->s_data->fmt_width = width;
        tc_dev->s_data->fmt_height = height;
        frmfmt1[0].size.width = width;
        frmfmt1[0].size.height = height;
        frmfmt2[0].size.width = width;
        frmfmt2[0].size.height = height;
        if (line_length != 0) {
                image->line_length = line_length;
        }
}

void vc_update_tegra_controls(struct tegracam_device *tc_dev) 
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        struct sensor_mode_properties *mode = tegracam_to_mode(tc_dev, 0);
        struct sensor_control_properties *control;

        if (mode != NULL) {
                control = &mode->control_properties;

                control->max_exp_time.val = cam->ctrl.exposure.max;
                control->max_framerate = cam->ctrl.framerate.max;
        }
}

static int vc_set_mode(struct tegracam_device *tc_dev)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        struct vc_state *state = &cam->state;
        __u32 left = 0;
        __u32 top = 0;
        __u32 width = cam->ctrl.frame.width;
        __u32 height = cam->ctrl.frame.height;
        __u32 tegra_width = 0;
        __u32 tegra_height = 0;
        __u32 tegra_line_length = 0;
        int ret = 0;

        ret  = vc_core_set_format(cam , tc_dev->s_data->colorfmt->code);

        vc_update_image_size_from_mode(tc_dev, &left, &top, &width, &height);
        tegra_width = width;
        tegra_height = height;
#ifdef VC_MIPI_JETSON_NANO
        vc_fix_image_size(tc_dev, &width, &height, &tegra_width, &tegra_height, &tegra_line_length);
#endif
        vc_overwrite_image_size(tc_dev, &width, &height, &tegra_width, &tegra_height, &tegra_line_length);
        state->frame.left = left;
        state->frame.top = top;
        state->frame.width = width;
        state->frame.height = height;
        vc_update_tegra_image_size(tc_dev, tegra_width, tegra_height, tegra_line_length);
        vc_update_tegra_controls(tc_dev);

        return ret;
}

static int vc_read_reg(struct camera_common_data *s_data, __u16 addr, __u8 *val)
{
            int ret = 0;
            __u32 reg_val = 0;

            ret = regmap_read(s_data->regmap, addr, &reg_val); 
            if (ret) {
                       vc_err(s_data->dev, "%s(): i2c read failed! (addr: 0x%04x)\n", __FUNCTION__, addr);
                       return ret;
            }
            
        *val = reg_val & 0xff;
        vc_notice(s_data->dev, "%s(): i2c read (addr: 0x%04x => value: 0x%02x)\n", __FUNCTION__, addr, *val);

            return 0;
}

static int vc_write_reg(struct camera_common_data *s_data, __u16 addr, __u8 val)
{
            int ret = 0;

            ret = regmap_write(s_data->regmap, addr, val);
            if (ret) {
                vc_err(s_data->dev, "%s(): i2c write failed! (addr: 0x%04x <= value: 0x%02x)\n", __FUNCTION__, addr, val);
            }

        vc_notice(s_data->dev, "%s(): i2c write (addr: 0x%04x => value: 0x%02x)\n", __FUNCTION__, addr, val);

            return ret;
}

static int vc_set_gain(struct tegracam_device *tc_dev, __s64 val)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        struct sensor_mode_properties *mode = tegracam_to_mode(tc_dev, 0);
        struct sensor_control_properties *control;
        int gain = 0;

        if (mode != NULL) {
                control = &mode->control_properties;

                gain =  ((cam->ctrl.gain.max - cam->ctrl.gain.min)*val) /
                        (control->max_gain_val - control->min_gain_val) 
                        + cam->ctrl.gain.min;

                return vc_sen_set_gain(cam, gain);
        }

        return -EINVAL;
}

static int vc_set_exposure(struct tegracam_device *tc_dev, __s64 val)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        return vc_sen_set_exposure(cam, val);
}

static int vc_set_frame_rate(struct tegracam_device *tc_dev, __s64 val)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        return vc_core_set_framerate(cam, val);
}

static int vc_set_trigger_mode(struct tegracam_device *tc_dev, __s64 val)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        int ret = vc_mod_set_trigger_mode(cam, val);
        vc_update_tegra_controls(tc_dev);

#if defined(VC_MIPI_JETSON_NANO) && defined(VC_MIPI_L4T_32_7_4)
        ret = vc_set_channel_trigger_mode(tc_dev, (__u8)val);
#endif

        return ret;
}

static int vc_set_io_mode(struct tegracam_device *tc_dev, __s64 val)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        return vc_mod_set_io_mode(cam, val);
}

static int vc_set_black_level(struct tegracam_device *tc_dev, __s64 val)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        return vc_sen_set_blacklevel(cam, val);
}

static int vc_set_single_trigger(struct tegracam_device *tc_dev, bool val) 
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        return vc_mod_set_single_trigger(cam);
}

static int vc_set_binning_mode(struct tegracam_device *tc_dev, __s64 val)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        return vc_sen_set_binning_mode(cam, val);
}

__u32 g_sleepR = 0;
__u32 g_sleepS = 0;
__u32 g_sleepP = 50;

#ifdef VC_CTRL_VALUE
static int vc_set_value(struct tegracam_device *tc_dev, __s64 val)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        struct device *dev = vc_core_get_sen_device(cam);
        
        if (0 < val && val < 10000) {
                g_tegra_width = val;
                vc_notice(dev, "%s(): Set testing tegra width = %u", __FUNCTION__, g_tegra_width);
        }
        if (10000 <= val && val < 20000) {
                g_tegra_height = val - 10000;
                vc_notice(dev, "%s(): Set testing tegra height = %u", __FUNCTION__, g_tegra_height);
        }
        if (20000 <= val && val < 30000) {
                g_tegra_line_length = val - 20000;
                vc_notice(dev, "%s(): Set testing tegra line_length = %u", __FUNCTION__, g_tegra_line_length);
        }
        if (30000 <= val && val < 40000) {
                g_width = val - 30000;
                vc_notice(dev, "%s(): Set testing sensor width = %u", __FUNCTION__, g_width);
        }
        if (40000 <= val && val < 50000) {
                g_height = val - 40000;
                vc_notice(dev, "%s(): Set testing sensor height = %u", __FUNCTION__, g_height);
        }
        if (50000 <= val && val < 60000) {
                __u32 gain = val - 50000;
                vc_sen_set_gain(cam, gain);
                vc_notice(dev, "%s(): Set testing gain = %u", __FUNCTION__, gain);
        }
        if (60000 <= val && val < 70000) {
                __u32 shs_min = val - 60000;
                cam->ctrl.vmax.min = shs_min;
                vc_sen_set_exposure(cam, cam->state.exposure);
                vc_core_update_controls(cam);
                vc_core_print_debug(cam);
                vc_notice(dev, "%s(): Set testing shs_min = %u", __FUNCTION__, shs_min);
        }
        if (70000 <= val && val < 80000) {
                __u32 vmax = val - 70000;
                cam->ctrl.vmax.def = vmax;
                vc_sen_set_exposure(cam, cam->state.exposure);
                vc_core_update_controls(cam);
                vc_core_print_debug(cam);
                vc_notice(dev, "%s(): Set testing vmax = %u", __FUNCTION__, vmax);
        }
        if (80000 <= val && val < 90000) {
                g_sleepS = val - 80000;
                vc_notice(dev, "%s(): Set testing sleepS = %u", __FUNCTION__, g_sleepS);
        }
        if (90000 <= val && val < 100000) {
                g_sleepR = val - 90000;
                vc_notice(dev, "%s(): Set testing sleepR = %u", __FUNCTION__, g_sleepR);
        }
        if (100000 <= val && val < 110000) {
                g_sleepP = val - 100000;
                vc_notice(dev, "%s(): Set testing sleepP = %u", __FUNCTION__, g_sleepP);
        }

        return 0;
}
#endif

static int vc_start_streaming(struct tegracam_device *tc_dev)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        int reset;
        int sleepR = 200;
        int sleepS = 200; 
        int ret = 0;

#ifdef VC_MIPI_JETSON_NANO
        switch (cam->desc.mod_id) {
        case MOD_ID_IMX183: sleepR = 100; sleepS =  50; break;
        case MOD_ID_IMX226: sleepR = 100; sleepS =  50; break;
        case MOD_ID_IMX273: sleepR = 100; sleepS =  10; break;
        case MOD_ID_IMX296: sleepR = 200; sleepS =  10; break;
        case MOD_ID_IMX412: sleepR =   0; sleepS =   0; break;
        case MOD_ID_IMX565: sleepR = 200; sleepS = 200; break;
        default: sleepR = 100; sleepS = 50; break;
        }
#endif
        if (g_sleepR != 0) {
                sleepR = g_sleepR;
        }
        if (g_sleepS != 0) {
                sleepS = g_sleepS;
        }
        ret  = vc_mod_set_mode(cam, &reset);
        if (!ret && reset) {
                usleep_range(1000*sleepR, 1000*sleepR);
        }
        ret |= vc_sen_set_roi(cam);
        ret |= vc_sen_set_exposure(cam, cam->state.exposure);
        if (!ret && reset) {
                ret |= vc_sen_set_gain(cam, cam->state.gain);
                ret |= vc_sen_set_blacklevel(cam, cam->state.blacklevel);
        }
        ret |= vc_sen_start_stream(cam);
        // ****************************************************************************************
        // NOTE: On some camera modules (e.g. IMX183, IMX273) the second and/or third image is 
        //       black if here isn't a sleep.
        usleep_range(1000*sleepS, 1000*sleepS);

        cam->state.former_binning_mode = cam->state.binning_mode;
        return ret;
}

static int vc_stop_streaming(struct tegracam_device *tc_dev)
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        int ret = 0;
        
        ret = vc_sen_stop_stream(cam);
        usleep_range(1000*g_sleepP, 1000*g_sleepP);

        return ret;
}

// NOTE: Don't remove this function. It is needed by the Tegra Framework. 
static int vc_set_group_hold(struct tegracam_device *tc_dev, bool val) {return 0;}

// NOTE: Don't remove this function. It is needed by the Tegra Framework. 
static int vc_power_get(struct tegracam_device *tc_dev) {return 0;}

// NOTE: Don't remove this function. It is needed by the Tegra Framework. 
// (called by tegracam_device_register)
static struct camera_common_pdata *vc_parse_dt(struct tegracam_device *tc_dev)
{
        return devm_kzalloc(tc_dev->dev, sizeof(struct camera_common_pdata), GFP_KERNEL);
}

static struct camera_common_sensor_ops vc_sensor_ops = {
        .frmfmt_table = NULL,
        .read_reg = vc_read_reg,
        .write_reg = vc_write_reg,
        .set_mode = vc_set_mode,
        .start_streaming = vc_start_streaming,
        .stop_streaming = vc_stop_streaming,
        .power_get = vc_power_get,
        .parse_dt = vc_parse_dt,
};

int vc_init_frmfmt(struct device *dev, struct vc_cam *cam)
{
        struct camera_common_frmfmt *frmfmt = NULL;
        int *fps = NULL;

        vc_dbg(dev, "%s(): Allocate memory and init frame parameters for %s\n", __FUNCTION__,
                cam->desc.sen_type);

        frmfmt = devm_kzalloc(dev, sizeof(*frmfmt), GFP_KERNEL);
        if (!frmfmt)
                return -ENOMEM;

        vc_sensor_ops.frmfmt_table = frmfmt;
        vc_sensor_ops.numfrmfmts = 1;

        fps = devm_kzalloc(dev, sizeof(int), GFP_KERNEL);
        if (!fps)
                return -ENOMEM;

        frmfmt->framerates = fps;
        frmfmt->num_framerates = 1;

        fps[0] = cam->ctrl.framerate.def;
        
        frmfmt->size.width = cam->ctrl.frame.width;
        frmfmt->size.height = cam->ctrl.frame.height;
        frmfmt->hdr_en = 0;
        frmfmt->mode = 0;

        vc_notice(dev, "%s(): Init frame (width: %d, height: %d, fps: %d)\n", __FUNCTION__,
                frmfmt->size.width, frmfmt->size.height, frmfmt->framerates[0]);
        return 0;
}

static int vc_init_lanes(struct tegracam_device *tc_dev) 
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        struct device *dev = tc_dev->dev;
        struct sensor_mode_properties *mode = tegracam_to_mode(tc_dev, 0);
        struct sensor_signal_properties *signal;
        int ret;

        if (mode != NULL) {
                signal = &mode->signal_properties;

                ret = vc_core_set_num_lanes(cam, signal->num_lanes);
                if (ret) 
                        return ret;

                vc_notice(dev, "%s(): Init lanes (num_lanes: %d)\n", __FUNCTION__, signal->num_lanes);
                return 0;
        }
        return -EINVAL;
}

static int read_property_u32(struct device_node *node, const char *name, int radix, __u32 *value)
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

static void vc_init_io(struct device *dev, struct vc_cam *cam)
{
        struct device_node *node = dev->of_node;
        int value = 1;
        int ret = 0;

        vc_notice(dev, "%s(): Init trigger and flash mode\n", __FUNCTION__);

        if (node != NULL) {
                ret = read_property_u32(node, "trigger_mode", 10, &value);
                if (ret) {
                        vc_err(dev, "%s(): Unable to read trigger_mode from device tree!\n", __FUNCTION__);
                } else {
                        vc_mod_set_trigger_mode(cam, value);
                }

                ret = read_property_u32(node, "io_mode", 10, &value);
                if (ret) {
                        vc_err(dev, "%s(): Unable to read io_mode from device tree!\n", __FUNCTION__);
                } else {
                        vc_mod_set_io_mode(cam, value);
                }
        }
}

static void vc_init_binning(struct device *dev, struct vc_cam *cam)
{
        struct vc_ctrl *ctrl = &cam->ctrl;
        struct device_node *np = dev->of_node;
        struct device_node *node_tmp = NULL;
        char temp_str[OF_MAX_STR_LEN];
        int err = 0;
        int i = 0;

        vc_notice(dev, "%s(): Init binning modes\n", __FUNCTION__);

        for (i = 0; i < MAX_NUM_SENSOR_MODES; i++) {
                ctrl->dt_binning_modes[i].mode_set = false;
                snprintf(temp_str, sizeof(temp_str), "%s%d", OF_SENSORMODE_PREFIX, i);
                node_tmp = of_get_child_by_name(np, temp_str);
                of_node_put(node_tmp);
                if (NULL == node_tmp) {
                        vc_dbg(dev, "%s(): Could not get child node pointer from device tree mode%d!\n", __FUNCTION__, i);
                        continue;
                }

                // The binning_mode info is optional, so there is no problem
                // when it is missing.
                err = read_property_u32(node_tmp, "binning_mode", 10, &ctrl->dt_binning_modes[i].binning_mode);
                if (err) {
                        vc_dbg(dev, "%s(): Unable to read binning_mode from device tree mode%d!\n", __FUNCTION__, i);
                        continue;
                }
                ctrl->dt_binning_modes[i].mode_set = true;
        }
}

void vc_init_tegra_controls(struct tegracam_device *tc_dev) 
{
        struct vc_cam *cam = tegracam_to_cam(tc_dev);
        struct device *dev = tc_dev->dev;
        struct sensor_mode_properties *mode = tegracam_to_mode(tc_dev, 0);
        struct sensor_control_properties *control;

        if (mode != NULL) {
                control = &mode->control_properties;

                vc_notice(dev, "%s(): Read control gain (min: %d, max: %d, default: %d)\n", __FUNCTION__, 
                        control->min_gain_val, control->max_gain_val, control->default_gain);

                vc_notice(dev, "%s(): Overwrite control exposure (min: %d, max: %d, default: %d)\n", __FUNCTION__, 
                        cam->ctrl.exposure.min, cam->ctrl.exposure.max, cam->ctrl.exposure.def);
                control->exposure_factor = 1;
                control->min_exp_time.val = cam->ctrl.exposure.min;
                control->max_exp_time.val = cam->ctrl.exposure.max;
                control->default_exp_time.val = cam->ctrl.exposure.def;
                control->step_exp_time.val = 1;

                vc_notice(dev, "%s(): Overwrite control framerate (min: %d, max: %d, default: %d)\n", __FUNCTION__, 
                        cam->ctrl.framerate.min, cam->ctrl.framerate.max, cam->ctrl.framerate.def);
                control->framerate_factor = 1;
                control->min_framerate = cam->ctrl.framerate.min;
                control->max_framerate = cam->ctrl.framerate.max;
                control->default_framerate = cam->ctrl.framerate.def;
                control->step_framerate = 1;
        }

        // NOTE: Set this state to enable controls in tegracam_ctrls.c -> tegracam_set_ctrls
        tc_dev->s_data->power->state = SWITCH_ON;
}

static const struct regmap_config vc_regmap_config = {
        .reg_bits = 16,
        .val_bits = 8,
        .cache_type = REGCACHE_RBTREE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,5,0)
        .use_single_rw = true,
#else
        .use_single_read = true,
#endif
};

static const __u32 ctrl_cid_list[] = {
        TEGRA_CAMERA_CID_SENSOR_MODE_ID,
        TEGRA_CAMERA_CID_GAIN,
        TEGRA_CAMERA_CID_EXPOSURE,
        TEGRA_CAMERA_CID_BLACK_LEVEL,
        TEGRA_CAMERA_CID_FRAME_RATE,
        TEGRA_CAMERA_CID_TRIGGER_MODE,
        TEGRA_CAMERA_CID_IO_MODE,
        TEGRA_CAMERA_CID_SINGLE_TRIGGER,
        TEGRA_CAMERA_CID_BINNING_MODE,
#ifdef VC_CTRL_VALUE
        TEGRA_CAMERA_CID_VALUE,
#endif
};

static struct tegracam_ctrl_ops vc_ctrl_ops = {
        .numctrls = ARRAY_SIZE(ctrl_cid_list),
        .ctrl_cid_list = ctrl_cid_list,
        .set_gain = vc_set_gain,
        .set_exposure = vc_set_exposure,
        .set_black_level = vc_set_black_level,
        .set_single_trigger = vc_set_single_trigger,
        .set_binning_mode = vc_set_binning_mode,
        .set_frame_rate = vc_set_frame_rate,
        .set_trigger_mode = vc_set_trigger_mode,
        .set_io_mode = vc_set_io_mode,
        .set_group_hold = vc_set_group_hold,
#ifdef VC_CTRL_VALUE
        .set_value = vc_set_value,
#endif
};

static int vc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
        // --------------------------------------------------------------------
        // NOTE: Don't modify this lines of code. It will cause a kernel crash.
        // "Unable to handle kernel paging request at virtual address 7dabb7951ec6f65c"
        struct device *dev = &client->dev;
        struct vc_cam *cam;
        struct tegracam_device *tc_dev;
#if defined(VC_MIPI_JETSON_NANO) && defined(VC_MIPI_L4T_32_7_4)
        struct camera_common_data *common_data;
#endif
        int ret;

        vc_notice(dev, "%s(): Probing UNIVERSAL VC MIPI Driver (v%s)\n", __func__, VERSION);
        // --------------------------------------------------------------------

         cam = devm_kzalloc(dev, sizeof(struct vc_cam), GFP_KERNEL);
        if (!cam)
                return -ENOMEM;

        tc_dev = devm_kzalloc(dev, sizeof(struct tegracam_device), GFP_KERNEL);
        if (!tc_dev)
                return 0;

#if defined(VC_MIPI_JETSON_NANO) && defined(VC_MIPI_L4T_32_7_4)
        common_data = devm_kzalloc(&client->dev, sizeof(struct camera_common_data), GFP_KERNEL);
        if (!common_data) {
                return -ENOMEM;
        }
#endif

        ret = vc_core_init(cam, client);
        if (ret) {
                vc_err(dev, "%s(): Error in vc_core_init!\n", __func__);
                return 0;
        }
        vc_init_io(dev, cam);
        vc_init_frmfmt(dev, cam);
        vc_init_binning(dev, cam);

        // Defined in tegracam_core.c
        // Initializes 
        //   * tc_dev->s_data
        // Calls 
        //   * camera_common_initialize
        tc_dev->client = client;
        tc_dev->dev = dev;
        strncpy(tc_dev->name, "vc_mipi", sizeof(tc_dev->name));
        tc_dev->dev_regmap_config = &vc_regmap_config;
        tc_dev->sensor_ops = &vc_sensor_ops;
        ret = tegracam_device_register(tc_dev);
        if (ret) {
                vc_err(dev, "%s(): Tegra camera device registration failed\n", __FUNCTION__);
                // goto free_vc_core;
                return 0;
        }

        // Defined in tegracam_core.c
        // Initializes
        //   * tc_dev->priv
        //   * tc_dev->s_data->priv
        tegracam_set_privdata(tc_dev, (void *)cam);
        
        // Defined in tegracam_v4l2.c
        // Initializes
        //   * tc_dev->s_data->tegracam_ctrl_hdl
        tc_dev->tcctrl_ops = &vc_ctrl_ops;
        ret = tegracam_v4l2subdev_register(tc_dev, true);
        if (ret) {
                       vc_err(dev, "%s(): Tegra camera subdev registration failed\n", __FUNCTION__);
                       // goto unregister_tc_dev;
                return 0;
            }

        // This functions need tc_dev->s_data to be initialized.
        ret = vc_init_lanes(tc_dev);
        if (ret)
                goto unregister_subdev;

        vc_init_tegra_controls(tc_dev);
        
        return 0;

unregister_subdev:
        tegracam_v4l2subdev_unregister(tc_dev);
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
        { "vc_mipi", 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, vc_id);

static const struct of_device_id vc_dt_ids[] = {
        { .compatible = "nvidia,vc_mipi", },
        { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, vc_dt_ids);

static struct i2c_driver vc_i2c_driver = {
        .driver = {
                .name = "vc_mipi",
                .owner = THIS_MODULE,
                .of_match_table = of_match_ptr(vc_dt_ids),
        },
        .id_table = vc_id,
        .probe = vc_probe,
        .remove = vc_remove,
};
module_i2c_driver(vc_i2c_driver);

MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Vision Components GmbH - VC MIPI NVIDIA driver");
MODULE_AUTHOR("Peter Martienssen, Liquify Consulting <peter.martienssen@liquify-consulting.de>");
MODULE_LICENSE("GPL v2");