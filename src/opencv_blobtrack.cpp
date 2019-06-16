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
#include <ardev/ardev.h>
#include <ardev/opencv_blobtrack.h>
#include <math.h>
#include <GL/gl.h>
#include <libthjc/geometry.h>

#include <opencv/highgui.h>

#define WITH_DEBUG_WINDOW
//#define WITH_TEMPORAL_AVERAGE

#include <ardev/debug.h>
OpenCVBlobTrackPreProcess::OpenCVBlobTrackPreProcess(CameraObject & CameraOb, double BlobMinSize, double BlobMaxSize, bool debug)
{
	camera = CameraOb.GetCamera();
	lastFrameWidth = 0;
	lastFrameHeight = 0;

	PatNumber = 0;

	MinSize = BlobMinSize;
	MaxSize = BlobMaxSize;

/*	memset(imSeq,0,sizeof(imSeq));
	StoredSize.width = 0;
	StoredSize.height = 0;*/

	Debug = debug;

	if (Debug)
	{
		cvNamedWindow("OpenCV Debug",1);
		cvNamedWindow("OpenCV Debug Sat",1);
		cvNamedWindow("OpenCV Debug Hue",1);
		cvNamedWindow("OpenCV Debug Value",1);
	}

}

OpenCVBlobTrackPreProcess::~OpenCVBlobTrackPreProcess()
{
	dbg_print(ARDBG_VERBOSE,"in opencv destructor\n");
	dbg_print(ARDBG_VERBOSE,"end of opencv destructor\n");
}


void OpenCVBlobTrackPreProcess::ProcessFrame(const ARImage & frame) {
	if (frame.x_size == 0 || frame.y_size == 0)
		return;

	//check image format
	if(frame.ColourFormat != GL_BGR && frame.ColourFormat != GL_RGB) {
		dbg_print(ARDBG_ERR,"Bad color format\n");
		throw "Bad colour format in opencv";
	}

	//now do the processing
	CvSize size = {frame.x_size, frame.y_size};

	dbg_print(ARDBG_VERBOSE,"Creating opencv image and copying data\n");
	IplImage* src = cvCreateImage( size, IPL_DEPTH_8U, 3);
	memcpy(src->imageData,frame.data,src->imageSize);

	IplImage* hsv_img = cvCreateImage(size, src->depth, src->nChannels);

	if(frame.ColourFormat == GL_BGR)
		cvCvtColor(src, hsv_img, CV_BGR2HSV);
	else
		cvCvtColor(src, hsv_img, CV_RGB2HSV);

	IplImage* h_img = cvCreateImage(size, src->depth, 1);
	IplImage* s_img = cvCreateImage(size, src->depth, 1);
	IplImage* v_img = cvCreateImage(size, src->depth, 1);
	IplImage* h_img_g = cvCreateImage(size, src->depth, 1);
	IplImage* s_img_g = cvCreateImage(size, src->depth, 1);

	dbg_print(ARDBG_VERBOSE,"Split frame to HSV\n");
	cvSplit(hsv_img, h_img, s_img, v_img, NULL);

	cvSmooth(h_img, h_img_g);
	cvSmooth(s_img, s_img_g);

	if (Debug)
	{
		cvShowImage("OpenCV Debug Sat", s_img_g);
		cvShowImage("OpenCV Debug Hue", h_img_g);
		cvShowImage("OpenCV Debug Value", v_img);
	}

	DropLowSat(h_img_g, s_img_g, 40, 0, size);
	DropLowSat(h_img_g, v_img, 100, 0, size);

	// iterate through blobs we are supposed to be finding and
	// find real world coordinates and store back in the map
	dbg_print(ARDBG_VERBOSE,"Get the blob coordinates, %d BLobs\n",Markers.size());

	for (std::map<int,BlobPair,intlt_bt>::iterator itr = Markers.begin();itr != Markers.end(); ++itr)
	{
		// find center of a blobs
		dbg_print(ARDBG_VERBOSE,"First Blob: %d %d\n", itr->second.Hue1_Min, itr->second.Hue1_Max);
		ARPoint P1 = GetBlobCenter(itr->second.Hue1_Min, itr->second.Hue1_Max, h_img_g, size);
		dbg_print(ARDBG_VERBOSE,"Second Blob\n");
		ARPoint P2 = GetBlobCenter(itr->second.Hue2_Min, itr->second.Hue2_Max, h_img_g, size);

		if (P1 == ARPoint(0,0,0) || P2 == ARPoint(0,0,0))
		{
			itr->second.pos = ARPosition(ARPoint(0,0,0),ARPoint(0,0,0));
		}
		else
		{
			dbg_print(ARDBG_VERBOSE,"Get the 3d position for the blob pair (%f %f)(%f %f) at %f\n", P1.x, P1.y,P2.x,P2.x,itr->second.Height);
			itr->second.pos = GetPosition(P1,P2,itr->second.Height);
			dbg_print(ARDBG_VERBOSE,"Got the 3d position for the blob pair\n");
		}
	}

	dbg_print(ARDBG_VERBOSE,"clean up images\n");
	cvReleaseImage(&src);
	cvReleaseImage(&hsv_img);
	cvReleaseImage(&h_img);
	cvReleaseImage(&s_img);
	cvReleaseImage(&v_img);
	cvReleaseImage(&h_img_g);
	cvReleaseImage(&s_img_g);
}

