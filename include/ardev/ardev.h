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

// Global Documentation

/**
\defgroup core_objects Core Objects

*/

#ifndef ARDEV_H
#define ARDEV_H

#include <stdlib.h>

/* This file decleares the 5 basic classes that are used for the
ardev Augmented Reality Development system interface */

/* Ardev is a class of static functions that can be used to control the
debugging process */

#include <ardev/ardev_types.h>

#include <pthread.h>
#include <math.h>
#include <list>
#include <set>
#include <GL/gl.h>

#include <ardev/ardevconfig.h>

#include <libthjc/geometry.h>
#include <libthjc/matrix.h>
#include <libthjc/misc.h>

#ifndef PI_OVER_ONEEIGHTY
#define PI_OVER_ONEEIGHTY (M_PI/180.0)
#endif

#ifndef RTOD
#define RTOD(x) (x*(180.0/M_PI))
#endif


using namespace std;



class RenderObject;
class PositionObject;
class TweakBar;

#define RENDERPAIR_POSITION_REVERSE 1

/** \brief Initialises ARDev, must be called once before using lib **/
void ARDevInit(int argc, const char **argv);

/** \brief A convienience grouping of a RenderObject pointer and a Position Object pointer,
 used for adding render objects to an ARDev environment. */
class RenderPair
{
	public:
		/// Construct a RenderPair from the given pointers and offset
		/// Flag Options are RENDERPAIR_POSITION_REVERSE or 0 for none
		RenderPair(){Rend = 0; Pos = 0; Flags=0;ThreadInitilised=NULL;Name=0;Pname=0;isAdded=false;};
		RenderPair(RenderObject * _R, PositionObject * _P,int _Flags = 0,const char* name=NULL,const char* pname=NULL)
		{
			Rend = _R; 	Pos = _P;
			Flags=_Flags;
			ThreadInitilised=false;
			Name=name?strdup(name):NULL;
			Pname=pname?strdup(pname):NULL;
			isAdded=false;
		};

		RenderPair(const RenderPair & render)
		{
			Rend = render.Rend; 	Pos = render.Pos;
			Flags= render.Flags;
			ThreadInitilised=false;
			Name=render.Name?strdup(render.Name):NULL;
			Pname=render.Pname?strdup(render.Pname):NULL;
			isAdded=false;
		}

		~RenderPair() {free(Name);free(Pname);};

		RenderPair& operator=(const RenderPair& rhs)
		{
			free(Name);
			free(Pname);
			Rend = rhs.Rend; 	Pos = rhs.Pos;
			Flags= rhs.Flags;
			//ThreadInitilised=false;
			Name=rhs.Name?strdup(rhs.Name):NULL;
			Pname=rhs.Pname?strdup(rhs.Pname):NULL;
			//isAdded=false;
			return *this;
		}

		/// True if both the pointers from the two RenderPairs are the same
		bool operator == (const RenderPair & rhs) {return Rend == rhs.Rend && Pos == rhs.Pos;};

		RenderObject * Rend; ///< The RenderObject
		PositionObject * Pos; ///< The PositionObject
		unsigned int Flags; ///< Flags for the render pair
		bool ThreadInitilised; ///< Has the threadInit function been called
		char* Name;
		char* Pname;
		bool isAdded; ///< Flag to prevent duplicates being added

		void Added(){isAdded = true;}
		void Removed(){isAdded= false;}
};



class OutputObject;
class ARObject;
class EventObject;

/** \brief The core ardev environment class provides management of an augmented reality debugging environment
 *
 * This includes starting and stopping the debugging output and adding and removing render objects from the environment
 */
class ARDev
{
	public:
		/// Default Constructor
		ARDev();
		/// Default Destructor
		~ARDev();
		/// The level of verbosity for debug output
		static int DebugLevel;

		/// Creates and starts a debugging environment with the given parameters, will resume an environment if it has been previously paused
		static int Start(OutputObject * output, const char * Name="default");
		/// Pause the named environment
		static int Pause(const char * Name="default");
		/// Resumes the named debugging environments
		static int Resume(const char * Name="default");
		/// Stops the named debugging environments
		static int Stop(const char * Name="default");
		/// Adds the given RenderPair to the named Environment (defaults to all)
		static int Add(RenderPair render, const char * Name="all");
		/// Remove the given RenderPair from the named Environment (defaults to all)
		static int Remove(RenderPair render, const char * Name="all");
		/// Lock the internal static data structures
		static void Lock();
		/// Unlock the internal static data structures
		static void Unlock();

