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

#ifndef FFMPEG_H
#define FFMPEG_H

#include <ffmpeg/avformat.h>

class ffmpeg
{
	public:
		
		ffmpeg(char * filename, int width, int height, int framerate = 25);
		~ffmpeg();
		
		int OpenStream();
		int WriteFrame(AVFrame * frame);
		int CloseStream();

		AVFrame *alloc_pictureframe(int pix_fmt, int width, int height);
		void free_pictureframe(AVFrame * picture);
	private:
		AVStream *add_video_stream(AVFormatContext *oc, int codec_id);
		
		void open_video(AVFormatContext *oc, AVStream *st);
		void write_video_frame(AVFormatContext *oc, AVStream *st, AVFrame * in_picture);
		void close_video(AVFormatContext *oc, AVStream *st);
	
		int Width, Height, FrameRate;
		char * Filename;
	
	    AVOutputFormat *fmt;
    	AVFormatContext *oc;
    	AVStream *video_st;

		AVFrame * picture;
		uint8_t *video_outbuf;
		int frame_count, video_outbuf_size;	
};

#endif

#endif

