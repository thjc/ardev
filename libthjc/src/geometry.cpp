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
 *	geometry.cpp											*
 *	Contains various useful geometry functions, for example	*
 *	calculating the distance between 2 points.				*
 *															*
 *	Geoffrey Biggs, 2003									*
 *	Toby Collett, 2003										*
 ************************************************************/

#include <libthjc/geometry.h>
#include <libthjc/matrix.h>
#include <libthjc/misc.h>

#include <math.h>

double mod2pi(double ang)
{
	while (ang > 2*PI) ang -= 2*PI;
	while (ang < 0) ang += 2*PI;
	return ang;
}

double modpi(double ang)
{
	while (ang > PI) ang -= 2*PI;
	while (ang < -1 * PI) ang += 2*PI;
	return ang;
}

ostream& operator<< (ostream &os, const Point2D &rhs) 
{
	os << "[" << rhs.x << "; " << rhs.y << "]";
	return os;
}

//	Calculates the distance between two points in 3D space.
//
//	Receives:	The two points to find the distance betwee
//	Returns:	The distance between them
double CalcDistance (Vector3D point1, Vector3D point2)
{
	return sqrt ((pow (point2.x - point1.x, 2) + pow (point2.y - point1.y, 2) + pow (point2.z - point1.z, 2)));
}

//	Calculates the distance between two points in 2D space.
//
//	Receives:	The two points to find the distance betwee
//	Returns:	The distance between them
double CalcDistance (const Point2D &point1, const Point2D &point2)
{
	double dx = point2.x - point1.x;
	double dy = point2.y - point1.y;

	return sqrt (dx*dx + dy*dy);
}

//	Calculates the distance between two points in 2D space.
//
//	Receives:	The coordinates of the two points
//	Returns:	The distance between them
double CalcDistance (double x1, double y1, double x2, double y2)
{
	return sqrt ((pow (x2 - x1, 2) + pow (y2 - y1, 2)));
}

//	Calculates angles of a vector around the three axes, suitable for use
//	in OpenGL.
//
//	Receives:	The vector to calculate the angles of
//				References to the variables to store the angels in
//	Returns:	Nothing
void CalcAngles (Vector3D vector, double &rotX, double &rotY, double &rotZ)
{
	rotX = RADTODEG (atan2 (vector.y, vector.z));
	rotY = RADTODEG (atan2 (vector.x, vector.z));
	rotZ = RADTODEG (atan2 (vector.x, vector.y));
	if (vector.z >= 0.0f)
	{
		rotX = -rotX;
	}
	else if (vector.z < 0.0f && vector.y >= 0.0f)
	{
		rotX -= 180.0f;
	}
	else if (vector.z < 0.0f && vector.y < 0.0f)
	{
		rotX += 180.0f;
	}
}

//	Calculates the distance in the (x,z) plane of two points in
//	3D space (ie the distance along the "ground"). Assumes that
//	the (x,z) plane is the ground, of course.
//
//	Receives:	The two points to find the distance betwee
//	Returns:	The horizontal distance between them
double CalcHorizDist (Vector3D point1, Vector3D point2)
{
	return sqrt ((pow (point2.x - point1.x, 2) + pow (point2.z - point1.z, 2)));
}

//	Calculates the speed something is travelling.
//
//	Receives:	The time interval that the object travelled during
//				The distance travelled during this interval.
//	Returns:	The speed during this time.
double CalcSpeed (int timeDiff, double distance)
{
	if (distance == 0.0f || timeDiff == 0)
		return 0.0f;

	return distance / (double) timeDiff;
}


//	Calculates the mid point of a line between two points.
//
//	Receives:	The two points representing the line.
//	Returns:	A point that is the midpoint of the line between them.
Vector3D CalcMidPoint (Vector3D point1, Vector3D point2)
{
	double midX, midY, midZ;

	midX = (point1.x + point2.x) / (double) 2;
	midY = (point1.y + point2.y) / (double) 2;
	midZ = (point1.z + point2.z) / (double) 2;

	return Vector3D (midX, midY, midZ);
}

Point2D CalcMidPoint (Point2D p1, Point2D p2)
{
	return Point2D((p1.x+p2.x)/2.0, (p1.y + p2.y)/2.0);
}

//	Finds the interesection point of two lines, or if they don't intersect
//	then it finds the point on each line where they are closest and the
//	distance down these lines.
//
//	Receives:	Two points on the left line
//				Two points on the right line
//				A reference to the point to fill with the point of intersection on the left line
//				A reference to the point to fill with the point of intersection on the right line
//				A reference to a double to fill with the distance down the left line
//				A reference to a double to fill with the distance down the right line
//	Returns:	Nothing
void IntersectLines (Vector3D &left1, Vector3D &left2, Vector3D &right1, Vector3D &right2, Vector3D &resultLeft, Vector3D &resultRight, double &distLeft, double &distRight)
{
	double d1343 = (left1 - right1).DotProduct (right2 - right1);
	double d2143 = (left2 - left1).DotProduct (right2 - right1);
	double d1321 = (left1 - right1).DotProduct (left2 - left1);
	double d4343 = (right2 - right1).DotProduct (right2 - right1);
	double d2121 = (left2 - left1).DotProduct (left2 - left1);

	distLeft = ((d2143 * d1343) - (d1321 * d4343)) / ((d2121 * d4343) - (d2143 * d2143));
	distRight = (d1343 + (distLeft * d2143)) / d4343;

	resultLeft = left1 + ((left2 - left1).Scale (distLeft));
	resultRight = right1 + ((right2 - right1).Scale (distRight));
}

