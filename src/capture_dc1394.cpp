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

#include <ardev/ardevconfig.h>

#include <ardev/capture.h>
#include <ardev/ardev.h>
#include <ardev/debug.h>
#include <libraw1394/raw1394.h>

#include <unistd.h>
extern "C"
{
	#include <dc1394/control.h>
	#include <dc1394/conversions.h>
}

CaptureDC1394::CaptureDC1394()
{
	// init private data
	camera = NULL;
	Open = false;

	// First we try to find a camera
	//dc1394camera_t **dccameras=NULL;
	dc1394camera_list_t * list;
	dc1394_t * d;
	dc1394error_t err;
	
	
	d = dc1394_new ();
	err=dc1394_camera_enumerate (d, &list);
	if ((err=dc1394_camera_enumerate(d,&list)) != DC1394_SUCCESS)
	{
		dbg_print(ARDBG_ERR,"Could not get Camera List: %d\n", err);
		return;
	}
	dbg_print(ARDBG_INFO,"Found %d firewire cameras, currently we are just using the first camera\n",list->num);
	if (list->num <=0)
		return;

	// we just use the first one returned and then free the rest
	camera = dc1394_camera_new (d, list->ids[0].guid);
	if (!camera) {
		dbg_print(ARDBG_ERR,"Failed to initialize camera with guid %llx", list->ids[0].guid);
		return;
	}
	dc1394_camera_free_list (list);

	// now we have selected a camera lets get some more info about it
	if (camera->bmode_capable>0)
	{
		// set b-mode and reprobe modes,... (higher fps formats might not be reported as available in legacy mode)
		if ((err=dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B)) != DC1394_SUCCESS)
		{
			dbg_print(ARDBG_ERR,"Could not set Camera bmode: %d\n", err);
			return;
		}
	}
//	if ((err=dc1394_get_camera_feature_set(camera, &features))!=DC1394_SUCCESS)
//	{
//		dbg_print(ARDBG_ERR,"Could not get Camera Features: %d\n", err);
//		return;
//	}

    // print some stuff for the user
//    if (ARDev::DebugLevel >= ARDBG_INFO)
//    {
//		dc1394_print_camera_info(camera);
//		dc1394_print_feature_set(&features);
//    }

	// check out wether we support mode 7
	if ((err=dc1394_format7_get_modeset(camera, &modeset))!=DC1394_SUCCESS)
	{
		dbg_print(ARDBG_ERR,"Could not Format 7 modeset: %d\n", err);
		return;
	}

	bool HasMode7 = false;
	for (unsigned int i=0;i<DC1394_VIDEO_MODE_FORMAT7_NUM;i++)
	{
		if (modeset.mode[i].present!=0)
		{
			HasMode7 = true;
			break;
		}
	}
	if (HasMode7)
	{
		dbg_print(ARDBG_INFO,"We have a mode 7 camera\n");
	}
	else
	{
		dbg_print(ARDBG_WARN,"No mode 7 mode support, sorry this module does not support your camera yet\n");
		return;
	}

	// get Iso Channel and Speed
	if ((err=dc1394_video_get_iso_speed(camera, &speed))!=DC1394_SUCCESS)
	{
		dbg_print(ARDBG_WARN,"Failed to get speed: %d\n",err);
		return;
	}
	dbg_print(ARDBG_INFO,"ISO speed = %d",speed);
	Open = true;

}

CaptureDC1394::~CaptureDC1394()
{
	free(camera);
}

