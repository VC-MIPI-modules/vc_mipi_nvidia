Struktur des Treiberpakets
===========================================================================================================================================
Datei                           Jetpack Version |43 |43a|441| Beschreibung
------------------------------------------------+---+---+---+------------------------------------------------------------------------------
/hardware/nvidia/platform/t210/porg/kernel-dts  |   |   |   |  
    tegra210-porg-p3448-common.dtsi             | x | ? | x | Inkludiert tegra210-porg-vc-camera.dtsi
    /porg-platforms                             |   |   |   |
        vc-camera.dtsi                  DRV >>> | x | ? | x | Hardwarebeschreibungsdatei der VC Kamera.
    /porg-plugin-manager                        |   |   |   |
        tegra210-porg-plugin-manager.dtsi       | x | ? | x | fragments und overrides für IMX219 entfernt.
                                                |   |   |   | Im Gegenzug wird in den dtsi der VC Cameras fragments und overrides 
                                                |   |   |   | hinzugefügt.
/kernel                                         |   |   |   |
    /kernel-4.9/arch/arm64/configs              |   |   |   |
        tegra_defconfig                         | x | ? | x | Ergänzt um CONFIG_VIDEO_VC_CAMERA=y
    /nvidia/drivers/media                       |   |   |   |
        /i2c                                    |   |   |   |
            vc-camera.c                 DRV >>> | x | ? | x | Implementiert den Treiber.
            Kconfig                             | x | ? | x | Ergänzt Option und Beschreibung der Kernelkonfiguration.
            Makefile                            | x | ? | x | Ergänzt den Build-Prozess um die Objektdatei vc-camera.o des Treibers.
            vc_mipi.h                           | x | ? | x | (Optional) Allgemeine Makros für alle VC Treiber.
        /platform/tegra/camera                  |   |   |   |
            camera_common.c             FMT >>> | x | x | x | Ergänzt um die Formate GREY, Y10, Y12.
                                                |   |   |   | (Optional) Funktion camera_common_try_fmt, camera_common_s_fmt, 
                                                |   |   |   |            camera_common_g_fmt ergänzt um dev_err Traces.
            sensor_common.c             FMT >>> | x | x | x | Ergänzt um die Formate "gray", "y10", "y12", "bayer_rggb8".
                                                |   |   |   | (Optional) Funktion extract_pixel_format
                                                |   |   |   |            ergänzt um pr_err Traces.
            /csi                                |   |   |   |
                csi2_fops.c                     | x | x | x | (Optional) Funktionen set_sensor_model und get_sensor_model ergänzt zum 
                                                |   |   |   |            Speichern der sensor_model Bezeichnung.
                                                |   |   |   | (Optional) Funktionen tegra_csi_status, csi2_start_streaming, 
                                                |   |   |   |            csi2_stop_streaming, csi2_mipi_cal um DEV_DBG Traces ergänzt.
                csi4_fops.c             INI >>> | x | x | x | Funktion csi4_stream_init um 3x csi4_stream_write ergänzt. (ACHTUNG!)
                                                |   |   |   | (Optional) Funktionen csi4_stream_check_status, csi4_start_streaming,
                                                |   |   |   |            csi4_stop_streaming
                                                |   |   |   |            um dev_info Traces ergänzt.
            /vi                                 |   |   |   | 
                channel.c                       | x | x | x | (Optional) Funktionen tegra_channel_queue_setup, 
                                                |   |   |   |            tegra_channel_alloc_buffer_queue um dev_err Traces ergänzt.
                                        STR >>> |   |   | x | Funktion tegra_channel_update_format, tegra_channel_fmts_bitmap_init,
                                                |   |   |   | free_ring_buffers, tegra_channel_set_stream, tegra_channel_s_dv_timings
                                                |   |   |   | tegra_channel_s_ctrl, __tegra_channel_set_format, tegra_channel_init
                                                |   |   |   | und struct common_custom_ctrls um STRIDE Feature ergänzt.
                core.c                          | x |   |   | (Keine Änderungen)
                vi2_fops.c                      | x | x | x | (Deaktiviert) Funktion tegra_channel_capture_setup,  
                                                |   |   |   |               tegra_channel_error_status ergänzt
                                                |   |   |   | (Optional) Funktionen vi2_channel_setup_queue, tegra_channel_capture_setup
                                                |   |   |   |            tegra_channel_ec_init, tegra_channel_clear_singleshot, 
                                                |   |   |   |            tegra_channel_vi_csi_recover, tegra_channel_capture_error,
                                                |   |   |   |            tegra_channel_ec_recover, tegra_channel_error_status,
                                                |   |   |   |            tegra_channel_capture_frame_single_thread,
                                                |   |   |   |            tegra_channel_capture_frame, tegra_channel_release_frame,
                                                |   |   |   |            tegra_channel_capture_done, tegra_channel_kthread_capture_start,
                                                |   |   |   |            vi2_channel_start_streaming
                vi2_formats.h           FMT >>> | x | x | x | Ergänzt um die Formate GREY, Y10, Y12 
                                                |   |   |   | (Deaktiviert) Ergänzt um das Format SRGGB10P
