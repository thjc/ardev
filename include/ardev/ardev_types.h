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

#ifndef _ARDEV_TYPES_H_
#define _ARDEV_TYPES_H_

#include <string.h>
#include <stdio.h>

#include <math.h>
#include <list>
#include <GL/gl.h>

#include <libthjc/geometry.h>
#include <libthjc/matrix.h>
#include <libthjc/misc.h>

using namespace std;


// radian <-> degree conversions
#ifndef PI_OVER_ONEEIGHTY
#define PI_OVER_ONEEIGHTY (M_PI/180.0)
#endif

#ifndef ONEEIGHTY_OVER_PI
#define ONEEIGHTY_OVER_PI (180.0/M_PI)
#endif

#ifndef RTOD
#define RTOD(x) (x*ONEEIGHTY_OVER_PI)
#endif

#ifndef DTOR
#define DTOR(x) (x*PI_OVER_ONEEIGHTY)
#endif



/** \brief
 * A basic 3d Point
 *
 * Describes a point in 3d space as 3 double precision values (X,Y,Z)
*/
class ARPoint
{
public:
	/// \brief Create an ARPoint with given x,y,z values
	ARPoint(double _x, double _y, double _z) {x = _x; y=_y; z=_z;};
	/// \brief Create an ARPoint object at 0,0,0
	ARPoint() {x=y=z=0.0;};
	ARPoint & RotateX(double theta); ///< \brief rotate point by theta radians about origin
	ARPoint & RotateY(double theta); ///< rotate point by theta radians about origin
	ARPoint & RotateZ(double theta); ///< rotate point by theta radians about origin
	ARPoint & RotateYPR(double Y, double P, double R); ///< rotate yaw, pitch and then roll in radians
	ARPoint & RotateRPY(double R, double P, double Y); ///< rotate roll, potch then yaw in radians

	double Mag() {return sqrt(x*x + y*y + z*z);}; ///< magnitude of the vector

	/// Add two points
	ARPoint operator + (const ARPoint & rhs) {return ARPoint(x+rhs.x, y+rhs.y, z+rhs.z);};
	/// Subtract two points
	ARPoint operator - (const ARPoint & rhs) const {return ARPoint(x-rhs.x, y-rhs.y, z-rhs.z);};
	/// Add rhs to this point
	ARPoint & operator += (const ARPoint & rhs) {x+=rhs.x; y+=rhs.y; z+=rhs.z; return *this;};
	/// scalar multiplcation
	ARPoint & operator *= (const double & rhs) {x*=rhs; y*=rhs; z*=rhs; return * this;};
	/// scalar multiplcation
	ARPoint operator * (const double & rhs) {return ARPoint(x*rhs,y*rhs,z*rhs);};
	/// comparison
	bool operator == (const ARPoint & rhs) const {return x==rhs.x && y==rhs.y && z==rhs.z;};

	/// dot product of two ARPoints
	double DotProduct(const ARPoint & rhs);

	/// Display the point on stdout
	void Print() const {printf("(%f %f %f)",x,y,z);};

	double x; ///< The X coordinate
	double y; ///< The Y coordinate
	double z; ///< The Z coordinate
};


/** \brief
 * Camera Type enumeration
 */
enum AR_CamType
{
	AR_CAMTYPE_SINGLE = 0,
	AR_CAMTYPE_LEFT = 1,
	AR_CAMTYPE_RIGHT = 2,
	AR_CAMTYPE_CONV_EXTERN = 4, // Updated externally
	AR_CAMTYPE_CONV_EXTERN_LEFT = 5, // Updated externally
	AR_CAMTYPE_CONV_EXTERN_RIGHT = 6, // Updated externally
	AR_CAMTYPE_CONV_GL = 8, // Updated internally from previous GL frame
	AR_CAMTYPE_CONV_GL_LEFT = 9, // Updated internally from previous GL frame
	AR_CAMTYPE_CONV_GL_RIGHT = 10 // Updated internally from previous GL frame

};

