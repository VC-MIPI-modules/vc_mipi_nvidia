/**********************************************************************//**
***************************************************************************
*** @file    vcimgnetsrv.c
***
*** @brief   Image Transfer over Ethernet.
***
***  This server program transfers images from a buffer to a client program
***  for displaying.
***
*** @author  Copyright (c) 2014-2018 Vision Components.
*** @author  All rights reserved.
*** @author  This software embodies materials and concepts which are
***          confidential to Vision Components.
***
*** @revisionHistory
***
***    Date          Version    Author  Changes
***    07.07.2014    0.01       MBE     Initial Version.
***
*** @endRevisionHistory
***************************************************************************
***************************************************************************/

#define WITHOUT_VCLIB

#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#ifdef WITHOUT_VCLIB
	#include "vclib-excerpt.h"
#else
	#include "vclib.h"
#endif
#include "vcimgnet.h"

typedef struct
{
	int           port;

	int           fdCtl;
	VCImgNetCtl   ctl;

	int           rest_ms;

} ImgNetSrvCfg;
#define  NULL_ImgNetSrvCfg  { -1, -1, NULL_VCImgNetCtl, -1 }



#define  VCIMGNETSRV_NAME           "VCImgNetSrv" /**<  Tool Name      */
#define  VCIMGNETSRV_MAINVERSION             (1)  /**<  Main Version   */
#define  VCIMGNETSRV_VERSION                 (0)  /**<       Version   */
#define  VCIMGNETSRV_SUBVERSION              (3)  /**<    Subversion   */




I32  process_opts(int argc, char *argv[], I32 *rest_ms)
{
	// Default Value
	*rest_ms   = 40;

	if(argc>1)
	{
		if(((*argv[1])<'0')||((*argv[1])>'9'))
		{
			printf("\n");
			printf("_______________________________________________________________________________\n");
			printf("                                                                               \n");
			printf("  %s v.%d.%d.%d.- VCLinux Image Transfer over Network.\n", VCIMGNETSRV_NAME, VCIMGNETSRV_MAINVERSION, VCIMGNETSRV_VERSION, VCIMGNETSRV_SUBVERSION);
			printf("  -----------------------------------------------------------------------------\n");
			printf("                                                                               \n");
			printf("  Usage: %s [ms]\n", argv[0]);
			printf("                                                                               \n");
			printf("  ms,  delay time between two images, default value: %d ms.                      \n", *rest_ms);
			printf("                                                                               \n");
			printf("_______________________________________________________________________________\n");
			printf("                                                                               \n");
			return(-1);
		}
		else
		{
			*rest_ms = (I32) atoll(argv[optind]);
		}
	}

	return(0);
}




void  draw_progress(image *img, I32 nr, U32 cnt, U8 fillIff1, U8 col, I32 colBg)
{
	I32  x, y, xs, xe;
	U8  *pPx = NULL;

	if(1==fillIff1)
	{
		xs = 0;
		xe =                           ((((nr-0)%(cnt+1)) * img->dx)/cnt);
	}
	else
	{
		xs = (0==((nr-0)%(cnt+1)))?(0):((((nr-1)%(cnt+1)) * img->dx)/cnt);
		xe =                           ((((nr-0)%(cnt+1)) * img->dx)/cnt);
	}

	if(colBg>=0)
	{
		for(y= 0; y< img->dy; y++)
		{
			pPx = img->st + img->pitch * y + 0;

			for(x= 0; x< xs; x++)
				*pPx++ = colBg;
			for(x=xs; x< xe; x++)
				*pPx++ = col;
			for(x=xe; x< img->dx; x++)
				*pPx++ = colBg;
		}
	}
	else
	{
		for(y= 0; y< img->dy; y++)
		{
			pPx = img->st + img->pitch * y + xs;

			for(x=xs; x< xe; x++)
				*pPx++ = col;
		}
	}
}




void  img_fill_patterned(image *img, U8 col, U32 step, U32 rep)
{
	I32 x, y;

	for(y= 0; y< img->dy; y++)
	{
		for(x= (rep-1)-(y%rep); x< img->dx; x+= step)
		{
			*(img->st + x + y * img->pitch) = col;
		}
	}
}