// find center of a blob
ARPoint OpenCVBlobTrackPreProcess::GetBlobCenter(int HueMin, int HueMax, IplImage * im, CvSize size)
{
	ARPoint Ret;
	IplImage *t1 = cvCreateImage(size, im->depth, im->nChannels);
	IplImage *t2 = cvCreateImage(size, im->depth, im->nChannels);

	// drop off top hues
	dbg_print(ARDBG_VERBOSE,"Top Threshold: %p %p %d %d %d\n",im,t1,HueMax, 0, CV_THRESH_TOZERO_INV);
	cvThreshold( im, t1, HueMax, 0, CV_THRESH_TOZERO_INV);
	// then the lower ones
	dbg_print(ARDBG_VERBOSE,"Bottom Threshold\n");
	cvThreshold( t1, t2, HueMin, 0, CV_THRESH_TOZERO);

	cvMorphologyEx(t2,t2,NULL,NULL,CV_MOP_OPEN);

	// allocate structures for segmentation
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* segment = NULL;

	double CurrentMaxSize = 0;
	cvFindContours( t2, storage, &segment, sizeof(CvContour), CV_RETR_EXTERNAL);
	for (; segment != 0; segment = segment->h_next)
	{
		// minimum number of points needed for fit ellipse
		if (segment->total < 6)
			continue;

		CvBox2D r = cvMinAreaRect2(segment);

		double area = fabs(cvContourArea(segment));
		double hwratio = r.size.width/r.size.height;
		if (area > CurrentMaxSize && area < MaxSize && hwratio > 1.0/3.0 && hwratio < 3.0)
		{
			CurrentMaxSize = area;
			Ret.x = r.center.x;
			Ret.y = r.center.y;
		}
	}
	if (CurrentMaxSize < MinSize)
		Ret = ARPoint(0,0,0);

	if (Debug)
	{
		#define min(x,y) (x<y?x:y)
		#define max(x,y) (x>y?x:y)

		CvPoint P1, P2;
		CvScalar RectColour = {{255,0,0,0}};
		P1.x = (int) max(Ret.x - 10,0);
		P1.y = (int)Ret.y;
		P2.x = (int)min(Ret.x + 10,size.width);
		P2.y = (int)Ret.y;
		cvLine(im,P1,P2,RectColour);
		P1.x = (int)Ret.x;
		P1.y = (int)max(Ret.y - 10,0);
		P2.x = (int)Ret.x;
		P2.y = (int)min(Ret.y + 10,size.height);
		cvLine(im,P1,P2,RectColour);

		cvShowImage("OpenCV Debug", im);
		cvWaitKey(10);
	}

	cvReleaseMemStorage(&storage);
	cvReleaseImage(&t1);
	cvReleaseImage(&t2);

	return Ret;
}


void OpenCVBlobTrackPreProcess::DropLowSat(IplImage * h_img, IplImage * s_img, int sat_thresh, int NewHue, CvSize size)
{
	for (int x = 0; x < size.width; ++x)
	{
		for (int y = 0; y < size.height; ++y)
		{
			if (((unsigned char) s_img->imageData[x+y*size.width]) < sat_thresh)
				h_img->imageData[x+y*size.width] = NewHue;
		}
	}
}


