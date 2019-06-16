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
#ifdef HAVE_FFMPEG

#include <ardev/ardev.h>
#include <ardev/output_ffmpeg.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <X11/Xlib.h>

#include <ardev/debug.h>
#include <pthread.h>


extern "C" 
{
AVStream *add_audio_stream(AVFormatContext *oc, int codec_id);
void open_audio(AVFormatContext *oc, AVStream *st);
void get_audio_frame(int16_t *samples, int frame_size, int nb_channels);
void write_audio_frame(AVFormatContext *oc, AVStream *st);
void close_audio(AVFormatContext *oc, AVStream *st);
AVStream *add_video_stream(AVFormatContext *oc, int codec_id);
AVFrame *alloc_picture(int pix_fmt, int width, int height);
void open_video(AVFormatContext *oc, AVStream *st);
void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height);
void fill_rgb_image(AVFrame *pict, int frame_index, int width, int height);
void write_video_frame(AVFormatContext *oc, AVStream *st,AVFrame *);
void close_video(AVFormatContext *oc, AVStream *st);
}



OutputMovie::OutputMovie(const char * _Filename,int _x, int _y) : FrameProcessObject()
{
	width = _x;
	height = _y;
	Filename = strdup(_Filename);
}

// sets up an output video stream...once this is done we simply use the default
// Render frame to produce the output and overload output frame to write the frame
int OutputMovie::Initialise(bool Active)
{
	dbg_print(ARDBG_INFO,"Initialising Output Object\n");
	
	// Lock mutex so we dont initilaise two windows at once
	Lock();

	// intialise our mpeg class
	encoder = new ffmpeg(Filename,width,height,STREAM_FRAME_RATE);
	encoder->OpenStream();

	Unlock();
	
	return 0;
}

void OutputMovie::Terminate()
{
	// clean up video output
	
	encoder->CloseStream();
	delete encoder;
	encoder = NULL;

}


void OutputMovie::ProcessFrame(const ARImage & Frame)
{
	//int ret;	

	AVFrame * rgb_picture = encoder->alloc_pictureframe(PIX_FMT_RGB24, Frame.x_size, Frame.y_size);
	//AVPicture yuv_picture;// = encoder->alloc_picture(PIX_FMT_YUV420P, Frame.x_size, Frame.x_size);

/*	ret = avpicture_alloc(&rgb_picture, PIX_FMT_RGB24, Frame.x_size, Frame.y_size);
	if (ret < 0)
	{
		dbg_print(ARDBG_ERR,"Failed to allocate AVPicture\n");
		return;
	}
	ret = avpicture_alloc(&yuv_picture, PIX_FMT_YUV420P, Frame.x_size, Frame.y_size);
	if (ret < 0)
	{
		dbg_print(ARDBG_ERR,"Failed to allocate AVPicture\n");
		avpicture_free(&rgb_picture);
		return;
	}*/
		
	// flip the frame
	for (unsigned int i = 0; i < Frame.y_size; ++i)
		memcpy(rgb_picture->data[0] + i*Frame.x_size*3,&Frame.data[(Frame.y_size - i - 1)*Frame.x_size * 3],Frame.x_size*3);

	// convert to yuv_picture
	//img_convert(&yuv_picture, PIX_FMT_YUV420P, &rgb_picture, PIX_FMT_RGB24, Frame.x_size, Frame.y_size);

	// write frame
	encoder->WriteFrame((AVFrame *)rgb_picture);

	//avpicture_free(&rgb_picture);
	//avpicture_free(&yuv_picture);
	
	encoder->free_pictureframe(rgb_picture);
}

#endif
