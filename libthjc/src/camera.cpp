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
/* -- 2005-07-19 -- 
 * ARDev Augmented Reality Robot Development Toolkit
 * Copyright 2005 - Toby Collett (ardev _at_ plan9.net.nz)
 *   University of Auckland Robotics Group
 *   http://www.ece.auckland.ac.nz/~robot
 *   
 * This program is licensed under the Open Software License 
 * version 2.1
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Open Software License for more details.
 * 
 * The full license should have been distributed along with this 
 * software in the file COPYING, if not it is available either
 * with the original software from the website above or from
 * www.opensouce.org
 * 
 */
/************************************************************
 *	camera.cpp												*
 *	Source file for the camera object used to store and		*
 *	manipulate camera data.									*
 *															*
 *	Toby Collet 2003										*
 ************************************************************/

#include <libthjc/camera.h>
#include <libthjc/matrix.h>
#include <math.h>

#include <string>
#include <fstream>
#include <iostream>
#include <libthjc/exception.h>

using namespace std;


#ifdef DEBUG
#define DOUT(x) cout << x << endl;
#else
#define DOUT(x)
#endif

// debug for calibration only
#ifdef DEBUG_CAL
#define DCAL(x) cout << x << endl;
#else
#define DCAL(x)
#endif


// load up the 3d coordinates from the file and build list with the 2d ones supplied
list<CalibrationPair> & Camera::LoadCalibPairs (list<Point2D> &points, list<CalibrationPair> &pairs, char * CalibFileName)
{
	ifstream CalibFile;
	CalibFile.open(CalibFileName, ios::in );
	if (!CalibFile.is_open())
		throw "Error Opening File";

	for (list<Point2D>::iterator ii = points.begin(); ii != points.end(); ii++)
	{
		CalibrationPair Temp;
		Temp.x = ii->x;
		Temp.y = ii->y;
		CalibFile >> Temp.X >> Temp.Y >> Temp.Z;
		pairs.push_back(Temp);
	}

	CalibFile.close();
	DOUT("Pairs Loaded from File");
	return pairs;
}

//	Loads camera's intrinsic parameters from a file
bool Camera::LoadIntrinsicParams (char *fileName)
{
	ifstream paramsFile;
	paramsFile.open(fileName, ios::in );
	if (!paramsFile.is_open())
		throw Exception ("Error opening file");
	string line;

	paramsFile >> line >> this->Ncx >> this->dx >> this->dx >> this->dy;

	paramsFile.close();
	DOUT("Loaded Camera Params");
	return true;
}


void Camera::CalcApprox_f_Tz(void)
{
	// 2.d
	Matrix A_Step2d(NumPairs, 2);
	Matrix b_Step2d(NumPairs, 1);
	
	double yi, wi;
	for (unsigned int i = 0; i < NumPairs; ++i)
	{
		yi = R.GetValue(1,0) * WorkingPairs[i].X + R.GetValue(1,1) * WorkingPairs[i].Y + R.GetValue(1,2) * WorkingPairs[i].Z + T.GetValue(1,0);
		wi = R.GetValue(2,0) * WorkingPairs[i].X + R.GetValue(2,1) * WorkingPairs[i].Y + R.GetValue(2,2) * WorkingPairs[i].Z;
		A_Step2d.SetValue(i,0,yi);
		A_Step2d.SetValue(i,1,-1 * WorkingPairs[i].y);

		b_Step2d.SetValue(i,0, wi * WorkingPairs[i].y);
	}

	// Solve in form Ax = b;
	Matrix x_Step2d = Matrix::Solve(A_Step2d, b_Step2d);
	f = x_Step2d.GetValue(0,0);
	T.SetValue(2,0,x_Step2d.GetValue(1,0));

	// Set initial distortion to 0
	kappa1 = 0;
}

