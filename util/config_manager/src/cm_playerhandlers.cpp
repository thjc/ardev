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
#ifdef HAVE_PLAYER
#include <stdlib.h>
 

#include <ardev/debug.h>
#include <ardev/exception.h>
#include <cm_objecthandler.h>
#include <cm_parameter_ardev.h>
#include <cm_playerhandlers.h>

//#include <ardev/camera_player.h>
#include <ardev/capture.h>

/* -----------------------------------------
    Capture Stage Handler Methods
   ----------------------------------------- */

CaptureStageHandler::CaptureStageHandler()
{
	Capture = NULL;
	CameraObject = new CameraObjectParameter("CameraObject","Name of Camera Object","camera");
	CameraPositionObject = new PositionObjectParameter("CameraPositionObject","Name of Camera Position Object","camera_position");
	RobotName = new StringParameter("RobotName","Robot Model name","robot");
	// Check out mem allocation
	if (!(CameraObject && CameraPositionObject && RobotName))
		throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	Parameters.push_back(CameraObject);
	Parameters.push_back(CameraPositionObject);
	Parameters.push_back(RobotName);		
}

CaptureStageHandler::~CaptureStageHandler()
{
	delete Capture;
	delete CameraObject;
	delete CameraPositionObject;
	delete RobotName;
}
	
CaptureObject & CaptureStageHandler::GetObject()
{
	if (Capture == NULL)
	{
		QByteArray asciiName = RobotName->Value.toAscii(); // Careful of lifetime
		Capture = new CaptureStage(asciiName,
						&CameraObject->GetClass()->GetObject(),&CameraPositionObject->GetClass()->GetObject(),PlayerClient.GetClass()->GetObject());
		
		if (Capture == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(Capture);
	return *Capture; 
}

void CaptureStageHandler::RemoveObject()
{
	delete Capture;
	Capture = NULL;
}

/* -----------------------------------------
    PositionActArray Handler Methods
   ----------------------------------------- */

PositionActArrayHandler::PositionActArrayHandler() :
	pos("Position","Initial Position Value","0 0 -100 0 0 0"),
	Index("ActIndex","Actuator Index","0")
{
	obj = NULL;
	//Parameters.push_back(&pos);
	Parameters.push_back(&Index);
}

PositionActArrayHandler::~PositionActArrayHandler()
{
	delete obj;
}
	
PositionObject & PositionActArrayHandler::GetObject()
{
	if (obj == NULL)
	{	
		obj = new PositionActArray(pos.Value,Index.Value);
		
		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void PositionActArrayHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
}

// Register Object Handlers

#define REG(X,Y,Z) dbg_print(ARDBG_INFO,"\t%s\n",X);ObjectHandlerRegistry[X] = ObjectRegistryEntry(Y,Z);
void RegisterPlayerObjectHandlers()
{
	dbg_print(ARDBG_INFO,"Registering Player Object Handlers\n");
	REG("PlayerClientInterface",PlayerClientInterfaceHandler::CreateHandler,ARIDE_MISC);
	REG("CaptureStage",CaptureStageHandler::CreateHandler,ARIDE_CAPTURE);
	REG("CapturePlayer",CapturePlayerHandler::CreateHandler,ARIDE_CAPTURE);
	REG("PositionPlayer",PositionPlayerHandler::CreateHandler,ARIDE_POSITION);
	REG("Position3dPlayer",Position3dPlayerHandler::CreateHandler,ARIDE_POSITION);
	REG("PositionPlayerPTZ",PositionPlayerPTZHandler::CreateHandler,ARIDE_POSITION);
	REG("RenderPlayerActArray",RenderPlayerActArrayHandler::CreateHandler,ARIDE_RENDER);
	REG("PositionActArray",PositionActArrayHandler::CreateHandler,ARIDE_POSITION);
	REG("RenderPlayerBumper",RenderPlayerBumperHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderPlayerIr",RenderPlayerIrHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderPlayerLaser",RenderPlayerLaserHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderPlayerLimb",RenderPlayerLimbHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderPlayerLocalise",RenderPlayerLocaliseHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderPlayerMap",RenderPlayerMapHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderPlayerPath",RenderPlayerPathHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderPlayerPTZ",RenderPlayerPTZHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderPlayerSonar",RenderPlayerSonarHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderGraphics2DHandler",RenderGraphics2DHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderGraphics3DHandler",RenderGraphics3DHandler::CreateHandler,ARIDE_RENDER);
#ifdef HAVE_GEOS
	REG("RenderPlayerVectorMap",RenderPlayerVectorMapHandler::CreateHandler,ARIDE_RENDER);
#endif
	
}

#endif
