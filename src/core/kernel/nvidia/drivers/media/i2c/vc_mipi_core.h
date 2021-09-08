#ifndef _VC_MIPI_CORE_H
#define _VC_MIPI_CORE_H

#define DEBUG

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/videodev2.h>

#define vc_dbg(dev, fmt, ...) dev_dbg(dev, fmt, ##__VA_ARGS__)
#define vc_info(dev, fmt, ...) dev_info(dev, fmt, ##__VA_ARGS__)
#define vc_notice(dev, fmt, ...) dev_err(dev, fmt, ##__VA_ARGS__)
#define vc_err(dev, fmt, ...) dev_err(dev, fmt, ##__VA_ARGS__)

#define FLAG_FLASH_ENABLED        0x0001
#define FLAG_READ_VMAX            0x0002
#define FLAG_TRIGGER_DISABLE      0x0004
#define FLAG_TRIGGER_EXTERNAL     0x0008
#define FLAG_TRIGGER_PULSEWIDTH   0x0010
#define FLAG_TRIGGER_SELF         0x0020
#define FLAG_TRIGGER_SINGLE  	  0x0040
#define FLAG_TRIGGER_SYNC         0x0080
#define FLAG_TRIGGER_STREAM_EDGE  0x0100
#define FLAG_TRIGGER_STREAM_LEVEL 0x0200

struct vc_desc_mode {
	__u8 data_rate[4];
	__u8 num_lanes;
	__u8 format;
	__u8 type;
	__u8 binning;
	__u8 reserved2[8];
};

struct vc_desc {
	// Module description
	__u8 magic[12];
	__u8 manuf[32];
	__u16 manuf_id;
	__u8 sen_manuf[8];
	__u8 sen_type[16];
	__u16 mod_id;
	__u16 mod_rev;
	__u16 chip_id_high;
	__u16 chip_id_low;
	__u16 chip_rev;
	// Sensor registers
	__u16 csr_mode;
	__u16 csr_h_start_h;
	__u16 csr_h_start_l;
	__u16 csr_v_start_h;
	__u16 csr_v_start_l;
	__u16 csr_h_end_h;
	__u16 csr_h_end_l;
	__u16 csr_v_end_h;
	__u16 csr_v_end_l;
	__u16 csr_o_width_h;
	__u16 csr_o_width_l;
	__u16 csr_o_height_h;
	__u16 csr_o_height_l;
	__u16 csr_exposure_h;
	__u16 csr_exposure_m;
	__u16 csr_exposure_l;
	__u16 csr_gain_h;
	__u16 csr_gain_l;
	// Exposure Settings
	__u32 clk_ext_trigger;
	__u32 clk_pixel;
	__u16 shutter_offset;
	// Reserved
	__u8 reserved[4];
	// Modes
	__u16 num_modes;
	__u16 bytes_per_mode;
	struct vc_desc_mode modes[24];
};

typedef struct vc_control {
	__u32 min;
	__u32 max;
	__u32 def;
} vc_control;

typedef struct vc_size {
	__u32 width;
	__u32 height;
} vc_size;

typedef struct vc_csr2 {
	__u32 l;
	__u32 m;
} vc_csr2;

typedef struct vc_csr4 {
	__u32 l;
	__u32 m;
	__u32 h;
	__u32 u;
} vc_csr4;

struct vc_sen_csr {
	struct vc_csr2 mode;
	__u8 mode_standby;
	__u8 mode_operating;
	struct vc_csr4 vmax;
	struct vc_csr4 hmax;
	struct vc_csr4 expo;
	struct vc_csr2 gain;
	struct vc_csr2 o_width;
	struct vc_csr2 o_height;
};

struct vc_csr {
	struct vc_sen_csr sen;
};

struct vc_ctrl {
	// Communication
	int mod_i2c_addr;
	struct i2c_client *client_sen;
	struct i2c_client *client_mod;
	// Controls
	struct vc_control exposure;
	struct vc_control gain;
	struct vc_control framerate;
	// Modes & Frame Formats
	struct vc_size framesize;	// Pixel
	// Control and status registers
	struct vc_csr csr;
	// Exposure
	__u32 sen_clk;			// Hz
	__u32 expo_period_1H;
	__u32 expo_toffset;
	__u32 expo_shs_min;
	__u32 expo_vmax;
	// __u32 expo_time_min2;
	// Special features
	__u16 flags;
};

struct vc_state {
	__u8 mode;
	__u32 exposure;			// µs
	__u32 gain;
	__u32 shs;
	__u32 vmax;
	__u32 exposure_cnt;
	__u32 retrigger_cnt;
	__u32 framerate;
	__u32 format_code;
	struct vc_size framesize;
	__u8 num_lanes;
	__u8 trigger_mode;
	int power_on;
	int streaming;
	__u8 flags;
};

struct vc_cam {
	struct vc_desc desc;
	struct vc_ctrl ctrl;
	struct vc_state state;
};

// --- Helper functions to allow i2c communication for customization ----------
int vc_read_i2c_reg(struct i2c_client *client, const __u16 addr);
int vc_write_i2c_reg(struct i2c_client *client, const __u16 addr, const __u8 value);

// --- Helper functions for internal data structures --------------------------
struct device *vc_core_get_sen_device(struct vc_cam *cam);
struct device *vc_core_get_mod_device(struct vc_cam *cam);
int vc_core_try_format(struct vc_cam *cam, __u32 code);
int vc_core_set_format(struct vc_cam *cam, __u32 code);
__u32 vc_core_get_format(struct vc_cam *cam);
int vc_core_set_frame(struct vc_cam *cam, __u32 width, __u32 height);
struct vc_size *vc_core_get_frame(struct vc_cam *cam);
int vc_core_set_num_lanes(struct vc_cam *cam, __u32 number);
__u32 vc_core_get_num_lanes(struct vc_cam *cam);
int vc_core_set_framerate(struct vc_cam *cam, __u32 framerate);
__u32 vc_core_get_framerate(struct vc_cam *cam);

// --- Function to initialze the vc core --------------------------------------
int vc_core_init(struct vc_cam *cam, struct i2c_client *client);

// --- Functions for the VC MIPI Controller Module ----------------------------
int vc_mod_set_power(struct vc_cam *cam, int on);
int vc_mod_set_mode(struct vc_cam *cam);
int vc_mod_is_trigger_enabled(struct vc_cam *cam);
int vc_mod_set_trigger_mode(struct vc_cam *cam, int mode);
int vc_mod_get_trigger_mode(struct vc_cam *cam);
int vc_mod_is_flash_enabled(struct vc_cam *cam);
int vc_mod_set_flash_mode(struct vc_cam *cam, int mode);

// --- Functions for the VC MIPI Sensors --------------------------------------
int vc_sen_set_roi(struct vc_cam *cam, int width, int height);
int vc_sen_set_gain(struct vc_cam *cam, int gain);
int vc_sen_set_exposure(struct vc_cam *cam, int exposure);
int vc_sen_start_stream(struct vc_cam *cam);
int vc_sen_stop_stream(struct vc_cam *cam);

#endif // _VC_MIPI_CORE_H