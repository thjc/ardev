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

#include <ardev/exception.h>
#include "cm_parameter_ardev.h"
#include "cm_objecthandler.h"

#include <ardev/output_x11.h>
#include <ardev/output_Manage.h>
#include <ardev/output_ffmpeg.h>
#include <ardev/capture.h>
#include <ardev/ardev.h>
#include <ardev/render_base.h>
#include <ardev/render_face_model.h>
#include <ardev/render_robot.h>

#include <ardev/debug.h>

// Environment Object Methods


EnvironmentObjectHandler::EnvironmentObjectHandler()
{
	Type="EnvironmentObjectHandler";
}

EnvironmentObjectHandler::~EnvironmentObjectHandler()
{
}


/* -----------------------------------------
    Camera Constant Handler Methods
   ----------------------------------------- */

CameraConstantHandler::CameraConstantHandler()
{
	Camera = NULL;
	y_fov = new DoubleParameter("f_fov","Camera Y FOV (degrees)","45");
	aspect = new DoubleParameter("Aspect","Camera aspect ratio","1.6");
	SensorWidth = new DoubleParameter("SensorWidth","Image sensor width (m)","0.008");
	CalibrationFile = new StringParameter("CalibrationFile","Optional Calibration File","");
	Origin = new ARPointParameter("Origin","Camera origin","0 0 0");
	Direction = new ARPointParameter("Direction","Camera direction (vector)","1 0 0");
	Up = new ARPointParameter("Up","Camera up vector","0 0 1");

	// Check out mem allocation
	if (!(y_fov && aspect && SensorWidth && CalibrationFile && Origin && Direction && Up))
		throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	Parameters.push_back(y_fov);
	Parameters.push_back(aspect);
	Parameters.push_back(SensorWidth);
	Parameters.push_back(Origin);
	Parameters.push_back(Direction);
	Parameters.push_back(Up);
	Parameters.push_back(CalibrationFile);
}

CameraConstantHandler::~CameraConstantHandler()
{
	delete Camera;
	delete (y_fov);
	delete (aspect);
	delete SensorWidth;
	delete CalibrationFile;
}

