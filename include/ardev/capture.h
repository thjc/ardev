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
#ifndef ARDEV_CAPTURE_H
#define ARDEV_CAPTURE_H

/***************************************************************************
 *            capture.h
 *
 *  Wed Apr 21 15:34:26 2004
 *  Copyright  2004  Toby Collett
 *  Email
 ****************************************************************************/

#include <ardev/ardevconfig.h>
#include <ardev/ardev.h>

#include <sys/time.h>
#include <libthjc/misc.h>

#ifdef HAVE_LINUX_VIDEODEV_H
#include <linux/videodev.h>
#endif

#ifdef HAVE_DC1394
#include <libraw1394/raw1394.h>
#include <dc1394/control.h>
#endif



/** \brief Captures a frame from a file or series of files
 *
 * If a delay is specified then each time that delay has passed then the next image in a sequence is loaded
 *
 * This class is mainly for debugging and testing purposes
 */
class CaptureFile : public CaptureObject
{
	public:
		/// \brief Load the image from the given filename, or if a sequence is desired pattern
		///
		/// Pattern is specified in printf style ie "image%d.jpeg", the next in the sequence is loaded after Delay
		///
		/// Image Magick is used to load the file, so in theory any format supported by ImageMagick will work
		CaptureFile(const char * FileName, int Delay=0);
		/// Return the current Frame
		const ARImage & GetFrame();
		/// returns the maximum possible return frame width
		int GetMaxWidth();
		/// returns the maximum possible return frame height
		int GetMaxHeight();
	private:
		char * FileName; ///< Copy fo the filename
		int Delay; ///< Sequence delay
		ARImage  Frame; ///< Current Frame
};

#ifdef HAVE_LINUX_VIDEODEV_H

/** \brief Captures a frame from a video4linux (v4l) device
 *
 */
class CaptureV4L : public CaptureObject
{
	public:
		/// Create a CaptureObject that will capture a (wdithxheight) image from the specified videodevice using the specified channel, format and tuner
		CaptureV4L(const char * _VideoDev ="/dev/video0", int _width=768, int _height=480, int _channel=1, int _format=1, int _tuner=-1);
		/// Destructor	
		virtual ~CaptureV4L() {free(VideoDev);};
		/// Get the current Frame
		const ARImage & GetFrame();
		/// Did the last call to GetFrame return a fresh image
		bool Fresh() {return !Blocking;};
		/// returns the maximum possible return frame width
		int GetMaxWidth();
		/// returns the maximum possible return frame height
		int GetMaxHeight();
	private:
		char * VideoDev; ///< Device file name
		int width; ///< Image Width
		int height; ///< Image Height
		int channel; ///< Capture Channel
		int format; ///< Capture Format 
		//int tuner; ///< The v4l tuner to use (-1 for none)
		
		int NumFrames;
		int CurrentFrame;
	
		bool Open; ///< Device is open
		int vid_fd; ///< Device File descriptor
		unsigned char * uservmem; ///< mem map pointer
		struct video_mmap m_vmem; ///< mem map structure
		bool Blocking;
		
		ARImage Frame; ///< Current Frame
};
#endif


#ifdef HAVE_DC1394
/** \brief Captures a frame from a firewire device
 *
 */
class CaptureDC1394 : public CaptureObject
{
	public:
		/// Create a CaptureObject that will capture a (wdithxheight) image from the specified videodevice using the specified channel, format and tuner
		CaptureDC1394();
		/// Destructor	
		virtual ~CaptureDC1394();

		/// Iinitialise the ARObject
		virtual int Initialise(bool Active = true);
		/// Terminate an ARObject			
		virtual void Terminate();


		/// Get the current Frame
		const ARImage & GetFrame();
		/// Did the last call to GetFrame return a fresh image
		bool Fresh() {return !Blocking;};
		/// returns the maximum possible return frame width
		int GetMaxWidth();
		/// returns the maximum possible return frame height
		int GetMaxHeight();
	private:
		bool Open; ///< Device is open
		bool Blocking;
		
		/// dc1394 private data
		dc1394camera_t * camera;
		dc1394featureset_t features;
		dc1394format7modeset_t modeset;
		//unsigned int channel;
		dc1394speed_t speed;
		
		StopWatch Timer;
		
		ARImage Frame; ///< Current Frame
};
#endif

#endif
