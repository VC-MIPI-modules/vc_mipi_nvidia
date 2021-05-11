LITEND img.org (fmt: RG10, dx: 1920, dy: 1080, pitch: 3840) - 7c02 4f02 7302 4602 7d02 3802 7f02 6702 7a02 4e02

DUNKEL img.org (fmt: RG10, dx: 1920, dy: 1080, pitch: 3840) - 0068 0064 0067 0064 0065 0063 0067 0064 0065 0064 
HELL   img.org (fmt: RG10, dx: 1920, dy: 1080, pitch: 3840) - 1023 1023 1023 1023 1023 1023 1023 1023 1023 1023 
ROT    img.org (fmt: RG10, dx: 1920, dy: 1080, pitch: 3840) - 0175 0136 0176 0139 0169 0129 0173 0137 0173 0142
GRUN   img.org (fmt: RG10, dx: 1920, dy: 1080, pitch: 3840) - 0107 0103 0109 0111 0107 0107 0114 0107 0105 0104 


peter@nano:~$ ./test/vcmipidemo -fa
Activating /dev/fb0 framebuffer output.
Suppressing ASCII capture at st[  176.981489] imx327c 7-001a: imx327c_power_on: Set power on dout.
comm[  177.008284] imx327c 7-001a: imx327c_set_mode(): fmt_width,fmt_height=1920,1080 pix_fmt=0x30314752 'RG10'   
                   cam_mode=0, err=0
comm[  177.020776] imx327c 7-001a: imx327c_start_streaming: Start stream
ctrl[  177.027933] imx327c 7-001a: imx327c_set_gain: Set gain = 10
ctrl[  177.033662] imx327c 7-001a: imx327c_set_gain: Set gain = 10
ctrl[  177.039671] imx327c 7-001a: imx327c_set_exposure: Set exposure = 5000
    [  177.046280] imx327c 7-001a: imx327c_set_exposure: min_exp_time,max_exp_time=29,7767184 
                   default_exp_time=10000
    [  177.056599] imx327c 7-001a: imx327c_set_exposure(): exposure_time=5000, SHS=0x3d3 (979)
ctrl[  177.066759] imx327c 7-001a: imx327c_set_frame_rate: Set frame rate = 30000000
comm[  177.130207] imx327c 7-001a: imx327c_start_streaming(): err=0
comm[  177.136338] imx327c 7-001a: imx327c_stop_streaming: Stop stream
    [  177.142606] imx327c 7-001a: vc_mipi_reset: Rest
    [  177.254227] imx327c 7-001a: vc_mipi_reset(): sensor_mode=-1 err=0
    [  177.260481] imx327c 7-001a: vc_mipi_reset: Rest
    [  177.372092] imx327c 7-001a: vc_mipi_reset(): sensor_mode=0 err=0
    [  177.382737] tegra-vii2c 546c0000.i2c: no acknowledge from address 0x1a
    [  177.389492] regmap_util_write_table_8:regmap_util_write_table:-121[  177.396016] imx327c 7-001a:  
                   imx327c_stop_streaming(): im1
comm[  177.405975] imx327c 7-001a: imx327c_stop_streaming(): err=-121
    [  177.405975]
    [  177.417471] imx327c 7-001a: Error turning off streaming
    [  177.425061] vi 54080000.vi: calibration failed with -121 error
comm[  177.432928] imx327c 7-001a: imx327c_power_on: Set power on
    [  177.684696] video4linux video0: tegra_channel_capture_frame_single_thread: frame start syncpt timeout! i=0: 
                   Before tegra_chan.
    [  177.726185] vi 54080000.vi: TEGRA_CSI_PIXEL_PARSER_STATUS 0x00000000
    [  177.756655] vi 54080000.vi: TEGRA_CSI_CIL_STATUS 0x00000000
    [  177.772689] vi 54080000.vi: TEGRA_CSI_CILX_STATUS 0x00000000
    [  177.988401] video4linux video0: tegra_channel_capture_frame_single_thread: frame start syncpt timeout! i=0: 
                   Before tegra_ch.
    [  178.031171] vi 54080000.vi: TEGRA_CSI_PIXEL_PARSER_STATUS 0x00000000
    [  178.061069] vi 54080000.vi: TEGRA_CSI_CIL_STATUS 0x00000010
    [  178.077654] vi 54080000.vi: TEGRA_CSI_CILX_STATUS 0x00040040
comm[  178.088250] imx327c 7-001a: imx327c_stop_streaming: Stop stream
    [  178.096824] imx327c 7-001a: vc_mipi_reset: Rest
    [  178.210330] imx327c 7-001a: vc_mipi_reset(): sensor_mode=-1 err=0
    [  178.218382] imx327c 7-001a: vc_mipi_reset: Rest
    [  178.332560] imx327c 7-001a: vc_mipi_reset(): sensor_mode=0 err=0
comm[  178.397052] imx327c 7-001a: imx327c_stop_streaming(): err=0
    [  178.397052]       
    [  178.460212] misc tegra_camera_ctrl: tegra_camera_update_isobw: Warning, Requested ISO BW     
                   18446744073709195366 has been capped0
comm[  178.481633] imx327c 7-001a: imx327c_power_off: Set power off