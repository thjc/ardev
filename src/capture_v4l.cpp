/* -- 2007-05-07 -- 
 *  ardev - an augmented reality library for robot developers
 *  Copyright 2005-2007 - Toby Collett (ardev _at_ plan9.net.nz)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 *
 */

#ifdef HAVE_LINUX_VIDEODEV_H

#include <ardev/capture.h>
#include <fcntl.h>
#include <linux/videodev.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <ardev/debug.h>

CaptureV4L::CaptureV4L(const char * _VideoDev, int _width, int _height, int _channel, int _format, int _tuner)
{
	dbg_print(ARDBG_INFO,"Trying to open video device %s\n",_VideoDev);

	// copy start params for later use
	VideoDev=strdup(_VideoDev);
	width=_width;
	height=_height;
	channel=_channel;
	format=_format;
	//tuner=_tuner;
	Open = true;
	int err;
	Blocking = false;
	
	// now begin initialisation
	struct video_picture vid_pic;
	struct video_window win;
	struct video_channel chan;
	struct video_tuner tuner;
	struct video_mbuf buf;
	
	//open the device
	vid_fd = open(VideoDev,O_RDWR);
	if (vid_fd <0)
	{
		dbg_print(ARDBG_ERR,"Failed to open video device %s\n",_VideoDev);
		Open = false;
	}
	
	
	// Set the channel (1=Composite)
	if(channel >= 0)
	{
		chan.channel = channel;
		err = ioctl(vid_fd, VIDIOCGCHAN, &chan);
		if(err==-1)
		{
				dbg_print(ARDBG_ERR,"GCHAN returned error %d\n",errno);
				Open = false;
		}
	
		chan.channel = channel;
		chan.type = VIDEO_TYPE_CAMERA;
		chan.norm = format;
	
		err = ioctl(vid_fd, VIDIOCSCHAN, &chan);
		if(err==-1)
		{
				dbg_print(ARDBG_ERR,"SCHAN returned error %d\n",errno);
				Open = false;
		}
		usleep(50000);	usleep(50000);

	}


	// Set the mode (format)
	if (_tuner >= 0)
	{
		err = ioctl(vid_fd, VIDIOCGTUNER, &tuner);
		if(err==-1)
		{
				dbg_print(ARDBG_ERR,"GTUNER returned error %d\n",errno);
				Open = false;
		}
	
		tuner.mode = format;
		
		err = ioctl(vid_fd, VIDIOCSTUNER, &tuner);
		if(err<0)
		{
				dbg_print(ARDBG_ERR,"STUNER returned error %d\n",errno);
				Open = false;
		}	
	}

	// Set properties of the image window where image will be captured
	err = ioctl(vid_fd, VIDIOCGWIN, &win);
	if (err < 0)
	{
    		dbg_print(ARDBG_ERR,"GWIN return error %d\n",errno);
    		Open = false;
	}
  
  	win.x =0;
	win.y=0;
	win.chromakey=0;
	win.flags=0;
  	win.width=width;
  	win.height=height;
	win.clipcount=0;
  
	err = ioctl(vid_fd, VIDIOCSWIN, &win);
	if (err < 0)
	{
    		dbg_print(ARDBG_ERR,"SWIN return error %d\n",errno);
    		Open = false;
	}

	// check the window size was set correctly
	err = ioctl(vid_fd, VIDIOCGWIN, &win);
	if (err < 0)
	{
    		dbg_print(ARDBG_ERR,"GWIN return error %d\n",errno);
    		Open = false;
	}
  	else if ((int)win.height != height || (int)win.width != width) 
	{
    		dbg_print(ARDBG_ERR,"Couldn't set resolution (%d, %d), exiting\n", width, height);
    		Open = false;
	}

	// Set the properties for the image to be captured
  	err = ioctl(vid_fd, VIDIOCGPICT, &vid_pic);
  	if(err)
	{
    		dbg_print(ARDBG_ERR,"GPICT returned error %d\n",errno);
    		Open = false;
	}

	vid_pic.palette = VIDEO_PALETTE_RGB24; 
	vid_pic.depth=24;

  	err = ioctl(vid_fd, VIDIOCSPICT, &vid_pic);
  	if(err)
	{
    		dbg_print(ARDBG_ERR,"SPICT returned error %d\n",errno);
    		Open = false;
	}
  	// check it was set correctly
	err = ioctl(vid_fd, VIDIOCGPICT, &vid_pic);
  	if(err)
	{
    		dbg_print(ARDBG_ERR,"GPICT returned error %d\n",errno);
    		Open = false;
	}

	if (vid_pic.palette != VIDEO_PALETTE_RGB24) 
	{
    		dbg_print(ARDBG_ERR,"Error setting palette\n");
    		Open = false;
	}
  

	// Request mmap-able capture buffers
  	err = ioctl(vid_fd, VIDIOCGMBUF, &buf);
	if (err < 0)
	{
    		dbg_print(ARDBG_ERR,"GMBUF return error %d\n",errno);
    		Open = false;
	}
	
	dbg_print(ARDBG_INFO,"Buffer Size=%d, frames=%d,first offest=%d, image shoulb be=%d\n",buf.size,buf.frames,buf.offsets[1],width*height*3);
	
	uservmem=(unsigned char *)mmap(0, buf.size, PROT_READ | PROT_WRITE, MAP_SHARED, vid_fd, 0);
	if (uservmem == reinterpret_cast<unsigned char *> (-1))
	{
		dbg_print(ARDBG_ERR,"Failed to allocate usermap: %d\n",errno);
		Open = false;
	}
	
	m_vmem.frame=0;
	m_vmem.width=width;
	m_vmem.height=height;
	m_vmem.format = VIDEO_PALETTE_RGB24;


	NumFrames = 1;//buf.frames;
	CurrentFrame = NumFrames - 1;
	
	// Start Capture of the first frame;
	for (int i = 0; i < NumFrames - 1; ++i)
	{
		m_vmem.frame = i;
		err=ioctl(vid_fd, VIDIOCMCAPTURE, &m_vmem);
		if(err)
		{
			dbg_print(ARDBG_ERR,"CAPTURE returned error %d\n",errno);
			Open = false;
		}
	}			
		
	if (Open)
	{	
		Frame.data = new unsigned char[width*height*3];
		if (Frame.data != NULL)
		{
			Frame.x_size = width;
			Frame.y_size = height;
			Frame.ColourFormat = GL_BGR;
		}
		

		dbg_print(ARDBG_INFO,"V4l succesfully initialised\n");

			

	}
	
}