#define  DELAY              50000
/// Iinitialise the ARObject
int CaptureDC1394::Initialise(bool Active)
{
	dc1394error_t err;
	dc1394switch_t status;
	if (!Open && camera)
	{
		dbg_print(ARDBG_ERR,"Camera device was not opened correctly\n");
		return -1;
	}
	ARObject::Initialise(Active);

	// cleanup should no longer be needed
	//dc1394_cleanup_iso_channels_and_bandwidth(camera);
	// on init we need to start up the Iso transfer for the camera
	dc1394_video_set_transmission(camera,DC1394_OFF);

	// now setup the capture
/*	if ((err=dc1394_dma_setup_format7_capture(camera,
					     DC1394_VIDEO_MODE_FORMAT7_0, DC1394_COLOR_CODING_RAW8, DC1394_ISO_SPEED_400,
					     (uint_t)DC1394_QUERY_FROM_CAMERA,
					     (uint_t)DC1394_QUERY_FROM_CAMERA, (uint_t)DC1394_QUERY_FROM_CAMERA,
					     (uint_t)DC1394_QUERY_FROM_CAMERA, (uint_t)DC1394_QUERY_FROM_CAMERA,
					     10,
					     1 // this will reduce latency
					     ))!=DC1394_SUCCESS)*/
	if ((err=dc1394_capture_setup(camera, 2, DC1394_CAPTURE_FLAGS_DEFAULT))) // 2 buffers and drop frames reduces latency
	{
		dbg_print(ARDBG_WARN,"Failed to setup capture\n");
		Open=false;
		return -1;
	}
	if ((err=dc1394_video_set_transmission(camera,DC1394_ON))!=DC1394_SUCCESS)
	{
		dbg_print(ARDBG_WARN,"Failed to start Iso transfer: %d\n",err);
		Open=false;
		return -1;
	}
	//delay to give it a chance to start
	usleep(DELAY);
	// now check it did start
	if ((err=dc1394_video_get_transmission(camera, &status))!=DC1394_SUCCESS)
	{
		dbg_print(ARDBG_WARN,"Failed to get Iso transfer status: %d\n",err);
		Open=false;
		return -1;
	}
	else
	{
		if (status==DC1394_OFF)
		{
			dbg_print(ARDBG_WARN,"Iso transfer did not start correctly\n");
			Open=false;
			return -1;
		}
	}
  dbg_print(ARDBG_INFO,"Iso Transfer Started\n");


	dbg_print(ARDBG_INFO,"Format7 Capture Started\n");

	Timer.GetElapsedDouble();

	return 0;

}

/// Terminate an ARObject
void CaptureDC1394::Terminate()
{
	// cleanup should no longer be needed
	//dc1394_cleanup_iso_channels_and_bandwidth(camera);
    dc1394_video_set_transmission(camera, DC1394_OFF);
	dc1394_camera_free(camera);
	camera = NULL;
	ARObject::Terminate();
}


int CaptureDC1394::GetMaxWidth()
{
	return 1360;
}

int CaptureDC1394::GetMaxHeight()
{
	return 1024;
}


//int dc1394_bayer_decoding_8bit(const unsigned char *bayer, unsigned char *rgb, uint_t sx, uint_t sy, uint_t tile, uint_t method);

const ARImage & CaptureDC1394::GetFrame()
{
	dc1394error_t err;
	dc1394video_frame_t * frame = NULL;
	// first grab the frame
  dbg_print(ARDBG_INFO,"Calling dc1394_capture_dma\n");
	err = dc1394_capture_dequeue (camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
	if (!frame) {
		dbg_print(ARDBG_WARN, "Failed to dequeue frame\n");
		return Frame;
	}
  dbg_print(ARDBG_INFO,"Calling dc1394_capture_dma done\n");

	// now we copy it out of the buffer
  dbg_print(ARDBG_INFO,"Copy frame from buffer\n");
  //dc1394_format7_get_image_size(camera, camera->video_mode, &Frame.x_size, &Frame.y_size);
  dbg_print(ARDBG_INFO,"Copy frame from buffer done\n");
	Frame.x_size = frame->size[0];
	Frame.y_size = frame->size[1];
  dbg_print(ARDBG_INFO,"Frame was %dx%d buffer at %p\n", Frame.x_size, Frame.y_size,frame->image);
	Frame.ByteDepth = 3;
	Frame.ColourFormat=GL_RGB;
	Frame.Allocate();
	assert(Frame.data);

	// do bayer decode
//	dc1394_bayer_decoding_8bit((const unsigned char *)camera->capture.capture_buffer, Frame.data,
  dc1394_bayer_decoding_8bit(frame->image, Frame.data,				   Frame.x_size,Frame.y_size,
				   DC1394_COLOR_FILTER_RGGB,DC1394_BAYER_METHOD_NEAREST);

	// then tell the driver we are done with the buffer
	if (dc1394_capture_enqueue (camera, frame) != DC1394_SUCCESS) {
		dbg_print(ARDBG_WARN, "Failed to enqueue frame\n");
		return Frame;
	}

	return Frame;
}

