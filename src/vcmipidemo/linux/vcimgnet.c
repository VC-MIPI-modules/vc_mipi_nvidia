/**********************************************************************//**
***************************************************************************
*** @file    vcimgnet.c
***
*** @brief   VCImgNet: User Program Interface Functions and Demo.
***
***          To compile as a demo (#define DEMO):
***
***             gcc -o vcimgnet  vcimgnet.c -lrt
***
***
*** @author  Copyright (c) 2014-2018 Vision Components.
*** @author  All rights reserved.
*** @author  This software embodies materials and concepts which are
***          confidential to Vision Components.
***
***
*** @revisionHistory
***
***    Date          Version    Author  Changes
***    08.07.2014    <VER>      MBE     Initial Version.
***
*** @endRevisionHistory
***************************************************************************
***************************************************************************/

#define WITHOUT_VCLIB

//
//  For a demonstration define the word DEMO by
//  uncommenting the following line.
//
//#define  DEMO


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef WITHOUT_VCLIB
	#include "vclib-excerpt.h"
#else
	#include "vclib.h"
#endif
#include "vcimgnet.h"




/*--*FUNCTION*-----------------------------------------------------------------*/
/**
* @brief  Establishes an Asynchronous Connection to an Image Transfer Server.
*
*  This function establishes an asynchronous connection to an image transfer
*  server in form of an image. At this moment only images of type IMAGE_GREY
*  are supported.
*  The image *p img  will be set to use the shared memory and so should be
*  unallocated beforehand.
*
* @note  The image data transfer is asynchronous. This may have the disadvantage
*        of transferring partially written data which can result in a half-old
*        half-new picture. The function is mainly developed for development
*        purposes, and since the connection is asynchronous, you can pass the
*        image to deeper functions where you then can easily view the contents
*        of processing results by copying to that image without the need of handling
*        synchronization, so that this attached image can be used to assign it,
*        for example, to the debug display struct.
*
*  @ref vcimgnet_detach().
*/
/*-----------------------------------------------------------------------------*/
I32  vcimgnet_attach(image *img, VCImgNetCfg *cfg)
{
	I32          ee, rc;
	void        *pmm = NULL;
	I32          bytes;
	VCImgNetCtl  ctl;
	I32          fdCtl = -1;
	I32          bpp, ch, chDxDiv, chDyDiv;


	if(   (IMAGE_GREY!=img->type)
		&&(IMAGE_RGB !=img->type)){return(ERR_TYPE);}

	rc =  check_image_type(img->type, &bpp, &ch);
	if(ERR_NONE>rc){ee=-1; goto fail;}

	bytes = img->dy * img->pitch * bpp/8 * ((ch<3)?(ch):(3));


	snprintf(cfg->shmId, SHMIDMAXLEN-1, "/vcimgnet_%d", getpid());

	/////////////////////////////////////////////
	//  Request Shared Memory

	cfg->shmFd  =   -1;
	img->st     = NULL;
	img->ccmp1  = NULL;
	img->ccmp2  = NULL;

	//If already there, remove the shm file.
	rc =  shm_unlink(cfg->shmId);
	if((-1==rc)&&(ENOENT!=errno)){ee=-2;goto fail;}

	cfg->shmFd =  shm_open(cfg->shmId, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(-1==cfg->shmFd){ee=-3;goto fail;}

	rc =  ftruncate(cfg->shmFd, bytes);
	if(-1==rc){ee=-4;goto fail;}

	pmm =  mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, cfg->shmFd, 0);
	if(MAP_FAILED==pmm){ee=-5;goto fail;}

	//Initialize to zero.
	memset(pmm, 0, bytes);

	img->st = (U8*) pmm;

	switch(ch)
	{
		case 1:
			img->ccmp1 = NULL;
			img->ccmp2 = NULL;
			break;
		case 2:
			chDxDiv = 1; chDyDiv = 1;
			img->ccmp1 = img->st + 1 * (bpp/8 * ((img->pitch-1)/chDxDiv + img->pitch * (img->dy-1)/chDyDiv));
			img->ccmp2 = NULL;
			break;
		case 3:
			chDxDiv = 1; chDyDiv = 1;
			img->ccmp1 = img->st + 1 * (bpp/8 * ((img->pitch-1)/chDxDiv + img->pitch * (img->dy-1)/chDyDiv));
			img->ccmp2 = img->st + 2 * (bpp/8 * ((img->pitch-1)/chDxDiv + img->pitch * (img->dy-1)/chDyDiv));
			break;
		case 4:
			chDxDiv = 2; chDyDiv = 1;
			img->ccmp1 = img->st + 1 * (bpp/8 * ((img->pitch-1)/chDxDiv + img->pitch * (img->dy-1)/chDyDiv));
			img->ccmp2 = img->st + 2 * (bpp/8 * ((img->pitch-1)/chDxDiv + img->pitch * (img->dy-1)/chDyDiv));
			break;
		case 5:
			chDxDiv = 2; chDyDiv = 2;
			img->ccmp1 = img->st + 1 * (bpp/8 * ((img->pitch-1)/chDxDiv + img->pitch * (img->dy-1)/chDyDiv));
			img->ccmp2 = img->st + 2 * (bpp/8 * ((img->pitch-1)/chDxDiv + img->pitch * (img->dy-1)/chDyDiv));
			break;
		default:
			ee=-6; goto fail;
	}


	cfg->img  = *img;


	/////////////////////////////////////////////
	//  Communicate to vcimgnet

	ctl.pid   = getpid();
	memset(ctl.shmId,'\0',SHMIDMAXLEN);
	memcpy(ctl.shmId,cfg->shmId,SHMIDMAXLEN-1);
	ctl.shmFd = cfg->shmFd;
	ctl.img   = *img;

	fdCtl =  open(VCIMGNET_FIFO_CTL, O_WRONLY);
	if(fdCtl<0){ee=-7;goto fail;}

	rc =  write(fdCtl, &ctl, sizeof(VCImgNetCtl));
	if(rc!=sizeof(VCImgNetCtl)){ee=-8;goto fail;}


	ee=0;
fail:
	switch(ee)
	{
		case  0: break;
		default:
				/*
				printf("Error, %s() failed: %s\n",
						( -1==ee)?("Image Type")
						:(-2==ee)?("shm_unlink")
						:(-3==ee)?("shm_open"  )
						:(-4==ee)?("ftruncate" )
						:(-5==ee)?("mmap"      )
						:(-6==ee)?("internal"  )
						:(-7==ee)?("open fifo" )
						:(-8==ee)?("write fifo")
						:("UNKNOWN"), strerror(errno));
				*/
				break;
	}
	if(ee<0)
	{
		if(-1!=cfg->shmFd){ shm_unlink(cfg->shmId); }
		pmm     = NULL;
		img->st = NULL;
	}
	if(fdCtl>=0){ close(fdCtl);  fdCtl = -1; }

	return(ee);
}