//	Converts a list of integers to a list of 2D coordinates.
//	The result list will be half the length of the original list.
//	If the original list is not an even number, the last integer will
//	be ignored.
//
//	Receives:	A list of integers. The length should be an even number
//				A reference to the list to store the result in
//	Returns:	Nothing
void ListIntsToListPoints (list<int> &intsList, list<Point2D> &pointsList)
{
	Point2D newPoint;

	pointsList.clear ();
	for (list<int>::iterator itr = intsList.begin (); itr != intsList.end (); itr++)
	{
		newPoint.x = *itr++;
		newPoint.y = *itr;
		pointsList.push_back (newPoint);
	}
}



//	Inside
//	Receives:	px,py,*dat
//	Returns:	char
bool Inside(int px, int py, int *dat) {
	int s[8]={0,1,1,2,2,3,3,0};	
	int i;
	double x1,x2,y1,y2;
	double tx=0;
	double ty=0;
	bool side;	//true=in, false=out
	double Px,Py;
	Px=(double)px;
	Py=(double)py;
	bool rslt=true;

//	printf("pt=<%f:%f>\n",Px,Py);
	for(i=0;i<8;i+=2) {
		x1=(double)dat[(s[i]*2)+0];
		y1=(double)dat[(s[i]*2)+1];
		x2=(double)dat[(s[i+1]*2)+0];
		y2=(double)dat[(s[i+1]*2)+1];

//		printf("p1=<%f:%f>, p2=<%f:%f>\n",x1,y1,x2,y2);

		if(i==0||i==4) {	//more horizontal
			if( (x2-x1)==0) {
				side=true;
			}
			else
			{
				ty=((y2-y1)/(x2-x1))*(Px-x1)+y1;
				if(i==0) {
					side= (Py<ty)?false:true;
				}
				else {
					side= (Py>ty)?false:true;
				}
			}
//			printf("\tH %d: ty=%f, py=%f\n",i,ty,Py);
		}
		else {	//more vertical
			if( (y2-y1)==0) {
					side=true;
			}
			else
			{
				tx=((x2-x1)/(y2-y1))*(Py-y1)+x1;
				if(i==2) {
					side= (Px<tx)?true:false;
				}
				else {
					side= (Px>tx)?true:false;
				}
//				printf("\tV %d: tx=%f, px=%f\n",i,tx,Px);
			}
		}
//		printf("\tside=%s\n",side?"true":"false");
		if(side==false) {
			rslt=false;
		}


	}

//	printf("%s\n",rslt?"inside":"outside");


	return rslt;
//	return true;
}

// Calculatest he point of intersecton between two rays
Vector3D Ray::Intersect(Ray Ray2)
{
	Vector3D LeftIn, RightIn, LeftP2, RightP2;
	double DistLeft, DistRight;
	LeftP2 = Origin + Direction;
	RightP2 = Ray2.Origin + Ray2.Direction;
	IntersectLines(Origin, LeftP2, Ray2.Origin, RightP2, LeftIn, RightIn, DistLeft, DistRight);
	return CalcMidPoint(LeftIn,RightIn);
}

double Ray::Dist2Point(Vector3D Point)
{
	Matrix P1(Origin);
	Matrix P2(Origin + Direction);
	Matrix Pb(Point);

	double tb = ((Pb - P1).T() * (P2 - P1)).GetValue(0,0) / ((P2 - P1).T() * (P2 - P1)).GetValue(0,0);

	Matrix Pb_pr = P1 + (P2 - P1) * tb;
	
	double dx, dy, dz;
	dx = Pb_pr.GetValue(0,0) - Pb.GetValue(0,0);
	dy = Pb_pr.GetValue(1,0) - Pb.GetValue(1,0);
	dz = Pb_pr.GetValue(2,0) - Pb.GetValue(2,0);

	return sqrt(dx*dx + dy*dy + dz*dz);
}


FieldFilter::FieldFilter (void)
{
	fieldPoints[0] = 0;
	fieldPoints[1] = 0;
	fieldPoints[2] = 1;
	fieldPoints[3] = 0;
	fieldPoints[4] = 1;
	fieldPoints[5] = 1;
	fieldPoints[6] = 0;
	fieldPoints[7] = 1;
}

FieldFilter::FieldFilter (Point2D topLeft, Point2D topRight, Point2D bottomRight, Point2D bottomLeft)
{
	fieldPoints[0] = (int) topLeft.x;
	fieldPoints[1] = (int) topLeft.y;
	fieldPoints[2] = (int) topRight.x;
	fieldPoints[3] = (int) topRight.y;
	fieldPoints[4] = (int) bottomRight.x;
	fieldPoints[5] = (int) bottomRight.y;
	fieldPoints[6] = (int) bottomLeft.x;
	fieldPoints[7] = (int) bottomLeft.y;
}


