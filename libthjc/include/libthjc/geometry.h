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
 *	geometry.h												*
 *	Contains various useful geometry functions, for example	*
 *	calculating the distance between 2 points.				*
 *															*
 *	Geoffrey Biggs, 2003									*
 *	Toby Collett, 2003										*
 ************************************************************/

#ifndef __GEOMETRY_H
#define __GEOMETRY_H

#include "vector3d.h"
#include <math.h>
#include <list>
#include <vector>
using namespace std;

#define PI 3.1415926535897932384626433832795

// get angle between 0 and 2 PI
double mod2pi(double ang);
// Get angle between - pi and +pi
double modpi(double ang);

//	A 2D point
class Point2D
{
public:
	double x, y;

	Point2D() {};
	Point2D(double n_x, double n_y) {x = n_x; y = n_y;};
	virtual ~Point2D() {};

	virtual Point2D& operator= (const Point2D &rhs)
	{
		x = rhs.x;
		y = rhs.y;
		return *this;
	}

	Point2D& operator= (double val)
	{
		x = y = val;
		return *this;
	}

	Point2D & Norm() {double d = sqrt(x*x+y*y); x = x/d; y=y/d; return *this;};
	Point2D Rotate(Point2D Origin, double Theta);
	double Ang() const {return atan2(y,x);};

	virtual bool operator== (const Point2D &rhs) const {return (x == rhs.x && y == rhs.y);};
	virtual bool operator!= (const Point2D &rhs) const {return (x != rhs.x || y != rhs.y);};

	Point2D operator - (const Point2D & rhs) const {return Point2D(x-rhs.x,y-rhs.y); };
	Point2D operator + (const Point2D & rhs) const {return Point2D(x+rhs.x,y+rhs.y); };
	Point2D operator * (const double rhs) const {return Point2D(x*rhs,y*rhs); };
	Point2D operator / (const double rhs) const {return Point2D(x/rhs,y/rhs); };

	Point2D & operator += (const Point2D &rhs) {x += rhs.x; y += rhs.y; return *this;};
	Point2D & operator -= (const Point2D &rhs) {x -= rhs.x; y -= rhs.y; return *this;};

	friend ostream& operator<< (ostream &os, const Point2D &rhs);
};

ostream& operator<< (ostream &os, const Point2D &rhs);

//double CalcDistance(const Point2D &, const Point2D &);
double CalcDistance (const Point2D & point1, const Point2D &point2);// {return CalcDistance(point1.x,point1.y,point2.x,point2.y);};

class ApproxPoint2D : public Point2D
{
public:
	ApproxPoint2D() {};

	ApproxPoint2D(Point2D Pt, double D) {x = Pt.x; y = Pt.y;delta = D;};
	virtual ~ApproxPoint2D() {};
	
	virtual bool operator== (const Point2D &rhs) const {return (CalcDistance(*this,rhs) < delta);};
	virtual bool operator!= (const Point2D &rhs) const {return (CalcDistance(*this,rhs) >= delta);};

	ApproxPoint2D& operator= (const ApproxPoint2D &rhs)
	{
		x = rhs.x;
		y = rhs.y;
		delta = rhs.delta;
		return *this;
	}
	double delta;
};

//	A 3D ray
class Ray
{
public:
	Ray() {};
	Ray(const Vector3D &Orig, const Vector3D &Dir) {Origin = Orig; Direction = Dir;};

	Vector3D Origin;
	Vector3D Direction;

	Vector3D Intersect(Ray Ray2);
	double Dist2Point(Vector3D Point);


};


//	A 2D ray
class Ray2D
{
public:
	Ray2D() {};
	Ray2D(const Point2D &Orig, const Point2D &Dir) {Origin = Orig; Direction = Dir;};

	Point2D Origin;
	Point2D Direction;

	Ray2D operator - (const Ray2D & rhs) const;
	Ray2D operator + (const Ray2D & rhs) const;

	Ray2D & operator += (const Ray2D &rhs) {*this = *this + rhs; return *this;};
	Ray2D & operator -= (const Ray2D &rhs) {*this = *this - rhs; return *this;};

	bool operator == (const Ray2D &rhs) const {return this->Origin == rhs.Origin && this->Direction == rhs.Direction;};
	bool operator != (const Ray2D &rhs) const {return this->Origin != rhs.Origin || this->Direction != rhs.Direction;};

	//Point2D Intersect(Ray2D Ray2);
	//double Dist2Point(Point2D Point);
};


class Line
{
public:
	Line() {};
	Line(Point2D n_P1, Point2D n_P2) {P1 = n_P1; P2 = n_P2;};

	double DistToPoint(const Point2D &Pb) const;
	Point2D ClosestPoint(const Point2D &Pb) const;

	Point2D P1;
	Point2D P2;
};


// An Arc of with Origin O and radius R between Theta 1 and Theta 2 in the CCW direction
// Theta 1 and Theta2 are between 0 and 2 PI
class CircleSeg
{
public:
	CircleSeg() {};
	CircleSeg(Point2D O, double Ang1, double Ang2, double Radius) {Origin = O; Theta1 = Ang1; Theta2 = Ang2; R = Radius;};
	CircleSeg(Line L, double Radius, bool Left = true);