/** \brief An ARPoisition object represents an origin and a direction in 3d space
 */
class ARPosition
{
	public:
		/// Default Constructor, origin 0,0,0, direction along x axis
		ARPosition() {Origin=ARPoint(0,0,0);Direction=ARPoint(0,0,0);};
		/// Constructs a ARPosition object from two ARPoint's
		ARPosition(ARPoint _Origin, ARPoint _Direction) {Origin=_Origin;Direction=_Direction;};
		/// The Origin (m,m,m)
		ARPoint Origin;
		/// The direction (rad,rad,rad)
		ARPoint Direction;
		/// Rotate and then translate based on supplied position
		ARPosition & Transform(const ARPosition & Transform)
		{
			Origin.RotateRPY(Transform.Direction.x, Transform.Direction.y, Transform.Direction.z);
			/*Origin.RotateZ(Transform.Direction.z);
			Origin.RotateY(Transform.Direction.y);
			Origin.RotateX(Transform.Direction.x);*/
			Direction += Transform.Direction;
			Origin += Transform.Origin;
			return * this;
		};

		/// basic operations
		ARPosition & operator += (const ARPosition & rhs) {Origin += rhs.Origin; Direction += rhs.Direction; return *this;};
		/// comparison
		bool operator == (const ARPosition & rhs) const {return Origin == rhs.Origin && Direction == rhs.Direction;};
		/// display the position
		void Print()
		{ Origin.Print(); Direction.Print();};
};

/** \brief
 * Describes a set of Camera Properties
 *
 * The ARCamera Object represents a physical or virtual camera. Its properties include the origin of the camera,
 * the direction it is facing, the up direction, y feild of veiw and aspect ratio.
 *
 * These parameters can then be used later for correct rendering of 3d elements.
*/
class ARCamera
{
	public:
		/// Default Constructor, Creates Camera facing along the X axis with fov=45 degrees and an aspect ratio of 1.
		ARCamera() {Direction=ARPoint(1,0,0);Up=ARPoint(0,0,1);aspect=1;y_fov=M_PI/4;CamType=AR_CAMTYPE_SINGLE;convergance=NULL;separation=NULL;sensor_width=0.008;};
		/// Creates camera with the specified origin facing at the LookAt point with the given Up, y fov and aspect
		ARCamera(ARPoint _Origin, ARPoint LookAt, ARPoint _Up, double y_fov, double aspect);
		/// Creates a camera using parameters pre-calibrated and provided in the named file
		///
		/// The file format is as follow:
		///    Translation matrix (meters)
		///    Rotation Matrix
		///    Focal distance (meters)
		///    pixel sensor width in meters (x and y)
		///    Image size in pixels (x and y)
		///    Image Scale factor (x and y)
		///
		ARCamera(const char * CalibFile);

		ARPoint Origin; ///< Camera Origin
		ARPoint Direction; ///< Camera Direction
		ARPoint Up; ///< Camera Up vector
		double y_fov; ///< Camera Y Field of View in radians
		double x_fov; ///< Camera X Field of View in radians
		double aspect; ///< Camera aspect ratio
		double sensor_width; ///< physical sensor array width (meters)
		double sensor_height; ///< physical sensor array height (meters)
		int frame_width; ///< Image width camera was calibrated with (pixels)
		int frame_height; ///< Image height camera was calibrated with (pixels)
		double FocalDistance; ///< Camera Focal Distance
		double sx; ///< Camera X scale factor
		double sy; ///< Camera Y scale factor


		double * convergance; ///< distance to convergance point
		double * separation; ///< for a stereo pair this gives the horizontal separation

		Matrix R; ///< Camera rotation Matrix;
		Matrix RInv; ///< Inverse Camera rotation Matrix
		Matrix T; ///< Camera Translation Matrix

		enum AR_CamType CamType; ///< Camera type, used for special handling of stereo cameras

