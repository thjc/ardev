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
//	A heavyweight 3D vector class
//	Written by Geoffrey Biggs for the Celshade ray tracer
// 6:54pm 11/08/2002

#ifndef __VECTOR3D_H
#define __VECTOR3D_H

// Useful mathematical things
#define ONEEIGHTY_OVER_PI	57.29577951308
#ifndef PI_OVER_ONEEIGHTY
#define PI_OVER_ONEEIGHTY	0.017453292520
#endif
#define RADTODEG(angle)		((angle) * ONEEIGHTY_OVER_PI)
#define DEGTORAD(angle)		((angle) * PI_OVER_ONEEIGHTY)

#include <iostream>
using namespace std;

// All angles are radians unless otherwise specified

class Vector3D
{
public:
	Vector3D (void);
	Vector3D (double newX, double newY, double newZ);
	Vector3D (const Vector3D &rhs);
	~Vector3D (void);

	//////////////////
	// Vector Operations
	//////////////////
	Vector3D CrossProduct (const Vector3D &rhs) const;
	double DotProduct (const Vector3D &rhs) const;
	double Length (void) const;
	void Zero (void);
	Vector3D& Normalise (void);
	void Print (void) const;

	//////////////////
	// Moving the vector
	//////////////////
	Vector3D& Translate (double tX, double tY, double tZ);
	Vector3D Scale (double value);
	Vector3D Scale (double sX, double sY, double sZ);
	Vector3D& RotateX (double angle);
	Vector3D& RotateY (double angle);
	Vector3D& RotateZ (double angle);
	Vector3D& RotateX (double angle, double aboutX, double aboutY, double aboutZ);
	Vector3D& RotateY (double angle, double aboutX, double aboutY, double aboutZ);
	Vector3D& RotateZ (double angle, double aboutX, double aboutY, double aboutZ);
	Vector3D& RotateX (double angle, Vector3D about);
	Vector3D& RotateY (double angle, Vector3D about);
	Vector3D& RotateZ (double angle, Vector3D about);

	//////////////////
	// Arithmatic operators
	//////////////////
	Vector3D operator + (double offset) const;
	Vector3D operator + (const Vector3D &rhs) const;
	Vector3D operator - (double offset) const;
	Vector3D operator - (const Vector3D &rhs) const;
	Vector3D operator * (double scale) const;
	Vector3D operator / (double scale) const;
	const Vector3D& operator = (const Vector3D &rhs);
	const Vector3D& operator = (double value);
	void operator += (double offset);
	void operator += (const Vector3D &rhs);
	void operator -= (double offset);
	void operator -= (const Vector3D &rhs);
	void operator *= (double scale);
	void operator /= (double scale);

	//////////////////
	// Logical operators
	//////////////////
	bool operator == (const Vector3D &rhs);
	bool operator != (const Vector3D &rhs);
	
	friend ostream& operator<< (ostream &os, Vector3D &rhs);


	//////////////////
	//	Vector data
	//////////////////
	double x;
	double y;
	double z;
};

ostream& operator<< (ostream &os, Vector3D &rhs);

#endif
