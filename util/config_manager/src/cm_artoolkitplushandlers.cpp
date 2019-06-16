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
#ifdef HAVE_ARTOOLKITPLUS
#include "cm_artoolkitplushandlers.h"
#include <ardev/debug.h>
/* -----------------------------------------
    ARToolKitPlusPreProcess Handler Methods
   ----------------------------------------- */

ARToolKitPlusPreProcessHandler::ARToolKitPlusPreProcessHandler() : 
	Cam("CameraObject","Camera Object","cam")
{
	Parameters.push_back(&Cam);
	Obj = NULL;
}

ARToolKitPlusPreProcessHandler::~ARToolKitPlusPreProcessHandler()
{
	delete Obj;
}
	
ARToolKitPlusPreProcess & ARToolKitPlusPreProcessHandler::GetObject()
{
	if (Obj == NULL)
	{
		Obj = new ARToolKitPlusPreProcess(Cam.GetClass()->GetObject());
		if (Obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(Obj);
	return *Obj;
}

void ARToolKitPlusPreProcessHandler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}

/* -----------------------------------------
    ARToolKitPlusPosition Handler Methods
   ----------------------------------------- */

ARToolKitPlusPositionHandler::ARToolKitPlusPositionHandler() : 
	MarkerID("MarkerID","marker ID","0"),
	Height("Height","marker height","0"),
	Pre("PreProcessor","ARToolKitPlus Pre Process Object","preprocess")
{
	Parameters.push_back(&MarkerID);
	Parameters.push_back(&Height);
	Parameters.push_back(&Pre);
	Obj = NULL;
}

ARToolKitPlusPositionHandler::~ARToolKitPlusPositionHandler()
{
	delete Obj;
}
	
PositionObject & ARToolKitPlusPositionHandler::GetObject()
{
	if (Obj == NULL)
	{
		Obj = new ARToolKitPlusPosition(Pre.GetClass()->GetObject(),MarkerID.Value,Height.Value);
		if (Obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(Obj);
	return *Obj;
}

void ARToolKitPlusPositionHandler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}


// Register Object Handlers

#define REG(X,Y,Z) dbg_print(ARDBG_INFO,"\t%s\n",X);ObjectHandlerRegistry[X] = ObjectRegistryEntry(Y,Z);
void RegisterARToolKitPlusObjectHandlers()
{
	dbg_print(ARDBG_INFO,"Registering ARToolKitPlus Object Handlers\n");
	REG("ARToolKitPlusPreProcess",ARToolKitPlusPreProcessHandler::CreateHandler,ARIDE_PREPROCESS);
	REG("ARToolKitPlusPosition",ARToolKitPlusPositionHandler::CreateHandler,ARIDE_POSITION);
}
#endif