void  img_print_unconnected(image *img, I32 atNr)
{
	char  acMsg[] = "No Application is connected!";
	I32   msgsize = 2;
	I32   msglen;
	image sImgCtr = NULL_IMAGE;

	msglen = strlen(acMsg);

	img_fill_patterned(img, 200, 5, 5);


	sImgCtr.type  = img->type;
	sImgCtr.dx    = min(img->dx - max(0,min(img->dx-1, img->dx/2-(8 * msgsize * msglen)/2)),(8 * msgsize * msglen));
	sImgCtr.dy    = min(img->dy - max(0,min(img->dy-1, img->dy/2+(10* msgsize         )/2)),(8 * msgsize         ));
	sImgCtr.pitch = img->pitch;
	sImgCtr.st    = img->st + max(0,min(img->dx-1, img->dx/2-(8 * msgsize * msglen)/2))
	                        + max(0,min(img->dy-1, img->dy/2+(10* msgsize         )/2)) * img->pitch;
	sImgCtr.ccmp1 = NULL;
	sImgCtr.ccmp2 = NULL;

	draw_progress(&sImgCtr, atNr, 8*msglen, 0, 255,64);
}




void  type_buffer(char *pcBuffer, unsigned int u32ByteCount)
{
	int i,j;

	for(i= 0; i< u32ByteCount/16; i++)
	{
		printf("\n%07x:", (i * 16));

		for(j= i*16; j< (i+1)*16; j++)
		{
			printf(" %02x", *(pcBuffer + j));
		}
		printf("   ");
		for(j= i*16; j< (i+1)*16; j++)
		{
			printf("%c", ((*(pcBuffer + j)<32)||(*(pcBuffer + j)>126))?('.'):(*(pcBuffer + j)) );
		}
	}
	printf("\n");
}




int  create_fifo_vcimgnetctl(int *fdCtl)
{
	int  rc, ee;

	*fdCtl = -1;

	//Create Fifo if not already present.
	rc =  mkfifo(VCIMGNET_FIFO_CTL, O_RDWR | 0666);
	if((rc<0)&&(errno!=EEXIST)){ee=-1;goto fail;}
	if((rc<0)&&(errno==EEXIST)){ee=-2;goto fail;}

	*fdCtl =  open(VCIMGNET_FIFO_CTL, O_RDWR | O_NONBLOCK);
	if(*fdCtl<0){ee=-3;goto fail;}

	ee=0;
fail:
	switch(ee)
	{
		case  0: break;
		case -1: break;
		case -2:
		         printf("Error, program seems to be already running (%s exists)!\n", VCIMGNET_FIFO_CTL);
		         break;
		case -3: break;
	}
	return(ee);
}




int  remove_fifo_vcimgnetctl(int *fdCtl)
{
	if(*fdCtl>0){ close(*fdCtl); *fdCtl = -1; }
	return(remove(VCIMGNET_FIFO_CTL));
}




int  shm_get_fd(char *shmId, int pid, int *shmFd)
{
	int             ee, bytes, dirFd;
	DIR            *dir   = NULL; //important for proper closing.
	struct dirent  *entry = NULL;
	char            path[1001], fntgt[1001];

	*shmFd = -1;

	if(pid<0){ee=-1; goto fail;}

	snprintf(path, 1000, "/proc/%d/fd/", pid);
	dir =  opendir(path);
	if(NULL==dir){ee=-2; goto fail;}
	dirFd = dirfd(dir);
	while(1)
	{
		errno = 0; //To detect readdir() error.
		entry =  readdir(dir);
		if(NULL==entry)
		{
			if(errno!=0){ee=-3; goto fail;} //error
			else        {ee=-5; goto fail;} //nothing found
		}
		if(DT_LNK==entry->d_type)
		{
			bytes =  readlinkat(dirFd, entry->d_name, fntgt, 1000);
			if(-1==bytes){ee=-4; goto fail;}

			fntgt[bytes] = '\0';
			if(0==strncmp(shmId,strrchr(fntgt,'/'),strlen(shmId)))
			{
				*shmFd = atoi(entry->d_name);
				break; //exits while(1).
			}
		}
	}

	ee=0;
fail:
	switch(ee)
	{
		case  0: break;
		case -1: /*printf("pid of %d is invalid!\n", pid);*/ break;
		case -2: /*printf("Could not open User Program fd directory '%s': %s\n", path, strerror(errno));*/ break;
		case -3:
		case -4:
				 /*
				 printf("error, %s() failed: %s\n", (ee==-3)?("readdir")
				                                   :(ee==-4)?("readlink")
				                                   :("UNKNOWN"), strerror(errno));
				 */
				 break;
		case -5: /*printf("Could not find fd of shm, no match!\n");*/ break;
	}

	if(NULL!=dir){ closedir(dir); dir = NULL; }

	return(ee);
}




