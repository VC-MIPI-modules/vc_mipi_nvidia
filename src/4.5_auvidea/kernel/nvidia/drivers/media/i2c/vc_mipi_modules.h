#ifndef _VC_MIPI_MODULES_H
#define _VC_MIPI_MODULES_H

#include "vc_mipi_core.h"

#define MOD_ID_OV9281           0x9281
#define MOD_ID_IMX183           0x0183
#define MOD_ID_IMX226           0x0226
#define MOD_ID_IMX250           0x0250
#define MOD_ID_IMX252           0x0252
#define MOD_ID_IMX273           0x0273
#define MOD_ID_IMX290           0x0290
#define MOD_ID_IMX296           0x0296
#define MOD_ID_IMX297           0x0297
#define MOD_ID_IMX327           0x0327
#define MOD_ID_IMX415           0x0415

int vc_mod_ctrl_init(struct vc_ctrl* ctrl, struct vc_desc* desc);

#endif // _VC_MIPI_MODULES_H