list<Point2D> FieldFilter::FilterPoints (list<Point2D> points)
{
	int s[8]={0,1,1,2,2,3,3,0};
	int i;
	double x1, x2, y1, y2;
	double tx = 0, ty = 0;
	bool side, result = true;	//true=in, false=out
	list<Point2D> insidePoints;

	for (list<Point2D>::iterator itr = points.begin (); itr != points.end (); itr++)
	{
		result = true;
//		printf("pt=<%f:%f>\n",itr->x,itr->y);
		for (i = 0; i < 8; i += 2) 
		{
			x1 = (double) fieldPoints[(s[i] * 2) + 0];
			y1 = (double) fieldPoints[(s[i] * 2) + 1];
			x2 = (double) fieldPoints[(s[i + 1] * 2) + 0];
			y2 = (double) fieldPoints[(s[i + 1] * 2) + 1];
//			printf("p1=<%f:%f>, p2=<%f:%f>\n",x1,y1,x2,y2);

			if (i == 0 || i == 4)	//more horizontal
			{
				if ((x2 - x1) == 0)
				{
					side = true;
				}
				else
				{
					ty = ((y2 - y1) / (x2 - x1)) * (itr->x - x1) + y1;
					if (i == 0)
					{
						side = (itr->y < ty) ? false : true;
					}
					else
					{
						side = (itr->y > ty) ? false : true;
					}
				}
//			printf("\tH %d: ty=%f, py=%f\n",i,ty,itr->y);
			}
			else	//more vertical
			{
				if ((y2 - y1) == 0)
				{
					side = true;
				}
				else
				{
					tx = ((x2 - x1) / (y2 - y1)) * (itr->y - y1) + x1;
					if (i == 2)
					{
						side = (itr->x < tx) ? true : false;
					}
					else
					{
						side = (itr->x > tx) ? true : false;
					}
				}
//				printf("\tV %d: tx=%f, px=%f\n",i,tx,itr->x);
			}
//			printf("\tside=%s\n",side?"true":"false");
			if (side == false)
				result = false;
		}

		if (result)
		{
//			printf ("Adding point %f %f\n", itr->x, itr->y);
			insidePoints.push_back (*itr);
		}
	}

	return insidePoints;
}

void FieldFilter::SetPoints (Point2D topLeft, Point2D topRight, Point2D bottomRight, Point2D bottomLeft)
{
	fieldPoints[0] = (int) topLeft.x;
	fieldPoints[1] = (int) topLeft.y;
	fieldPoints[2] = (int) topRight.x;
	fieldPoints[3] = (int) topRight.y;
	fieldPoints[4] = (int) bottomRight.x;
	fieldPoints[5] = (int) bottomRight.y;
	fieldPoints[6] = (int) bottomLeft.x;
	fieldPoints[7] = (int) bottomLeft.y;
}

void FieldFilter::SetPoints (int *newPoints)
{
	fieldPoints[0] = newPoints[0];
	fieldPoints[1] = newPoints[1];
	fieldPoints[2] = newPoints[2];
	fieldPoints[3] = newPoints[3];
	fieldPoints[4] = newPoints[4];
	fieldPoints[5] = newPoints[5];
	fieldPoints[6] = newPoints[6];
	fieldPoints[7] = newPoints[7];
}



// this function matches one point from the list of left and right points and returns them as the left and right points passed into it, it also removes the points fomr the list
bool MatchPoints(list<Point2D> &LeftCamPoints, list<Point2D> &RightCamPoints, Point2D &LeftPoint, Point2D &RightPoint)
{
	if (LeftCamPoints.size() != RightCamPoints.size())
		throw "Not Matching Number of Points";

	list<Point2D>::iterator BestLeft, BestRight;
			
	list<Point2D>::iterator *LeftPoints, *RightPoints;
	LeftPoints = new list<Point2D>::iterator[LeftCamPoints.size()];
	RightPoints = new list<Point2D>::iterator[RightCamPoints.size()];
	
	unsigned int NumPoints = static_cast<unsigned int> (LeftCamPoints.size());
	
	if (LeftPoints == NULL || RightPoints == NULL)
		throw "Bad Mem Allocation";
	
	{
		unsigned int i = 0;
		list<Point2D>::iterator itr_l = LeftCamPoints.begin();
		list<Point2D>::iterator itr_r = RightCamPoints.begin();
		for (; itr_l != LeftCamPoints.end(); ++itr_l, ++itr_r, ++i)
		{
			LeftPoints[i] = itr_l;
			
			RightPoints[i] = itr_r;
		}
	}

	//cout << "built arrays" << endl;

	// work out if left to right or top to bottom is better to use
	Matrix Dists(0,2);
	for (unsigned int i = 0; i < NumPoints-1; ++i)
	{
		for (unsigned int j = i; j < NumPoints; ++j)
		{
			Matrix Temp(1,2);
			Temp.SetValue(0,0,fabs(LeftPoints[i]->x - LeftPoints[j]->x));
			Temp.SetValue(0,1,fabs(LeftPoints[i]->y - LeftPoints[j]->y));
			Dists.InsertRows(0,Temp);
		}
	}

	//cout << "worked out left or top" << endl;

	BestLeft = LeftPoints[0];
	BestRight = RightPoints[0];

	if (Dists.GetCol(0).Avg() > Dists.GetCol(1).Avg())
	{
		// x dist is better so find left most point from both arrays
		// Left First
		for (unsigned int i = 0; i < NumPoints; ++i)
		{
			if (BestLeft->x > LeftPoints[i]->x)
			{
				BestLeft = LeftPoints[i];
			}
		}

		// Right First
		for (unsigned int i = 0; i < NumPoints; ++i)
		{
			if (BestRight->x > RightPoints[i]->x)
			{
				BestRight = RightPoints[i];
			}
		}
	}
	else
	{
		// y dist is better so find top most point from both arrays
		// Left First
		for (unsigned int i = 0; i < NumPoints; ++i)
		{
			if (BestLeft->y > LeftPoints[i]->y)
			{
				BestLeft = LeftPoints[i];
			}
		}

		// Right First
		for (unsigned int i = 0; i < NumPoints; ++i)
		{
			if (BestRight->y > RightPoints[i]->y)
			{
				BestRight = RightPoints[i];
			}
		}
	}
	
	LeftPoint = *BestLeft;
	RightPoint = *BestRight;
	
	LeftCamPoints.erase(BestLeft);
	RightCamPoints.erase(BestRight);

	delete[] LeftPoints;
	delete[] RightPoints;
	return true;		
}