int  shm_check_user_closed(char *shmId, int pid, int shmFd)
{
	int   bytes;
	char  shmpath[1001], fntgt[1001];

	if(pid<0){return(-1);}

	snprintf(shmpath, 1000, "/proc/%d/fd/%d", pid, shmFd);

	bytes =  readlink(shmpath, fntgt, 1000);
	if(-1==bytes)
	{
		//Not present anymore?
		switch(errno)
		{
			case ENOENT:
			case EACCES: //pid not present anymore, so path resolution fails.
				return( 1);
			default:
				return(-2);
		}
	}

	if(0==strncmp(shmId,strrchr(fntgt,'/'),strlen(shmId)))
	{
		//present and valid.
		return(0);
	}
	else
	{
		//present, but other descriptor.
		return(1);
	}
}




void  ctl_reset(VCImgNetCtl *ctl)
{
	image  imgNull = NULL_IMAGE;

	ctl->pid      = -1; //Prevents Execution until Data over Pipe.
	ctl->shmId[0] = '\0';
	ctl->shmFd    = -1;
	ctl->img      = imgNull;
}




void  ctl_cp(VCImgNetCtl *from, VCImgNetCtl *to)
{
	to->img = from->img;
	to->pid = from->pid;
	memcpy(to->shmId, from->shmId, sizeof(char) * SHMIDMAXLEN);
	to->shmFd = from->shmFd;
}




void  ctl_print(VCImgNetCtl *ctl)
{
	printf("    PID:    %d\n", ctl->pid);
	printf("    shmId:  %s\n", ctl->shmId);
	printf("    image (dx,dy,pitch):(%4d,%4d,%4d)\n",
			ctl->img.dx, ctl->img.dy, ctl->img.pitch);
}