		// rough way to get environment index
		static int GetEnvIndex(const char* Name);

		/// Notifies ARObjects of events
		static void PublishEvent(EventObject* event);
		/// Actually notifies ARObjects of events
		static void* EventWorker(void* obj);
		/// Register yourself for event notification
		static void RegisterForEvents(ARObject* obj);
		// Removes events on ardev stop
		static void CleanUpEvents();



	private:
		/// The list of actve debugging environments
		static list<OutputObject*> ActiveARList;
		/// Static Lock accessable via Lock and Unlock
		static pthread_mutex_t lock;

		/// The list of objects registered for event handling
		static set<ARObject*> EventHandlerList;
};

/** \brief Object type for events
 * For fast event enumeration
 *
 * OE  		Object Event
 * ROE  	Render Object Event
 * POE  	Position Object Event
 * RPE_ActArray Render Position for Act Array
 * Event on camera sucessful connect
 */
enum EventObjectType
{
	ROE_Clear,
	EOE_AbsoluteEnvironment,
	EOE_RelativeEnvironment,
	POE_SelectedDeltaPosition,
	POE_SelectedPosition,
	POE_CameraDeltaPosition,
	RPE_ActArray,
	RPE_TransformControls,
	ROE_ShowTransformControls,
	ROE_HideTransformControls,
	ROE_ResetControls,
	ROE_PickedObject,
	ROE_TransformMask,
	POE_Transform,
	MOE_Click,
	MOE_Move,
	OE_CaptureConnect,
	POE_DisconnectPlayerInterfaces,
	POE_AutoOnPlayerInterfaces,
	POE_AutoOffPlayerInterfaces,
	NONE
};

/** \brief EventObject wrapper class contains event type
 * Base class for all ObjectEvents
 */
class EventObject
{
	public:
		/// Constructor
		EventObject(EventObjectType type);
		// Constructor for events that may require alot of processing time
		EventObject(EventObjectType type, bool threaded);
		/// Default Destructor
		virtual ~EventObject() {};

		EventObjectType GetEventType() {return eventType;};
		bool Threaded() {return thread;};

	protected:
		EventObjectType eventType;
		bool thread;

};

/** \brief Position Object Event with arguments for modifying current position
 * 	Can be extended to provide additional functionality, setting position etc
 */
class PositionEventObject : public EventObject
{
	public:
		/// Constructor
		PositionEventObject(EventObjectType type,ARPoint deltaOrigin,ARPoint deltaDirection,ARPosition camera);
		PositionEventObject(EventObjectType type,ARPosition deltaPosition,ARPosition camera = ARPosition());
		/// Default Destructor
		virtual ~PositionEventObject() {};

		ARPosition GetDeltaPosition() {return deltaPosition;};
		ARPosition GetCameraPosition() {return cameraPosition;};

	protected:
		ARPosition deltaPosition;
		ARPosition cameraPosition;
};

class EnvironmentEventObject : public EventObject
{
	public:
		/// Constructor
		EnvironmentEventObject(EventObjectType type,int i);
		/// Default Destructor
		virtual ~EnvironmentEventObject() {};

		int GetRelativeIndex() {return index;};
		int GetAbsoluteIndex() {return index;};

	protected:
		int index;
};

/** \brief Position Object Event with arguments for modifying current position
 * 	Can be extended to provide additional functionality, setting position etc
 */
class RenderPositionEventObject : public EventObject
{
	public:
		/// Constructor
		RenderPositionEventObject(EventObjectType type,ARPosition deltaPosition, int index);
		/// Default Destructor
		virtual ~RenderPositionEventObject() {};

		ARPosition GetDeltaPosition() {return Pos;};
		int GetIndex() {return Index;};

	protected:
		ARPosition Pos;
		int Index;
};

/** \brief When an object is picked, broadcast object and selection within object
 */
class PickedEventObject : public EventObject
{
	public:
		/// Constructor
		PickedEventObject():EventObject(NONE){Picked=NULL;};
		PickedEventObject(EventObjectType type,ARPoint mouse, RenderObject* picked, ARPosition cam);
		PickedEventObject(EventObjectType type,ARPosition mask);
		/// Default Destructor
		virtual ~PickedEventObject() {};

