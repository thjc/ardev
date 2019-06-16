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
#include "cm_artoolkithandlers.h"
#ifdef HAVE_LIBAR
#include <ardev/debug.h>
/* -----------------------------------------
    ARToolKitPreProcess Handler Methods
   ----------------------------------------- */

ARToolKitPreProcessHandler::ARToolKitPreProcessHandler() : 
	Threshold("Threshold","Threshold Level","100") ,
	Cam("CameraObject","Camera Object","cam")
{
	Parameters.push_back(&Threshold);
	Parameters.push_back(&Cam);
	Obj = NULL;
}

ARToolKitPreProcessHandler::~ARToolKitPreProcessHandler()
{
	delete Obj;
}
	
ARToolKitPreProcess & ARToolKitPreProcessHandler::GetObject()
{
	if (Obj == NULL)
	{
		Obj = new ARToolKitPreProcess(Threshold.Value,Cam.GetClass()->GetObject());
		if (Obj == NULL)
			throw aride_exception(ARIDE_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	theMainWindow->InitialiseObject(Obj);
	return *Obj;
}

void ARToolKitPreProcessHandler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}

/* -----------------------------------------
    ARToolKitPosition Handler Methods
   ----------------------------------------- */

ARToolKitPositionHandler::ARToolKitPositionHandler() : 
	PattWidth("PatternSize","Pattern Size","100") ,
	PattFile("PatternFile","Pattern File",""),
	Pre("PreProcessor","ARToolKit Pre Process Object","preprocess")
{
	Parameters.push_back(&PattWidth);
	Parameters.push_back(&PattFile);
	Parameters.push_back(&Pre);
	Obj = NULL;
}

ARToolKitPositionHandler::~ARToolKitPositionHandler()
{
	delete Obj;
}
	
PositionObject & ARToolKitPositionHandler::GetObject()
{
	if (Obj == NULL)
	{
		Obj = new ARToolKitPosition(Pre.GetClass()->GetObject(),PattFile.Value, PattWidth.Value);
		if (Obj == NULL)
			throw aride_exception(ARIDE_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	theMainWindow->InitialiseObject(Obj);
	return *Obj;
}

void ARToolKitPositionHandler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}


// Register Object Handlers

#define REG(X,Y,Z) dbg_print(ARDBG_INFO,"\t%s\n",X);ObjectHandlerRegistry[X] = ObjectRegistryEntry(Y,Z);
void RegisterARToolKitObjectHandlers()
{
	dbg_print(ARDBG_INFO,"Registering ARToolKit Object Handlers\n");
	REG("ARToolKitPreProcess",ARToolKitPreProcessHandler::CreateHandler,ARIDE_PREPROCESS);
	REG("ARToolKitPosition",ARToolKitPositionHandler::CreateHandler,ARIDE_POSITION);
}
#endif
