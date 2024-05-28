#ifndef _VC_MIPI_MODULES_H
#define _VC_MIPI_MODULES_H

#include "vc_mipi_core.h"

#define MOD_ID_IMX178           0x0178
#define MOD_ID_IMX183           0x0183
#define MOD_ID_IMX226           0x0226
#define MOD_ID_IMX264           0x0264
#define MOD_ID_IMX265           0x0265
#define MOD_ID_IMX250           0x0250
#define MOD_ID_IMX252           0x0252
#define MOD_ID_IMX273           0x0273
#define MOD_ID_IMX290           0x0290
#define MOD_ID_IMX296           0x0296
#define MOD_ID_IMX297           0x0297
#define MOD_ID_IMX327           0x0327
#define MOD_ID_IMX335           0x0335
#define MOD_ID_IMX392           0x0392
#define MOD_ID_IMX412           0x0412
#define MOD_ID_IMX415           0x0415
#define MOD_ID_IMX462           0x0462
#define MOD_ID_IMX565           0x0565
#define MOD_ID_IMX566           0x0566
#define MOD_ID_IMX567           0x0567
#define MOD_ID_IMX568           0x0568
#define MOD_ID_IMX900           0x0900
#define MOD_ID_OV7251           0x7251
#define MOD_ID_OV9281           0x9281

int vc_mod_is_color_sensor(struct vc_desc *desc);
int vc_mod_ctrl_init(struct vc_ctrl* ctrl, struct vc_desc* desc);

#endif // _VC_MIPI_MODULES_H
