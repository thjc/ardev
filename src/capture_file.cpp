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
#include <string.h>
#include <Magick++.h>
#include <sys/time.h>
#include <stdio.h>
#include <ardev/ardev.h>
#include <ardev/capture.h>

using namespace std;
using namespace Magick;

CaptureFile::CaptureFile(const char * _FileName, int _Delay) : CaptureObject()
{
	FileName = strdup(_FileName);
	Delay = _Delay;
	
	GetFrame();
}

int CaptureFile::GetMaxWidth()
{
	return Frame.x_size;
}

int CaptureFile::GetMaxHeight()
{
	return Frame.y_size;
}

const ARImage & CaptureFile::GetFrame()
	{
		static struct timeval LastTime = {0,0};
		static int FileAppend = 0;

		if (Delay==0 && Frame.data != NULL)
			return Frame;
				
		// Check for valid FileName
		if(FileName==NULL)
			return Frame;
		
		// if a delay is set see if we are past it (delay in micro seconds)
		struct timeval Now;
		gettimeofday(&Now, NULL);
		int SecDiff = Now.tv_sec - LastTime.tv_sec;
		int  uSecDiff = Now.tv_usec - LastTime.tv_usec;
		
		if (SecDiff > Delay/1000000 || (SecDiff == Delay/1000000 && uSecDiff >= Delay % 1000000))
		{
			LastTime = Now;
		}
		else
			return Frame;
		
		// Generate file name (allows for image sequences
		int bufferlen = strlen(FileName)+16;
		char * buffer = new char[bufferlen];
		FILE * ftemp;
		do
		{
			snprintf(buffer, bufferlen, FileName,FileAppend);
			
			// Check file exists
			if ((ftemp = fopen(buffer,"r")) == NULL)
			{
				if (FileAppend == 0)
					break;			
				FileAppend = 0;
			}
		} while (ftemp == NULL);
		if (ftemp ==NULL)
			return Frame;
		else
			fclose(ftemp);
		
		
		Image im;
		try {
			im.read(FileName);
			if ((Frame.y_size != im.rows()) || (Frame.x_size != im.columns()) && (Frame.data != NULL))
			{
				delete Frame.data;
				Frame.data = NULL;
			}
			
			if (Frame.data == NULL)
			{
				Frame.y_size = im.rows();
				Frame.x_size= im.columns();
				Frame.data = new unsigned char[im.rows()*im.columns()*3];
				im.write(0,0,im.columns(),im.rows(), "RGB", CharPixel, Frame.data);
			}		
			if (Delay != 0)
				++FileAppend;
			return Frame;
		}
		catch(  Exception &e)
		{
			return Frame;
		}
	}