CameraObject & CameraConstantHandler::GetObject()
{
	if (Camera == NULL)
	{
		if (CalibrationFile->Value != "")
		{
			QByteArray asciiFile = CalibrationFile->Value.toAscii(); // Careful of lifetime
			Camera = new CameraConstant(static_cast<const char *> (asciiFile));
		}
		else
			Camera = new CameraConstant();

		if (Camera == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

		if (CalibrationFile->Value == "")
		{
			Camera->GetCameraRef().y_fov = DTOR(y_fov->Value);
			Camera->GetCameraRef().aspect = aspect->Value;
			Camera->GetCameraRef().sensor_width = SensorWidth->Value;
			Camera->GetCameraRef().Origin = Origin->Value;
			Camera->GetCameraRef().Direction = Direction->Value;
			Camera->GetCameraRef().Up = Up->Value;
		}
	}
	CurrentProject->InitialiseObject(Camera);
	return * Camera;
}

void CameraConstantHandler::RemoveObject()
{
	delete Camera;
	Camera = NULL;
}

/* -----------------------------------------
    Output X11 Handler Methods
   ----------------------------------------- */

OutputX11Handler::OutputX11Handler()
{
	Cam = new CameraObjectParameter("Camera","Camera Object Name","camera");
	CamPos = new PositionObjectParameter("CameraPosition","Camera Position Object Name","camera_position");
	Cap = new CaptureObjectParameter("Capture","Capture Object Name","capture");
	Width = new IntParameter("Width","Width of Output Window","320");
	Height = new IntParameter("Height","Height of Output Window","240");
	DisplayName = new StringParameter("DisplayName","Display eg :0.1, blank for default","");
	Name = new StringParameter("Name","Environment Name","envname");
	FullScreen = new BooleanParameter("FullScreen","Render full screen","False");

	Parameters.push_back(Name);
	Parameters.push_back(Cam);
	Parameters.push_back(CamPos);
	Parameters.push_back(Cap);
	Parameters.push_back(Width);
	Parameters.push_back(Height);
	Parameters.push_back(DisplayName);
	Parameters.push_back(FullScreen);

	Obj = NULL;
}

OutputX11Handler::~OutputX11Handler()
{
	delete Cam;
	delete CamPos;
	delete Cap;
	delete Width;
	delete Height;
	delete DisplayName;
	delete FullScreen;
	delete Name;

	//delete Obj;
}

OutputObject & OutputX11Handler::GetObject()
{
	if (Obj == NULL)
	{
		QByteArray dispName = DisplayName->Value.toAscii(); // Careful of lifetime
		const char* dispNamePtr = dispName.size() ? dispName.constData() : NULL;
		QByteArray name = Name->Value.toAscii();
		const char* namePtr = name.size() ? name.constData() : NULL;
		Obj = new OutputX11(&Cap->GetClass()->GetObject(),&Cam->GetClass()->GetObject(),&CamPos->GetClass()->GetObject(),Width->Value,Height->Value,dispNamePtr,namePtr,FullScreen->Value);
		if (Obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	//CurrentProject->InitialiseObject(Obj);
	return *Obj;
}

void OutputX11Handler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}

/* -----------------------------------------
    Output Shared X11 Handler Methods
   ----------------------------------------- */
OutputSharedX11Handler::OutputSharedX11Handler()
{
	Cam = new CameraObjectParameter("Camera","Camera Object Name","camera");
	CamPos = new PositionObjectParameter("CameraPosition","Camera Position Object Name","camera_position");
	Cap = new CaptureObjectParameter("Capture","Capture Object Name","capture");
	Width = new IntParameter("Width","Width of Output Window","320");
	Height = new IntParameter("Height","Height of Output Window","240");
	DisplayName = new StringParameter("DisplayName","Display eg :0.1, blank for default","");
	Name = new StringParameter("Name","Environment Name","envname");
	FullScreen = new BooleanParameter("FullScreen","Render full screen","False");

	Parameters.push_back(Name);
	Parameters.push_back(Cam);
	Parameters.push_back(CamPos);
	Parameters.push_back(Cap);
	Parameters.push_back(Width);
	Parameters.push_back(Height);
	Parameters.push_back(DisplayName);
	Parameters.push_back(FullScreen);

	Obj = NULL;
}

OutputSharedX11Handler::~OutputSharedX11Handler()
{
	delete Cam;
	delete Name;
	delete CamPos;
	delete Cap;
	delete Width;
	delete Height;
	delete DisplayName;
	delete FullScreen;

	//OutputX11::RemoveSharedOutput();
	Obj = NULL;
}

OutputObject & OutputSharedX11Handler::GetObject()
{
	if (Obj == NULL)
	{
		QByteArray dispNameBytes = DisplayName->Value.toAscii(); // Careful of lifetime
		const char* dispName = dispNameBytes.size() ? dispNameBytes.constData() : NULL;
		QByteArray nameBytes = Name->Value.toAscii();
		const char* name = nameBytes.size() ? nameBytes.constData() : NULL;
		Obj = OutputX11::GetSharedOutput(&Cap->GetClass()->GetObject(),&Cam->GetClass()->GetObject(),&CamPos->GetClass()->GetObject(),Width->Value,Height->Value,dispName,name,FullScreen->Value);
		if (Obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	//CurrentProject->InitialiseObject(Obj);
	return *Obj;
}

void OutputSharedX11Handler::RemoveObject()
{
	QByteArray nameBytes = Name->Value.toAscii();
	OutputX11::RemoveSharedOutput(nameBytes);
	Obj = NULL;
}

/* -----------------------------------------
    Output Manage Handler Methods
   ----------------------------------------- */

OutputManageHandler::OutputManageHandler()
{
	Cam = new CameraObjectParameter("Camera","Camera Object Name","camera");
	CamPos = new PositionObjectParameter("CameraPosition","Camera Position Object Name","camera_position");
	Cap = new CaptureObjectParameter("Capture","Capture Object Name","capture");
	Width = new IntParameter("Width","Width of Output Window","320");
	Height = new IntParameter("Height","Height of Output Window","240");
	DisplayName = new StringParameter("DisplayName","Display eg :0.1, blank for default","");
	FullScreen = new BooleanParameter("FullScreen","Render full screen","False");

	Parameters.push_back(Cam);
	Parameters.push_back(CamPos);
	Parameters.push_back(Cap);
	Parameters.push_back(Width);
	Parameters.push_back(Height);
	Parameters.push_back(DisplayName);
	Parameters.push_back(FullScreen);

	Obj = NULL;
}
//-----------------------------------------------------------------------------------------------------------------
OutputManageHandler::~OutputManageHandler()
{
	delete Cam;
	delete CamPos;
	delete Cap;
	delete Width;
	delete Height;
	delete DisplayName;
	delete FullScreen;

	delete Obj;
}
//-----------------------------------------------------------------------------------------------------------------
OutputObject & OutputManageHandler::GetObject()
{
	if (Obj == NULL)
	{
		QByteArray dispNameBytes = DisplayName->Value.toAscii(); // Careful of lifetime
		const char* dispName = dispNameBytes.size() ? dispNameBytes.constData() : NULL;
		Obj = new OutputManage(&Cap->GetClass()->GetObject(),&Cam->GetClass()->GetObject(),&CamPos->GetClass()->GetObject(),Width->Value,Height->Value,dispName, FullScreen->Value);
		if (Obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	//CurrentProject->InitialiseObject(Obj);
	return *Obj;
}
//-----------------------------------------------------------------------------------------------------------------
void OutputManageHandler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}
//-----------------------------------------------------------------------------------------------------------------


/* -----------------------------------------
    Output Movie Handler Methods
   ----------------------------------------- */
#ifdef HAVE_FFMPEG
OutputMovieHandler::OutputMovieHandler() :
	Filename("Filename","Output Filename","movie.mpeg"),
	Width("Width","Width of Output Window","320"),
	Height("Height","Height of Output Window","240")
{
	Obj = NULL;
	Parameters.push_back(&Filename);
	Parameters.push_back(&Width);
	Parameters.push_back(&Height);
}

OutputMovieHandler::~OutputMovieHandler()
{
	delete Obj;
}

FrameProcessObject & OutputMovieHandler::GetObject()
{
	if (Obj == NULL)
	{
		QByteArray asciiValue = Filename.Value.toAscii(); // Careful of lifetime
		Obj = new OutputMovie(asciiValue,Width.Value,Height.Value);
		if (Obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(Obj);
	return *Obj;
}

void OutputMovieHandler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}
#endif

/* -----------------------------------------
    Capture Null Handler Methods
   ----------------------------------------- */

CaptureNullHandler::CaptureNullHandler()
	: Background("Background","Background Colour","0 0 0 0")
{
	Capture = NULL;
	Parameters.push_back(&Background);
}

CaptureNullHandler::~CaptureNullHandler()
{
	if (Capture)
		delete Capture;
}

CaptureObject & CaptureNullHandler::GetObject()
{
	if (Capture != NULL)
	{
		CurrentProject->InitialiseObject(Capture);
		return *Capture;
	}

	Capture = new CaptureNull(Background.Value);

	if (Capture == NULL)
		throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	CurrentProject->InitialiseObject(Capture);
	return * Capture;
}

void CaptureNullHandler::RemoveObject()
{
	delete Capture;
	Capture = NULL;
}

#ifdef HAVE_IMAGEMAGICK
/* -----------------------------------------
    Capture File Handler Methods
   ----------------------------------------- */

CaptureFileHandler::CaptureFileHandler()
{
	Capture = NULL;
	Filename = new StringParameter("Filename","Static Capture Filename","capture.jpeg");
	Delay = new IntParameter("Delay","Delay before laoding next image in sequence (0 to disable)","0");
	// Check out mem allocation
	if (!(Filename && Delay))
		throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	Parameters.push_back(Filename);
	Parameters.push_back(Delay);
}

CaptureFileHandler::~CaptureFileHandler()
{
	delete Capture;
	delete (Filename);
	delete (Delay);
}

CaptureObject & CaptureFileHandler::GetObject()
{
	if (Capture != NULL)
	{
		CurrentProject->InitialiseObject(Capture);
		return *Capture;
	}

	QByteArray asciiFile = Filename->Value.toAscii(); // Careful of lifetime
	Capture = new CaptureFile(asciiFile, Delay->Value);

	if (Capture == NULL)
		throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	CurrentProject->InitialiseObject(Capture);
	return * Capture;
}

void CaptureFileHandler::RemoveObject()
{
	delete Capture;
	Capture = NULL;
}

#endif

/* -----------------------------------------
    Capture V4L Handler Methods
   ----------------------------------------- */

#ifdef HAVE_LINUX_VIDEODEV_H
CaptureV4LHandler::CaptureV4LHandler()
{
	Capture = NULL;
	Device = new StringParameter("Device","V4L Device Dile","/dev/video0");
	Width = new IntParameter("Width","Requested Image Width","480");
	Height = new IntParameter("Height","Requested Image Height","300");
	Channel = new IntParameter("Channel","Video Channel","1");
	Format = new IntParameter("Format","Video Format","1");
	// Check out mem allocation
	if (!(Device && Width && Height && Channel && Format))
		throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	Parameters.push_back(Device);
	Parameters.push_back(Width);
	Parameters.push_back(Height);
	Parameters.push_back(Channel);
	Parameters.push_back(Format);
}

CaptureV4LHandler::~CaptureV4LHandler()
{
	delete Capture;
	delete (Device);
	delete (Width);
	delete Height;
	delete Channel;
	delete Format;
}

CaptureObject & CaptureV4LHandler::GetObject()
{
	if (Capture == NULL)
	{
		QByteArray asciiDevice = Device->Value.toAscii(); // Careful of lifetime
		Capture = new CaptureV4L(asciiDevice, Width->Value, Height->Value, Channel->Value, Format->Value);

		if (Capture == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(Capture);
	return *Capture;
}

void CaptureV4LHandler::RemoveObject()
{
	delete Capture;
	Capture = NULL;
}
#endif

#ifdef HAVE_DC1394
/* -----------------------------------------
    Capture DC1394 Handler Methods
   ----------------------------------------- */

CaptureDC1394Handler::CaptureDC1394Handler()
{
	Capture = NULL;
}

CaptureDC1394Handler::~CaptureDC1394Handler()
{
	delete Capture;
}

CaptureObject & CaptureDC1394Handler::GetObject()
{
	if (Capture == NULL)
	{
		Capture = new CaptureDC1394();

		if (Capture == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(Capture);
	return *Capture;
}

void CaptureDC1394Handler::RemoveObject()
{
	delete Capture;
	Capture = NULL;
}
#endif

/* -----------------------------------------
    Render Teapot Handler Methods
   ----------------------------------------- */

RenderTeapotHandler::RenderTeapotHandler() :
	Colour("Colour","Colour of Teapot","255 0 0 0"),
	Size("Size","Size of Teapot","0.010")
{
	obj = NULL;
	Parameters.push_back(&Colour);
	Parameters.push_back(&Size);
}

RenderTeapotHandler::~RenderTeapotHandler()
{
	delete obj;
}

RenderObject & RenderTeapotHandler::GetObject()
{
	if (obj == NULL)
	{
		obj = new RenderTeapot(Colour.Value,Size.Value);

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	BaseObject = obj;
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void RenderTeapotHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
	BaseObject = obj;
}
/* -----------------------------------------
    Render Model Handler Methods
   ----------------------------------------- */

RenderModelHandler::RenderModelHandler() :
	ModelName("ModelName","relative path to the 3ds model file","./model/default.3ds"),
	TextureBase("TextureBase","relative path that holds the texture files","./model/"),
	Scale("Scale","Scaling to be applied to the model","1.0")
{
	obj = NULL;
	Parameters.push_back(&ModelName);
	Parameters.push_back(&TextureBase);
	Parameters.push_back(&Scale);
}

RenderModelHandler::~RenderModelHandler()
{
	delete obj;
}

RenderObject & RenderModelHandler::GetObject()
{
	if (obj == NULL)
	{
		QByteArray asciiModel = ModelName.Value.toAscii(); // Careful of lifetime
		QByteArray asciiTex = TextureBase.Value.toAscii();
		obj = new RenderModel(asciiModel,asciiTex,Scale.Value);

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	BaseObject = obj;
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void RenderModelHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
	BaseObject = obj;
}

/* -----------------------------------------
    Render B21r Handler Methods
   ----------------------------------------- */

RenderB21rHandler::RenderB21rHandler() :
	Visible("Visible","Visible","false")
{
	obj = NULL;
	Parameters.push_back(&Visible);
}

RenderB21rHandler::~RenderB21rHandler()
{
	delete obj;
}

RenderObject & RenderB21rHandler::GetObject()
{
	if (obj == NULL)
	{
		obj = new RenderB21r(Visible.Value);

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	BaseObject = obj;
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void RenderB21rHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
	BaseObject = obj;
}

#ifdef WITH_LIBFEP
/* -----------------------------------------
    Render Virtual Face Handler Methods
   ----------------------------------------- */

RenderVirtualFaceHandler::RenderVirtualFaceHandler() :
	ModelName("ModelName","relative path to the 3ds model file","./model/face.wfm")
{
	obj = NULL;
	Parameters.push_back(&ModelName);
}

RenderVirtualFaceHandler::~RenderVirtualFaceHandler()
{
	delete obj;
}

RenderObject & RenderVirtualFaceHandler::GetObject()
{
	if (obj == NULL)
	{
		obj = new RenderFace(ModelName.Value);

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	BaseObject = obj;
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void RenderVirtualFaceHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
	BaseObject = obj;
}
#endif

/* -----------------------------------------
    Render Axes Handler Methods
   ----------------------------------------- */

RenderAxesHandler::RenderAxesHandler()
{
	obj = NULL;
}

RenderAxesHandler::~RenderAxesHandler()
{
	delete obj;
}

RenderObject & RenderAxesHandler::GetObject()
{
	if (obj == NULL)
	{
		obj = new RenderAxes();

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	BaseObject = obj;
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void RenderAxesHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
	BaseObject = obj;
}

/* -----------------------------------------
    PositionConstant Handler Methods
   ----------------------------------------- */

PositionConstantHandler::PositionConstantHandler() :
	pos("Position","Constant Position Value","0 0 0 0 0 0")
{
	obj = NULL;
	Parameters.push_back(&pos);
}

PositionConstantHandler::~PositionConstantHandler()
{
	delete obj;
}

PositionObject & PositionConstantHandler::GetObject()
{
	if (obj == NULL)
	{
		obj = new PositionConstant(pos.Value);

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void PositionConstantHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
}

/* -----------------------------------------
    PositionControllable Handler Methods
   ----------------------------------------- */

PositionControllableHandler::PositionControllableHandler() :
	pos("Position","Initial Position Value","0 0 0 0 0 0")
{
	obj = NULL;
	Parameters.push_back(&pos);
}

PositionControllableHandler::~PositionControllableHandler()
{
	delete obj;
}

PositionObject & PositionControllableHandler::GetObject()
{
	if (obj == NULL)
	{
		obj = new PositionControllable(pos.Value);

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void PositionControllableHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
}

/* -----------------------------------------
    PositionRotate Handler Methods
   ----------------------------------------- */

PositionRotateHandler::PositionRotateHandler() :
	pos("Position","Constant Position Value","0 0 0 0 0 0"),
	rate("Rate","Rotation Rate per axis (degrees/second)","0 0 0")
{
	obj = NULL;
	Parameters.push_back(&pos);
	Parameters.push_back(&rate);
}

PositionRotateHandler::~PositionRotateHandler()
{
	delete obj;
}

PositionObject & PositionRotateHandler::GetObject()
{
	if (obj == NULL)
	{
		obj = new PositionRotate(pos.Value,ARPoint(DTOR(rate.Value.x),DTOR(rate.Value.y),DTOR(rate.Value.z)));

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void PositionRotateHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
}

/* -----------------------------------------
    Calibrated Position Handler Methods
   ----------------------------------------- */

CalibratedPositionHandler::CalibratedPositionHandler()
{
	obj = NULL;
	CalibrationFile = new StringParameter("CalibrationFile","Calibration File","");

	// Check out mem allocation
	if (!(CalibrationFile))
		throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	Parameters.push_back(CalibrationFile);
}

CalibratedPositionHandler::~CalibratedPositionHandler()
{
	delete CalibrationFile;
	delete obj;
}

PositionObject & CalibratedPositionHandler::GetObject()
{
	if (obj == NULL)
	{
		if (CalibrationFile->Value != "")
		{
			ARCamera Temp(CalibrationFile->Value.toAscii());
			obj = new PositionConstant(ARPosition(Temp.Origin,Temp.Direction));
		}
		else
			obj = new PositionNull();

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(obj);
	return * obj;
}

void CalibratedPositionHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
}

// Register Object Handlers

#define REG(X,Y,Z) dbg_print(ARDBG_INFO,"\t%s\n",X);ObjectHandlerRegistry[X] = ObjectRegistryEntry(Y,Z);
void RegisterDefaultObjectHandlers()
{
	dbg_print(ARDBG_INFO,"Registering Default Object Handlers\n");
#ifdef HAVE_DC1394
	REG("CaptureDC1394",CaptureDC1394Handler::CreateHandler,ARIDE_CAPTURE);
#endif
	REG("CaptureNull",CaptureNullHandler::CreateHandler,ARIDE_CAPTURE);
#ifdef HAVE_IMAGEMAGICK
	REG("CaptureFile",CaptureFileHandler::CreateHandler,ARIDE_CAPTURE);
#endif
#ifdef HAVE_LINUX_VIDEODEV_H
	REG("CaptureV4L",CaptureV4LHandler::CreateHandler,ARIDE_CAPTURE);
#endif
	REG("CameraConstant",CameraConstantHandler::CreateHandler,ARIDE_CAMERA);
	REG("OutputX11",OutputX11Handler::CreateHandler,ARIDE_OUTPUT);
	REG("OutputSharedX11",OutputSharedX11Handler::CreateHandler,ARIDE_OUTPUT);
#ifdef HAVE_FFMPEG
	REG("OutputMovie",OutputMovieHandler::CreateHandler,ARIDE_SECONDARYOUTPUT);
#endif
	REG("PositionNull",PositionNullHandler::CreateHandler,ARIDE_POSITION);
	REG("PositionConstant",PositionConstantHandler::CreateHandler,ARIDE_POSITION);
	REG("PositionControllable",PositionControllableHandler::CreateHandler,ARIDE_POSITION);
	REG("PositionRotate",PositionRotateHandler::CreateHandler,ARIDE_POSITION);
	REG("CalibratedPosition",CalibratedPositionHandler::CreateHandler,ARIDE_POSITION);
	REG("RenderTeapot",RenderTeapotHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderModel",RenderModelHandler::CreateHandler,ARIDE_RENDER);
	REG("RenderB21r",RenderB21rHandler::CreateHandler,ARIDE_RENDER);
#ifdef HAVE_LIBFEP
	REG("RenderVirtualFace",RenderVirtualFaceHandler::CreateHandler,ARIDE_RENDER);
#endif
	REG("RenderAxes",RenderAxesHandler::CreateHandler,ARIDE_RENDER);
}