double GetErr(CalibrationPair * Pairs, int NumPairs, double f, double k, double tz, Matrix K, Matrix RT, int sizex, int sizey)
{
	
	Matrix TestPoint(4,1);
	Matrix ImagePoint(3,1);
	TestPoint.SetValue(3,0,1);
	Matrix Errors(NumPairs,1);
	
	Matrix PTemp2(3,4, static_cast<unsigned int> (0));
	Matrix PFull(4,4, 1);
	
	// projection matrix
	PTemp2.SetValue(0,0, f);
	PTemp2.SetValue(1,1, f);
	PTemp2.SetValue(2,2,1);

	RT.SetValue(2,3,tz);
	//PFull = K * PTemp2 * RT;


	for (int ii = 0; ii < NumPairs ; ++ii)
	{

		TestPoint.SetValue(0,0,Pairs[ii].X);
		TestPoint.SetValue(1,0,Pairs[ii].Y);
		TestPoint.SetValue(2,0,Pairs[ii].Z);

		ImagePoint = PTemp2 * RT * TestPoint;
		double x,y;
		x = (ImagePoint.GetValue(0,0)/ImagePoint.GetValue(2,0));
		y = (ImagePoint.GetValue(1,0)/ImagePoint.GetValue(2,0));
		ImagePoint.SetValue(2,0,1);

		double r = sqrt(x*x + y*y);
		x += x*k*r*r;
		y += y*k*r*r;

		ImagePoint.SetValue(0,0,x);
		ImagePoint.SetValue(1,0,y);

		ImagePoint = K * ImagePoint;

		x = sizex-(ImagePoint.GetValue(0,0)/ImagePoint.GetValue(2,0));
		y = sizey-(ImagePoint.GetValue(1,0)/ImagePoint.GetValue(2,0));


		ImagePoint.SetValue(0,0,x);
		ImagePoint.SetValue(1,0,y);
		ImagePoint.SetValue(2,0,1);

		

		Errors.SetValue(ii,0, sqrt((Pairs[ii].x - ImagePoint.GetValue(0,0)) * (Pairs[ii].x - ImagePoint.GetValue(0,0)) + (Pairs[ii].y - ImagePoint.GetValue(1,0)) * (Pairs[ii].y - ImagePoint.GetValue(1,0))));
	}
	return Errors.Avg();
}


