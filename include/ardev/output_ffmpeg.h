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

#ifndef OUTPUT_FFMPEG_H
#define OUTPUT_FFMPEG_H

/***************************************************************************
 *            output_ffmpeg.h
 *
 *  Tue Apr 27 15:34:26 2004
 *  Copyright  2004  Toby Collett
 *  Email
 ****************************************************************************/

#include <ardev/ardev.h>
#include <pthread.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <X11/Xlib.h>

#include <ardev/ffmpeg.h>

#define STREAM_FRAME_RATE 12 /* images/s */

/** \brief Output the frames of a Primary OutputObject to a mpeg movie file 
 */
class OutputMovie : public FrameProcessObject
{
	public:
		/// Create the OutputMovie Object with target filename and size
		OutputMovie(const char * Filename = "Output.mpeg", int x=300,int y=300);
		int Initialise(bool); ///< Initialise the output stream
		void Terminate(); ///< Close the output stream
		void ProcessFrame(const ARImage & Frame); ///< Write a frame to the stream
	private:
		char * Filename; ///< Movie Filename
		ffmpeg * encoder; ///< Current Encoder	
		int width; ///< Width of output frame
		int height; ///< Height of output frame
};

#endif

#endif
