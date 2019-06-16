/* -- 2007-05-07 -- 
 * libthjc - utility library
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
/************************************************************
 *	camera.h												*
 *	Header file for the camera object used to store and		*
 *	manipulate camera data.									*
 *															*
 *	Geoffrey Biggs, Toby Collet 2003						*
 ************************************************************/

#ifndef __CAMERA_H
#define __CAMERA_H

#include <libthjc/geometry.h>
#include <libthjc/vector3d.h>
#include <libthjc/matrix.h>



#include <list>
using namespace std;

//	Used to identify whether a camera is the left or right camera
typedef enum { CAM_LEFT, CAM_RIGHT }	CAM_ID;

//	Pair of coordinates for calibration
class CalibrationPair
{
public:
	double x; // image coord
	double y;
	double X; // respective 3d coord
	double Y;
	double Z;
};

//	This class represents a camera.
class Camera
{
public:

	Camera() : T(3,1), R(3,3), K(3,3) , P(3,4), RT(4,4), PFull(4,4), BPFull(4,4) {Pairs=NULL;WorkingPairs=NULL;};


	int width, height;			// The width and height of the camera image in pixels

	double Ncx; // Number of sensor elements in row
	double Nfx; // number of pixels in row
	double dx; // distance between sensor elements (mm/pixel)
	double dy;
	double Cx; // location of center of frame in pixels
	double Cy;


	double sx; // scale
	double sy;
	Matrix T; // Translation Matrix
	Matrix R; // Rotation Matrix
	Matrix K;
	Matrix P;
	Matrix RT;
	double f; // Effective Focal length
	double kappa1; // Distortion Coefficient
	Matrix PFull;
	Matrix BPFull;

	unsigned int NumPairs;
	CalibrationPair * WorkingPairs;
	CalibrationPair * Pairs;


	Camera& operator= (const Camera &rhs)
	{
/*		position = rhs.position;
		direction = rhs.direction;
		focalLength = rhs.focalLength;*/
		width = rhs.width;
		height = rhs.height;
		return *this;
	}

	// Accessor Functions
	double GetTx() const;
	double GetTy() const;
	double GetTz() const;
	double GetRx() const;
	double GetRy() const;
	double GetRz() const;

	void SetTx(double Value);
	void SetTy(double Value);
	void SetTz(double Value);
	void SetRx(double Value);
	void SetRy(double Value);
	void SetRz(double Value);
	
	double GetFocalDistance() const;
	void SetFocalDistance(double Value);

	void UpdateRT();

	// Evaluation Functions
	list<Point2D> GetProjectedPoints() const;
	Matrix GetErrMatrix() const;
	Matrix Get3DErrMatrix() const;

	
	// 3d helpers
	Ray GetBPRay(Point2D Point) const;
	Point2D PixelToPoint(Point2D Pixel) const;
	Vector3D Point2DToPoint3D(Point2D Point) const;
	Vector3D CRFToWRF(Vector3D Point3D_CRF) const;

	Point2D ProjectPoint(Vector3D WorldPoint) const;

public:
	void CalcApprox_f_Tz ();
	bool LoadIntrinsicParams (char *fileName);
	void Calibrate(list<CalibrationPair> &points);
	list<CalibrationPair> & LoadCalibPairs (list<Point2D> &points, list<CalibrationPair> &pairs, char * CalibFile);
	void Calibrate_SA();
private:
//	void CalcApprox_f_Tz(unsigned int NumPairs, CalibrationPair * WorkingPairs);
};

Matrix solve_RPY_transform (Matrix R);
Matrix apply_RPY_transform ( double R, double P, double Y);


#endif
