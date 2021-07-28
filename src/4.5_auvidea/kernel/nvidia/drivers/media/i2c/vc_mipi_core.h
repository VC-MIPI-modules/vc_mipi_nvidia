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

#define FLAG_MODE_1_LANE        0x01
#define FLAG_MODE_2_LANES       0x02
#define FLAG_MODE_4_LANES       0x04
#define FLAG_TRIGGER_IN         0x08
#define FLAG_FLASH_OUT          0x10

struct vc_desc {
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
	__u8 regs[50];
	__u16 nr_modes;
	__u16 bytes_per_mode;
	__u8 mode1[16];
	__u8 mode2[16];
	__u8 mode3[16];
	__u8 mode4[16];
};

struct vc_control {
	__u32 enabled;
	__u32 min;
	__u32 max;
	__u32 default_val;
};

struct vc_fmt {
	__u32 code;
	enum v4l2_colorspace colorspace;
};

struct vc_frame {
	__u32 width;
	__u32 height;
};

struct vc_mode {
	__u32 code;
	__u8 flags;
	__u8 value;
};

struct vc_csr2 {
	__u32 l;
	__u32 m;
};

struct vc_csr3 {
	__u32 l;
	__u32 m;
	__u32 h;
};

struct vc_csr4 {
	__u32 l;
	__u32 m;
	__u32 h;
	__u32 u;
};

struct vc_sen_csr {
	struct vc_csr2 mode;
	__u8 mode_standby;
	__u8 mode_operating;
	struct vc_csr3 vmax;
	struct vc_csr3 expo;
	struct vc_csr4 expo2;
	struct vc_csr4 retrig;
	struct vc_csr2 gain;
	struct vc_csr2 o_width;
	struct vc_csr2 o_height;
};

struct vc_csr {
	struct vc_sen_csr sen;
};

struct vc_ctrl {
	int mod_id;
	int mod_i2c_addr;
	// Communication
	struct i2c_client *client_sen;
	struct i2c_client *client_mod;
	// Controls
	struct vc_control exposure;
	struct vc_control gain;
	struct vc_control framerate;
	// Modes & Frame Formats
	struct vc_mode *modes;
	int default_mode;
	struct vc_fmt *fmts;
	int default_fmt;
	struct vc_frame o_frame;
	// Control and status registers
	struct vc_csr csr;
	// Exposure
	__u32 sen_clk;
	__u32 expo_time_min2;
	__u32 expo_vmax;
	__u32 expo_toffset;
	__u32 expo_h1period;
	// Features
	__u8 flags;
};

struct vc_state {
	__u32 exposure;
	__u32 gain;
	// Modes & Frame Formats
	struct vc_fmt *fmt;
	struct vc_frame frame;
	// Features
	__u8 flags;
	// Status flags
	int power_on;
	int streaming;
};

struct vc_cam {
	struct vc_desc desc;
	struct vc_ctrl ctrl;
	struct vc_state state;
};


// --- Helper functions for internal data structures --------------------------
int vc_core_set_num_lanes(struct vc_cam *cam, __u32 number);
struct vc_fmt *vc_core_find_format(struct vc_cam *cam, __u32 code);
int vc_core_set_format(struct vc_cam *cam, __u32 code);
__u32 vc_core_get_format(struct vc_cam *cam);
int vc_core_set_frame(struct vc_cam *cam, __u32 width, __u32 height);
struct vc_frame *vc_core_get_frame(struct vc_cam *cam);

// --- Function to initialze the vc core --------------------------------------
int vc_core_init(struct vc_cam *cam, struct i2c_client *client);

// --- Functions for the VC MIPI Controller Module ----------------------------
int vc_mod_set_power(struct vc_cam *cam, int on);
int vc_mod_set_mode(struct vc_cam *cam);
void vc_mod_set_trigger_in(struct vc_cam *cam, int enable);
void vc_mod_set_flash_out(struct vc_cam *cam, int enable);

// --- Functions for the VC MIPI Sensors --------------------------------------
int vc_sen_set_roi(struct vc_cam *cam, int width, int height);
int vc_sen_set_gain(struct vc_cam *cam, int value);
int vc_sen_start_stream(struct vc_cam *cam);
int vc_sen_stop_stream(struct vc_cam *cam);

// TODO: Cleaned up
int vc_sen_set_exposure_dirty(struct vc_cam *cam, int value);

#endif // _VC_MIPI_CORE_H