------------------------------------------------+---+---+---+------------------------------------------------------------------------------



Zusätzliche Änderungen im DT für Auvidea JNX30 in Bezug zu NVIDIA Jetpack 4.3 (22 Dateien geändert, 2+4 Dateien hinzugefügt)
===========================================================================================================================================
Datei                                                             | FCT | Beschreibung
------------------------------------------------------------------+-----+------------------------------------------------------------------
hardware/nvidia                                                   |     |
  26/platform/t210                                                |     |
      22/porg/kernel-dts                                          |     |
            tegra210-porg-p3448-common.dtsi                   >>> | V4L | Änderungen an spi@7000d600 und sdhci@700b0400
           2/porg-plugin-manager                                  |     |
                tegra210-porg-plugin-manager.dtsi                 | SD  | Änderungen an sdhci2 (SD Card) und 
                                                              >>> | V4L | IMX219 single und dual overrides entfernt
                tegra210-porg-eeprom-manager.dtsi             >>> | V4L | Änderungen an eeprom-manager für i2c1 und i2c3
          19/porg-platforms                                       |     |
                imx226.dtsi                               -+- >>> | V4L | Hinzugefügt DT für IMX296 und IMX226 
                imx226c.dtsi                               |      |     |  
                imx296_vgl_dual.dtsi                       |      |     | 
                imx296_vgl_quad.dtsi                      -+      |     |
                tegra210-camera-rbpcv2-quad-imx219.dtsi           | V4L | Hinzugefügt DT für 4x IMX219
                tegra210-porg-camera-rbpcv2-quad-imx219.dtsi      | V4L | Hinzugefügt DT für 4x IMX219
                tegra210-porg-camera-rbpcv2-dual-imx219.dtsi  ??? | V4L | Änderungen für 2x IMX219
                tegra210-porg-gpio-p3448-0000-a00.dtsi    -+-     | SD  | Änderungen an gpio@6000d000 damit spi1 und spi2 funktionieren
                tegra210-porg-gpio-p3448-0000-a01.dtsi     |      |     |
                tegra210-porg-gpio-p3448-0002-a02.dtsi     |      |     |          
                tegra210-porg-gpio-p3448-0002-b00.dtsi    -+      |     |           
                tegra210-porg-pinmux-p3448-0000-a00.dtsi  -+-     | SD  | Änderungen an pinmux@700008d4/../spi2_mosi_pb4, ../spi2_miso_pb5
                tegra210-porg-pinmux-p3448-0000-a01.dtsi   |      |     | ../spi2_sck_pb6, ../spi2_cs0_pb7, ../spi2_cs1_pdd0
                tegra210-porg-pinmux-p3448-0000-a02.dtsi   |      |     | und pinmux@700008d4/../spi1_mosi_pc0, ../spi1_miso_pc1, 
                tegra210-porg-pinmux-p3448-0000-b00.dtsi   |      |     | ../spi1_sck_pc2, ../spi1_cs0_pc3
                tegra210-porg-pinmux-p3448-0002-a02.dtsi   |      |     | spi1 und spi2 von unused_lowpower nach pinmux_default: common 
                tegra210-porg-pinmux-p3448-0002-b00.dtsi  -+      |     | verschoben
                tegra210-porg-pwm-fan.dtsi                        | FAN | Änderung an pwm_fan_shared_data: pfsd (Lüfter)
                tegra210-porg-super-module-e2614.dtsi         >>> | V4L | Änderung an i2c@7000c400/rt5659.1-001a@1a (Audio) deaktiviert 
       4/jetson/kernel-dts/                                       |     |
            tegra210-jetson-e-base-p2595-0000-a00.dts         >>> | V4L | Änderungen an i2c@546c0000
            tegra210-jetson-cv-base-p2597-2180-a00.dts        >>> | V4L | Änderungen an i2c@546c0000 und spi@7000d400
           2/jetson-platforms                                     |     |
                tegra210-jetson-e-pinmux-p2530-0930-e03.dtsi      | SD  | Änderungen an pinmux@700008d4/../spi2_mosi_pb4 und spi2_miso_pb5
                tegra210-jetson-cv-pinmux-p2597-2180-a00.dtsi     | SD  | Änderungen an pinmux@700008d4/../spi1_mosi_pc0 und spi1_miso_pc1
   2/soc/t210/kernel-dts/tegra210-soc/                            |     |
        tegra210-sdhci.dtsi                                       | SD  | Änderungen an sdhci@700b0400 (SD Card)
        tegra210-soc-base.dtsi                                >>> | V4L | Änderungen an i2c@546c0000