class TempPointPair
{
public:
	list<Point2D>::iterator First;
	list<Point2D>::iterator Second;
	double distance;
	
	bool operator < (const TempPointPair & rhs) const {return distance < rhs.distance;};
};

#define MATCH_THRESH 500
bool MatchPointsStretch(list<Point2D> &LeftCamPoints, list<Point2D> &RightCamPoints, unsigned int NumPoints)
{
	list<Point2D> RetL;
	list<Point2D> RetR;
	
	list<TempPointPair> Temp;
	
	if (LeftCamPoints.size() < NumPoints || RightCamPoints.size() < NumPoints)
		return false;

	for (unsigned int i = 0; i< NumPoints;++i)
	{
		Temp.clear();
		for (list<Point2D>::iterator itrL = LeftCamPoints.begin(); itrL != LeftCamPoints.end();++itrL)
		{
			TempPointPair TempPair;
			TempPair.First = itrL;
			TempPair.Second = RightCamPoints.begin();
			TempPair.distance = CalcDistance(*TempPair.First, *TempPair.Second);
			
			for (list<Point2D>::iterator itrR = ++RightCamPoints.begin(); itrR != RightCamPoints.end();++itrR)
			{
				double Temp = CalcDistance(*itrL, *itrR);
				if (Temp < TempPair.distance)
				{
					TempPair.distance = Temp;
					TempPair.Second = itrR;
				}	
			}
			Temp.push_back(TempPair);
		}
		Temp.sort();
		if (Temp.front().distance < MATCH_THRESH)
		{
			RetL.push_back(*(Temp.front().First));
			RetR.push_back(*(Temp.front().Second));
		}
		else
		{
			return false;
		}
		
		LeftCamPoints.erase(Temp.front().First);
		RightCamPoints.erase(Temp.front().Second);
	}
	LeftCamPoints.clear();
	RightCamPoints.clear();
	
	LeftCamPoints = RetL;
	RightCamPoints = RetR;
	return true;
}