int  ctl_update_by_fifo(int *fdCtl, VCImgNetCtl *ctl)
{
	int          ee, rc, fdShm =-1;
	VCImgNetCtl  ctlNew, ctlOld;
	int          bytes;
	I32          bpp, ch, chDxDiv, chDyDiv;

	ctlNew.img.st    = NULL;
	ctlNew.img.ccmp1 = NULL;
	ctlNew.img.ccmp2 = NULL;

	//fifo is nonblocking.
	rc =  read(*fdCtl, &ctlNew, sizeof(VCImgNetCtl));
	if((0==rc)||((0>rc)&&(EAGAIN==errno))){ee=+1;goto fail;} //No Data
	if( 0>rc){ee=-1;goto fail;} //Error

	fdShm =  shm_open(ctlNew.shmId, O_RDWR, S_IRUSR | S_IWUSR);
	if((-1==fdShm)&&(ENOENT==errno)){ee=+2;goto fail;} //Program Data not available
	if(-1==fdShm){ee=-2;goto fail;}

	rc =  check_image_type(ctlNew.img.type, &bpp, &ch);
	if(ERR_NONE>rc){ee=-3; goto fail;}

	bytes = ctlNew.img.dy * ctlNew.img.pitch * bpp/8 * ((ch<3)?(ch):(3));

	rc =  ftruncate(fdShm, bytes);
	if(-1==rc){ee=-4;goto fail;}

	ctlNew.img.st =  mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fdShm, 0);
	if(MAP_FAILED==ctlNew.img.st){ee=-5;goto fail;}

	switch(ch)
	{
			case 1:
				ctlNew.img.ccmp1 = NULL;
				ctlNew.img.ccmp2 = NULL;
				break;
			case 2:
				chDxDiv = 1; chDyDiv = 1;
				ctlNew.img.ccmp1 = ctlNew.img.st + 1 * (bpp/8 * ((ctlNew.img.pitch-1)/chDxDiv + ctlNew.img.pitch * (ctlNew.img.dy-1)/chDyDiv));
				ctlNew.img.ccmp2 = NULL;
				break;
			case 3:
				chDxDiv = 1; chDyDiv = 1;
				ctlNew.img.ccmp1 = ctlNew.img.st + 1 * (bpp/8 * ((ctlNew.img.pitch-1)/chDxDiv + ctlNew.img.pitch * (ctlNew.img.dy-1)/chDyDiv));
				ctlNew.img.ccmp2 = ctlNew.img.st + 2 * (bpp/8 * ((ctlNew.img.pitch-1)/chDxDiv + ctlNew.img.pitch * (ctlNew.img.dy-1)/chDyDiv));
				break;
			case 4:
				chDxDiv = 2; chDyDiv = 1;
				ctlNew.img.ccmp1 = ctlNew.img.st + 1 * (bpp/8 * ((ctlNew.img.pitch-1)/chDxDiv + ctlNew.img.pitch * (ctlNew.img.dy-1)/chDyDiv));
				ctlNew.img.ccmp2 = ctlNew.img.st + 2 * (bpp/8 * ((ctlNew.img.pitch-1)/chDxDiv + ctlNew.img.pitch * (ctlNew.img.dy-1)/chDyDiv));
				break;
			case 5:
				chDxDiv = 2; chDyDiv = 2;
				ctlNew.img.ccmp1 = ctlNew.img.st + 1 * (bpp/8 * ((ctlNew.img.pitch-1)/chDxDiv + ctlNew.img.pitch * (ctlNew.img.dy-1)/chDyDiv));
				ctlNew.img.ccmp2 = ctlNew.img.st + 2 * (bpp/8 * ((ctlNew.img.pitch-1)/chDxDiv + ctlNew.img.pitch * (ctlNew.img.dy-1)/chDyDiv));
				break;
			default:
				ee=-6; goto fail;
	}

	ctl_cp(ctl, &ctlOld);
	ctl_cp(&ctlNew, ctl);

	if(ctlOld.pid>=0)
	{
		//Detach Shared Memory of the old Image.
		rc =  munmap(ctlOld.img.st, bytes);
		if(rc< 0){ee=-7;goto fail;}
	}

	ee=0;
fail:
	switch(ee)
	{
		case +1: ee = 0;  break; //No Data at Pipe, proceed normally.
		case +2: ee = 0;  break; //Program Data not available, proceed normally.
		case  0: break;
		case -1:
		case -2:
		case -3:
		case -4:
				/*
				printf("Error, %s() failed: %s\n", (-1==ee)?("read")
												  :(-2==ee)?("shm_open")
												  :(-3==ee)?("Image Type")
												  :(-4==ee)?("ftruncate")
												  :(-5==ee)?("mmap")
												  :(-6==ee)?("internal")
												  :(-7==ee)?("munmap")
												  :("UNKNOWN"), strerror(errno));
				*/
				break;
		default: /*printf("Unknown Error %d: %d\n", ee, rc);*/ break;
	}
	if(-1!=fdShm){close(fdShm); fdShm=-1;}

	return(ee);
}