void Camera::Calibrate(list<CalibrationPair> &points)
{
	NumPairs = static_cast<unsigned int> (points.size());
	DOUT("Calibrating");
	DCAL("NumPairs: " << NumPairs);

	// Step One: Orientation, position (x and Y) and scale factor

	// 1.a.ii
	double dpri_x = dx*(Ncx/Nfx);

	DCAL("dpri_x: " << dpri_x );
	DCAL("dpri_y: " << dy  );


	// 1.a.iii
	if (WorkingPairs != NULL) delete[] WorkingPairs;
	if (Pairs != NULL) delete[] Pairs;

	WorkingPairs = new CalibrationPair[NumPairs];
	Pairs = new CalibrationPair[NumPairs];
	if (WorkingPairs == NULL || Pairs == NULL)
		throw "Allocation Error";
	{
		unsigned int i = 0;
		for (list<CalibrationPair>::iterator itr = points.begin(); itr != points.end(); ++itr,++i)
		{
			Pairs[i] = *itr;
			WorkingPairs[i] = *itr;
			WorkingPairs[i].x = dpri_x * (itr->x - Cx);
			WorkingPairs[i].y = dy * (itr->y - Cy);
		}
	}
	DOUT("Converted to array and centered etc");


	// 1.b
	// Solve in for Ax = b
	// First fill out matricies
	Matrix A_Step1b(NumPairs,7);
	Matrix b_Step1b(NumPairs,1);

	for (unsigned int i = 0; i < NumPairs; ++i)
	{
		A_Step1b.SetValue(i,0,WorkingPairs[i].y * WorkingPairs[i].X);
		A_Step1b.SetValue(i,1,WorkingPairs[i].y * WorkingPairs[i].Y);
		A_Step1b.SetValue(i,2,WorkingPairs[i].y * WorkingPairs[i].Z);
		A_Step1b.SetValue(i,3,WorkingPairs[i].y);
		A_Step1b.SetValue(i,4,-1 * WorkingPairs[i].x * WorkingPairs[i].X);
		A_Step1b.SetValue(i,5,-1 * WorkingPairs[i].x * WorkingPairs[i].Y);
		A_Step1b.SetValue(i,6,-1 * WorkingPairs[i].x * WorkingPairs[i].Z);

		b_Step1b.SetValue(i,0, WorkingPairs[i].x);
	}

	Matrix x_Step1b = Matrix::Solve(A_Step1b, b_Step1b);

	DCAL("A" << endl << A_Step1b << "x" << endl << x_Step1b << "b" << endl << b_Step1b );

	// 1c seperate parameters found in 1b
	
	// 1.c.1
	double mag_Ty = 1/sqrt(x_Step1b.GetValue(4,0)*x_Step1b.GetValue(4,0) + x_Step1b.GetValue(5,0)*x_Step1b.GetValue(5,0) + x_Step1b.GetValue(6,0)*x_Step1b.GetValue(6,0));
	DCAL("mag_Ty: " << mag_Ty );

	// 1.c.2
	// chose a point away from the image center
	double MaxDist = 0, TempDist = 0;
	int MaxDist_i = 0;
	for (unsigned int i = 0; i < NumPairs; ++i)
	{
		if ((TempDist = WorkingPairs[i].x * WorkingPairs[i].x + WorkingPairs[i].y * WorkingPairs[i].y) > MaxDist)
		{
			MaxDist = TempDist;
			MaxDist_i = i;
		}
	}

	DCAL("Working with point " << MaxDist_i);

	// temp calcs to get sign
	double r1, r2, r3, r4, r5, r6, Tx, x, y;
	r1 = x_Step1b.GetValue(0,0) * mag_Ty;
	r2 = x_Step1b.GetValue(1,0) * mag_Ty;
	r3 = x_Step1b.GetValue(2,0) * mag_Ty;
	r4 = x_Step1b.GetValue(4,0) * mag_Ty;
	r5 = x_Step1b.GetValue(5,0) * mag_Ty;
	r6 = x_Step1b.GetValue(6,0) * mag_Ty;
	Tx = x_Step1b.GetValue(3,0) * mag_Ty;
	x = r1 * WorkingPairs[MaxDist_i].X + r2 * WorkingPairs[MaxDist_i].Y + r3 * WorkingPairs[MaxDist_i].Z + Tx;
	y = r4 * WorkingPairs[MaxDist_i].X + r5 * WorkingPairs[MaxDist_i].Y + r6 * WorkingPairs[MaxDist_i].Z + mag_Ty;

	DCAL(r1 << " " << r2 << " " << r3 << " " << r4 << " " << r5 << " " << r6 << " " << Tx << " " << x << " " << y << " " << WorkingPairs[MaxDist_i].X << " " << WorkingPairs[MaxDist_i].Y << " " << WorkingPairs[MaxDist_i].Z );
	DCAL(WorkingPairs[MaxDist_i].x << " " << WorkingPairs[MaxDist_i].y );
	DCAL(mag_Ty );

	int sgn_Ty = (x*WorkingPairs[MaxDist_i].x) > 0 && (y*WorkingPairs[MaxDist_i].y) > 0 ? 1 : -1;
	DCAL("sign Ty: " << sgn_Ty );
    
	// 1.c.3
	sx = sqrt(x_Step1b.GetValue(0,0)*x_Step1b.GetValue(0,0) + x_Step1b.GetValue(1,0)*x_Step1b.GetValue(1,0) + x_Step1b.GetValue(2,0)*x_Step1b.GetValue(2,0)) * mag_Ty;
	sy = 1;


	DCAL("scale (x,y): " << sx << "," << sy );

	// 1.c.4
	// set up first 2 rows of R matrix
	R.SetValue(0,0,x_Step1b.GetValue(0,0) * sgn_Ty * mag_Ty / sx);
	R.SetValue(0,1,x_Step1b.GetValue(1,0) * sgn_Ty * mag_Ty / sx);
	R.SetValue(0,2,x_Step1b.GetValue(2,0) * sgn_Ty * mag_Ty / sx);

	R.SetValue(1,0,x_Step1b.GetValue(4,0) * sgn_Ty * mag_Ty);
	R.SetValue(1,1,x_Step1b.GetValue(5,0) * sgn_Ty * mag_Ty);
	R.SetValue(1,2,x_Step1b.GetValue(6,0) * sgn_Ty * mag_Ty);


	DCAL("Got First Bit of R");
	// calculate last row using orthonormal property and cross product
	double r7, r8, r9, lambda;
	r7 = R.GetValue(0,1) * R.GetValue(1,2) - R.GetValue(0,2) * R.GetValue(1,1);
	r8 = R.GetValue(0,2) * R.GetValue(1,0) - R.GetValue(0,0) * R.GetValue(1,2);
	r9 = R.GetValue(0,0) * R.GetValue(1,1) - R.GetValue(0,1) * R.GetValue(1,0);

	lambda = sqrt(r7*r7 + r8*r8 + r9*r9);

	R.SetValue(2,0,r7/lambda);
	R.SetValue(2,1,r8/lambda);
	R.SetValue(2,2,r9/lambda);

	DCAL("R" << endl );

	// set T Matrix
	T.SetValue(0,0,x_Step1b.GetValue(3,0) * mag_Ty * sgn_Ty /*/ sx*/); // not convinced that this should be taken out but will test it both ways
	T.SetValue(1,0,mag_Ty * sgn_Ty);

	// Step Two, compute Effective Focal Length, distortion coefficients and z position
	CalcApprox_f_Tz();

	if (f < 0)
	{
		DCAL("Focal dist error: " << f << " Trying other solution" );
		// try other solution
		R.SetValue(0,2,R.GetValue(0,2) * -1);
		R.SetValue(1,2,R.GetValue(1,2) * -1);
		R.SetValue(2,0,R.GetValue(2,0) * -1);
		R.SetValue(2,1,R.GetValue(2,1) * -1);
		
		CalcApprox_f_Tz();
		
		if (f < 0)
		{
			DCAL("Still Error with f: " << f );
			throw "Bad Calibration Coords (handed issue?)";
		}
	}	

	cout << T;

	// 2.e, stepest descesnt solution for f, Tx and kappa
	#ifdef NONLINEAR
	Calibrate_SA();
	#endif

	// Need to complete at a later date, first test the rest of the algorithm
	cout << T;
	DCAL("f: " << f);

	// Calculate full projection matrix from 3d to 2d
	// Temp Matricies to generate full projection matrix
	Matrix PTemp1(3,3, 1);
	Matrix PTemp2(3,4, static_cast<unsigned int> (0));
	Matrix PTemp3(4,4, 1);
	
	// scale and center matrix
	PTemp1.SetValue(0,0,-1.0/dx);
	PTemp1.SetValue(1,1,-1.0/dy);
	PTemp1.SetValue(0,2,Cx);
	PTemp1.SetValue(1,2,Cy);

	K = PTemp1;

	// projection matrix
	PTemp2.SetValue(0,0, f);
	PTemp2.SetValue(1,1, f);
	PTemp2.SetValue(2,2,1);

	P = PTemp2;

	// rotation and translation
	PTemp3.SetValue(0,0,R.GetValue(0,0));
	PTemp3.SetValue(0,1,R.GetValue(0,1));
	PTemp3.SetValue(0,2,R.GetValue(0,2));
	PTemp3.SetValue(1,0,R.GetValue(1,0));
	PTemp3.SetValue(1,1,R.GetValue(1,1));
	PTemp3.SetValue(1,2,R.GetValue(1,2));
	PTemp3.SetValue(2,0,R.GetValue(2,0));
	PTemp3.SetValue(2,1,R.GetValue(2,1));
	PTemp3.SetValue(2,2,R.GetValue(2,2));

	PTemp3.SetValue(0,3,T.GetValue(0,0));
	PTemp3.SetValue(1,3,T.GetValue(1,0));
	PTemp3.SetValue(2,3,T.GetValue(2,0));

	RT = PTemp3;

	PFull = PTemp1 * PTemp2 * PTemp3;

	// Get Scale matrix
	Matrix S(3,3,1);
	S.SetValue(0,0,sx);
	S.SetValue(1,1,sy);

	// back projection simple case
	Matrix Invf(4,4, static_cast<unsigned int> (0));
	Invf.SetValue(0,0,1/f);
	Invf.SetValue(1,1,1/f);
	Invf.SetValue(2,2,1/f);
	Invf.SetValue(3,2,1/f);

	// back rotation and scaling etc
	Matrix Temp4(3,3);
	Temp4 = R.T()*S.Inv();
	Matrix Temp5(4,4,1);
	Temp5.SetValue(0,0,Temp4.GetValue(0,0));
	Temp5.SetValue(0,1,Temp4.GetValue(0,1));
	Temp5.SetValue(0,2,Temp4.GetValue(0,2));

	Temp5.SetValue(1,0,Temp4.GetValue(1,0));
	Temp5.SetValue(1,1,Temp4.GetValue(1,1));
	Temp5.SetValue(1,2,Temp4.GetValue(1,2));

	Temp5.SetValue(2,0,Temp4.GetValue(2,0));
	Temp5.SetValue(2,1,Temp4.GetValue(2,1));
	Temp5.SetValue(2,2,Temp4.GetValue(2,2));

	BPFull = Temp5 * Invf;

	delete[] WorkingPairs;
	//delete[] Pairs;
	
	WorkingPairs = NULL;

	// All Done now return the Parameters

}