list<Point2D> & StretchField(list<Point2D> & Pts, double Width, double Height, Point2D &BottomLeft, Point2D &BottomRight, Point2D &TopRight, Point2D &TopLeft)
{
	if (BottomLeft == BottomRight && BottomLeft == TopRight && BottomLeft == TopLeft)
		return Pts;

	// Sort Bottom Points
	// First get bottom line
	double m = (BottomRight.y - BottomLeft.y)/(BottomRight.x - BottomLeft.x);
	double c = BottomLeft.y - m*BottomLeft.x;
	
	for (list<Point2D>::iterator itr = Pts.begin(); itr != Pts.end(); ++itr)
	{
		itr->y = itr->y - (Height - itr->y) * (m*itr->x + c)/(Height - (m*itr->x + c));
	}
	TopRight.y = TopRight.y - (Height - TopRight.y) * (m*TopRight.x + c)/(Height - (m*TopRight.x + c));
	BottomRight.y = BottomRight.y - (Height - BottomRight.y) * (m*BottomRight.x + c)/(Height - (m*BottomRight.x + c));
	BottomLeft.y = BottomLeft.y - (Height - BottomLeft.y) * (m*BottomLeft.x + c)/(Height - (m*BottomLeft.x + c));
	TopLeft.y = TopLeft.y - (Height - TopLeft.y) * (m*TopLeft.x + c)/(Height - (m*TopLeft.x + c));

	// Sort Top Points
	// First get Top line
	m = (TopRight.y - TopLeft.y)/(TopRight.x - TopLeft.x);
	c = TopLeft.y - m*TopLeft.x;


	for (list<Point2D>::iterator itr = Pts.begin(); itr != Pts.end(); ++itr)
	{
		itr->y = itr->y + (itr->y) * (Height - (m*itr->x + c))/(m*itr->x + c);
	}
	TopRight.y = TopRight.y + (TopRight.y) * (Height - (m*TopRight.x + c))/(m*TopRight.x + c);
	BottomRight.y = BottomRight.y + (BottomRight.y) * (Height - (m*BottomRight.x + c))/(m*BottomRight.x + c);
	BottomLeft.y = BottomLeft.y + (BottomLeft.y) * (Height - (m*BottomLeft.x + c))/(m*BottomLeft.x + c);
	TopLeft.y = TopLeft.y + (TopLeft.y) * (Height - (m*TopLeft.x + c))/(m*TopLeft.x + c);
	
	// Sort Left Points
	// First get bottom line
	m = (BottomLeft.y - TopLeft.y)/(BottomLeft.x - TopLeft.x);
	c = BottomLeft.y - m*BottomLeft.x;
	
	for (list<Point2D>::iterator itr = Pts.begin(); itr != Pts.end(); ++itr)
	{
		itr->x = itr->x - (Width - itr->x) * ((itr->y - c)/m)/(Width - ((itr->y - c)/m));
	}
	TopRight.x = TopRight.x - (Width - TopRight.x) * ((TopRight.y - c)/m)/(Width - ((TopRight.y - c)/m));
	BottomRight.x = BottomRight.x - (Width - BottomRight.x) * ((BottomRight.y - c)/m)/(Width - ((BottomRight.y - c)/m));
	BottomLeft.x = BottomLeft.x - (Width - BottomLeft.x) * ((BottomLeft.y - c)/m)/(Width - ((BottomLeft.y - c)/m));
	TopLeft.x = TopLeft.x - (Width - TopLeft.x) * ((TopLeft.y - c)/m)/(Width - ((TopLeft.y - c)/m));


	// Sort Right Points
	// First get Top line
	m = (BottomRight.y - TopRight.y)/(BottomRight.x - TopRight.x);
	c = TopRight.y - m*TopRight.x;


	for (list<Point2D>::iterator itr = Pts.begin(); itr != Pts.end(); ++itr)
	{
		itr->x = itr->x + (itr->x) * (Width - ((itr->y - c)/m))/((itr->y - c)/m);
	}
	TopRight.x = TopRight.x + (TopRight.x) * (Width - ((TopRight.y - c)/m))/((TopRight.y - c)/m);
	BottomRight.x = BottomRight.x + (BottomRight.x) * (Width - ((BottomRight.y - c)/m))/((BottomRight.y - c)/m);
	BottomLeft.x = BottomLeft.x + (BottomLeft.x) * (Width - ((BottomLeft.y - c)/m))/((BottomLeft.y - c)/m);
	TopLeft.x = TopLeft.x + (TopLeft.x) * (Width - ((TopLeft.y - c)/m))/((TopLeft.y - c)/m);

	return Pts;
}



bool MatchPoints(list<Point2D> &LeftCamPoints, list<Point2D> &RightCamPoints, unsigned int NumPoints)
{
	if (LeftCamPoints.size() < NumPoints || RightCamPoints.size() < NumPoints)
		return false;
		
	while(LeftCamPoints.size() > NumPoints)
		RemoveFurtherPoint(LeftCamPoints, RightCamPoints);
		
	while(RightCamPoints.size() > NumPoints)
		RemoveFurtherPoint(RightCamPoints, LeftCamPoints);
		
	list<Point2D> TempLeftList, TempRightList;
	while(LeftCamPoints.size() > 0)
	{
		Point2D TempLeft, TempRight;
		MatchPoints(LeftCamPoints, RightCamPoints, TempLeft, TempRight);
		TempLeftList.push_back(TempLeft);
		TempRightList.push_back(TempRight);
	}
	
	while(TempLeftList.size() > 0)
	{
		LeftCamPoints.push_back(*(TempLeftList.begin()));
		TempLeftList.erase(TempLeftList.begin());
		
		RightCamPoints.push_back(*(TempRightList.begin()));
		TempRightList.erase(TempRightList.begin());
	}
	
	return true;
}

void RemoveFurtherPoint(list<Point2D> &FirstPoints, list<Point2D> &SecondPoints)
{
	Matrix Dists(FirstPoints.size(),1);
	unsigned int i = 0;
	// find point from first list that is furthest from all the points inthe second list
	for (list<Point2D>::iterator itr1 = FirstPoints.begin(); itr1 != FirstPoints.end(); ++itr1, ++i)
	{
		double dist = 99999;
		for (list<Point2D>::iterator itr2 = SecondPoints.begin(); itr2 != SecondPoints.end(); ++itr2)
		{
			double TempDist = CalcDistance(*itr1, *itr2);
			dist = TempDist < dist ? TempDist : dist;
		}
		Dists.SetValue(i,0,dist);
	}
	unsigned int BadPoint = Dists.MaxIndex();
	
	i = 0;
	// find point from first list that is furthest from all the points inthe second list
	for (list<Point2D>::iterator itr1 = FirstPoints.begin(); itr1 != FirstPoints.end(); ++itr1, ++i)
	{
		if (i == BadPoint)
		{
			FirstPoints.erase(itr1);
			break;
		}
	}
	
}



