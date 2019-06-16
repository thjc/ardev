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
#ifdef HAVE_ARTOOLKITPLUS
#include <ARToolKitPlus/TrackerSingleMarker.h>
#include <ardev/artoolkitplus.h>
#include <ardev/debug.h>
#include <ardev/ardev.h>
#include <ARToolKitPlus/Camera.h>
#include <math.h>
#include <GL/gl.h>
#include <libthjc/geometry.h>

using namespace ARToolKitPlus;

ARToolKitPlusPreProcess::ARToolKitPlusPreProcess(CameraObject & CameraOb) 
{
	//init args
	camera = CameraOb.GetCamera();
	lastFrameWidth = 640;
	lastFrameHeight = 480;
	minfocnt = 0;
	tmp_markers = NULL;
	
	useBCH = true;
	
	// One off Initialisation of the artoolkitplus
    // create a tracker that does:
    //  - 6x6 sized marker images
    //  - samples at a maximum of 6x6
    //  - can load a maximum of 1 pattern
    //  - can detect a maximum of 16 patterns in one image
    //  - with an arbitrary default image size
    tracker = new ARToolKitPlus::TrackerSingleMarker(lastFrameWidth,lastFrameHeight);
    
	tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_LUM);

	if(!tracker->init(NULL, 1.0f, 1000.0f))            // load NULL camera
	{
		dbg_print(ARDBG_ERR,"ERROR: init() failed\n");
	}
	dbg_print(ARDBG_INFO,"Init is done\n");

	// disable undistortion as we dont have a proper calibration file to suppert it anyway
    tracker->setUndistortionMode(ARToolKitPlus::UNDIST_NONE);
    
    // setup a fake camera so we can set the width and height
	DummyCam = new Camera;
    memset(DummyCam->dist_factor,0,sizeof (DummyCam->dist_factor));
    memset(DummyCam->mat,0,sizeof (DummyCam->mat));
    DummyCam->changeFrameSize(lastFrameWidth,lastFrameHeight);
    tracker->setCamera(DummyCam);	
    
	// the marker in the BCH test image has a thin border...
    tracker->setBorderWidth(useBCH ? 0.125f : 0.250f);

	tracker->activateAutoThreshold(true);

    // switch to simple ID based markers
    // use the tool in tools/IdPatGen to generate markers
    tracker->setMarkerMode(useBCH ? ARToolKitPlus::MARKER_ID_BCH : ARToolKitPlus::MARKER_ID_SIMPLE);
    
    // allocate storage for the GrayScale image
    GrayImage.x_size = lastFrameWidth;
    GrayImage.y_size = lastFrameHeight;
    GrayImage.Allocate();
}

void ARToolKitPlusPreProcess::ProcessFrame(const ARImage & frame) 
{
	dbg_print(ARDBG_VERBOSE,"ARtoolkitPlus process frame called\n");
	if (frame.x_size == 0 || frame.y_size == 0)
		return;
	
	//check width and height
	if(frame.x_size != lastFrameWidth || frame.y_size != lastFrameHeight) {
		lastFrameWidth = frame.x_size;
		lastFrameHeight = frame.y_size;
		tracker->changeCameraSize(lastFrameWidth,lastFrameHeight);
		printf("\n");
		GrayImage.x_size = lastFrameWidth;
		GrayImage.y_size = lastFrameHeight;
	    GrayImage.Allocate();
	}

	//check image format 
	if(frame.ColourFormat != GL_BGR && frame.ColourFormat != GL_RGB) {
		dbg_print(ARDBG_ERR,"Bad color format\n");
		throw "Bad colour format in artoolkit";
	}
	
	// convert frame to gray scale
	for (unsigned int i = 0; i < frame.x_size * frame.y_size; ++i)
		GrayImage.data[i] = (frame.data[i*3]+frame.data[i*3+1] + frame.data[i*3+2])/3;

	dbg_print(ARDBG_VERBOSE,"Done converting to grayscale\n");

	//now do the processing
	dbg_print(ARDBG_VERBOSE,"Calling DetectMarker\n");
	if(tracker->arDetectMarker(const_cast<unsigned char*>(GrayImage.data), 0, &tmp_markers, &minfocnt) < 0)
	{
		dbg_print(ARDBG_INFO,"arDetectMarker failed\n");
		return;
	}
	dbg_print(ARDBG_VERBOSE,"ProcessFrame is Done, found %d markers\n",minfocnt);
	for(int i =0 ;i < minfocnt; ++i) 
	{
		dbg_print(ARDBG_VERBOSE,"Marker %d: id=%d area=%d cf=%f\n",i,tmp_markers[i].id,tmp_markers[i].area,tmp_markers[i].cf);
	}


}