void Camera::Calibrate_SA()
{
#define F_STEP 0.01
#define K_STEP 0.0001
#define T_STEP 1
#define STOP_THRESH 0.00001

		Matrix Errs(27,4);
		Matrix f_Mat(3,1);
		Matrix k_Mat(3,1);
		Matrix t_Mat(3,1);

		Matrix RT(4,4,1);
		// rotation and translation
		RT.SetValue(0,0,R.GetValue(0,0));
		RT.SetValue(0,1,R.GetValue(0,1));
		RT.SetValue(0,2,R.GetValue(0,2));
		RT.SetValue(1,0,R.GetValue(1,0));
		RT.SetValue(1,1,R.GetValue(1,1));
		RT.SetValue(1,2,R.GetValue(1,2));
		RT.SetValue(2,0,R.GetValue(2,0));
		RT.SetValue(2,1,R.GetValue(2,1));
		RT.SetValue(2,2,R.GetValue(2,2));

		RT.SetValue(0,3,T.GetValue(0,0));
		RT.SetValue(1,3,T.GetValue(1,0));
		RT.SetValue(2,3,T.GetValue(2,0));

		cout << "RT=" << RT;

		Matrix K(3, 3, 1);
		
		// scale and center matrix
		K.SetValue(0,0,-1.0/dx);
		K.SetValue(1,1,-1.0/dy);
		K.SetValue(0,2,Cx);
		K.SetValue(1,2,Cy);

		f_Mat.SetValue(1,0,f);
		k_Mat.SetValue(1,0,0);
		t_Mat.SetValue(1,0,T.GetValue(2,0));

		double LastErr = 9999;

		int CountIt = 0;
		bool Continue = true;
		while(Continue)
		{
			CountIt++;
			f_Mat.SetValue(0,0,f_Mat.GetValue(1,0)-F_STEP);
			f_Mat.SetValue(2,0,f_Mat.GetValue(1,0)+F_STEP);

			k_Mat.SetValue(0,0,k_Mat.GetValue(1,0)-K_STEP);
			k_Mat.SetValue(2,0,k_Mat.GetValue(1,0)+K_STEP);

			t_Mat.SetValue(0,0,t_Mat.GetValue(1,0)-T_STEP);
			t_Mat.SetValue(2,0,t_Mat.GetValue(1,0)+T_STEP);

			for (unsigned int i = 0; i < 3; ++i)
			{
				for (unsigned int j = 0; j < 3; ++j)
				{
					for (unsigned int k = 0; k < 3; ++k)
					{
						//cout << i << " " << j << " " << k << " ";
						Errs.SetValue(((k*3+j)*3+i),0,GetErr(Pairs,27,f_Mat.GetValue(i,0),k_Mat.GetValue(j,0),t_Mat.GetValue(k,0),K,RT,width,height));
						Errs.SetValue(((k*3+j)*3+i),1,f_Mat.GetValue(i,0));
						Errs.SetValue(((k*3+j)*3+i),2,k_Mat.GetValue(j,0));
						Errs.SetValue(((k*3+j)*3+i),3,t_Mat.GetValue(k,0));
						//cout << Errs.GetValue(((k*3+j)*3+i),0) << endl;
					}
				}
			}
			double MinErr = Errs.GetCol(0).Min();
			unsigned int MinIndex;
			for (MinIndex = 0; MinIndex< 27;++MinIndex)
			{
				if (Errs.GetValue(MinIndex,0) == MinErr)
					break;
			}
			//if (Errs.GetValue(MinIndex,1) == f_Mat.GetValue(1,0) && Errs.GetValue(MinIndex,2) == k_Mat.GetValue(1,0) && Errs.GetValue(MinIndex,3) == t_Mat.GetValue(1,0))
			if (LastErr + STOP_THRESH > MinErr  && LastErr - STOP_THRESH < MinErr)
			{
				cout << "Got Solution" << endl;
				Continue = false;
			}
			else
			{
				f_Mat.SetValue(1,0,Errs.GetValue(MinIndex,1));
				k_Mat.SetValue(1,0,Errs.GetValue(MinIndex,2));
				t_Mat.SetValue(1,0,Errs.GetValue(MinIndex,3));

			}
			if (CountIt % 10 == 0)
			{
				cout << CountIt << " f: " << f_Mat.GetValue(1,0);
				cout << "\tkappa: " << k_Mat.GetValue(1,0);
				cout << "\ttz: " << t_Mat.GetValue(1,0);
				cout << "\tErr: " << MinErr << endl;
			}
			LastErr = MinErr;

		}
		f = f_Mat.GetValue(1,0);
		kappa1 = k_Mat.GetValue(1,0);
		T.SetValue(2,0,t_Mat.GetValue(1,0));
		RT.SetValue(2,3,T.GetValue(2,0));


}