Point2D Line::ClosestPoint(const Point2D &Pb) const
{
	//matrix free for speed
	double dx21 = P2.x - P1.x;
	double dy21 = P2.y - P1.y;

	double tb = ((Pb.x - P1.x)*dx21 + (Pb.y - P1.y)*dy21)/(dx21*dx21+dy21*dy21);
	tb = tb < 0 ? 0 : (tb > 1 ? 1 : tb);

	return Point2D(P1.x + dx21*tb, P1.y + dy21*tb);
}

double Line::DistToPoint(const Point2D &Pb) const
{
	Point2D Pb_pr = ClosestPoint(Pb);
	double dx, dy;
	dx = Pb.x - Pb_pr.x;
	dy = Pb.y - Pb_pr.y;

	return sqrt(dx*dx + dy*dy);	
}


/****************************************
 * Various Intersection Functions       *
 ****************************************/

vector<Point2D> Intersections;

// returns at most 2 intersecitons if they exist
vector<Point2D> & Intersection(const Circle &C1, const Circle &C2)
{
	Intersections.clear();
	vector<Point2D> &Ret = Intersections;
	double d; // distance between circles;	
	d = CalcDistance(C1.Origin,C2.Origin);
	if (d > (C1.R + C2.R + 0.01))
		return Ret;

	double x = (d*d - C2.R*C2.R + C1.R*C1.R)/(2*d);
	//double y = sqrt(C1.R*C1.R - x*x);
	double Temp = C1.R*C1.R - x*x;
	double y = sqrt(Temp < 0 ? 0 : Temp);
	double y2 = -y;
    
	double Theta = atan2(C2.Origin.y-C1.Origin.y,C2.Origin.x - C1.Origin.x);

	Ret.push_back(Point2D(cos(Theta)*x - sin(Theta)*y + C1.Origin.x,sin(Theta)*x + cos(Theta)*y + C1.Origin.y));
	if (fabs(d - (C1.R + C2.R)) < 0.01)
		return Ret;

	Ret.push_back(Point2D(cos(Theta)*x - sin(Theta)*y2 + C1.Origin.x,sin(Theta)*x + cos(Theta)*y2 + C1.Origin.y));
	return Ret;
}




// returns at most 2 intersecitons if they exist
vector<Point2D> & Intersection(const Circle &C, const CircleSeg &A)
{
	// first get the interesctions of the equiv two circles
	vector<Point2D> &Ret = Intersection(C,Circle(A.Origin, A.R));
	if (Ret.empty())
		return Ret;

	// now check to make sure the exist on the arc
	double Ang = mod2pi(atan2(Ret.front().y-A.Origin.y , Ret.front().x - A.Origin.x));
	if ((Ang < A.GetTheta1() || Ang > A.GetTheta2()) && A.GetTheta1() < A.GetTheta2())
		Ret.erase(Ret.begin());
	else if ((Ang < A.GetTheta1() && Ang > A.GetTheta2()) && A.GetTheta1() > A.GetTheta2())
		Ret.erase(Ret.begin());

	if (Ret.empty())
		return Ret;

	Ang = mod2pi(atan2(Ret.back().y-A.Origin.y , Ret.back().x - A.Origin.x));
	if ((Ang < A.GetTheta1() || Ang > A.GetTheta2()) && A.GetTheta1() < A.GetTheta2())
		Ret.pop_back();
	else if ((Ang < A.GetTheta1() && Ang > A.GetTheta2()) && A.GetTheta1() > A.GetTheta2())
		Ret.pop_back();

	return Ret;
}


vector<Point2D> & Intersection(const Circle &C, const Line &L)
{
	Intersections.clear();
	vector<Point2D> &Ret = Intersections;

	const Point2D &Origin = C.Origin;
	Point2D P1 = L.P1 - Origin;
	Point2D P2 = L.P2 - Origin;

	double dx = P2.x - P1.x;
	double dy = P2.y - P1.y;
	double dr = sqrt(dx*dx + dy*dy);
	double D = P1.x*P2.y - P2.x*P1.y;

	double Incidence = (C.R*C.R*dr*dr - D*D);
    if (Incidence < 0)
		return Ret;
	else if (Incidence == 0)
	{
		double x = D*dy/(dr*dr);
		double y = -1*D*dx/(dr*dr);

		if (x < P2.x && x > P1.x && y < P2.y && y > P1.y)
			Ret.push_back(Point2D(x,y) + Origin);
	}
	else
	{
		double x1,y1,x2,y2;
		if (dy < 0)
		{
			x1 = (D*dy - dx*sqrt(Incidence))/(dr*dr);
			x2 = (D*dy + dx*sqrt(Incidence))/(dr*dr);
		}
		else
		{
			x1 = (D*dy + dx*sqrt(Incidence))/(dr*dr);
			x2 = (D*dy - dx*sqrt(Incidence))/(dr*dr);
		}
		y1 = (-1*D*dx + fabs(dy)*sqrt(Incidence))/(dr*dr);
		y2 = (-1*D*dx - fabs(dy)*sqrt(Incidence))/(dr*dr);

		bool OLX1, OLX2, OLY1, OLY2;
		if (P1.x < P2.x)
		{
			OLX1 = (x1 + 0.001) > P1.x && (x1 - 0.001) < P2.x;
			OLX2 = (x2 + 0.001) > P1.x && (x2 - 0.001) < P2.x;
		}
		else
		{
			OLX1 = (x1 - 0.001) < P1.x && (x1 + 0.001) > P2.x;
			OLX2 = (x2 - 0.001) < P1.x && (x2 + 0.001) > P2.x;
		}

		if (P1.y < P2.y)
		{
			OLY1 = (y1 + 0.001) > P1.y && (y1 - 0.001) < P2.y;
			OLY2 = (y2 + 0.001) > P1.y && (y2 - 0.001) < P2.y;
		}
		else
		{
			OLY1 = (y1 - 0.001) < P1.y && (y1 + 0.001) > P2.y;
			OLY2 = (y2 - 0.001) < P1.y && (y2 + 0.001) > P2.y;
		}

		if (OLX1 && OLY1)
			Ret.push_back(Point2D(x1,y1) + Origin);
//		if (x2 < P2.x && x2 > P1.x && y2 < P2.y && y2 > P1.y)
		if (OLX2 && OLY2)
			Ret.push_back(Point2D(x2,y2) + Origin);
	}
	return Ret;
}