		ARPoint GetMouseCoordinates() {return Mouse;};
		ARPosition GetRenderMask() {return Mask;};
		ARPosition GetCameraPosition() {return Cam;};
		RenderObject* GetPickedObject() {return Picked;};
		int GetIndex() {return Index;};

	protected:
		ARPosition Mask;
		ARPoint Mouse;
		RenderObject* Picked;
		ARPosition Cam;

		int Index;
};

/** \brief When an object is picked, broadcast object and selection within object
 */
class MouseEventObject : public EventObject
{
	public:
		/// Constructor
		MouseEventObject(EventObjectType type,ARPoint mouse, int button);
		/// Default Destructor
		virtual ~MouseEventObject() {};

		ARPoint GetMouseCoordinates() {return Mouse;};
		int GetButton() {return Button;};

	protected:
		ARPoint Mouse;
		int Button;
};

// ARDev Objects...all inherit from ARObject

/** \brief Base ARObject that all ARObjects inherit from.
 *
 * All the core ARDev Objects inherit from this class allowing the standard initilaisation update etc. Also
 * provides basic locking functionality
 */
class ARObject
{
	public:
		ARObject(); ///< Default Constructor
		virtual ~ARObject(); ///< Default Destructor

		/// Iinitialise the ARObjectR
		virtual int Initialise(bool Active = true) {if(!(IsActive = Active)) Pause(); initialised = true; return 0;};
		/// Terminate an ARObject
		virtual void Terminate() {initialised = false; if (thread) StopThread();};

		/// Create a thread for updating the object data
		void StartThread();
		/// Stop the objects update thread
		void StopThread();
		/// If using an update thread reimplement this function to contain the update loop
		virtual void * Main() {return NULL;};

		/// Pause the update thread loop if needed
		virtual void Pause();
		/// Resume the update thread loop if needed
		virtual void Resume();

		virtual void Lock(); ///< Locks the internal mutex
		virtual void Unlock(); ///< Unlocks the internal mutex

		/// is the device initialised
		virtual bool Initialised() {return initialised;};

		/// event handling requires the object to register
		virtual void Event(EventObject* event) {};
		/// register to receive events that occur
		void RegisterForEvents() { ARDev::RegisterForEvents(this); };

	protected:
		/// DummyMain that launches the threads Main method
		static void * DummyMain(void * ThisPtr);

		pthread_mutex_t lock; ///< Class specific mutex, accessed with Lock and Unlock
		pthread_t thread; ///< Active thread identifier if thread started
		bool IsActive; ///< record if the update thread is active

		bool initialised; ///< record the initialised state of the device

		friend class ARDev;
};



/* \brief Base class for all Misc Objects
 *
 */
typedef ARObject MiscObject;

/** \brief Base class for all Objects that are capable of capturing an input frame
 *
 * \ingroup core_objects
 */
class CaptureObject : public ARObject
{
	public:
	/// Default Constructor
	CaptureObject() {};
	/// Abstract member that returns the current Frame, Call update to capture a new frame
	virtual const ARImage &GetFrame() = 0;
	/// Did the last call to GetFrame return a fresh image
	virtual bool Fresh() {return true;};
	/// returns the maximum possible return frame width
	virtual int GetMaxWidth()=0;
	/// returns the maximum possible return frame height
	virtual int GetMaxHeight()=0;
};

/** \brief Null implementation of a capture object, simply returns Null Image
 */
class CaptureNull : public CaptureObject
{
	public:
		CaptureNull(ARColour background)
		{
			Frame.x_size = 640;
			Frame.y_size = 480;
			Frame.ColourFormat = GL_RGB;
			Frame.ByteDepth = 3;
			Frame.Allocate();
			for (unsigned y = 0; y < Frame.y_size; y++)
			{
				for (unsigned x = 0; x < Frame.x_size; x++)
				{
					Frame.GetPixel(3 * x, y) = (unsigned char)(background.r * 255);
					Frame.GetPixel(3 * x + 1, y) = (unsigned char)(background.g * 255);
					Frame.GetPixel(3 * x + 2, y) = (unsigned char)(background.b * 255);
				}
			}
			fresh = true; // we are fresh at startup
		}
		/// Return Null Image
		const ARImage & GetFrame()
		{
			fresh = false; // never be fresh again
			return Frame;
		}
		virtual bool Fresh() {return fresh;};
		/// returns the maximum possible return frame width
		virtual int GetMaxWidth() {return 640;};
		/// returns the maximum possible return frame height
		virtual int GetMaxHeight() {return 480;};

