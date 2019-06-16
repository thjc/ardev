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
 
#include <ardev/ardevconfig.h>
#ifdef HAVE_OPENCV
 
#include "cm_opencvblobhandlers.h"
#include <ardev/debug.h>
/* -----------------------------------------
    OpenCVBlobPreProcess Handler Methods
   ----------------------------------------- */

OpenCVBlobTrackPreProcessHandler::OpenCVBlobTrackPreProcessHandler() : 
	Cam("CameraObject","Camera Object","cam"),
	MinSize("MinBlobSize","Minimum Blob Size","200"),
	MaxSize("MaxBlobSize","Maximum BLob Size","10000"),
	Debug("Debug","Display Debug Window","False")
{
	Parameters.push_back(&Cam);
	Parameters.push_back(&MinSize);
	Parameters.push_back(&MaxSize);
	Parameters.push_back(&Debug);
	
	Obj = NULL;
}

OpenCVBlobTrackPreProcessHandler::~OpenCVBlobTrackPreProcessHandler()
{
	delete Obj;
}
	
OpenCVBlobTrackPreProcess & OpenCVBlobTrackPreProcessHandler::GetObject()
{
	if (Obj == NULL)
	{
		Obj = new OpenCVBlobTrackPreProcess(Cam.GetClass()->GetObject(), MinSize.Value, MaxSize.Value, Debug.Value);
		if (Obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(Obj);
	return *Obj;
}

void OpenCVBlobTrackPreProcessHandler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}

/* -----------------------------------------
    OpenCVBlobPreProcess Handler Methods
   ----------------------------------------- */

OpenCVBlobTrackPositionHandler::OpenCVBlobTrackPositionHandler() : 
	Hue1_Min("Hue1Min","Minimum Value for Hue1 (0-180)","10"),
	Hue1_Max("Hue1Max","Maximum Value for Hue1 (0-180)","20"),
	Hue2_Min("Hue2Min","Minimum Value for Hue2 (0-180)","30"),
	Hue2_Max("Hue2Max","Maximum Value for Hue2 (0-180)","40"),
	Height("Height","Height of marker plane (m)","0.2"),
	Pre("PreProcessor","OpenCV Blob Pre Process Object","preprocess")
{
	Parameters.push_back(&Hue1_Min);
	Parameters.push_back(&Hue1_Max);
	Parameters.push_back(&Hue2_Min);
	Parameters.push_back(&Hue2_Max);
	Parameters.push_back(&Height);	
	Parameters.push_back(&Pre);
	Obj = NULL;
}

OpenCVBlobTrackPositionHandler::~OpenCVBlobTrackPositionHandler()
{
	delete Obj;
}
	
PositionObject & OpenCVBlobTrackPositionHandler::GetObject()
{
	BlobPair_t bp;
	bp.Hue1_Min = Hue1_Min.Value;
	bp.Hue1_Max = Hue1_Max.Value;
	bp.Hue2_Min = Hue2_Min.Value;
	bp.Hue2_Max = Hue2_Max.Value;
	bp.Height = Height.Value;
	
	
	if (Obj == NULL)
	{
		Obj = new OpenCVBlobTrackPosition(Pre.GetClass()->GetObject(),bp);
		if (Obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(Obj);
	return *Obj;
}

void OpenCVBlobTrackPositionHandler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}


// Register Object Handlers

#define REG(X,Y,Z) dbg_print(ARDBG_INFO,"\t%s\n",X);ObjectHandlerRegistry[X] = ObjectRegistryEntry(Y,Z);
void RegisterOpenCVBlobObjectHandlers()
{
	dbg_print(ARDBG_INFO,"Registering OpenCV Blob Track Object Handlers\n");
	REG("OpenCVBlobTrackPreProcess",OpenCVBlobTrackPreProcessHandler::CreateHandler,ARIDE_PREPROCESS);
	REG("OpenCVBlobTrackPosition",OpenCVBlobTrackPositionHandler::CreateHandler,ARIDE_POSITION);
}

#endif