vector<Point2D> & Intersection(const CircleSeg &A, const Line &L)
{
	// first get the interesctions of the equiv circle and lin
	vector<Point2D> &Ret = Intersection(Circle(A.Origin, A.R),L);
	if (Ret.empty())
		return Ret;
	// now check to make sure the exist on the arc
	double Ang = mod2pi(atan2(Ret.front().y-A.Origin.y , Ret.front().x - A.Origin.x));
	if ((Ang < A.GetTheta1() || Ang > A.GetTheta2()) && A.GetTheta1() < A.GetTheta2())
		Ret.erase(Ret.begin());
	else if ((Ang < A.GetTheta1() && Ang > A.GetTheta2()) && A.GetTheta1() > A.GetTheta2())
		Ret.erase(Ret.begin());

	if (Ret.empty())
		return Ret;

	Ang = mod2pi(atan2(Ret.back().y-A.Origin.y , Ret.back().x - A.Origin.x));
	if ((Ang < A.GetTheta1() || Ang > A.GetTheta2()) && A.GetTheta1() < A.GetTheta2())
		Ret.pop_back();
	else if ((Ang < A.GetTheta1() && Ang > A.GetTheta2()) && A.GetTheta1() > A.GetTheta2())
		Ret.pop_back();

	return Ret;
}

vector<Point2D> & Intersection(const Line &L1, const Line &L2, bool Segment)
{

	Intersections.clear();
	vector<Point2D> &Ret = Intersections;
	// if lines are paralell no intersections
	if (fabs((L1.P2 - L1.P1).Ang() - (L2.P2 - L2.P1).Ang()) < 0.001 || fabs((L1.P1 - L1.P2).Ang() - (L2.P2 - L2.P1).Ang()) < 0.001)
		return Ret;
	Vector3D L1P1(L1.P1.x, L1.P1.y,0), L1P2(L1.P2.x,L1.P2.y,0), L2P1(L2.P1.x,L2.P1.y,0), L2P2(L2.P2.x,L2.P2.y,0);
	Vector3D Temp1, Temp2;
	double D1, D2;
	IntersectLines(L1P1, L1P2, L2P1, L2P2, Temp1, Temp2, D1, D2);
	if (Segment && (D1 < 0.000001 || D1 > 0.999999 || D2 < 0.000001 || D2 > 0.999999 || (fabs(Temp1.x - Temp2.x) > 0.001 &&  fabs(Temp1.y - Temp2.y) > 0.001)))
		return Ret;
	
	Ret.push_back(Point2D(Temp1.x,Temp1.y));
	return Ret;
}


Point2D Point2D::Rotate(Point2D Origin, double Theta)
{
	Point2D Temp(*this - Origin);
	double d = sqrt(Temp.x*Temp.x + Temp.y*Temp.y);
	double Ang = Temp.Ang() - Theta;
	Temp.x = d*cos(Ang);
	Temp.y = d*sin(Ang);
	return Temp + Origin;
}

Ray2D Ray2D::operator - (const Ray2D & rhs) const
{
	Ray2D Ret;
	Ret.Origin = Origin + rhs.Origin;
	double ang = Direction.Ang() - rhs.Direction.Ang();
	Ret.Direction = Point2D(cos(ang),sin(ang));
	return Ret;
}

Ray2D Ray2D::operator + (const Ray2D & rhs) const
{
	Ray2D Ret;
	Ret.Origin = Origin + rhs.Origin;
	double ang = Direction.Ang() + rhs.Direction.Ang();
	Ret.Direction = Point2D(cos(ang),sin(ang));
	return Ret;
}