		ARImage Frame;
		bool fresh;
};

/** \brief Base class for all Objects that are capable of returning a ARCamera
 *
 */
class CameraObject : public ARObject
{
	public:
		/// Default Constructor
		CameraObject() {};
		/// Abstract Function to return current Camera paramters, use Update to get updated parameters
		virtual const ARCamera GetCamera() {return GetCameraRef();};
		/// Abstract Function to return current Camera reference
		virtual ARCamera & GetCameraRef() = 0;
};

/// \brief Null CameraObject Implementation
class CameraConstant : public CameraObject
{
	public:
		CameraConstant(const ARCamera & CamIn = ARCamera()) {Cam = CamIn;};
		/// Return a Null ARCamera with default values
		ARCamera & GetCameraRef() {return Cam;};

	private:
		ARCamera Cam;
};

/// \brief Base Class for objects that return an ARPosition
class PositionObject : public ARObject
{
	public:
		/// Default Constructor
		PositionObject() {present = true; Next = NULL; RegisterForEvents(); parent=NULL; name=NULL;};
		/// destructor
		virtual ~PositionObject() {};
		/// Abstract function that returns the current Position, call Update to get new position
		virtual ARPosition GetPosition() = 0;
		virtual void AdjustPosition(ARPosition position) = 0;
		virtual void SetPosition(ARPosition position){};
		/// Pointer to next position object if multiple tranformations are needed
		PositionObject * Next;
		/// Is the position object present, ie in fov for optical tracking
		virtual bool Present() {return present;};
		/// Calculates the full position chain transformation
		ARPosition GetTransformedPosition();
		/// Checks if all the position objects in the chain are present
		bool FullChainPresent();

		RenderPair* GetParent() {return parent;};
		void SetParent(RenderPair* Parent){parent = Parent;};

		ARPosition GetPositionMask(); ///< Masks which dimesions we can transform the object in, located on

		virtual void Event(EventObject* event);

		void SetName(const char* Name);
		const char* GetName() const;

	protected:
		RenderPair* parent;
		/// Is the position object present, ie in fov for optical tracking
		bool present;

		char* name;
};

/// \brief Null PositionObject Implementation
class PositionNull : public PositionObject
{
	public:
		/// return Null ARPosition Object
		ARPosition GetPosition() {return ARPosition();};
		virtual void AdjustPosition(ARPosition position){};
};

/// \brief Constant PositionObject Implementation
class PositionConstant : public PositionObject
{
	public:
		/// Specifies the position this object will always return
		PositionConstant(ARPosition in) {Pos = in;};
		/// return Null ARPosition Object
		virtual ARPosition GetPosition() {return Pos;};
		virtual void AdjustPosition(ARPosition position){ Pos += position;};
		virtual void SetPosition(ARPosition position){ Pos = position;};

		ARPosition Pos;
};

/// \brief Controllable PositionObject Implementation
class PositionControllable : public PositionObject
{
	public:
		/// Specifies the position this object will always return
		PositionControllable(ARPosition in) {Pos = in; RegisterForEvents();};
		/// return Null ARPosition Object
		virtual ARPosition GetPosition() {return Pos;};

		/// Adjust camera position using a point relative to the world reference frame
		virtual void AdjustPosition(ARPosition WRFPosition);

		/// Adjust camera position using a point relative to the camera reference frame
		virtual void AdjustPosition(ARPosition CRFPosition,ARPosition CameraPosition);

		virtual void Event(EventObject* event);

		ARPosition Pos;
};

/// \brief PositionObject Implementation in which
class PositionRenderable : public PositionControllable
{
	public:
		/// Specifies the position this object will always return
		PositionRenderable(ARPosition in, EventObjectType type, int index):PositionControllable(in) {Index=index; Type=type; };

		virtual void Event(EventObject* event)
		{
			if(event->GetEventType() == Type)
			{
				RenderPositionEventObject* e = reinterpret_cast<RenderPositionEventObject*>(event);
				if(e->GetIndex()==Index)
				{
					SetPositionWRF(e->GetDeltaPosition());
				}
			}
		}

