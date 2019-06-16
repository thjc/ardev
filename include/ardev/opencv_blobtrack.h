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
#ifndef OPENCV_BLOBTRACK_H
#define OPENCV_BLOBTRACK_H

#include <ardev/ardevconfig.h>

#ifdef HAVE_OPENCV

#include <ardev/ardev.h>
#include <ardev/debug.h>
#include <map>

#include <opencv/cv.h>

struct intlt_bt
{
	public: bool operator () (const int & lhs, const int &rhs)
	{
		return lhs < rhs;
	};
};

typedef struct BlobPair
{
	int Hue1_Min;
	int Hue1_Max;
	int Hue2_Min;
	int Hue2_Max;
	double Height;

	ARPosition pos; /// for internal use
} BlobPair_t;

/** \brief this class implements the artoolkit frame processor
 * that is used by artoolkitposition objects to retrieve marker locations
 */
class OpenCVBlobTrackPreProcess : public FrameProcessObject
{
	public:
		/// constructor that loads the camera file etc
		OpenCVBlobTrackPreProcess(CameraObject & Camera, double BlobMinSize = 200, double BlobMaxSize = 10000, bool debug = false);

		/// destructor
		~OpenCVBlobTrackPreProcess();

		/// processes the frame to extract markers
		void ProcessFrame(const ARImage & frame);

		/// robots are represented by two colour blobs, the first centered the 2nd in the forward direction
		/// these are represented by a minimum and maximum hue
		///
		/// add a pattern file to the watch list, returns its integer ID, -1 on error
		/// The height represents the height of the plane to intersect with
		int AddBlobs(BlobPair_t & blob);
		/// removes a pattern from the watch list
		void RemoveBlobs(int id);

		/// finds the location of the given marker
		bool GetMarkerPos(int id, ARPosition &result);

	private:
		unsigned int lastFrameWidth; 		//< last width of a frame, used to scale calibration data
		unsigned int lastFrameHeight;		//< last height of a frame, used to scale calibration data
		ARCamera camera;				//< the camera
		double MinSize;					//< Min size of blob
		double MaxSize;					//< Max size of blob
		bool Debug; 					//< Set wether the debug windows are enabled
		int PatNumber;

		std::map<int,BlobPair,intlt_bt> Markers; //< A map of the marker Heights

		// find center of a blob
		ARPoint GetBlobCenter(int HueMin, int HueMax, IplImage * im, CvSize size);

		/// Get real world position based on image points
		ARPosition GetPosition(ARPoint P1, ARPoint P2, double Height);

		/// set hue values in h_img to value NewHue if Sat is less than sat_thresh
		void DropLowSat(IplImage * h_img, IplImage * s_img, int sat_thresh, int NewHue, CvSize size);
};

/** \brief this class request pattern locations from the ARToolKitPreProcess object
 */
class OpenCVBlobTrackPosition : public PositionObject
{
	public:
		OpenCVBlobTrackPosition(OpenCVBlobTrackPreProcess & pre, const BlobPair_t & blob) : Pre(pre), Blob(blob)
		{
			Elapsed = 0;
			MarkerID = 0;
			memset(&lastGoodPosition,0,sizeof(lastGoodPosition));
		};

		~OpenCVBlobTrackPosition()
		{
		};

		/// initalise tracking
		int Initialise(bool Active = true)
		{
			dbg_print(ARDBG_VERBOSE,"Adding new blob pair to opencv tracker: %d %d %d %d\n",Blob.Hue1_Min, Blob.Hue1_Max,Blob.Hue2_Min,Blob.Hue2_Max);
			MarkerID = Pre.AddBlobs(Blob);
			dbg_print(ARDBG_VERBOSE,"Blob was assigned Marker ID: %d\n",MarkerID);
			return 0;
		};

		/// stop tracking
		void Terminate()
		{
			Pre.RemoveBlobs(MarkerID);
		};

		/// queries the ARToolKitPreProcess object and returns marker position
		virtual ARPosition GetPosition();
		virtual void AdjustPosition(ARPosition position){};

		/// Is the position object present, ie in fov for optical tracking
		virtual bool Present();

	protected:
		OpenCVBlobTrackPreProcess & Pre;
		unsigned int MarkerID;
		ARPosition lastGoodPosition;
		StopWatch Timer;
		double Elapsed;
		BlobPair_t Blob;
};

#endif
#endif