int ARToolKitPlusPreProcess::AddPattern(int id, double Height) {
	MarkerHeights[id] = Height;

	return id;
}

void ARToolKitPlusPreProcess::RemovePattern(int id) {
	MarkerHeights[id] = 0;
}

bool ARToolKitPlusPreProcess::GetMarkerPos(int id, ARPosition &result) 
{
	dbg_print(ARDBG_VERBOSE,"Calling Get MarkerPos for %d\n", id);
	//find the best marker
	int bestMark = -1;
	float bestMarkCF = 0.0001;
	for(int i =0 ;i < minfocnt; i++) {
		if(tmp_markers[i].id == id && tmp_markers[i].cf > bestMarkCF) {
			bestMark = i;
			bestMarkCF = tmp_markers[i].cf;
		}
	}
	
	if(bestMark == -1) 
	{	//no marker found
		dbg_print(ARDBG_VERBOSE,"Marker (%d) Not Found\n", id);
		//return the special values
		return false;
	}
	
	dbg_print(ARDBG_VERBOSE,"Mark Found at: %f %f %f %f\n",tmp_markers[bestMark].vertex[0][0],tmp_markers[bestMark].vertex[0][1],tmp_markers[bestMark].vertex[2][0],tmp_markers[bestMark].vertex[2][1]);

	// Get ray in center of marker and one from edge
	ARPoint Center = camera.GetCRFPointFromPixel((tmp_markers[bestMark].vertex[0][0] + tmp_markers[bestMark].vertex[2][0])/2, 
		(tmp_markers[bestMark].vertex[0][1] + tmp_markers[bestMark].vertex[2][1])/2);
	ARPoint Direction = camera.GetCRFPointFromPixel(tmp_markers[bestMark].vertex[3-tmp_markers[bestMark].dir][0],tmp_markers[bestMark].vertex[3-tmp_markers[bestMark].dir][1]);
	
	//printf("Center: "); Center.Print(); printf("\n");

	// convert to WRF
	Center = camera.GetWRFDirFromCRF(Center)-camera.Origin;
	Direction = camera.GetWRFDirFromCRF(Direction)-camera.Origin;

	//printf("Center: "); Center.Print(); printf("\n");

	Ray ray1(Vector3D(camera.Origin.x,camera.Origin.y,camera.Origin.z), Vector3D(Center.x,Center.y,Center.z).Normalise());
	Ray ray2(Vector3D(camera.Origin.x,camera.Origin.y,camera.Origin.z), Vector3D(Direction.x,Direction.y,Direction.z).Normalise());

	Ray p_norm(Vector3D(0,0,MarkerHeights[id]),Vector3D(0,0,1));//MarkerHeights[id]+1));
	Plane p(p_norm);
	
	dbg_print(ARDBG_VERBOSE,"Plane Norm: %f %f %f, %f %f %f\n",p_norm.Origin.x,p_norm.Origin.y,p_norm.Origin.z,p_norm.Direction.x,p_norm.Direction.y,p_norm.Direction.z);
	
	vector<Vector3D> Point1s = (p.Intersect(ray1));
	if (Point1s.empty())
	{
		dbg_print(ARDBG_WARN,"No intersection found\n");
		return false;	
	}
	Vector3D Point1 = Point1s.front();
	
	vector<Vector3D> Point2s = (p.Intersect(ray2));
	if (Point2s.empty())
	{
		dbg_print(ARDBG_WARN,"No intersection found\n");
		return false;	
	}
	Vector3D Point2 = Point2s.front();

	dbg_print(ARDBG_VERBOSE,"Result in 3D: %f %f %f\n",Point1.x,Point1.y,Point1.z);

	result.Origin.x = Point1.x;
	result.Origin.y = Point1.y;
	result.Origin.z = Point1.z;
	
	double rot = atan2( Point2.y - Point1.y, Point2.x - Point1.x);
	result.Direction.x = 0.0;
	result.Direction.y = 0.0;
	result.Direction.z = rot-3*M_PI/4;

	return true;
}


ARPosition ARToolKitPlusPosition::GetPosition() 
{
	return lastGoodPosition;
}

bool ARToolKitPlusPosition::Present()
{
	ARPosition tmp;

	if(Pre.GetMarkerPos(MarkerID, tmp)) 
	{
		lastGoodPosition = tmp;
		DroppedFrames = 0;
		present = true;
	}
	else 
	{
		if (DroppedFrames > 100)
			present = false;
		else
			++DroppedFrames;
		dbg_print(ARDBG_VERBOSE,"GetPosition failed for marker %d, using last good position\n", MarkerID);
	}
	return present;
}

#endif
