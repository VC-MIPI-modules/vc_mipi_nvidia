/**********************************************************************//**
***************************************************************************
*** @file    vclib-excerpt.c
***
*** @brief   Excerpt of the libvclib.
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
#include "vclib-excerpt.h"


//This is the only vclib function.
//Please ignore the bad style fact for having a code block in a header.
I32  check_image_type(I32 type, I32 *size, I32 *clss)
{
	if(type & 0x80) type = 0;

	switch(type)
	{
		case IMAGE_GREY:
		case IMAGE_BAYER:   *clss = 1;
							*size  = 8;
							break;
		case IMAGE_VECTOR:  *clss = 2;
							*size  = 8;
							break;
		case IMAGE_RGB:
		case IMAGE_CBCR444:
		case IMAGE_YUVNORM:
		case IMAGE_IHS:     *clss = 3;
							*size  = 8;
							break;
		case IMAGE_CBCR422: *clss = 4;
							*size  = 8;
							break;
		case IMAGE_CBCR411: *clss = 5;
							*size  = 8;
							break;
		case IMAGE_GREY16:
		case IMAGE_YCBYCR:  *clss = 1;
							*size  = 16;
							break;
		case IMAGE_GREY32:
		case IMAGE_CMPLX16:
		case IMAGE_RGBO:    *clss = 1;
							*size  = 32;
							break;

		default:            return ERR_TYPE;
	}

	return type;
}