	double GetTheta1() const {return Theta1;};
	double GetTheta2() const {return Theta2;};
	void SetTheta1(double Theta) {while(Theta > 2 * PI) Theta -= 2*PI; while(Theta < 0) Theta += 2*PI;Theta1 = Theta;};
	void SetTheta2(double Theta) {while(Theta > 2 * PI) Theta -= 2*PI; while(Theta < 0) Theta += 2*PI;Theta2 = Theta;};

	double DistToPoint(const Point2D &Pb) const;
	Point2D ClosestPoint(const Point2D &Pb) const;

	double GetLength() const;

	Point2D Origin;
	double R;
private:
	double Theta1;
	double Theta2;

};


class Circle
{
public:
	Circle() {};
	Circle(Point2D O, double Radius) {Origin = O; R = Radius;};

	double DistToPoint(const Point2D &Pb) const;
	Point2D ClosestPoint(const Point2D &Pb) const;

	Point2D Origin;
	double R;
};


// 3D Geometry Objects 

class Object3D
{
	public: 
		virtual ~Object3D() {};
	
		virtual vector<Vector3D> Intersect(const Ray & R) = 0;
};

class Sphere : public Object3D
{
	public:
		Sphere(const Vector3D& Origin, double Radius);
		virtual ~Sphere() {};
		
		vector<Vector3D> Intersect(const Ray & R);
		
		Vector3D Origin;
		double Radius;
};
		
class Plane : public Object3D
{
	public:
		Plane(const Ray & Normal);
		virtual ~Plane() {};
		
		vector<Vector3D> Intersect(const Ray & R);
		bool InPlane(const Vector3D & Point);
		bool BoundCircle(const Vector3D & Point, double Radius);
		/// is the specified point on the plane and bound by a circle segment 
		/// defined by the point 'Start' and within the angle (radians)
		bool BoundCircleSegment(const Vector3D & Point, const Vector3D & Start,
		double Theta);
		
		Ray Normal;
};		

//	An image filter: filters data that is outside the specified square
class FieldFilter
{
public:
	FieldFilter (void);
	FieldFilter (Point2D topLeft, Point2D topRight, Point2D bottomRight, Point2D bottomLeft);

	list<Point2D> FilterPoints (list<Point2D> points);
	void SetPoints (Point2D topLeft, Point2D topRight, Point2D bottomRight, Point2D bottomLeft);
	void SetPoints (int *newPoints);
	int* GetPoints (void)			{ return fieldPoints; }

private:
	int fieldPoints[8];
};

double CalcDistance (Vector3D point1, Vector3D point2);
double CalcDistance (double x1, double y1, double x2, double y2);

void CalcAngles (Vector3D vector, double &rotX, double &rotY, double &rotZ);
double CalcHorizDist (Vector3D point1, Vector3D point2);
double CalcSpeed (int timeDiff, double distance);
Vector3D CalcMidPoint (Vector3D point1, Vector3D point2);
Point2D CalcMidPoint (Point2D point1, Point2D point2);

void IntersectLines (Vector3D &left1, Vector3D &left2, Vector3D &right1, Vector3D &right2, Vector3D &resultLeft, Vector3D &resultRight, double &distLeft, double &distRight);
void ListIntsToListPoints (list<int> &intsList, list<Point2D> &pointsList);
// returns at most 2 intersecitons if they exist
vector<Point2D> &Intersection(const Circle &C, const CircleSeg &A);
vector<Point2D> &Intersection(const Circle &C1, const Circle &C2);
vector<Point2D> &Intersection(const Circle &C, const Line &L);
vector<Point2D> &Intersection(const CircleSeg &A, const Line &L);
vector<Point2D> &Intersection(const Line &L1, const Line &L2, bool Segment = true);

//pass point, and array of x y points, tl, tr, br, bl. return if point inside square or not
bool Inside(int px, int py, int *dat);
int pnpoly(int npol, float *xp, float *yp, float x, float y);


//double DistLine2Point(const Vector3D &Left1, const Vector3D & Left2, const Vector3D & Right);
//double DistLine2Point(const Line, const Point2D);


bool MatchPoints(list<Point2D> &LeftCamPoints, list<Point2D> &RightCamPoints, Point2D &LeftPoint, Point2D &RightPoint);
bool MatchPoints(list<Point2D> &LeftCamPoints, list<Point2D> &RightCamPoints, unsigned int NumPoints);
void RemoveFurtherPoint(list<Point2D> &FirstPoints, list<Point2D> &SecondPoints);
list<Point2D> & StretchField(list<Point2D> & Pts, double Width, double Height, Point2D &TopLeft, Point2D &TopRight, Point2D &BottomRight, Point2D &BottomLeft);
bool MatchPointsStretch(list<Point2D> &LeftCamPoints, list<Point2D> &RightCamPoints, unsigned int NumPoints);

#endif
