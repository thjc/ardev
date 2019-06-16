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
//	6:55pm 11/08/2002

#include <libthjc/vector3d.h>

#include <math.h>
#include <stdio.h>

Vector3D::Vector3D (void)
{
	x = y = z = 0;
}

Vector3D::Vector3D (double newX, double newY, double newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}

Vector3D::Vector3D (const Vector3D &rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
}

Vector3D::~Vector3D (void)
{
}

Vector3D Vector3D::CrossProduct (const Vector3D &rhs) const
{
	double resultX, resultY, resultZ;

	resultX = (y * rhs.z) - (z * rhs.y);
	resultY = (z * rhs.x) - (x * rhs.z);
	resultZ = (x * rhs.y) - (y * rhs.x);

	return Vector3D (resultX, resultY, resultZ);
}

double Vector3D::DotProduct (const Vector3D &rhs) const
{
	return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
}

double Vector3D::Length (void) const
{
	return (double) sqrt ((x * x) + (y * y) + (z * z));
}

void Vector3D::Zero (void)
{
	x = y = z = 0.0f;
}

Vector3D& Vector3D::Normalise (void)
{
	double length = this->Length ();

	x /= length;
	y /= length;
	z /= length;

	return *this;
}

Vector3D& Vector3D::Translate (double tX, double tY, double tZ)
{
	x += tX;
	y += tY;
	z += tZ;
	return *this;
}

Vector3D Vector3D::Scale (double value)
{
	Vector3D result;
	result.x = x * value;
	result.y = y * value;
	result.z = z * value;
	return result;
}

Vector3D Vector3D::Scale (double sX, double sY, double sZ)
{
	Vector3D result;
	result.x = x * sX;
	result.y = y * sY;
	result.z = z * sZ;
	return result;
}

Vector3D& Vector3D::RotateX (double angle)
{
	double newY, newZ;
	newY = y*cos(angle) - z*sin(angle);
	newZ = y*sin(angle) + z*cos(angle);
	y = newY;
	z = newZ;
	return *this;
}

Vector3D& Vector3D::RotateY (double angle)
{
	double newX, newZ;
	newX = x*cos(angle) + z*sin(angle);
	newZ = -x*sin(angle) + z*cos(angle);
	x = newX;
	z = newZ;
	return *this;
}

Vector3D& Vector3D::RotateZ (double angle)
{
	double newX, newY;
	newX = x*cos(angle) - y*sin(angle);
	newY = x*sin(angle) + y*cos(angle);
	x = newX;
	y = newY;
	return *this;
}

Vector3D& Vector3D::RotateX (double angle, double aboutX, double aboutY, double aboutZ)
{
	this->Translate (-aboutX, -aboutY, -aboutZ);
	this->RotateX (angle);
	this->Translate (aboutX, aboutY, aboutZ);
	return *this;
}

Vector3D& Vector3D::RotateY (double angle, double aboutX, double aboutY, double aboutZ)
{
	this->Translate (-aboutX, -aboutY, -aboutZ);
	this->RotateY (angle);
	this->Translate (aboutX, aboutY, aboutZ);
	return *this;
}

Vector3D& Vector3D::RotateZ (double angle, double aboutX, double aboutY, double aboutZ)
{
	this->Translate (-aboutX, -aboutY, -aboutZ);
	this->RotateZ (angle);
	this->Translate (aboutX, aboutY, aboutZ);
	return *this;
}

Vector3D& Vector3D::RotateX (double angle, Vector3D about)
{
	this->Translate (-about.x, -about.y, -about.z);
	this->RotateX (angle);
	this->Translate (about.x, about.y, about.z);
	return *this;
}

Vector3D& Vector3D::RotateY (double angle, Vector3D about)
{
	this->Translate (-about.x, -about.y, -about.z);
	this->RotateY (angle);
	this->Translate (about.x, about.y, about.z);
	return *this;
}

Vector3D& Vector3D::RotateZ (double angle, Vector3D about)
{
	this->Translate (-about.x, -about.y, -about.z);
	this->RotateZ (angle);
	this->Translate (about.x, about.y, about.z);
	return *this;
}

Vector3D Vector3D::operator + (double offset) const
{
	return Vector3D (x + offset, y + offset, z + offset);
}

Vector3D Vector3D::operator + (const Vector3D &rhs) const
{
	return Vector3D (x + rhs.x, y + rhs.y, z + rhs.z);
}

Vector3D Vector3D::operator - (double offset) const
{
	return Vector3D (x - offset, y - offset, z - offset);
}

Vector3D Vector3D::operator - (const Vector3D &rhs) const
{
	return Vector3D (x - rhs.x, y - rhs.y, z - rhs.z);
}

Vector3D Vector3D::operator * (double scale) const
{
	return Vector3D (x * scale, y * scale, z * scale);
}

Vector3D Vector3D::operator / (double scale) const
{
	return Vector3D (x / scale, y / scale, z / scale);
}

const Vector3D& Vector3D::operator = (const Vector3D &rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;

	return *this;
}

const Vector3D& Vector3D::operator = (double value)
{
	x = y = z = value;
	
	return *this;
}

void Vector3D::operator += (double offset)
{
	x += offset;
	y += offset;
	z += offset;
}

void Vector3D::operator += (const Vector3D &rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
}

void Vector3D::operator -= (double offset)
{
	x -= offset;
	y -= offset;
	z -= offset;
}

void Vector3D::operator -= (const Vector3D &rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
}

void Vector3D::operator *= (double scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
}

void Vector3D::operator /= (double scale)
{
	x /= scale;
	y /= scale;
	z /= scale;
}

bool Vector3D::operator == (const Vector3D &rhs)
{
	if (x == rhs.x &&
		y == rhs.y &&
		z == rhs.z)
		return true;

	return false;
}

bool Vector3D::operator != (const Vector3D &rhs)
{
	if (x == rhs.x &&
		y == rhs.y &&
		z == rhs.z)
		return false;

	return true;
}

void Vector3D::Print(void) const
{
	printf("(%f, %f, %f)",x,y,z);
}

ostream& operator<< (ostream &os, Vector3D &rhs)
{
	os << "[" << rhs.x << "; " << rhs.y << "; " << rhs.z << "]";
	return os;
}