double Camera::GetTx() const
{
	return T.GetValue(0,0);
}

double Camera::GetTy() const
{
	return T.GetValue(1,0);
}

double Camera::GetTz() const
{
	return T.GetValue(2,0);
}

double Camera::GetRx() const
{
	return solve_RPY_transform(R).GetValue(0,0);
}

double Camera::GetRy() const
{
	return solve_RPY_transform(R).GetValue(1,0);
}

double Camera::GetRz() const
{
	return solve_RPY_transform(R).GetValue(2,0);
}

void Camera::SetTx(double Value) 
{
	T.SetValue(0,0,Value);
	UpdateRT();
}

void Camera::SetTy(double Value) 
{
	T.SetValue(1,0,Value);
	UpdateRT();
}

void Camera::SetTz(double Value) 
{
	T.SetValue(2,0,Value);
	UpdateRT();
}

void Camera::SetRx(double Value) 
{
	R = apply_RPY_transform(Value, GetRy(), GetRz());
	UpdateRT();
}

void Camera::SetRy(double Value) 
{
	R = apply_RPY_transform(GetRx(),Value, GetRz());
	UpdateRT();
}

void Camera::SetRz(double Value) 
{
	R = apply_RPY_transform(GetRx(), GetRy(), Value);
	UpdateRT();
}