int  imgsrv_vcimgnetclient(int sockC, ImgNetSrvCfg *cfg)
{
	//The Communication Packet Header has the following Structure:
	//
	//   bytes = hbuf32[0]; //Only(!) used for Sending.
	//   x0    = hbuf32[1];
	//   y0    = hbuf32[2];
	//   dx    = hbuf32[3];
	//   dy    = hbuf32[4];
	//   incrx = hbuf32[5];
	//   incry = hbuf32[6];
	//
	#define HDR_BYTES (28)
	int   hbuf32[HDR_BYTES];
	int   ee, rc;
	int   bytes, lbytes, rcnt, xcnt, ycnt, x, y;
	int   x0, y0, dx, dy, incrx, incry, nr=0;
	char *pImg=NULL, *pPx= NULL;
	I32   bpp, ch;

	lbytes = -1; //forces pImg buffer allocation first time.

	while(1)
	{
		//Check if user program removed fd
		if(cfg->ctl.pid>=0)
		{
			rc =  shm_check_user_closed(cfg->ctl.shmId, cfg->ctl.pid, cfg->ctl.shmFd);
			if(rc<0){ee=-1+100*rc;goto fail;}
			if(rc>0)
			{
				if(cfg->ctl.pid>=0)
				{
					//Detach Shared Memory of the old Image.
					rc =  check_image_type(cfg->ctl.img.type, &bpp, &ch);
					if(ERR_NONE>rc){ee=-2+100*rc; goto fail;}

					bytes = cfg->ctl.img.dy * cfg->ctl.img.pitch * bpp/8 * ((ch<3)?(ch):(3));

					rc =  munmap(cfg->ctl.img.st, bytes);
					if(rc< 0){ee=-3+100*rc;goto fail;}
				}

				rc =  shm_unlink(cfg->ctl.shmId);
				if((-1==rc)&&(ENOENT!=errno)){ee=-4;goto fail;}

				ctl_reset(&(cfg->ctl));
			}
		}

		//Check if Fifo changed (User Program)
		rc =  ctl_update_by_fifo(&(cfg->fdCtl), &(cfg->ctl));
		if(rc<0){ee=-5+100*rc;goto fail;}

		rcnt =  recv(sockC, (char *)(&(hbuf32[1])), HDR_BYTES-4, 0);
		if(rcnt<0          ){ee=-6;goto fail;}
		if(rcnt==0         ){ee=+1;goto fail;}
		if(rcnt<HDR_BYTES-4){ee=-7;goto fail;}

		x0    = hbuf32[1];
		y0    = hbuf32[2];
		dx    = hbuf32[3];
		dy    = hbuf32[4];
		incrx = hbuf32[5];
		incry = hbuf32[6];

		//Quit program by client request.
		if(0==dx){ee=+2;goto fail;}

		//User Program limits Dimensions.
		if(cfg->ctl.pid>=0)
		{
			if(   x0>cfg->ctl.img.dx){ x0 = cfg->ctl.img.dx-1;         }
			if(   y0>cfg->ctl.img.dy){ y0 = cfg->ctl.img.dy-1;         }
			if(dx+x0>cfg->ctl.img.dx){ dx = max(0,cfg->ctl.img.dx-x0); }
			if(dy+y0>cfg->ctl.img.dy){ dy = max(0,cfg->ctl.img.dy-y0); }
		}

		//Get Image Type based information
		rc =  check_image_type(cfg->ctl.img.type, &bpp, &ch);
		if(ERR_NONE>rc){ee=-8+100*rc; goto fail;}

		//Get Image Bytecount, both values are floored.
		xcnt = (dx/incrx);
		ycnt = (dy/incry);
		if((0==xcnt)||(0==ycnt)){ee=-9;goto fail;}

		bytes = ycnt * xcnt * bpp/8 * ((ch<3)?(ch):(3)) + HDR_BYTES;

		//Check if Allocation Memory Size has to be changed.
		//First run: lbytes==-1 -> allocate.
		if(bytes!=lbytes)
		{
			if(NULL!=pImg)
			{
				free(pImg);
				pImg = NULL;
			}
			pImg =  malloc(bytes);
			if(NULL==pImg){ee=-10;goto fail;}

			lbytes = bytes;
		}

		//Write Header to Buffer
		hbuf32[0] = bytes;
		hbuf32[1] = x0   ;
		hbuf32[2] = y0   ;
		hbuf32[3] = dx   ;
		hbuf32[4] = dy * ((ch<3)?(ch):(3));
		hbuf32[5] = incrx;
		hbuf32[6] = incry;
		for(x= 0; x< HDR_BYTES/4; x++)
		{
			*(((int*)pImg) + x) = hbuf32[x];
		}

		pPx = pImg + HDR_BYTES;
		//User Program connected?
		if(cfg->ctl.pid>=0)
		{
			//Fill Buffer with Image Pixels
			{
				{
					for(y= y0; y< y0+dy; y+= incry)
					{
						for(x= x0; x< x0+dx; x+= incrx)
						{
							*pPx++ = *(cfg->ctl.img.st +  y * cfg->ctl.img.pitch  + x);
						}
					}
				}
				if(ch>1)
				{
					for(y= y0; y< y0+dy; y+= incry)
					{
						for(x= x0; x< x0+dx; x+= incrx)
						{
							*pPx++ = *(cfg->ctl.img.ccmp1 +  y * cfg->ctl.img.pitch  + x);
						}
					}
				}
				if(ch>2)
				{
					for(y= y0; y< y0+dy; y+= incry)
					{
						for(x= x0; x< x0+dx; x+= incrx)
						{
							*pPx++ = *(cfg->ctl.img.ccmp2 +  y * cfg->ctl.img.pitch  + x);
						}
					}
				}
			}
		}
		else
		{
			image imgTmp;
			imgTmp.type  = IMAGE_GREY;
			imgTmp.st    = (U8*)pPx;
			imgTmp.ccmp1 = NULL;
			imgTmp.ccmp2 = NULL;
			imgTmp.dx    = dx/incrx;
			imgTmp.dy    = dy/incry;
			imgTmp.pitch = imgTmp.dx;
			img_print_unconnected(&imgTmp,nr++);
		}


		rc =  send(sockC, (char *)pImg, bytes, 0);
		if(rc<0){ee=-11;goto fail;}

		if(0!=cfg->rest_ms)
			usleep(1000 * cfg->rest_ms);
	}


	ee=0;
fail:
	switch(ee%100)
	{
		case  +1:   ee= 0; /*printf("OK, Client Closed Connection.\n");           */ break;
		case  +2:   ee= 0; /*printf("OK, Quit by Client Request.\n");             */ break;
		case  -1:  /*printf("Error calling shm_check_user_closed(): %d.\n", rc);  */ break;
		case  -2:  /*printf("Error Image Type!\n");                               */ break;
		case  -3:  /*printf("Error calling munmap(): %s\n", strerror(errno));     */ break;
		case  -4:  /*printf("Error calling shm_unlink(): %s\n", strerror(errno)); */ break;
		case  -5:  /*printf("Error calling ctl_update_by_fifo(): %d\n", rc);      */ break;
		case  -6:  if(ECONNRESET==errno)
					{
						ee= 0; //Ignore 'Connection Reset by Peer' Errors.
					}
					else
					{
						/*printf("Error receiving Data: %s\n", strerror(errno));*/
					}
					break;
		case  -7:  /*printf("Error, Client sent wrong size of data.\n");          */ break;
		case  -8:  /*printf("Error Image Type!\n");                               */ break;
		case  -9:  /*printf("Error, Row or Colum Count is zero!\n");              */ break;
		case -10:  /*printf("Error allocating Memory!\n");                        */ break;
		case -11:  /*printf("Error sending Data: %s\n", strerror(errno));         */ break;
		default:   /*printf("Unknown Error: %d\n", ee);                           */ break;
	}

	if(NULL!=pImg){ free(pImg); pImg= NULL; }

	return(ee);
}




