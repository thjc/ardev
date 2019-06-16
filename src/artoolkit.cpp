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
 #include "config.h"
 #ifdef HAVE_LIBAR
 
#include <ardev/ardev.h>
#include <ardev/artoolkit.h>
#include <math.h>
#include <AR/ar.h>
#include <GL/gl.h>
#include <libthjc/geometry.h>

#include <ardev/debug.h>
ARToolKitPreProcess::ARToolKitPreProcess(/*const char *CameraFile, */const int Threshold, CameraObject & CameraOb) {
	//init args
	threshold = Threshold;
	camera = CameraOb.GetCamera();
	lastFrameWidth = 0;
	lastFrameHeight = 0;
	minfocnt = 0;
	minfo = NULL;
	
	unscaledCamParam.xsize = 320;
	unscaledCamParam.ysize = 240;
	unscaledCamParam.mat[0][0] = 390.40936389976349;
	unscaledCamParam.mat[0][1] = 0.0;
	unscaledCamParam.mat[0][2] = 166.5;
	unscaledCamParam.mat[0][3] = 0.0;
	unscaledCamParam.mat[1][0] = 0.0;
	unscaledCamParam.mat[1][1] = 389.68724428891949;
	unscaledCamParam.mat[1][2] = 128.5;
	unscaledCamParam.mat[1][3] = 0.0;
	unscaledCamParam.mat[2][0] = 0.0;
	unscaledCamParam.mat[2][1] = 0.0;
	unscaledCamParam.mat[2][2] = 1.0;
	unscaledCamParam.mat[2][3] = 0.0;
	unscaledCamParam.dist_factor[0] = 166.50000000000000;
	unscaledCamParam.dist_factor[1] = 132.50000000000000;
	unscaledCamParam.dist_factor[2] = 201.00000000000000;
	unscaledCamParam.dist_factor[3] = 1.0250151952322306;
	
	//set camera calibration
	if(arInitCparam( &unscaledCamParam ) != 0) {
		dbg_print(ARDBG_ERR,"arInitCparam failed\n");
	}
	

}

void ARToolKitPreProcess::ProcessFrame(const ARImage & frame) {
	if (frame.x_size == 0 || frame.y_size == 0)
		return;
	
	//check width and height, and scale camparam if necessary
	if(frame.x_size != lastFrameWidth || frame.y_size != lastFrameHeight) {
		lastFrameWidth = frame.x_size;
		lastFrameHeight = frame.y_size;

		//scale the params
		ARParam  cparam;
		arParamChangeSize( &unscaledCamParam, lastFrameWidth, lastFrameHeight, &cparam );

		//set camera calibration
		if(arInitCparam( &cparam ) != 0) {
			dbg_print(ARDBG_ERR,"arInitCparam failed after scaling\n");
		}
		
	}

	//check image format 
	if(frame.ColourFormat != GL_BGR && frame.ColourFormat != GL_RGB) {
		dbg_print(ARDBG_ERR,"Bad color format\n");
		throw "Bad colour format in artoolkit";
	}

	//now do the processing
	if(arDetectMarker(frame.data, threshold, &minfo, &minfocnt)!=0) {
        dbg_print(ARDBG_ERR,"arDetectMarker failed\n");
    } 
	else {
//		dbg_print(ARDBG_INFO,"arDetectMarker found %d markers\n",minfocnt);
		for(int i =0; i<minfocnt; i++) {
//		dbg_print(ARDBG_INFO,"\tmark %d type %d <%3.2f:%3.2f>\n",i,minfo[i].id,minfo[i].pos[0],minfo[i].pos[1]);
		}
	}

}

int ARToolKitPreProcess::AddPattern(const char *filename, double Height) {
	int newPatNum = -1;

	bool tryLoad = true;
	if(filename == NULL) {
		tryLoad = false;
	}
	else if(strlen(filename) == 0) {
		tryLoad = false;
	}
	
	if(tryLoad) {
		newPatNum = arLoadPatt((char *)filename);
		dbg_print(ARDBG_INFO,"pattern file %s is ID %d\n",filename,newPatNum);
	}
	
	if(newPatNum == -1) {
		dbg_print(ARDBG_ERR,"arLoadPatt failed for file %s\n",filename);
	}

	MarkerHeights[newPatNum] = Height;

	return newPatNum;
}