------------------------------------------------------------------+-----+------------------------------------------------------------------



Struktur des Device Trees (vc-camera.dtsi) 
===========================================================================================================================================
Element Kamera 1                        | Treiber                          | Beschreibung                   (Pfad: /hardware/nvidia/...)
----------------------------------------+----------------------------------+---------------------------------------------------------------
/tegra-camera-platform                  | "nvidia, tegra-camera-platform"  | num_csi_lanes = <4>
    /modules/module@0                   |                                  | badge = "porg_front_RBPCV2", position = "front";
        /drivernode0  >--               |                                  | pcl_id = "v4l2_sensor", devname = "imx296_vgl 6-001a"
/host1x                 |               |                                  |
    /vi/ports/port@0    |      IN <---  | ???                              | port-index = <0>, bus-width = <1>
    /nvcsi/channel@0    |            |  | ???                              |
            /port@1     |     OUT >---  |                                  |
            /port@0     |      IN <---  |                                  | port-index = <0>, bus-width = <1>
    /i2c@546c0000       v            |  | "nvidia,tegra210-vii2c"          | .../soc/t210/kernel-dts/tegra210-soc/tegra210-soc-base.dtsi
        /rbpcv2_imx296_vgl_a@1a      |  | "nvidia,imx296_vgl"              | vc-camera.dtsi
                                     |  |                                  | reset-gpios = <&gpio CAM2_PWDN GPIO_ACTIVE_HIGH>     
            /ports/port@0     OUT >---  |                                  |
            /mode0 - /modeX             |                                  |
                                        |                                  |
Elemente Kamera 2                       |                                  |
----------------------------------------+----------------------------------+---------------------------------------------------------------
/tegra-camera-platform                  | "nvidia, tegra-camera-platform"  | num_csi_lanes = <4>
    /modules/module@1                   |                                  | badge = "porg_rear_RBPCV2", position = "rear";
        /drivernode0  >--               |                                  | pcl_id = "v4l2_sensor", devname = "imx296_vgl 0-001a"
/host1x                 |               |                                  |
    /vi/ports/port@1    |      IN <---  | ???                              | port-index = <2>, bus-width = <1>
    /nvcsi/channel@1    |            |  | ???                              |
            /port@3     |     OUT >---  |                                  |
            /port@2     |      IN <---  |                                  | port-index = <2>, bus-width = <1>
    /i2c@7000c000       v            |  |                                  |
        rbpcv2_imx296_vgl_c@1a       |  | "nvidia,tegra210-i2c"            | .../soc/t210/kernel-dts/tegra210-soc/tegra210-soc-base.dtsi
                                     |  |                                  | reset-gpios = <&gpio CAM1_PWDN GPIO_ACTIVE_HIGH>     
            /ports/port@0     OUT >---  | "nvidia,imx296_vgl"              | vc-camera.dtsi
            /mode0 - /modeX             |                                  |
                                        |                                  |