		virtual void SetPositionWRF(ARPosition WRFPosition)
		{
			Pos = WRFPosition;
		}

		EventObjectType Type;
		//ARPosition Pos;
		int Index;
};

/// \brief Constant PositionObject Implementation
class PositionRotate : public PositionConstant
{
	public:
		/// Specifies the position this object will always return
		PositionRotate(ARPosition in, ARPoint _Rate) : PositionConstant(in) {Rate=_Rate;Timer.GetElapsedDouble();};
		/// return Null ARPosition Object
		ARPosition GetPosition()
		{
			double Elapsed=Timer.GetElapsedDouble()/1000000;
			Pos.Direction.x += Rate.x * Elapsed;
			Pos.Direction.y += Rate.y * Elapsed;
			Pos.Direction.z += Rate.z * Elapsed;
			return Pos;
		};

		ARPoint Rate; //< Rate in radians per second, for each axis

	protected:
		StopWatch Timer;

};

/** \brief FrameProcessObject provides an interface to access the captured frame. This object
 * can either be added to the pre process list to process the frame before AR items are added
 * (i.e. for vision based object tracking) or the post process list to access the final rendered
 * frame (i.e. for saving a video stream)

  ProcessFrame will be called once each time a new frame is captured from the cameras
  This is called in the main output loop, so It is up to the FrameProcessObject to
  make sure it returns immediately if it is still processing an old frame.
*/
class FrameProcessObject : public ARObject
{
	public:
		virtual void ProcessFrame(const ARImage & frame) = 0; ///< Process a single Frame
};

/// Current mouse Button related to buttonstate
enum MouseButton
{
	Left,
	Right,
	Middle,
	WheelUp,
	WheelDown,
	NA
};

/// Button state for mouse and keyboard
enum ButtonState
{
	Pressed,
	Released,
	Moved
};

typedef list<RenderPair> RenderList;
typedef vector<RenderPair*> RenderVector;
typedef list<FrameProcessObject *> PostProcessList;
typedef list<FrameProcessObject *> PreProcessList;

/** \brief Base class for objects that control a rendering pipeline and output the result to the user
*/
class OutputObject : public ARObject
{
	public:
		OutputObject(); ///< Default Constructor
		virtual ~OutputObject();
		virtual void RenderFrame(RenderList & List, const ARImage & Frame, const ARCamera & cam, const ARPosition & pos, unsigned int Tick, double * Distance=NULL); ///< Render a single Frame
		virtual RenderObject* PickingFrame(RenderList & List, const ARCamera & cam, const ARPosition & pos, int x, int y); ///< Render a single Frame in picking mode
		virtual const ARImage & GetFrame(); ///< Grab the current frame from the frame buffer and return it as a ARImage
		/// Display the current frame
		virtual void ShowFrame() {};
		bool MouseInput(int x, int y, MouseButton button,ButtonState state); ///< true if handled by gui
		bool KeyInput(char keycode,ButtonState state); ///< true if handled by gui
		//virtual void Event(RenderObjectEvent event); ///< Dispatch an event to all render objects.
		void WindowResized(int width, int height);

		int width; ///< Width of the output
		int height; ///< Height of the output
		unsigned int TexWidth; ///< Backdrop texture width (size of allocated tecture, ie mod2 rounded) current
		unsigned int TexHeight; ///< Backdrop texture height (size of allocated tecture, ie mod2 rounded) current
		unsigned int OldTexWidth; ///< Backdrop texture width (size of allocated tecture, ie mod2 rounded) previous
		unsigned int OldTexHeight; ///< Backdrop texture height (size of allocated tecture, ie mod2 rounded) previous
		unsigned int Backdrop; ///< Backdrop Texture Identifier

		/// Adds the given RenderPair to the list
		virtual void Add(RenderPair & render);
		/// Remove the given RenderPair
		virtual void Remove(RenderPair & render);

		/// Add the given Post Process Item
		void AddPost(FrameProcessObject * out);
		/// Remove the given Post Process Item
		void RemovePost(FrameProcessObject * out);

		/// Add the given PreProcessObject
		void AddPre(FrameProcessObject * out);
		/// Remove the given PreProcessObject
		void RemovePre(FrameProcessObject * out);

		virtual void Event(EventObject* event);
		void PublishEvent(EventObject* event);

		/// Called when a capture object connects, useful for setting up background textures etc
		virtual void CaptureConnect(){};