void Camera::UpdateRT()
{
	RT.SetValue(0,3,T.GetValue(0,0));
	RT.SetValue(1,3,T.GetValue(1,0));
	RT.SetValue(2,3,T.GetValue(2,0));
	RT.SetValue(0,0,R.GetValue(0,0));
	RT.SetValue(0,1,R.GetValue(0,1));
	RT.SetValue(0,2,R.GetValue(0,2));
	RT.SetValue(1,0,R.GetValue(1,0));
	RT.SetValue(1,1,R.GetValue(1,1));
	RT.SetValue(1,2,R.GetValue(1,2));
	RT.SetValue(2,0,R.GetValue(2,0));
	RT.SetValue(2,1,R.GetValue(2,1));
	RT.SetValue(2,2,R.GetValue(2,2));
}

double Camera::GetFocalDistance() const
{
	return 	f;
}

void Camera::SetFocalDistance(double Value)
{
	f = Value;	
}


	// Evaluation Functions
list<Point2D> Camera::GetProjectedPoints() const
{
	Matrix TestPoint(4,1);
	Matrix ImagePoint(3,1);
	TestPoint.SetValue(3,0,1);

	list<Point2D> ProjPoints;
	Point2D ProjPoint;
	
	Matrix PTemp2(3,4, static_cast<unsigned int> (0));
	Matrix PFull(4,4, 1);
	
	// projection matrix
	PTemp2.SetValue(0,0, f);
	PTemp2.SetValue(1,1, f);
	PTemp2.SetValue(2,2,1);

//	RT.SetValue(2,3,tz);
	//PFull = K * PTemp2 * RT;


	for (unsigned int ii = 0; ii < NumPairs ; ++ii)
	{

		TestPoint.SetValue(0,0,Pairs[ii].X);
		TestPoint.SetValue(1,0,Pairs[ii].Y);
		TestPoint.SetValue(2,0,Pairs[ii].Z);

		ImagePoint = PTemp2 * RT * TestPoint;
		double x,y;
		x = (ImagePoint.GetValue(0,0)/ImagePoint.GetValue(2,0));
		y = (ImagePoint.GetValue(1,0)/ImagePoint.GetValue(2,0));
		ImagePoint.SetValue(2,0,1);

		double r = sqrt(x*x + y*y);
		x += x*kappa1*r*r;
		y += y*kappa1*r*r;

		ImagePoint.SetValue(0,0,x);
		ImagePoint.SetValue(1,0,y);

		ImagePoint = K * ImagePoint;

		x = Ncx-(ImagePoint.GetValue(0,0)/ImagePoint.GetValue(2,0));
		y = 2*Cy-(ImagePoint.GetValue(1,0)/ImagePoint.GetValue(2,0));

		ProjPoint.x = x;
		ProjPoint.y = y;

		ProjPoints.push_back(ProjPoint);
	}
	return ProjPoints;
}