void ARToolKitPreProcess::RemovePattern(int id) {
	if(arFreePatt(id)!=1) {
		dbg_print(ARDBG_ERR,"arFreePatt failed to free pattern %d\n",id);
	}
}

bool ARToolKitPreProcess::GetMarkerPos(int id, ARPosition &result) {
	//find the best marker
	int bestMark = -1;
	float bestMarkCF = 0.0;
	for(int i =0 ;i < minfocnt; i++) {
		if(minfo[i].id == id && minfo[i].cf > bestMarkCF) {
			bestMark = i;
			bestMarkCF = minfo[i].cf;
		}
	}
	
	if(bestMark == -1) {	//no marker found
		//return the special values
		return false;
	}
	
	printf("Mark Found at: %f %f %f %f\n",minfo[bestMark].vertex[0][0],minfo[bestMark].vertex[0][1],minfo[bestMark].vertex[2][0],minfo[bestMark].vertex[2][1]);

	// Get ray in center of marker and one from edge
	ARPoint Center = camera.GetCRFPointFromPixel((minfo[bestMark].vertex[0][0] + minfo[bestMark].vertex[2][0])/2, 
		(minfo[bestMark].vertex[0][1] + minfo[bestMark].vertex[2][1])/2);
	ARPoint Direction = camera.GetCRFPointFromPixel(minfo[bestMark].vertex[3-minfo[bestMark].dir][0],minfo[bestMark].vertex[3-minfo[bestMark].dir][1]);
	
	printf("Center: "); Center.Print(); printf("\n");

	// convert to WRF
	Center = camera.GetWRFDirFromCRF(Center)-camera.Origin;
	Direction = camera.GetWRFDirFromCRF(Direction)-camera.Origin;

	printf("Center: "); Center.Print(); printf("\n");

	Ray ray1(Vector3D(camera.Origin.x,camera.Origin.y,camera.Origin.z), Vector3D(Center.x,Center.y,Center.z).Normalise());
	Ray ray2(Vector3D(camera.Origin.x,camera.Origin.y,camera.Origin.z), Vector3D(Direction.x,Direction.y,Direction.z).Normalise());

	Ray p_norm(Vector3D(0,0,MarkerHeights[id]),Vector3D(0,0,1));//MarkerHeights[id]+1));
	Plane p(p_norm);
	
	printf("Plane Norm: %f %f %f, %f %f %f\n",p_norm.Origin.x,p_norm.Origin.y,p_norm.Origin.z,p_norm.Direction.x,p_norm.Direction.y,p_norm.Direction.z);
	
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

	printf("Result in 3D: %f %f %f\n",Point1.x,Point1.y,Point1.z);

	result.Origin.x = Point1.x;
	result.Origin.y = Point1.y;
	result.Origin.z = Point1.z;
	
	double rot = atan2( Point2.y - Point1.y, Point2.x - Point1.x);
	result.Direction.x = 0.0;//rot;
	result.Direction.y = 0.0;//sin(rot);
	result.Direction.z = -rot;//cos(rot);

	// 
/*
	r1 = camera.GetRayFromImage(minfo[bestMark].vertex[0][0],minfo[bestMark].vertex[0][1],lastFrameWidth,lastFrameHeight);
	r2 = camera.GetRayFromImage(minfo[bestMark].vertex[1][0],minfo[bestMark].vertex[1][1],lastFrameWidth,lastFrameHeight);
	r3 = camera.GetRayFromImage(minfo[bestMark].vertex[2][0],minfo[bestMark].vertex[2][1],lastFrameWidth,lastFrameHeight);
	r4 = camera.GetRayFromImage(minfo[bestMark].vertex[3][0],minfo[bestMark].vertex[3][1],lastFrameWidth,lastFrameHeight);
	
	double stmscale = (camera.sensor_width/lastFrameWidth);  //assuming square pixels. convert pixels to mm
	
	//only scane y and z because x is already set to the focal length (mm)
	r1.y *= stmscale;
	r1.z *= stmscale;
	r2.y *= stmscale;
	r2.z *= stmscale;
	r3.y *= stmscale;
	r3.z *= stmscale;
	r4.y *= stmscale;
	r4.z *= stmscale;

	dbg_print(ARDBG_INFO, "scale = %f, r1 = <%f, %f, %f>\n",stmscale,r1.x,r1.y,r1.z);
	
	//calculate focalDistance and x_fov
	double focalDistance = 0.0;
	double x_fov = camera.y_fov * camera.aspect;
	focalDistance = (camera.sensor_width/2.0)/tan(x_fov/2.0);


	
//	dbg_print(ARDBG_INFO, "focalDistance = %f, x_fov = %f, y_fov = %f, aspect = %f, sensorwidth = %f\n",focalDistance, x_fov, camera.y_fov,camera.aspect,camera.sensor_width);
	
	//calculate height of target using any two diagonally opposite sides of target
	double patternDiagonalWidth = sqrt(2.0*(patternWidth*patternWidth));
	double screenDiagonalWidth = sqrt( ((r1.z - r3.z)*(r1.z-r3.z)) + ((r1.y - r3.y)*(r1.y-r3.y)));	//use y and z, not x and y. x and y not equal to screen x and y
	double height = (patternDiagonalWidth/screenDiagonalWidth)*focalDistance;
	
//	dbg_print(ARDBG_INFO, "focalDistance = %f, height = %f, pdw = %f, sdw = %f\n", focalDistance, height, patternDiagonalWidth, screenDiagonalWidth);
	
	Ray p(Vector3D(height,0,0), Vector3D(-1,0,0));
	Plane h(p);
	
	Ray ray1(Vector3D(0,0,0), Vector3D(r1.x,r1.y,r1.z));
	Ray ray2(Vector3D(0,0,0), Vector3D(r2.x,r2.y,r2.z));
	Ray ray3(Vector3D(0,0,0), Vector3D(r3.x,r3.y,r3.z));
	Ray ray4(Vector3D(0,0,0), Vector3D(r4.x,r4.y,r4.z));
	
	vector<Vector3D> ray1v = h.Intersect(ray1);
	vector<Vector3D> ray2v = h.Intersect(ray2);
	vector<Vector3D> ray3v = h.Intersect(ray3);
	vector<Vector3D> ray4v = h.Intersect(ray4);
	
	if(ray1v.size() == 0 || ray2v.size() == 0 || ray3v.size() == 0 || ray4v.size() ==0) {
		dbg_print(ARDBG_ERR, "GetMarkerPos:: one of the vectors didnt intersect the plane. (it should have)\n");
		return false;
	}

	dbg_print(ARDBG_INFO," height = %5.2f, p1 = <%5.2f, %5.2f, %5.2f>,  p2 = <%5.2f, %5.2f, %5.2f>,  p3 = <%5.2f, %5.2f, %5.2f>,  p4 = <%5.2f, %5.2f, %5.2f>\n",height, ray1v[0].x, ray1v[0].y, ray1v[0].z,  ray2v[0].x, ray2v[0].y, ray2v[0].z,  ray3v[0].x, ray3v[0].y, ray3v[0].z,  ray4v[0].x, ray4v[0].y, ray4v[0].z);
	
	result.Origin.x = (ray1v[0].x+ray2v[0].x+ray3v[0].x+ray4v[0].x)/4.0;
	result.Origin.y = (ray1v[0].y+ray2v[0].y+ray3v[0].y+ray4v[0].y)/4.0;
	result.Origin.z = (ray1v[0].z+ray2v[0].z+ray3v[0].z+ray4v[0].z)/4.0;
	
	double rot = atan2( minfo[bestMark].vertex[3-minfo[bestMark].dir][1] - minfo[bestMark].pos[1],  minfo[bestMark].vertex[3-minfo[bestMark].dir][0] - minfo[bestMark].pos[0]);
	result.Direction.x = rot;
	result.Direction.y = 0.0;//sin(rot);
	result.Direction.z = 0.0;//cos(rot);
	*/
	return true;
}


ARPosition ARToolKitPosition::GetPosition() {
	ARPosition tmp;
	
	if(Pre.GetMarkerPos(MarkerID, tmp)) {
		lastGoodPosition = tmp;
	}
	else {
		dbg_print(ARDBG_ERR,"GetPosition failed for marker %d, using last good position\n", MarkerID);
	}
	
	return lastGoodPosition;
}
#endif
