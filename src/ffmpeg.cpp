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

/*
 * Libavformat API example: Output a media file in any supported
 * libavformat format. The default codecs are used.
 * 
 * Copyright (c) 2003 Fabrice Bellard
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.  
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897931
#endif

#include <ardev/ffmpeg.h>


ffmpeg::ffmpeg(char * filename, int width, int height, int framerate)
{
	Filename = strdup(filename);
	Width = width;
	Height = height;
	FrameRate = framerate;
	picture = NULL;
	
	// First set up the movie output
	av_register_all();
	
    /* auto detect the output format from the name. default is
       mpeg. */
    fmt = guess_format(NULL, Filename, NULL);
    if (!fmt) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        fmt = guess_format("mpeg", NULL, NULL);
    }
    if (!fmt) {
        fprintf(stderr, "Could not find suitable output format\n");
        //return -1;
    }
    
    /* allocate the output media context */
    oc = av_alloc_format_context();
    if (!oc) {
        fprintf(stderr, "Memory error\n");
        //return -1;
    }
    oc->oformat = fmt;
    snprintf(oc->filename, sizeof(oc->filename), "%s", Filename);

    /* add the audio and video streams using the default format codecs
       and initialize the codecs */
    video_st = NULL;
    if (fmt->video_codec != CODEC_ID_NONE) {
        video_st = add_video_stream(oc, CODEC_ID_MPEG4);//fmt->video_codec);
    }

    /* set the output parameters (must be done even if no
       parameters). */
    if (av_set_parameters(oc, NULL) < 0) {
        fprintf(stderr, "Invalid output format parameters\n");
        exit(1);
    }

    dump_format(oc, 0, Filename, 1);

    /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
    if (video_st)
        open_video(oc, video_st);

}

ffmpeg::~ffmpeg()
{
	delete Filename;
}

int ffmpeg::OpenStream()
{
	
	    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (url_fopen(&oc->pb, Filename, URL_WRONLY) < 0) {
            fprintf(stderr, "Could not open '%s'\n", Filename);
            return -1;
        }
    }
    
    /* write the stream header, if any */
    av_write_header(oc);
	
	return 0;
}

int ffmpeg::WriteFrame(AVFrame * frame)
{
	write_video_frame(oc, video_st,frame);
	return 0;
}

int ffmpeg::CloseStream()
{
	    /* close each codec */
    if (video_st)
        close_video(oc, video_st);

    /* write the trailer, if any */
    av_write_trailer(oc);
    
    /* free the streams */
    for(int i = 0; i < oc->nb_streams; i++) {
        av_freep(&oc->streams[i]);
    }

    if (!(fmt->flags & AVFMT_NOFILE)) {
        /* close the output file */
        url_fclose(&oc->pb);
    }

    /* free the stream */
    av_free(oc);
	return 0;
}



/**************************************************************/
/* video output */


/* add a video output stream */
AVStream *ffmpeg::add_video_stream(AVFormatContext *oc, int codec_id)
{
    AVCodecContext *c;
    AVStream *st;

    st = av_new_stream(oc, 0);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }
    
    c = st->codec;
    c->codec_id = static_cast<CodecID> (codec_id);
    c->codec_type = CODEC_TYPE_VIDEO;

    /* put sample parameters */
    c->bit_rate = 800000;
    /* resolution must be a multiple of two */
    c->width = Width;  
    c->height = Height;
    /* frames per second */
	c->time_base = (AVRational){1,FrameRate};
//    c->frame_rate = FrameRate;  
//    c->frame_rate_base = 1;
	c->pix_fmt = PIX_FMT_YUV420P;

    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B frames */
        c->max_b_frames = 2;
    }
    if (c->codec_id == CODEC_ID_MPEG1VIDEO){
        /* needed to avoid using macroblocks in which some coeffs overflow 
           this doesnt happen with normal video, it just happens here as the 
           motion of the chroma plane doesnt match the luma plane */
        c->mb_decision=2;
    }
    // some formats want stream headers to be seperate
    if(!strcmp(oc->oformat->name, "mp4") || !strcmp(oc->oformat->name, "mov") || !strcmp(oc->oformat->name, "3gp"))
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    
    return st;
}


AVFrame *ffmpeg::alloc_pictureframe(int pix_fmt, int width, int height)
{
    AVFrame *picture;
    uint8_t *picture_buf;
    int size;
 
    picture = avcodec_alloc_frame();
    if (!picture)
		return NULL;
	size = avpicture_get_size(pix_fmt, width, height);
	picture_buf = (uint8_t*)malloc(size);
	if (!picture_buf) 
	{
		av_free(picture);
		return NULL;
	}
	avpicture_fill((AVPicture *)picture, picture_buf,
                pix_fmt, width, height);
	picture->pict_type = pix_fmt;
    return picture;
}

void ffmpeg::free_pictureframe(AVFrame * picture)
{
	avpicture_free((AVPicture *) picture);
	free(picture);
}

void ffmpeg::open_video(AVFormatContext *oc, AVStream *st)
{
    AVCodec *codec;
    AVCodecContext *c;
//	int ret;

    c = st->codec;

    /* find the video encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    /* open the codec */
    if (avcodec_open(c, codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    video_outbuf = NULL;
    if (!(oc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        video_outbuf_size = 200000;
        video_outbuf = reinterpret_cast<uint8_t*> (malloc(video_outbuf_size));
    }

    /* allocate the encoded raw picture */
	picture = alloc_pictureframe(c->pix_fmt, c->width, c->height);
    //ret = avpicture_alloc(&picture, c->pix_fmt, c->width, c->height);
    if (NULL == picture) {
        fprintf(stderr, "Could not allocate picture\n");
        exit(1);
    }

}


void ffmpeg::write_video_frame(AVFormatContext *oc, AVStream *st, AVFrame * in_picture)
{
    int out_size, ret;
    AVCodecContext *c;
    AVFrame *output_frame;

    c = st->codec;
	//printf("input %d out %d\n",in_picture->pict_type,c->pix_fmt);
    if (in_picture->pict_type != c->pix_fmt) 
	{
		// as we only generate a YUV420P picture, we must convert it
		//   to the codec pixel format if needed 
		//printf("converting image\n");
        img_convert((AVPicture *)picture, c->pix_fmt, 
                        (AVPicture *)in_picture, in_picture->pict_type,
                        c->width, c->height);
		output_frame = picture;
    } 
	else 
	{
		//printf("Frame Count: %d wxh (%d, %d)\n",frame_count,c->width, c->height);
		output_frame = in_picture;
    }

    /* encode the image */
	//printf("encode the frame\n");
    out_size = avcodec_encode_video(c, video_outbuf, video_outbuf_size, output_frame);
	//printf("encoded the frame\n");
    /* if zero size, it means the image was buffered */
    if (out_size != 0) \
	{
        AVPacket pkt;
        av_init_packet(&pkt);
            
        pkt.pts= c->coded_frame->pts;
        if(c->coded_frame->key_frame)
            pkt.flags |= PKT_FLAG_KEY;
        pkt.stream_index= st->index;
        pkt.data= video_outbuf;
        pkt.size= out_size;
            
        /* write the compressed frame in the media file */
		//printf("write the frame\n");
        ret = av_write_frame(oc, &pkt);
    } 
	else 
	{
       ret = 0;
    }
    if (ret != 0) 
	{
        fprintf(stderr, "Error while writing video frame\n");
        exit(1);
    }
		
    frame_count++;
}

void ffmpeg::close_video(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);
    free_pictureframe(picture);
    av_free(video_outbuf);
}

#endif