		/// Sets the direction point based on the camera origin and facing the given point.
		void SetDirectionFromPoint(ARPoint _In);
		/// Gets the current direction taking into accoutn any convergence for a streo pair
		//double GetDirection() const;

		/// print the camera info to stdout
		void Print() const {Origin.Print();Direction.Print();Up.Print();printf("\n");};

		ARPoint GetCRFPointFromPixel(double x, double y); ///< Get Point in CRF from pixel
		ARPoint GetWRFDirFromCRF(ARPoint CRFPoint); ///< return a point in a ray from camera origin to CRFPOint in WRF, camera origin can be used with this return value to form a ray

		double Rx,Ry,Rz;
		double GetRx() {return Rx;};
		double GetRy() {return Ry;};
		double GetRz() {return Rz;};

};


/// simple image storage structure, client is in charge of allocating is data array
class ARImage{
	public:
		/// Construct an empty Image
		ARImage() {DataSize=0;ColourFormat=GL_RGB; x_size=y_size=0; ByteDepth=3; data = NULL; Allocate();};
		~ARImage() {delete [] data; data=NULL;};
		unsigned int x_size; ///< The width of the image in pixels
		unsigned int y_size; ///< The height of the image in pixels
		unsigned char * data; ///< The image data, user allocated
		unsigned char ByteDepth; ///< The colour depth in bytes
		unsigned int ColourFormat; ///< the Colour format as an opengl enumeration

		unsigned char & GetPixel(int x, int y) {return data[y*x_size*ByteDepth + x];}; ///< Get a Pixel reference
		const unsigned char & GetPixel(int x, int y) const {return data[y*x_size*ByteDepth + x];}; ///< Get a const pixel reference

		unsigned char * Allocate()
		{
			if (DataSize == x_size*y_size*ByteDepth)
				return data;

			delete [] data;
			DataSize = x_size*y_size*ByteDepth;
			return data = new unsigned char[DataSize];
		}; ///< Allocate the image buffer, size and depth must be set first

		int Erase()
		{
			if (data)
				memset(data,0,x_size*y_size*ByteDepth);
			return 0;
		}; ///< Erase the image area, buffer must be allocated first

		/// copy constructor (deep copy)
		ARImage(const ARImage & rhs)
		{
			//printf("Copy constructor called\n");
			DataSize =0;
			x_size = rhs.x_size;
			y_size = rhs.y_size;
			ByteDepth = rhs.ByteDepth;
      data = NULL; // need to set this to NULL so Allocate knows there is no existing memory allocated
			if (Allocate())
				memcpy(data,rhs.data,x_size*y_size*ByteDepth);
		};

		/// assignment operator
		ARImage & operator =(const ARImage & rhs)
		{
			//printf("Assignement operator called\n");
			x_size = rhs.x_size;
			y_size = rhs.y_size;
			ByteDepth = rhs.ByteDepth;
			ColourFormat = rhs.ColourFormat;
			Allocate();

			if (data)
				memcpy(data,rhs.data,x_size*y_size*ByteDepth);

			return *this;
		};
		unsigned int GetDataSize() {return DataSize;}; ///< Return the current storage size for the image data
	protected:
		unsigned int DataSize; ///< The current data buffer size


};


/** \brief
 * A basic colour value
 *
 * Describes a 4 value colour, r, g, b, a
 * with values between 0 and 1
 *
 * For alpha 0 is fully transparent and 1 is solid
 */
class ARColour
{
	public:
		/// \brief Create a default ARColour, defaults to solid white
		ARColour() {r=g=b=a=1;};
		/// \brief Create an ARColour with colour and alpha
		ARColour(float _r, float _g, float _b, float _a) {r=_r;g=_g;b=_b;a=_a;};
		/// \brief Create an ARColour with only colour
		ARColour(float _r, float _g, float _b)  {r=_r;g=_g;b=_b;a=1;};

		float r; ///< The Red Value
		float g; ///< The Green Value
		float b; ///< The Blue Value
		float a; ///< The Alpha Value
};


#endif //_ARDEV_TYPES_H_