		static Vector3D UnProject(double x, double y);
		static Vector3D Project(double x, double y, double z);
		RenderVector Intersect(ARPosition& cam_pos, ARPoint world);
		static Vector3D PixelDirection(double x, double y); // Return the given pixel as a direction from the camera location

		/// Pick and the select objects using current mouse coordinates
		RenderObject* PickObject(int x, int y);
		void SelectPickedObject();

		const char * Name; ///< The name of the ARDev Object, used for calls to stop, add and remove
		bool Alive; ///< If this is true then the thread should clean up and terminate ASAP
		bool Paused; ///< If this is true then the thread should clean up and terminate ASAP

		// methods for changing the current environment when using shared output objects
		void AddEnvironment(const char* name);
		void RemoveEnvironment(const char* name);
		void RelativeEnvironment(int i);
		void AbsoluteEnvironment(int i);
		static int CurrentEnvironment() {return OutputObject::currentEnv;};

		void SetCloseCallback(void (*callback)(void*), void* data);

		//added to make FLTK work
		void loopFunc(bool& Loop);//does the loop which was previously in the main function
		void callInitGlut();

		void RenderSelectionTools(RenderPair render);

		bool spawnThread;//flag to spawn thread in ARDev::Start
		RenderObject* selected;
		RenderObject* pickedObj;

		// Selection Transform Controls
		RenderPair * controls; ///< pointer to the transform controls, possible redundant
		bool setupTransformControls; ///< have we setup the transform controls

	protected:

		/// The list of active RenderObjects, accessable via Add and Remove
		RenderList RenderObjects;
		/// The list of active PostProcessObjects, accessable via Add and Remove
		PostProcessList PostProcessObjects;
		/// The list of active PreProcessObjects, accessable via Add and Remove
		PreProcessList PreProcessObjects;

		CaptureObject * Capture; ///< The associated Capture Object
		CameraObject * Camera; ///< The associated Camera Object
		PositionObject * CameraPosition; ///< The position object that returns the position of the camera

		void *Main(); ///< main thread loop

		/// Utility function to draw text on the display
		void DrawText(GLint x, GLint y, char * s, GLfloat r, GLfloat g, GLfloat b);

		/// Benchmarking
		StopWatch TimerFPS;
		StopWatch Timer;
		double FrameRate;
		double OtherTime;
		double CaptureTime;
		double TextureTime;
		double PreprocessTime;
		double RenderTime;
		double PostprocessTime;

		static vector<CameraObject*> cameras; ///< list of cameras for each environment
		static vector<CaptureObject*> captures; ///< list of capture streams for each environment
		static vector<PositionObject*> camera_positions; ///< list of camera positions for each environment
		static vector<std::string> envNames; ///< environment names

		static int currentEnv; ///< current environment index
		static OutputObject* Shared; ///< the shared output object
		static int envFresh; ///< the envirnoment is fresh so the backfrop must be updated

		void ChangeEnvironment(); ///< Change environment to match the currentEnv variable

		void (*callback)(void*); ///< callback function called on close
		void* callbackData; ///< data to pass to callback function

		bool picked; ///< did we pick in the previous frame
#ifdef HAVE_ANTTWEAKBAR
    	int TWMOUSEWHEEL;
    	TweakBar* tweakBar;
#endif
};

class PickingColour
{
	public:
		/// \brief Create a default PickingColour, defaults to solid white
		PickingColour() {r=g=b=a=s=255;};
		/// \brief Create a PickingColour with colour and alpha
		//PickingColour(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a) {r=_r;g=_g;b=_b;a=_a;};
		/// \brief Create a PickingColour with only colour
		PickingColour(unsigned char _r, unsigned char _g, unsigned char _b)  {r=_r;g=_g;b=_b;a=255;s=255;};

		unsigned char r; ///< The Red Value
		unsigned char g; ///< The Green Value
		unsigned char b; ///< The Blue Value
		unsigned char a; ///< The Alpha Value ///Used for selected within an object
		unsigned char s; ///< Changes to indicate which alpha value occured when selected
};

/** \brief The base class for Objects that are able to render themselves in OpenGL
 */