int  imgsrv_interact(int sockC, ImgNetSrvCfg *cfg)
{
	return(imgsrv_vcimgnetclient(sockC,cfg));
}




int  imgsrv_loop(ImgNetSrvCfg *cfg)
{
	int                 ee, rc;
	struct sockaddr_in  cliAddr, servAddr;
	int                 sock  = -1;
	int                 sockC = -1;
	socklen_t           len   = sizeof(struct sockaddr_in);
	int                 y     = 1;


	sock =  socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0){ee=-1;goto fail;}

	servAddr.sin_family      = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port        = htons(cfg->port);

	rc =  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
	if(rc<0){ee=-2;goto fail;}

	rc =  bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
	if(rc<0){ee=-3;goto fail;}

	while(1)
	{
		printf("Listen on port %u\n", cfg->port);

		rc =  listen(sock, 1);
		if(rc<0){ee=-4;goto fail;}

		sockC =  accept(sock, (struct sockaddr*) &cliAddr, &len);
		if(sockC< 0)
		{
			continue;
		}

		printf("Client connected!\n");

		rc =  imgsrv_interact(sockC, cfg);
		if(rc<0){ee=-5+10*rc;goto fail;}

		printf("Client disconnected!\n");

		close(sockC);
		sockC=-1;
	}

	ee=0;