Elemente für Kamera 1+2                 |                                  |
----------------------------------------+----------------------------------+---------------------------------------------------------------
/gpio@6000d000                          | ???                              |
    /camera-trigger-high                |                                  | gpio-hog, output-high, gpios = < CAM_TRIGGER 0>
    /camera-control-output-low          |                                  | gpio-hog  output-low, gpios = < CAM1_PWDN 0  CAM2_PWDN 0>
----------------------------------------+----------------------------------+---------------------------------------------------------------
#define CAM1_PWDN     TEGRA_GPIO(S, 7)
#define CAM2_PWDN     TEGRA_GPIO(T, 0)
#define CAM_TRIGGER   TEGRA_GPIO(V, 0)



Struktur des Treibers (vc-camera.c)
===========================================================================================================================================

G--- static int vc_mipi_common_reg_read(struct camera_common_data *s_data, u16 addr)
G--- static int vc_mipi_common_reg_write(struct camera_common_data *s_data, u16 addr, u8 val)
G--- static int imx226_write_table(struct imx226 *priv, const imx226_reg table[])
G--- static int reg_write(struct i2c_client *client, const u16 addr, const u8 data)
G--- static int reg_read(struct i2c_client *client, const u16 addr)

+--+ static int imx226_probe(struct i2c_client *client, const struct i2c_device_id *id)
|  |
|  | struct imx226_subdev_internal_ops
|  +--- static int imx226_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
|  |
|  |
|  | struct imx226_common_ops
|  +--+ static int imx226_power_on(struct camera_common_data *s_data)
|  |  +--- static void imx226_gpio_set(struct camera_common_data *s_data, unsigned int gpio, int val)
|  |
|  +--+ static int imx226_power_off(struct camera_common_data *s_data)
|  |  +--- static void imx226_gpio_set(struct camera_common_data *s_data, unsigned int gpio, int val)
|  |
|  +--- static int imx226_write_reg(struct camera_common_data *s_data, u16 addr, u8 val)
|  +--- static int imx226_read_reg(struct camera_common_data *s_data, u16 addr, u8 *val)
|  +--+ static struct camera_common_pdata *imx226_parse_dt(struct tegracam_device *tc_dev)
|  |  +--- static int read_property_u32(struct device_node *node, const char *name, int radix, u32 *value)
|  |
|  +--- static int imx226_power_get(struct tegracam_device *tc_dev)
|  +--- static int imx226_power_put(struct tegracam_device *tc_dev)
|  +--+ static int imx226_set_mode(struct tegracam_device *tc_dev)
|  |  +--- static int vc_mipi_reset (struct tegracam_device *tc_dev, int  sen_mode)
|  |
|  +--- static int imx226_start_streaming(struct tegracam_device *tc_dev)
|  +--+ static int imx226_stop_streaming(struct tegracam_device *tc_dev)
|  |  +--- static int vc_mipi_reset (struct tegracam_device *tc_dev, int  sen_mode)
|  |  +--- static int vc_mipi_common_trigmode_write(struct i2c_client *rom, u32 sensor_ext_trig, u32 exposure_time, u32 io_config, u32 enable_extrig, u32 sen_clk)
|  |
|  |
|  | struct imx226_ctrl_ops
|  +--- static int imx226_set_gain(struct tegracam_device *tc_dev, s64 val)
|  +--- static int imx226_set_exposure(struct tegracam_device *tc_dev, s64 val)
|  +--- static int imx226_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
|  +--- static int imx226_set_group_hold(struct tegracam_device *tc_dev, bool val)
|  |
|  |
|  +--- void set_sensor_model (char *model);
|  +--+ static int imx226_board_setup(struct imx226 *priv)
|     +--- static int vc_mipi_reset (struct tegracam_device *tc_dev, int  sen_mode )
|     +--- static int imx226_video_probe(struct i2c_client *client)
|     +--- static struct i2c_client * imx226_probe_vc_rom(struct i2c_adapter *adapter, u8 addr)
|
|
+--+ static int imx226_remove(struct i2c_client *client)



Sequenzdiagramme
===========================================================================================================================================
    imx226_probe
        - 
    imx226_parse_dt
    imx226_board_setup