CircleSeg::CircleSeg(Line L, double Radius, bool Left)
{
	R = Radius; 
	vector<Point2D> Pts = Intersection(Circle(L.P1,Radius),Circle(L.P2,Radius));
	
	double Theta = atan2(L.P2.y - L.P1.y, L.P2.x - L.P1.x);
	Point2D O1 = Pts.front();
	Point2D O2 = Pts.back();

	/*Point2D t1 = O1.Rotate(L.P1,Theta);
	Point2D t2 = O2.Rotate(L.P1,Theta);*/


	//cout << "rot " << O1.Rotate(L.P1,-Theta) << O2.Rotate(L.P1,-Theta) << endl;
	if (O1.Rotate(L.P1,Theta).y > O2.Rotate(L.P1,Theta).y)
	{
		// swap them
		Point2D Temp = O1;
		O1 = O2;
		O2 = Temp;
	}
	if (!Left)
	{
		Origin = O1;
		Theta1 = atan2(L.P2.y - Origin.y, L.P2.x - Origin.x);
		Theta2 = atan2(L.P1.y - Origin.y, L.P1.x - Origin.x);	
	}
	else
	{
		Origin = O2;
		Theta1 = atan2(L.P1.y - Origin.y, L.P1.x - Origin.x);
		Theta2 = atan2(L.P2.y - Origin.y, L.P2.x - Origin.x);	
	}
	Theta1 = mod2pi(Theta1);
	Theta2 = mod2pi(Theta2);

}


double CircleSeg::DistToPoint(const Point2D &Pb) const
{
	return CalcDistance(Pb, ClosestPoint(Pb));
}

Point2D CircleSeg::ClosestPoint(const Point2D &Pb) const
{
	Point2D OtherEnd;
	OtherEnd = (Pb - Origin).Norm()*2*R + Origin;

	vector<Point2D> Closest = Intersection(Circle(Origin, R), Line(Origin, OtherEnd));

	Point2D & Temp = Closest.front();
	double Ang = (Temp-Origin).Ang();
	if (Theta1 <= mod2pi(Ang) && mod2pi(Ang) <= Theta2)
		return Temp;

	Point2D P1 = Point2D(R*cos(Theta1),R*sin(Theta1)) + Origin;
	Point2D P2 = Point2D(R*cos(Theta2),R*sin(Theta2)) + Origin;
	return CalcDistance(Temp,P1) < CalcDistance(Temp,P2) ? P1 : P2;
}

double CircleSeg::GetLength() const
{
	return R * fabs(Theta1 - Theta2);
}


double Circle::DistToPoint(const Point2D &Pb) const
{
	return CalcDistance(Pb, ClosestPoint(Pb));
}

Point2D Circle::ClosestPoint(const Point2D &Pb) const
{
	return Intersection(*this, Line(Origin, Pb)).front();
}

//http://astronomy.swin.edu.au/~pbourke/geometry/insidepoly/
int pnpoly(int npol, float *xp, float *yp, float x, float y)
{
    int i, j, c = 0;
    for (i = 0, j = npol-1; i < npol; j = i++) {
    if ((((yp[i] <= y) && (y < yp[j])) ||
            ((yp[j] <= y) && (y < yp[i]))) &&
        (x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i]))
        c = !c;
    }
    return c;
}

// -----------
// Sphere Code
// -----------

Sphere::Sphere(const Vector3D& _Origin, double _Radius)
{
	Origin = _Origin;
	Radius = _Radius;
}

vector<Vector3D> Sphere::Intersect(const Ray & R)
{
	// Vector S is Ray Origin - Sphere origin
	Vector3D S = R.Origin - Origin;
	
	double SdotD = S.DotProduct(R.Direction);
	double SdotS = S.DotProduct(S);
	double DdotD = R.Direction.DotProduct(R.Direction);

	double Det=(Radius*Radius-SdotS)/DdotD + pow((SdotD/DdotD),2);
	double L1 = - SdotD/DdotD + sqrt(Det);
	double L2 = - SdotD/DdotD - sqrt(Det);
	
	vector<Vector3D> Intersections;
	if (Det > 0)
	{
		// two intersection points
		Intersections.push_back(R.Origin + R.Direction * L1);
		Intersections.push_back(R.Origin + R.Direction * L2);
	}
	else if (Det == 0)
	{
		// line is tangent to sphere only one intersection
		Intersections.push_back(R.Origin + R.Direction * L1);
	
	}
	return Intersections;
}

// -----------
// Plane Code
// -----------

Plane::Plane(const Ray & _Normal)
{
	Normal = _Normal;
}

vector<Vector3D> Plane::Intersect(const Ray & R)
{
	vector<Vector3D> Intersections;
	double VdotN = Normal.Direction.DotProduct(R.Direction);
	
	if (VdotN == 0)
		return Intersections;
		
/*	double t = -(R.Origin.DotProduct(Normal.Direction) +
		Normal.Origin.Length()) / VdotN;*/
		
	double t = Normal.Direction.DotProduct(Normal.Origin - R.Origin)/VdotN;
		
	if (t < 0)
		return Intersections;
		
	Intersections.push_back(R.Origin + R.Direction * t);
	return Intersections;
}

bool Plane::InPlane(const Vector3D & Point)
{
	return fabs(Normal.Direction.DotProduct(Point) + Normal.Origin.Length()) < 0.001;
}

bool Plane::BoundCircle(const Vector3D & Point, double Radius)
{
	if (!InPlane(Point))
		return false;

	return (Point - Normal.Origin).Length() <= Radius;
}

bool Plane::BoundCircleSegment(const Vector3D & Point, const Vector3D & Start, double Theta)
{
	if (!BoundCircle(Point, Start.Length()))
		return false;

	double Ang = acos(Point.DotProduct(Start)/(Point.Length()*Start.Length()));
	return fabs(Ang) < Theta;
}
