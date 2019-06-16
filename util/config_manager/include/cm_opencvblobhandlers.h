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
#ifndef ARIDE_OPENCVBLOBHANDLERS_H
#define ARIDE_OPENCVBLOBHANDLERS_H

#include "cm_objecthandler.h"
#include "cm_parameter.h"
#include <ardev/opencv_blobtrack.h>

void RegisterOpenCVBlobObjectHandlers();


class OpenCVBlobTrackPreProcessHandler : public FrameProcessObjectHandler
{
	public:
		OpenCVBlobTrackPreProcessHandler();
		~OpenCVBlobTrackPreProcessHandler();
	
		OpenCVBlobTrackPreProcess & GetObject();
		void RemoveObject();
	
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new OpenCVBlobTrackPreProcessHandler);};
	protected:
		OpenCVBlobTrackPreProcess * Obj;
	
		CameraObjectParameter Cam;
		DoubleParameter MinSize,MaxSize;
		BooleanParameter Debug;
};

// --- OpenCVBlobTrackPreProcess Parameter ---
class OpenCVBlobTrackPreProcessParameter : public StringParameter
{
public: 
		OpenCVBlobTrackPreProcessParameter(QString _Name, QString _Description, QString _DefaultValue="") : StringParameter(_Name,_Description,_DefaultValue) {Type="ARToolKitPreProcess";};
		virtual ~OpenCVBlobTrackPreProcessParameter() {};

		OpenCVBlobTrackPreProcessHandler * GetClass() 
		{
			if (Value == "")
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			aride_object * temp = CurrentProject->GetObjectByGUID(Value);
			if (temp == NULL)
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			if (temp->Section == ARIDE_PREPROCESS && temp->Type == "OpenCVBlobTrackPreProcess")
				return reinterpret_cast<OpenCVBlobTrackPreProcessHandler *> (temp->Handler);
			throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
		};				
};

class OpenCVBlobTrackPositionHandler : public PositionObjectHandler
{
	public:
		OpenCVBlobTrackPositionHandler();
		~OpenCVBlobTrackPositionHandler();
	
		PositionObject & GetObject();
		void RemoveObject();
	
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new OpenCVBlobTrackPositionHandler);};
	protected:
		OpenCVBlobTrackPosition * Obj;

		IntParameter Hue1_Min;
		IntParameter Hue1_Max;
		IntParameter Hue2_Min;
		IntParameter Hue2_Max;
		DoubleParameter Height;
	
		OpenCVBlobTrackPreProcessParameter Pre;
};

#endif
