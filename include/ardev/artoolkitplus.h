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
#ifndef ARTOOKITPLUS_H
#define ARTOOKITPLUS_H

#include <ardev/ardevconfig.h>
#ifdef HAVE_ARTOOLKITPLUS

#include <ARToolKitPlus/ar.h>
#include <ardev/ardev.h>

#include <map>

namespace ARToolKitPlus
{
	class Camera;
	class TrackerSingleMarker;
}

struct intlt
{
	public: bool operator () (const int & lhs, const int &rhs)
	{
		return lhs < rhs;
	};
};


/** \brief this class implements the artoolkit frame processor
 * that is used by artoolkitposition objects to retrieve marker locations
 */
class ARToolKitPlusPreProcess : public FrameProcessObject
{
	public:
		/// constructor that loads the camera file etc	
		ARToolKitPlusPreProcess(CameraObject & Camera);
	
		/// processes the frame to extract markers
		void ProcessFrame(const ARImage & frame);
		
		/// add a pattern file to the watch list, returns its integer ID, -1 on error
		/// The height represents the height of the plane to intersect with
		int AddPattern(int id, double Height=0);
		/// removes a pattern from the watch list
		void RemovePattern(int id);

		/// finds the location of the given marker 
		bool GetMarkerPos(int id, ARPosition &result);
	
	private:
		ARToolKitPlus::Camera * DummyCam;
		unsigned int lastFrameWidth; 		//< last width of a frame, used to scale calibration data
		unsigned int lastFrameHeight;		//< last height of a frame, used to scale calibration data
		int minfocnt;				//< number of minfo structures returned last time
		ARToolKitPlus::ARMarkerInfo *tmp_markers;		//< the information about the found markers
		ARCamera camera;				//< the camera

		bool    	useBCH;
		ARToolKitPlus::TrackerSingleMarker *tracker;
		ARImage GrayImage;
		
		std::map<int,double,intlt> MarkerHeights; //< A map of the marker Heights

};

/** \brief this class request pattern locations from the ARToolKitPreProcess object
 */
class ARToolKitPlusPosition : public PositionObject
{
	public:
		/// adds pattern file to the watch list
		ARToolKitPlusPosition(ARToolKitPlusPreProcess & pre,int ID,double Height) : Pre(pre) 
		{
			DroppedFrames = 0;
			MarkerID = Pre.AddPattern(ID, Height);
		}; 
		/// make sure we remove the pattern file fromt he watch listARToolKitPreProcess	
		~ARToolKitPlusPosition()
		{
			Pre.RemovePattern(MarkerID);
		}; 
	
		/// queries the ARToolKitPreProcess object and returns marker position
		virtual ARPosition GetPosition();

		/// queries the ARToolKitPreProcess object for marker presense
		virtual bool Present();

		// no point in adjusting position for artoolkitplus
		void AdjustPosition(ARPosition) {};

	protected:
		ARToolKitPlusPreProcess & Pre;
		unsigned int MarkerID;
		ARPosition lastGoodPosition;
		int DroppedFrames;
};

#endif
#endif