/*--*FUNCTION*-----------------------------------------------------------------*/
/**
* @brief  Removes Image Connection.
*
*  This function the connection between the user application and the image
*  transfer client. Call this function to cleanly disconnect at the end.
*
*  @ref vcimgnet_attach().
*/
/*-----------------------------------------------------------------------------*/
I32  vcimgnet_detach(VCImgNetCfg *cfg)
{
	if(-1!=cfg->shmFd){ close(cfg->shmFd); cfg->shmFd=-1;}
	shm_unlink(cfg->shmId);
	cfg->img.st = NULL;

	return(0);
}




//Needed by macro:  vc_vcimgnet_check_header_ver(), see docs in vcimgnet.h.
I32  vc_libvcimgnet_check_header_ver_func(int MainVersion, int Version, int SubVersion)
{
	I32  ee;

	if(MainVersion != LIBVCIMGNET_MAINVERSION){ ee=-1; goto fail; }
	if(Version      > LIBVCIMGNET_VERSION    ){ ee=+1; goto fail; }

	ee=0;
fail:
	return(ee);
}





/*--*FUNCTION*-----------------------------------------------------------------*/
/**
* @brief  Version Information of the libvcimgnet as Numbers.
*
* @param     MainVersion  Main Version Number.
* @param     Version      Version Number.
* @param     SubVersion   Subversion Number.
*/
/*-----------------------------------------------------------------------------*/
void  GetLibVer_LIBVCIMGNET(int *MainVersion, int *Version, int *SubVersion)
{
	*MainVersion= LIBVCIMGNET_MAINVERSION;
	*Version    = LIBVCIMGNET_VERSION;
	*SubVersion = LIBVCIMGNET_SUBVERSION;
}





/*--*FUNCTION*-----------------------------------------------------------------*/
/** 
* @brief  Version Information of the libvcimgnet as String.
*
* @retval  Pointer to the Output String.
*/
/*-----------------------------------------------------------------------------*/
char *GetLibVerString_LIBVCIMGNET(void)
{
	static char string[257];

	sprintf(string, "LIBVCIMGNET   MainVersion=%d  Version=%d  SubVersion=%d  Date:%s  Time=%s", LIBVCIMGNET_MAINVERSION, LIBVCIMGNET_VERSION, LIBVCIMGNET_SUBVERSION, __DATE__, __TIME__);
 
	return(string);
}







#ifdef  DEMO
	void  generate_example_img(image *img, I32 nr)
	{
		U8  *pPx= NULL;
		I32   x, y;

		if(0==nr) //prohibits black image
			nr = -1;

		pPx = img->st;
		for(y= 0; y< img->dy; y++)
		{
			for(x= 0; x< img->dx; x++)
			{
				*(pPx + y * img->pitch + x) = x * nr * y + nr;
			}
		}
	}


	I32  main()
	{
		I32          rc, ee;
		I32          nr   = 0;
		image        img  = NULL_IMAGE;
		VCImgNetCfg  cfg  = NULL_VCImgNetCfg;


		/// Define the dimension of the image buffer beforehand.
		/// The function writes the suitable image pointer to the
		/// image struct for further use.

		img.type  = IMAGE_GREY;
		img.pitch = 1600;
		img.dx    = img.pitch;
		img.dy    = 1200;
		img.st    = NULL;

		rc =  vcimgnet_attach(&img, &cfg);
		if(rc<0){ee=-1;goto fail;}

		while(1)
		{
			generate_example_img(&img, nr);
			nr++;

			printf("Return Key: Next Image, 'q' and Return Key: Exit\n");
			if('q'==getchar())
				break;
		}

		ee=0;
	fail:
		switch(ee)
		{
			case  0:  break;
			case -1:  printf("Error attaching: %d!\n", rc); break;
			default:  printf("Error %d, rc: %d\n", ee, rc); break;
		}

		vcimgnet_detach(&cfg);

		return(ee);
	}
#endif //DEMO