int OpenCVBlobTrackPreProcess::AddBlobs(BlobPair_t & blob)
{
	dbg_print(ARDBG_VERBOSE,"Adding new blob\n");
	Markers[++PatNumber] = blob;
	return PatNumber;
}

void OpenCVBlobTrackPreProcess::RemoveBlobs(int id)
{
	Markers.erase(id);
}

bool OpenCVBlobTrackPreProcess::GetMarkerPos(int id, ARPosition &result)
{
	result = Markers[id].pos;
	if (result == ARPosition(ARPoint(0,0,0),ARPoint(0,0,0)))
	{
		return false;
	}
	else
		return true;
}

ARPosition OpenCVBlobTrackPreProcess::GetPosition(ARPoint P1, ARPoint P2, double Height)
{
	ARPosition result;
	// Get ray in center of marker and one from edge
	dbg_print(ARDBG_VERBOSE,"Get CRF of two points\n");
	ARPoint Center = camera.GetCRFPointFromPixel((P1.x + P2.x)/2,(P1.y + P2.y)/2);
	ARPoint Direction = camera.GetCRFPointFromPixel((P1.x - Center.x),(P1.y - Center.y));

	// convert to WRF
	dbg_print(ARDBG_VERBOSE,"convert to WRF\n");
	Center = camera.GetWRFDirFromCRF(Center)-camera.Origin;
	Direction = camera.GetWRFDirFromCRF(Direction)-camera.Origin;

	dbg_print(ARDBG_VERBOSE,"Build rays and make plane\n");
	Ray ray1(Vector3D(camera.Origin.x,camera.Origin.y,camera.Origin.z), Vector3D(Center.x,Center.y,Center.z).Normalise());
	Ray ray2(Vector3D(camera.Origin.x,camera.Origin.y,camera.Origin.z), Vector3D(Direction.x,Direction.y,Direction.z).Normalise());

	Ray p_norm(Vector3D(0,0,Height),Vector3D(0,0,1));
	Plane p(p_norm);

	dbg_print(ARDBG_VERBOSE,"Find intersection of ground plane and the rays\n");
	vector<Vector3D> Point1s = (p.Intersect(ray1));
	if (Point1s.empty())
	{
		dbg_print(ARDBG_WARN,"No intersection found\n");
		return result;
	}
	Vector3D Point1 = Point1s.front();


	vector<Vector3D> Point2s = (p.Intersect(ray2));
	if (Point2s.empty())
	{
		dbg_print(ARDBG_WARN,"No intersection found\n");
		return result;
	}
	Vector3D Point2 = Point2s.front();


	dbg_print(ARDBG_VERBOSE,"Store in result position\n");
	result.Origin.x = Point1.x;
	result.Origin.y = Point1.y;
	result.Origin.z = Point1.z - Height;

	dbg_print(ARDBG_VERBOSE,"Rotate Result\n");
	double rot = atan2( Point2.y - Point1.y, Point2.x - Point1.x);
	result.Direction.x = 0.0;
	result.Direction.y = 0.0;
	result.Direction.z = (rot+M_PI/2);

	dbg_print(ARDBG_VERBOSE,"Return Result\n");
	return result;
}


ARPosition OpenCVBlobTrackPosition::GetPosition()
{
	ARPosition tmp;
	Lock();
	if(MarkerID > 0 && Pre.GetMarkerPos(MarkerID, tmp))
	{
		lastGoodPosition = tmp;
	}
	Unlock();

	return lastGoodPosition;
}

/// Is the position object present, ie in fov for optical tracking
bool OpenCVBlobTrackPosition::Present()
{
	bool ret = true;
	Lock();
	ARPosition tmp;
	Elapsed += Timer.GetElapsedDouble();
	if(MarkerID > 0 && Pre.GetMarkerPos(MarkerID, tmp))
	{
		Elapsed = 0;
	}
	else
	{
		if (Elapsed > 3000000) // if we havent seen a marker for more than 5 seconds
			ret = false;
	}
	Unlock();
	return ret;
};