class RenderObject : public ARObject
{
	public:
		/// Default Constructor
		RenderObject() {enabled = true; selectable=false; selected = false; Size=0.1; colourConfig=false; alphaConfig=false; parent=NULL;};
		/// Constructor setting the colour value
		RenderObject(const ARColour & aColour) {enabled = true; selectable=false; selected = false; Size=0.1; Colour = aColour; colourConfig=true; alphaConfig=true; parent=NULL;};
		/// Initialise thread specific data, ie gl textures
		/// This will be called once for each thread inside the thread context
		virtual void ThreadInit() {};
		/// Function that causes the object to render itself
		/// used to render a place holder for known real world parts of an object
		/// If using an update thread remember to use the Lock and Unlock functions when accessing shared data during rendering
		virtual void RenderBase() {};
		/// Function that causes the object to render the solid portions of itself
		/// If using an update thread remember to use the Lock and Unlock functions when accessing shared data during rendering
		virtual void Render() {};
		/// Function that causes the object to render the transparent portions of itself
		/// If using an update thread remember to use the Lock and Unlock functions when accessing shared data during rendering
		virtual void RenderTransparent() {};
		/// Function that causes an object to draw a bounding volume for the purpose of picking
		/// NOTE: Render a non coloured bounding object
		/// If using an update thread remember to use the Lock and Unlock functions when accessing shared data during rendering
		virtual void RenderBounding() {};
		/// Returns the shortest distance from a Ray to the object (-1 for no itnersection)
		virtual double TraceDistance(const Ray & Offset, const Ray & R, const ARPoint & Rotation);
		/// Is the object enabled, i.e. should it be rendered. allows for user to select which
		/// components are rendered at run time.
		virtual bool Enabled() {return enabled;}
		/// Is the object enabled, i.e. should it be rendered. allows for user to select which
		/// components are rendered at run time.
		virtual void SetEnabled(const bool EnableState)
		{
			enabled = EnableState;
			//if (!enabled && initialised)
			//	Terminate(); //this isn't necessary as it causes the app to pause
			if (enabled && !initialised)
				Initialise();
		}
		/// Is the object selected
		virtual bool Selected() {return selected;}
		/// Select or deselect the object
		virtual void SetSelected(const bool selectState){selected = selectState;}

		/// Is the object able to be selected
		virtual bool Selectable() { return selectable; }
		virtual void SetSelectable(const bool SelectState);

		/// Notifies the object of an event
		/// If using an update thread remember to use the Lock and Unlock functions when accessing shared data

		virtual void Event(EventObject* event) {};

		virtual void SetColour(ARColour colour){Colour = colour;}
		virtual ARColour GetColour() {return Colour;}

		// useful to know if a renderobject actually uses colour and alpha when rendering
		// needs to be manually set
		bool colourConfig;
		bool alphaConfig;

		/// For render object specifi gui information/ tweaks
		virtual void DisplaySetup(TweakBar* tweakBar, const char* Name){}

		/// Gets the RenderObjects Parent
		RenderPair* GetParent() {return parent;};
		/// Sets the RenderObjects Parent, should only be set on creation of the renderpair
		void SetParent(RenderPair* Parent){parent = Parent;};
		/// Get the unique picking colour for picking with
		PickingColour GetPickingColour() {return pickingColour;};

		ARPosition MovementMask; ///< stores a mask for adjustable movement, allows its position to move if the position allows movement

	protected:
		RenderPair* parent;/// pointer to the renderpair parent
		double Size; ///< size
		bool enabled; ///< enabled
		bool selectable; ///< selectable
		bool selected; ///< currently selected
		ARColour Colour; ///< Render Colour
	private:
		PickingColour pickingColour; ///< unique colour used for picking
		static PickingColour picking; ///< used to assign a unique picking colour
};

/// \brief Basic rendering of a pair of lines from the origin along the X and Z axes
class RenderAxes : public RenderObject
{
	public:
		RenderAxes() : RenderObject() {colourConfig=false;};
		/// Render the X, Y and Z axes
		void Render()
		{
			glBegin(GL_LINES);
			glColor3f(0,1,1); // Cyan -> X
			glVertex3f(0,0,0);
			glVertex3f(1.000,0,0);
			glColor3f(1,0,1); // Magenta -> Y
			glVertex3f(0,0,0);
			glVertex3f(0,1.000,0);
			glColor3f(1,1,0); // Yellow -> Z
			glVertex3f(0,0,0);
			glVertex3f(0,0,1.000);
			glEnd();
		}
};

#endif
