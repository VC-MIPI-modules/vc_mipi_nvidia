/**********************************************************************//**
***************************************************************************
*** @file    vclib-excerpt.h
***
*** @brief   Excerpt of the libvclib header file 'vclib.h'.
***
*** @author  Copyright (c) 2018 Vision Components.
*** @author  All rights reserved.
*** @author  This software embodies materials and concepts which are
***          confidential to Vision Components.
***
*** @revisionHistory
***
***    Date          Version    Author  Changes
***    23.10.2018        0.0       MBE  File Generated.
***
*** @endRevisionHistory
***************************************************************************
***************************************************************************/
#ifndef _VCLIB_h
#ifndef  __VCLIB_EXCERPT_H__
#define  __VCLIB_EXCERPT_H__

#ifdef __cplusplus
 extern "C" {
#endif

	#include "vctypes.h"

	#define  VCLIB_MAINVERSION    (  6)  /**<  Main Version of the vclib/extlib   */
	#define  VCLIB_VERSION        (  5)  /**<       Version of the vclib/extlib   */
	#define  VCLIB_SUBVERSION     (  0)  /**<    Subversion of the vclib/extlib   */

	#define  VCLIB_EXCERPT_MAINVERSION   (  1)  /**<  Main Version of the excerpt */
	#define  VCLIB_EXCERPT_VERSION       (  0)  /**<       Version of the excerpt */
	#define  VCLIB_EXCERPT_SUBVERSION    (  0)  /**<    Subversion of the excerpt */


	/*--*GROUP*------------------------------------------------------------------*/
	/**
	*@cond ( vclib || extlib )
	*
	* @defgroup errorTypes   Error Types
	*
	* @note  Additionally to documented return values any library function may
	*        return the Error Code @ref ERR_LICENCE. For more information see
	*        @ref init_licence().
	* @{
	*/
		#define  ERR_NONE        ( 0  )    /*!<@brief  No Error                     */
		#define  ERR_FORMAT      (-1  )    /*!<@brief  Image Format Error           */
		#define  ERR_TYPE        (-2  )    /*!<@brief  Image Type Error             */
		#define  ERR_MEMORY      (-3  )    /*!<@brief  Out of Memory                */
		#define  ERR_PARAM       (-4  )    /*!<@brief  Parameter Error              */
		#define  ERR_LICENCE     (-5  )    /*!<@brief  Licence required             */
		#define  ERR_SINGULAR    (-6  )    /*!<@brief  Singular Value detected      */
		#define  ERR_OVERRUN     (-7  )    /*!<@brief  Memory Overrun               */
		#define  ERR_FIO         (-8  )    /*!<@brief  File I/O Error               */
		#define  ERR_BOUNDS      (-9  )    /*!<@brief  Out of Bounds                */
		#define  ERR_INIT        (-10 )    /*!<@brief  Missing Initialisation       */
		#define  ERR_LOWPREC     (-11 )    /*!<@brief  Max. Precision not available */
		#define  ERR_OPEN        (-19 )    /*!<@brief  Open Error                   */
		#define  ERR_MODEL       (-51 )    /*!<@brief  Model does not fit Licence   */
		#define  ERR_JPG         (-71 )    /*!<@brief  Unrecognized JPEG Token      */
		#define  ERR_CORR        (-100)    /*!<@brief  Object not found / vc_corr   */
		#define  ERR_FSIGN       (-201)    /*!<@brief  File Signature Error         */
		#define  ERR_INCONS      (-202)    /*!<@brief  File inconsistent            */
		#define  ERR_FFMT        (-203)    /*!<@brief  Unsupported File Format      */
		#define  ERR_READ        (-204)    /*!<@brief  Read Error                   */
		#define  ERR_WRITE       (-205)    /*!<@brief  Write Error                  */
		#define  ERR_HOUGH0      (-500)    /*!<@brief  Hough Line Overrun           */
		#define  ERR_HOUGH1      (-501)    /*!<@brief  Hough Maximum Iterations     */

		/* RLC error types                                                          */

		#define  ERR_SLC         (-200)    /*!<@brief  RLC is missing SLC           */
		#define  ERR_RLC_ONOVR   (-201)    /*!<@brief  RLC Object Number Overrun    */
		#define  ERR_SLC_INCON   (-202)    /*!<@brief  SLC is inconsistent          */
		#define  ERR_RLCFMT      (-203)    /*!<@brief  RLC Format Error             */
		#define  ERR_RLC_MEM     (-204)    /*!<@brief  Memory Allocation Error      */
		#define  ERR_RLC_SIZE    (-205)    /*!<@brief  RLC Size Overrun             */

	/**@}@endcond*//*---End of Error Types---------------------------------------*/


	#ifndef  NOMINMAX
		#ifndef max
			/** @def max(a,b)
			 *       Outputs the maximum of @p a and @p b. */
			#define  max(a,b)    (((a) > (b)) ? (a) : (b))
		#endif
		#ifndef min
			/** @def min(a,b)
			 *       Outputs the minimum of @p a and @p b. */
			#define  min(a,b)     (((a) < (b)) ? (a) : (b))
		#endif
	#endif

	/*--*STRUCT*--------------------------------------------------------------*/
	/**
	*
	*  @brief  Main Structure for Describing Pixel Images.
	*
	*    This structure describes the type of the image, its dimensions and
	*    its location in memory. It is the most important structure of the
	*    \VCLib.
	*
	*/
	typedef struct
	{
		U8  *st;       /*!<  Image Data Start Address (Channel 0 Pixel Data).         */
		U32  type;     /*!<  Type of Image Data, Listed in '@ref imageVariableTypes'. */
		I32  dx;       /*!<  Horizontal Visible Pixel Count.                          */
		I32  dy;       /*!<  Vertical Visible Pixel Count.                            */
		I32  pitch;    /*!<  Address Offset from a Pixel to its Lower Neighbour Pixel.*/
		U8  *ccmp1;    /*!<  Color Component 1  (Channel 1 Pixel Data).               */
		U8  *ccmp2;    /*!<  Color Component 2  (Channel 2 Pixel Data).               */
	} image;
	/*------------------------------------------------------------------------*/
	#define  NULL_IMAGE  {NULL, IMAGE_GREY, 0, 0, 0, NULL, NULL}

	///
	/// @brief  @unitNr{8,Bit} Values per Pixel (0-255), One Channel (@p st).
	///
	#define  IMAGE_GREY      (0)
	///
	/// @brief  @unitNr{8,Bit} Values per Pixel (0-255), One Channel (@p st), Patterned (RGRG..GBGB..RGRG...).
	///
	#define  IMAGE_BAYER     (1)
	///
	/// @brief  @unitNr{8,Bit} Values per Pixel (0-255), Three Channels (@p st:  R, @p ccmp1:  G, @p ccmp2:  B).
	///
	#define  IMAGE_RGB       (2)
	///
	/// @brief  @unitNr{8,Bit} Values per Pixel (0-255), Three Channels (@p st:  Y, @p ccmp1:  U, @p ccmp2:  V).
	///
	#define  IMAGE_CBCR444   (3)
	///
	/// @brief  @unitNr{8,Bit} Values per Pixel (0-255), Three Channels (@p st:  Y, @p ccmp1:  U, @p ccmp2:  V);
	///         U and V Channels have @p dx, @p dy and @p pitch divided by 2, which means that each @times{2,2} Y pixel
	///         block share the same U and V pixel.
	///
	#define  IMAGE_CBCR411   (4)
	///
	/// @brief  @unitNr{8,Bit} Values per Pixel (0-255), Three Channels (@p st:  Y, @p ccmp1: U*, @p ccmp2: V*).
	///
	#define  IMAGE_YUVNORM   (5)
	///
	/// @brief  @unitNr{8,Bit} Values per Pixel (0-255), Three Channels (@p st:  I, @p ccmp1:  H, @p ccmp2:  S).
	///
	#define  IMAGE_IHS       (6)
	///
	/// @brief Only used Internally.
	///
	#define  IMAGE_RGBO      (7)
	///
	/// @brief  @unitNr{8,Bit} Values per Pixel (0-255), Two Channels (@p st: Magnitude, @p ccmp1: Direction (f. ex. @ref gradient_2x2())).
	///
	#define  IMAGE_VECTOR    (8)
	///
	/// @brief  @unitNr{16,Bit} Values per Pixel (0-65535), One Channel (@p st: Always cast to (U16*)).
	///
	#define  IMAGE_GREY16    (9)
	///
	/// @brief  @unitNr{32,Bit} Values per Pixel (0-2<sup>32</sup>), One Channel (@p st: Always cast to (U32*)).
	///
	#define  IMAGE_GREY32    (10)
	///
	/// @brief @unitNr{16,Bit} Values per Pixel (0-65535), Two Interleaving Channels (@p st: [Real1, Imaginary1, Real2, ...]; Always cast to (I16*)), See @ref vc_fft().
	///
	#define  IMAGE_CMPLX16   (11)
	///
	/// @brief  @unitNr{8,Bit} Values per Pixel (0-255), Three Channels (@p st:  Y, @p ccmp1: U, @p ccmp2: V);
	///         U and V Channels have @p dx and @p pitch (not @p dy) divided by 2, which means that each two Y pixel
	///         in a row share the same U and V pixel.
	///
	#define  IMAGE_CBCR422   (13)
	///
	/// @brief Only used Internally.
	///
	#define  IMAGE_YCBYCR    (14)
	///
	/// @brief  Unset/Unused Information.
	///
	#define  IMAGE_UNSET     (0xFFFF)

	#define  str_imageType(t) \
	             (IMAGE_GREY    == (t))?("GREY   ")\
	            :(IMAGE_BAYER   == (t))?("BAYER  ")\
	            :(IMAGE_RGB     == (t))?("RGB    ")\
	            :(IMAGE_CBCR444 == (t))?("CBCR444")\
	            :(IMAGE_CBCR411 == (t))?("CBCR411")\
	            :(IMAGE_YUVNORM == (t))?("YUVNORM")\
	            :(IMAGE_IHS     == (t))?("IHS    ")\
	            :(IMAGE_RGBO    == (t))?("RGBO   ")\
	            :(IMAGE_VECTOR  == (t))?("VECTOR ")\
	            :(IMAGE_GREY16  == (t))?("GREY16 ")\
	            :(IMAGE_GREY32  == (t))?("GREY32 ")\
	            :(IMAGE_CMPLX16 == (t))?("CMPLX16")\
	            :(IMAGE_CBCR422 == (t))?("CBCR422")\
	            :(IMAGE_YCBYCR  == (t))?("YCBYCR ")\
	            :(IMAGE_UNSET   == (t))?("UNSET  ")\
	            :("UNKNOWN")



	I32  check_image_type(I32 type, I32 *size, I32 *clss);

#ifdef __cplusplus
 }
#endif

#endif //__VCLIB_EXCERPT_H__
#endif