fail:
	switch(ee%10)
	{
		case -1:  /*printf("Could not create socket: %s\n", strerror(errno));                  */ break;
		case -2:  /*printf("Could not set options for socket %d: %s\n", sock, strerror(errno));*/ break;
		case -3:  /*printf("Could not bind to port %d: %s\n", cfg->port, strerror(errno));     */ break;
		case -4:  /*printf("Could not listen at socket %d: %s\n", sock, strerror(errno));      */ break;
		case -5:  /*printf("Interaction failed (%d)!\n", rc);                                  */ break;
		default:  /*printf("Unknown Error %d!\n", ee);                                         */ break;
	}
	if(sock >0){ close(sock );  sock  = -1; }
	if(sockC>0){ close(sockC);  sockC = -1; }

	return(ee);
}




void  sig_handler(int signo)
{
	static   volatile  sig_atomic_t   processing_signal_iff1 = 0;

	if(1==processing_signal_iff1)
	{
		raise(signo);
	}
	else
	{
		processing_signal_iff1 = 1;

		switch(signo)
		{
			case SIGINT:
			case SIGTERM:
			case SIGQUIT:
	    		remove(VCIMGNET_FIFO_CTL);
				break;
		}

		processing_signal_iff1 = 0;
		//Now call the default handler
		signal(signo, SIG_DFL);
		raise(signo);
	}
}




int  main(int argc, char **argv)
{
	int           ee, rc;
	int           fdCtl   = -1;
	image         imgNull = NULL_IMAGE;
	ImgNetSrvCfg  cfg     = NULL_ImgNetSrvCfg;

	//You can pass a rest time in ms between two image transfers
	//by applying it as command line parameter.
	cfg.port = VCIMGNET_PORT;

	printf("Start VC Image Net Server ...\n");

	rc =  process_opts(argc, argv, &(cfg.rest_ms));
	if(rc<0){ee=-1234; goto fail;}

	if(
	     (signal(SIGINT, sig_handler) == SIG_ERR)
	   ||(signal(SIGTERM,sig_handler) == SIG_ERR)
	   ||(signal(SIGQUIT,sig_handler) == SIG_ERR)
	  )
	{ee=-1;goto fail;}


	rc =  create_fifo_vcimgnetctl(&(cfg.fdCtl));
	if(rc<0){ee=-2;goto fail;}


	//Reset for Startup.
	ctl_reset(&(cfg.ctl));

	cfg.ctl.pid      = -1; //Prevents Execution until Data over Pipe.
	cfg.ctl.shmId[0] = '\0';
	cfg.ctl.img      = imgNull;


	rc =  imgsrv_loop(&cfg);
	if(rc<0){ee=-3;goto fail;}


	ee = 0;
fail:
	switch(ee)
	{
		case  0: break;
		case -1: printf("Signal Handling failed!\n");                      break;
		case -2: printf("Lock File/shmkey generation failed (%d)!\n", rc); break;
		case -3: printf("Server Loop returned with Error %d!\n", rc);      break;
		case -1234: break; //Invalid argument
		default: break;
	}
	switch(ee)
	{
		case  0:
		case -4:
		case -3:
			remove_fifo_vcimgnetctl(&fdCtl);
		case -2:
		case -1:
		case -1234:
		default:
			break;
	}

	return(ee);
}




