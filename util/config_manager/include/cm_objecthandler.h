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
#include <stdlib.h>

#ifndef ARIDE_OBJECTHANDLER_H
#define ARIDE_OBJECTHANDLER_H

#include <QString>

#include <ardev/ardev.h>

#include <vector>
using namespace std;

#include "cm_registry.h"
#include "cm_parameter.h"
#include "cm_parameter_ardev.h"

#include "cm_pluginhandler.h"

class ObjectHandler
{
	public:
		ObjectHandler() : Name("Name","ObjectName",""),
			Enabled("Enabled","Enable Object","True")
		{
			Parameters.push_back(&Name);
			Parameters.push_back(&Enabled);
		};
		virtual ~ObjectHandler() {};

		virtual void RemoveObject() {}; // destroys current object if it exists, re implement if the class supports the get object method
		QString Type;
		list<Parameter * > Parameters;

		NameParameter Name;
		BooleanParameter Enabled;
		virtual void SetEnabled (const bool) {};

		virtual void update()
		{
			for (list<Parameter * >::iterator itr = Parameters.begin(); itr != Parameters.end(); ++itr)
			{
				(*itr)->update();
			}
		};
};



template <class T>
class ArideObjectHandler : public ObjectHandler
{
	public:
		ArideObjectHandler() {};
		virtual ~ArideObjectHandler() {};
		virtual T & GetObject()=0; // returns current object creates if necessary
};

typedef ArideObjectHandler<CameraObject> CameraObjectHandler;
typedef ArideObjectHandler<CaptureObject> CaptureObjectHandler;
typedef ArideObjectHandler<OutputObject> OutputObjectHandler;
typedef ArideObjectHandler<PositionObject> PositionObjectHandler;
//typedef ArideObjectHandler<RenderObject> RenderObjectHandler;
typedef ArideObjectHandler<FrameProcessObject> FrameProcessObjectHandler;


class RenderObjectHandler : public ArideObjectHandler<RenderObject>
{
	public:
		RenderObjectHandler(){BaseObject = NULL;};
		virtual void SetEnabled (const bool state) {if(BaseObject) BaseObject->SetEnabled(state);};

		RenderObject * BaseObject;
};

/* ---------------------------------------------
    Handlers for environment and render pair
   --------------------------------------------- */

/*class RenderPairObjectHandler : public ObjectHandler
{
	public:
		RenderPairObjectHandler();
		~RenderPairObjectHandler();

		RenderObjectParameter *Render;
		PositionObjectParameter *Pos;
};*/

class EnvironmentObjectHandler : public ObjectHandler
{
	public:
		EnvironmentObjectHandler();
		~EnvironmentObjectHandler();

		OutputObjectParameter * Out;
		CameraObjectParameter * Cam;
		CaptureObjectParameter * Cap;
};

/* ---------------------------------------------
    Default Objects (such as constant and null)
   --------------------------------------------- */

template<class T,class Base>
class NullHandler : public Base
{
	public:
		NullHandler() {};

		T & GetObject() {CurrentProject->InitialiseObject(&obj);return obj;};
		void RemoveObject() {};

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new NullHandler<T,Base>);};

	protected:
		T obj;
};

//typedef NullHandler<CameraConstant,CameraObjectHandler> CameraConstantHandler;
//typedef NullHandler<CaptureNull,CaptureObjectHandler> CaptureNullHandler;
//typedef NullHandler<OutputNull,OutputObjectHandler> OutputNullHandler;
typedef NullHandler<PositionNull,PositionObjectHandler> PositionNullHandler;
//typedef NullHandler<RenderNull,RenderObjectHandler> RenderNullHandler;


/* -------------------------------------
    Camera Objects, ie file and v4l
   ------------------------------------- */
class CameraConstant;

class CameraConstantHandler : public CameraObjectHandler
{
	public:
		CameraConstantHandler();
		~CameraConstantHandler();

		CameraObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new CameraConstantHandler);};
	protected:
		CameraConstant * Camera;
		DoubleParameter * y_fov;
		DoubleParameter * aspect;
		DoubleParameter * SensorWidth;
		StringParameter * CalibrationFile;
		ARPointParameter * Origin;
		ARPointParameter * Direction;
		ARPointParameter * Up;
};


/* --------------------------------------------
    Output (X11/mpeg) Object Handlers
   -------------------------------------------- */
class OutputX11;
class OutputMovie;
class OutputManage;

class OutputX11Handler : public OutputObjectHandler
{
	public:
		OutputX11Handler();
		~OutputX11Handler();

		OutputObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new OutputX11Handler);};
	protected:
		OutputX11 * Obj;

		StringParameter * Name;
		CameraObjectParameter * Cam;
		PositionObjectParameter * CamPos;
		CaptureObjectParameter * Cap;
		IntParameter * Width;
		IntParameter * Height;
		StringParameter * DisplayName;
		BooleanParameter * FullScreen;
};

class OutputSharedX11Handler : public OutputObjectHandler
{
	public:
		OutputSharedX11Handler();
		~OutputSharedX11Handler();

		OutputObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new OutputSharedX11Handler);};
	protected:
		OutputX11 * Obj;

		StringParameter * Name;
		CameraObjectParameter * Cam;
		PositionObjectParameter * CamPos;
		CaptureObjectParameter * Cap;
		IntParameter * Width;
		IntParameter * Height;
		StringParameter * DisplayName;
		BooleanParameter * FullScreen;
};

/* --------------------------------------------
    OutputManage Object Handlers
   -------------------------------------------- */

class OutputManageHandler : public OutputObjectHandler
{
	public:
		OutputManageHandler();
		~OutputManageHandler();

		OutputObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new OutputManageHandler);};
	protected:
		OutputManage * Obj;

		CameraObjectParameter * Cam;
		PositionObjectParameter * CamPos;
		CaptureObjectParameter * Cap;
		IntParameter * Width;
		IntParameter * Height;
		StringParameter * DisplayName;
		BooleanParameter * FullScreen;
};

#ifdef HAVE_FFMPEG
class OutputMovieHandler : public FrameProcessObjectHandler
{
	public:
		OutputMovieHandler();
		~OutputMovieHandler();

		FrameProcessObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new OutputMovieHandler);};
	protected:
		OutputMovie * Obj;

		StringParameter Filename;
		IntParameter Width;
		IntParameter Height;
};
#endif
/* -------------------------------------
    Capture Objects, ie file and v4l
   ------------------------------------- */
class CaptureNull;
class CaptureFile;
class CaptureV4L;
class CaptureDC1394;

class CaptureNullHandler : public CaptureObjectHandler
{
	public:
		CaptureNullHandler();
		~CaptureNullHandler();

		CaptureObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new CaptureNullHandler);};
	protected:
		CaptureNull * Capture;
		ARColourParameter Background;
};

class CaptureFileHandler : public CaptureObjectHandler
{
	public:
		CaptureFileHandler();
		~CaptureFileHandler();

		CaptureObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new CaptureFileHandler);};
	protected:
		CaptureFile * Capture;
		StringParameter *Filename;
		IntParameter *Delay;
};

#ifdef HAVE_LINUX_VIDEODEV_H

class CaptureV4LHandler : public CaptureObjectHandler
{
	public:
		CaptureV4LHandler();
		~CaptureV4LHandler();

		CaptureObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new CaptureV4LHandler);};
	protected:
		CaptureV4L * Capture;
		StringParameter *Device;
		IntParameter *Width, *Height, *Channel, *Format;
};
#endif

#ifdef HAVE_DC1394
class CaptureDC1394Handler : public CaptureObjectHandler
{
	public:
		CaptureDC1394Handler();
		~CaptureDC1394Handler();

		CaptureObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new CaptureDC1394Handler);};
	protected:
		CaptureDC1394 * Capture;
};
#endif

/* -------------------------------------
    Basic Render Objects, ie axes and teapot
   ------------------------------------- */
class RenderTeapot;
class RenderAxes;
class RenderModel;
#ifdef WITH_LIBFEP
class RenderFace;
#endif

class RenderTeapotHandler : public RenderObjectHandler
{
	public:
		RenderTeapotHandler();
		~RenderTeapotHandler();

		RenderObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderTeapotHandler);};
	protected:
		RenderTeapot * obj;
		ARColourParameter Colour;
		DoubleParameter Size;
};

class RenderAxesHandler : public RenderObjectHandler
{
	public:
		RenderAxesHandler();
		~RenderAxesHandler();

		RenderObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderAxesHandler);};
	protected:
		RenderAxes * obj;
};

class RenderModelHandler : public RenderObjectHandler
{
	public:
		RenderModelHandler();
		~RenderModelHandler();

		RenderObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderModelHandler);};
	protected:
		RenderModel * obj;
		StringParameter ModelName;
		StringParameter TextureBase;
		DoubleParameter Scale;
};

class RenderB21r;

class RenderB21rHandler : public RenderObjectHandler
{
	public:
		RenderB21rHandler();
		~RenderB21rHandler();

		RenderObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderB21rHandler);};
	protected:
		RenderB21r * obj;
		BooleanParameter Visible;
};

#ifdef HAVE_LIBFEP

class RenderVirtualFaceHandler : public RenderObjectHandler
{
	public:
		RenderVirtualFaceHandler();
		~RenderVirtualFaceHandler();

		RenderObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderVirtualFaceHandler);};
	protected:
		RenderFace * obj;
		StringParameter ModelName;
};
#endif
/* -------------------------------------
    Basic Position Objects, ie constant
   ------------------------------------- */
class PositionConstant;
class PositionControllable;
class PositionRotate;

class PositionConstantHandler : public PositionObjectHandler
{
	public:
		PositionConstantHandler();
		~PositionConstantHandler();

		PositionObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new PositionConstantHandler);};
	protected:
		PositionConstant * obj;
		ARPositionParameter pos;
};

class PositionControllableHandler : public PositionObjectHandler
{
	public:
		PositionControllableHandler();
		~PositionControllableHandler();

		PositionObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new PositionControllableHandler);};
	protected:
		PositionControllable * obj;
		ARPositionParameter pos;
};

class PositionRotateHandler : public PositionObjectHandler
{
	public:
		PositionRotateHandler();
		~PositionRotateHandler();

		PositionObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new PositionRotateHandler);};
	protected:
		PositionRotate * obj;
		ARPositionParameter pos;
		ARPointParameter rate;
};

/* -------------------------------------
    Camera Objects, ie file and v4l
   ------------------------------------- */
class CalibratedPositionHandler : public PositionObjectHandler
{
	public:
		CalibratedPositionHandler();
		~CalibratedPositionHandler();

		PositionObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new CalibratedPositionHandler);};
	protected:
		PositionObject * obj;
		StringParameter * CalibrationFile;
};

class PluginHandler : public ObjectHandler
{
	public:
		PluginHandler(){Obj=NULL;}
		virtual ~PluginHandler(){};

		virtual ARObject & GetObject()
		{
			if (Obj == NULL)
			{
				Obj = new ARObject();
				if (Obj == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			}
			return *Obj;
		}
		virtual void RemoveObject(){delete Obj; Obj = NULL;};

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new PluginHandler);};
	protected:
		ARObject* Obj;
};

void RegisterDefaultObjectHandlers();

#endif