Matrix Camera::GetErrMatrix() const
{
	list<Point2D> ProjPoints = GetProjectedPoints();

	Matrix Errors(NumPairs,1);
	
	list<Point2D>::const_iterator itr = ProjPoints.begin();
	for (unsigned int ii = 0; ii < NumPairs ; ++ii, ++itr)
	{
		Errors.SetValue(ii,0, sqrt((Pairs[ii].x - itr->x) * (Pairs[ii].x - itr->x) + (Pairs[ii].y - itr->y) * (Pairs[ii].y - itr->y)));
	}
	return Errors;
}


// project the 2d image points to rays and the calculate the minimum distance from the ray to the 
// 3D point that the image point it matched to
Matrix Camera::Get3DErrMatrix() const
{
	Matrix Errors(NumPairs,1);
	cout << "num pairs" << NumPairs << endl;
	for (unsigned int i = 0; i < NumPairs ; ++i)
	{
		Point2D TempPoint;
		TempPoint.x = Pairs[i].x;
		TempPoint.y = Pairs[i].y;

		Ray TempRay = GetBPRay(TempPoint);
        
		Vector3D Temp3D;
		Temp3D.x = Pairs[i].X;
		Temp3D.y = Pairs[i].Y;
		Temp3D.z = Pairs[i].Z;

		double Err = TempRay.Dist2Point(Temp3D);

		Errors.SetValue(i,0,Err);

	}
	return Errors;
}


/***********************************************************************\
* This routine solves for the roll, pitch and yaw angles (in radians)	*
* for a given orthonormal rotation matrix (from Richard P. Paul,        *
* Robot Manipulators: Mathematics, Programming and Control, p70).       *
* Note 1, should the rotation matrix not be orthonormal these will not  *
* be the "best fit" roll, pitch and yaw angles.                         *
* Note 2, there are actually two possible solutions for the matrix.     *
* The second solution can be found by adding 180 degrees to Rz before   *
* Ry and Rx are calculated.                                             *
\***********************************************************************/
Matrix solve_RPY_transform (Matrix R)
{
	Matrix Ret(3,1);
    double    sg,
              cg;

    Ret.SetValue(2,0,atan2 (R.GetValue(0,1), R.GetValue(0,0)));
	
	sg = sin(Ret.GetValue(2,0));
	cg = cos(Ret.GetValue(2,0));

    Ret.SetValue(1,0,atan2 (-R.GetValue(0,2), R.GetValue(0,0) * cg + R.GetValue(0,1) * sg));
    Ret.SetValue(0,0,atan2 (R.GetValue(2,0) * sg - R.GetValue(2,1)* cg, R.GetValue(1,1) * cg - R.GetValue(1,0) * sg));
	
	return Ret;
}

Matrix apply_RPY_transform (double R, double P, double Y)
{
	Matrix Ret(3,3);
    double    sa,
              ca,
              sb,
              cb,
              sg,
              cg;

	sa = sin(R);
	ca = cos(R);
	sb = sin(P);
	cb = cos(P);
	sg = sin(Y);
	cg = cos(Y);

	Ret.SetValue(0,0,cb*cg);
	Ret.SetValue(1,0,cg * sa * sb - ca * sg);
	Ret.SetValue(2,0,sa * sg + ca * cg * sb);
	Ret.SetValue(0,1,cb * sg);
	Ret.SetValue(1,1,sa * sb * sg + ca * cg);
	Ret.SetValue(2,1,ca * sb * sg - cg * sa);
	Ret.SetValue(0,2,-sb);
	Ret.SetValue(1,2,cb * sa);
	Ret.SetValue(2,2,ca * cb);
	
	return Ret;
}

