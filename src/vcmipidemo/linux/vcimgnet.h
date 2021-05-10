/**********************************************************************//**
***************************************************************************
*** @file    vcimgnet.h
***
*** @brief   Connects to vcimgnetsrv Helper Application.
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
***    10.07.2014    1.0        MBE     Initial Version.
***
*** @endRevisionHistory
***************************************************************************
***************************************************************************/
///@cond (vcimgnet)
///@{
#ifndef  _VC_IMG_NET_H_
#define  _VC_IMG_NET_H_

#ifdef __cplusplus
 extern "C" {
#endif

#define  LIBVCIMGNET_MAINVERSION    (  1)  /**<  Main Version of the libvcimgnet   */
#define  LIBVCIMGNET_VERSION        (  0)  /**<       Version of the libvcimgnet   */
#define  LIBVCIMGNET_SUBVERSION     (  0)  /**<    Subversion of the libvcimgnet   */

/*--*GROUP*-------------------------------------------------------------------*/
/**
 * @addtogroup  tools   Tools
 *
 *@{
 */

	/*--*GROUP*---------------------------------------------------------------*/
	/**
	 * @addtogroup  tools_vcimgnet   Continuous Image Transfer: 'libvcimgnet' and 'vcimgnetsrv'
	 *
	 * Preliminary Note: Read the documentation of @ref vcimgnet_attach() for information about asynchronity!
	 *
	 *  Connecting from the user program is as easy as follows.
	 *  You set the desired resolution of the transfer image and attach it to the running <em>vcimgnetsrv</em> process.
	 *  While being attached, the actual pixel data at the moment of transfer will be submitted to the
	 *  connected client.
	 *
	 * @image html  vcimgnetsrv_connection.png
	 * @image latex vcimgnetsrv_connection.pdf
	 *
	 * @code
	 *  image disp;
	 *  VCImgNetCfg  imgNetCfg;
	 *  
	 *  disp.type  = IMAGE_GREY;
	 *  disp.pitch = 1280;
	 *  disp.dx    = 1280;
	 *  disp.dy    = 1024;
	 *  disp.st    = NULL;
	 *  
	 *  vcimgnet_attach(&disp, &imgNetCfg); 
	 *  
	 *  // Do your work here, the volatile data of the disp image</code></li>
	 *  // will be transferred continuously.</code></li>
	 *  
	 *  vcimgnet_detach(&imgNetCfg); 
	 * @endcode
	 *
	 *  A good way to transfer several state images of processing is to use the shared
	 *  image for the <em>debug display</em> environment. You can easily copy your interim images to
	 *  one of the <em>debug display</em> sub windows and keep the overview of the whole process.
	 *  See the VCLib documentation for information how to use the <em>debug display</em> environment.
	 *
	 *@{
	 */

	#define  VCIMGNET_PORT       (2002)
	#define  VCIMGNET_LOCKFILE   "/var/lock/vcimgnet.lock"
	#define  VCIMGNET_FIFO_CTL   "/var/lock/vcimgnetctl"


	#define SHMIDMAXLEN  (256) /*<! If changed, you must change SHMIDMAXLEN_ARRAYNULLER() accordingly! */
	#define SHMIDMAXLEN_ARRAYNULLER(V) {V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,\
	                                    V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,\
	                                    V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,\
	                                    V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,\
	                                    V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,\
	                                    V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,\
	                                    V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,\
	                                    V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V }
	/**********************************************************************//**
	* @brief  Controls vcimgnet.
	***************************************************************************/
	typedef struct
	{
		int    pid;                 /*!<   Process ID of the Requester                */
		char   shmId[SHMIDMAXLEN];  /*!<   Shared Memory Id to the Image Data (st)    */
		int    shmFd;               /*!<   Image Transfer Server Check: 'shm closed?' */
		image  img;                 /*!<   Maximum Image Dimensions to be transferred */
	} VCImgNetCtl, VCImgNetCfg;
	#define NULL_VCImgNetCtl { -1, SHMIDMAXLEN_ARRAYNULLER(0), -1, NULL_IMAGE}
	#define NULL_VCImgNetCfg { -1, SHMIDMAXLEN_ARRAYNULLER(0), -1, NULL_IMAGE}


	I32  vcimgnet_attach(image *psImg, VCImgNetCfg *cfg);
	I32  vcimgnet_detach(VCImgNetCfg *cfg);

	/**@}*/
	/*--*GROUP*---------------------------------------------------------------*/


/**@}*/
/*--*GROUP*-------------------------------------------------------------------*/

/*--*GROUP*-------------------------------------------------------------------*/
/**
 * @addtogroup  helper   Helper Functions
 *@{
 */
    /*--*DEFINE*------------------------------------------------------------------*/
    /**
    *  \def vc_libvcimgnet_check_header_ver()
    *
    * @brief  Version Compatibility Check between Header and Linked Library.
    *
    * This macro compares the version information at the header used at compile time
    * with the version of the library linked at compile time.
    * If the header used is incompatible with the library linked against,
    * this macro returns an error.
    *
    * @retval    -1           if Main Version of Library differs from the Header's Main Version (General Incompatibility).
    * @retval    +1           if Version of Library is smaller than the Header's Version (Features may be missing).
    * @retval     0           else.
    */
    #define  vc_libvcimgnet_check_header_ver()  vc_libvcimgnet_check_header_ver_func(LIBVCIMGNET_MAINVERSION,LIBVCIMGNET_VERSION,LIBVCIMGNET_SUBVERSION)
    I32                                         vc_libvcimgnet_check_header_ver_func(int MainVersion, int Version, int SubVersion);
    
	void  GetLibVer_LIBVCIMGNET(int *MainVersion, int *Version, int *SubVersion);
	char *GetLibVerString_LIBVCIMGNET(void);
/**@}*/
/*--*GROUP*-------------------------------------------------------------------*/


#ifdef __cplusplus
 }
#endif

#endif
///@}
///@endcond