int CaptureV4L::GetMaxWidth()
{
	return width;
}

int CaptureV4L::GetMaxHeight()
{
	return height;
}

const ARImage & CaptureV4L::GetFrame()
{
	struct timeval Before,Now;
	if (Open)
	{
		int err;
		if (!Blocking)
		{
			m_vmem.frame = CurrentFrame;
	
			// Capture a frame from the camera
			err=ioctl(vid_fd, VIDIOCMCAPTURE, &m_vmem);
			if(err)
			{
				dbg_print(ARDBG_ERR,"CAPTURE returned error %d\n",errno);
			}
	
			CurrentFrame++; 
			if (CurrentFrame == NumFrames)
				CurrentFrame = 0;
		}
		
		// Synch to make sure capture is complete
		gettimeofday(&Before,NULL);
		unsigned long Flags;
		if((Flags=fcntl(vid_fd, F_GETFL))<0)
			printf("error getting flags: %d %s",errno,strerror(errno));
		if(fcntl(vid_fd, F_SETFL, Flags | O_NONBLOCK))		
			printf("error setting flags: %d %s",errno,strerror(errno));
		err=ioctl(vid_fd, VIDIOCSYNC, &m_vmem);
		fcntl(vid_fd, F_SETFL, Flags);		
		if (err<0 && errno == EWOULDBLOCK)
		{
			Blocking = true;
			return Frame;
		}
			
		Blocking = false;
		gettimeofday(&Now,NULL);
		//printf("Synch took %d usecs\n",(Now.tv_sec-Before.tv_sec)*1000000 + Now.tv_usec - Before.tv_usec);
	  	if(err)
		{
    		dbg_print(ARDBG_ERR,"SYNC returned error %d\n",errno);
		}

		Frame.data = uservmem+Frame.x_size*Frame.y_size*3*CurrentFrame;
		//memcpy(Frame.data,uservmem+Frame.x_size*Frame.y_size*3*CurrentFrame,Frame.x_size*Frame.y_size*3);
	}
	return Frame;
}

#endif
