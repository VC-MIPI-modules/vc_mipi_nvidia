/*
 * VC MIPI driver header file.
 *
 * Copyright (c) 2020, Vision Components GmbH <mipi-tech@vision-components.com>"
 *
 */

#ifndef _VC_MIPI_H      /* [[[ */
#define _VC_MIPI_H

/*............. Jetson models: */
#define JETSON_MODEL_NANO_A02   0   /* Nano A02 - single camera port */
#define JETSON_MODEL_NANO_B01   1   /* Nano B01 - dual camera port */
#define JETSON_MODEL_TX2        2   /* TX2 */
#define JETSON_MODEL_XAVIER     3   /* Xavier */

#define JETSON_MODEL            JETSON_MODEL_NANO_B01   /* Jetson model */

/*............. Frame alignment modes: */
#define FRAME_WIDTH_ALIGN_32        0   /* Nano,Xavier: default frame width alignment to multiple of 32 (8-bit data: k*64)           */
#define FRAME_WIDTH_ALIGN_128_GT    1   /* TX2/32.3.1: align sensor frame width to nearest multiple of 128 above (8-bit data: k*256) */
#define FRAME_WIDTH_ALIGN_128_LT    2   /* TX2/32.3.1: align sensor frame width to nearest multiple of 128 below                     */

#if JETSON_MODEL == JETSON_MODEL_TX2    // TX2/32.3.1:
/*
* TX2/32.3.1 default:
* 16-bit data: aligns sensor frame width to nearest greater/equal number, multiple of 128
* 8-bit data:  aligns sensor frame width to nearest greater/equal number, multiple of 256
*/
//  #define FRAME_WIDTH_ALIGN_MODE  FRAME_WIDTH_ALIGN_128_GT   /* frame width align mode */
  #define FRAME_WIDTH_ALIGN_MODE  FRAME_WIDTH_ALIGN_32

#else   // Nano/Xavier: default frame stride alignemnt: k*32 (16-bit data)
  #define FRAME_WIDTH_ALIGN_MODE  FRAME_WIDTH_ALIGN_32
#endif

void set_sensor_model (char *model);

#endif  /* ]]] */