Ray Camera::GetBPRay(Point2D Point) const
{
	// Get point in image plane
	Point2D	UDistCamera = PixelToPoint(Point);

	// Convert to 3d poitn in CRF
	Vector3D P_CRF = Point2DToPoint3D(UDistCamera);

	// convert to WRF
	Vector3D P_WRF = CRFToWRF(P_CRF);

	// get camera origin in WRF
	Vector3D C_WRF = CRFToWRF(Vector3D(0,0,0));
    
	Ray Ret;
	Ret.Origin = C_WRF;
	Ret.Direction = P_WRF - C_WRF;
	Ret.Direction.Normalise();

	return Ret;
}


// conver image pixel to 2d point in the image plane
Point2D Camera::PixelToPoint(Point2D Pixel) const
{
	Point2D DistCamera, UDistCamera;

	// Get Distorted 2d camera point
	double dpri_x = dx*(Ncx/Nfx);
	DistCamera.x = (Pixel.x-Cx) * dpri_x;
	DistCamera.y = (Pixel.y-Cy) * dy;

	// Get 2d undistorted coordinates
    double r = sqrt(DistCamera.x*DistCamera.x + DistCamera.y*DistCamera.y);
	UDistCamera.x = DistCamera.x - DistCamera.x*kappa1*r*r;
	UDistCamera.y = DistCamera.y - DistCamera.y*kappa1*r*r;

	return UDistCamera;
}

// convert 2d poitn in image plain to 3d point in CRF
Vector3D Camera::Point2DToPoint3D(Point2D Point) const
{
	Vector3D Ret;
	Ret.x = Point.x;
	Ret.y = Point.y;
	Ret.z = f;

	return Ret;
}

// convert 3d poitn in CRF to 3d point in WRF
Vector3D Camera::CRFToWRF(Vector3D Point3D_CRF) const
{
	// Get homogenous coord form 3d point
	Matrix P_CRF(3,1);
	P_CRF.SetValue(0,0,Point3D_CRF.x);
	P_CRF.SetValue(1,0,Point3D_CRF.y);
	P_CRF.SetValue(2,0,Point3D_CRF.z);
	//P_CRF.SetValue(3,0,1);

	Matrix RTemp(R);

	Matrix Temp( RTemp.Inv()*(P_CRF - T));

	return Vector3D(Temp.GetValue(0,0), Temp.GetValue(1,0), Temp.GetValue(2,0));
}

Point2D Camera::ProjectPoint(Vector3D WorldPoint) const
{
	Matrix TestPoint(4,1);
	Matrix ImagePoint(3,1);
	TestPoint.SetValue(3,0,1);

	Point2D ProjPoint;
    	
	Matrix PTemp2(3,4, static_cast<unsigned int> (0));
	Matrix PFull(4,4, 1);
	
	// projection matrix
	PTemp2.SetValue(0,0, f);
	PTemp2.SetValue(1,1, f);
	PTemp2.SetValue(2,2,1);


	TestPoint.SetValue(0,0,WorldPoint.x);
	TestPoint.SetValue(1,0,WorldPoint.y);
	TestPoint.SetValue(2,0,WorldPoint.z);

	ImagePoint = PTemp2 * RT * TestPoint;
	double x,y;
	x = (ImagePoint.GetValue(0,0)/ImagePoint.GetValue(2,0));
	y = (ImagePoint.GetValue(1,0)/ImagePoint.GetValue(2,0));
	ImagePoint.SetValue(2,0,1);

	double r = sqrt(x*x + y*y);
	x += x*kappa1*r*r;
	y += y*kappa1*r*r;

	ImagePoint.SetValue(0,0,x);
	ImagePoint.SetValue(1,0,y);

	ImagePoint = K * ImagePoint;

	x = Ncx-(ImagePoint.GetValue(0,0)/ImagePoint.GetValue(2,0));
	y = 2*Cy-(ImagePoint.GetValue(1,0)/ImagePoint.GetValue(2,0));

	ProjPoint.x = x;
	ProjPoint.y = y;

	return ProjPoint;
}
