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
/*
Implementation of core classes ARDev RenderObject CaptureObject OutputObject PosistionObject
*/

#include <string.h>

#include <ardev/ardev.h>
#include <ardev/render_base.h>
#include <ardev/debug.h>

#include <libthjc/matrix.h>
#include <libthjc/misc.h>
#include <libthjc/camera.h>
#include <libthjc/vector3d.h>

#include <config.h>
#include <fstream>
#include <algorithm>
#include <unistd.h>

#include <pthread.h>
#include <GL/glut.h>
#include <map>

#include <ardev/anttweakbar.h>

void ARDevInit(int argc, const char *argv[])
{
	glutInit(&argc,const_cast<char**> (argv));
}
//added this function so an object of OutputObject and call this from its child class
void OutputObject::callInitGlut(){

	const char * Temp = "";
	ARDevInit(1,&Temp);
}

// -------------------------------------------------------
// AR Object
// -------------------------------------------------------

ARObject::ARObject()
{
	IsActive = false;
	initialised = false;
	thread = 0;

	if(pthread_mutex_init(&lock, NULL) < 0)
		dbg_print(ARDBG_ERR,"Unable to initialise mutex\n");
}

ARObject::~ARObject()
{
	if(thread)
	{
		StopThread();
	}
	if(pthread_mutex_destroy(&lock) < 0)
		dbg_print(ARDBG_ERR,"Unable to destroy mutex\n");

}

void ARObject::Lock()
{
	dbg_print(ARDBG_THREAD,"Locking Object %p\n",this);
	if(pthread_mutex_lock(&lock) < 0)
		dbg_print(ARDBG_ERR,"Unable to lock mutex %p\n",this);
	dbg_print(ARDBG_THREAD,"Locked Object %p\n",this);
}

void ARObject::Unlock()
{
	dbg_print(ARDBG_THREAD,"Unlocking Object %p\n",this);
	if(pthread_mutex_unlock(&lock) < 0)
		dbg_print(ARDBG_ERR,"Unable to unlock mutex %p\n",this);
	dbg_print(ARDBG_THREAD,"Unlocked Object %p\n",this);
}


void ARObject::StartThread()
{
	int err;
	if ((err=pthread_create(&thread,NULL,ARObject::DummyMain,this)))
	{
		dbg_print(ARDBG_ERR,"failed to create thread: %d\n",err);
	}
}

void ARObject::StopThread()
{
	if (thread)
	{
		if (pthread_cancel(thread) != 0)
			dbg_print(ARDBG_ERR, "Attempted to stop thread that doesn't exist\n");
		pthread_join(thread,NULL);
		thread = 0;
	}
	if (!IsActive)
	{
		Resume();
	}
}

void * ARObject::DummyMain(void * ThisPtr)
{
	return reinterpret_cast<ARObject *> (ThisPtr)->Main();
}

void ARObject::Pause()
{
	if (IsActive)
	{
		Lock();
		IsActive = false;
	}
}

void ARObject::Resume()
{
	if (!IsActive)
	{
		IsActive = true;
		Unlock();
	}
}



// -------------------------------------------------------

ARPoint & ARPoint::RotateX(double angle)
{
	double newY, newZ;
	newY = (y * cos (angle)) - (z * sin (angle));
	newZ = (y * sin (angle )) + (z * cos (angle));
	y = newY;
	z = newZ;
	return *this;
}

ARPoint & ARPoint::RotateY(double angle)
{
	double newX, newZ;
	newX = (x * cos (angle)) + (z * sin (angle));
	newZ = (-x * sin (angle)) + (z * cos (angle));
	x = newX;
	z = newZ;
	return *this;
}

ARPoint & ARPoint::RotateZ(double angle)
{
	double newX, newY;
	newX = (x * cos (angle)) - (y * sin (angle));
	newY = (x * sin (angle)) + (y * cos (angle));
	x = newX;
	y = newY;
	return *this;
}

ARPoint & ARPoint::RotateYPR(double Y, double P, double R)
{

	Matrix Rot(3,3);
	Rot.SetValue(0,0,cos(P)*cos(Y));
	Rot.SetValue(0,1,cos(P)*sin(Y));
	Rot.SetValue(0,2,-sin(P));

	Rot.SetValue(1,0,sin(R)*sin(P)*cos(Y) - cos(R)*sin(Y));
	Rot.SetValue(1,1,sin(R)*sin(P)*sin(Y) + cos(R)*cos(Y));
	Rot.SetValue(1,2,cos(P)*sin(R));

	Rot.SetValue(2,0,cos(R)*sin(P)*cos(Y) + sin(R)*sin(Y));
	Rot.SetValue(2,1,cos(R)*sin(P)*sin(Y) - sin(R)*cos(Y));
	Rot.SetValue(2,2,cos(P)*cos(R));

	Matrix Point(3,1);
	Point.SetValue(0,0,x);
	Point.SetValue(1,0,y);
	Point.SetValue(2,0,z);

	Point = Rot*Point;
	x = Point.GetValue(0,0);
	y = Point.GetValue(1,0);
	z = Point.GetValue(2,0);

	return *this;
}
ARPoint & ARPoint::RotateRPY(double R, double P, double Y)
{
	Matrix Rot(3,3);
	Rot.SetValue(0,0,cos(Y)*cos(P));
	Rot.SetValue(0,1,sin(Y)*cos(R)+cos(Y)*sin(P)*sin(R));
	Rot.SetValue(0,2,sin(Y)*sin(R)-cos(Y)*sin(P)*cos(R));

	Rot.SetValue(1,0,-sin(Y)*cos(P));
	Rot.SetValue(1,1, cos(Y)*cos(R)-sin(Y)*sin(P)*sin(R));
	Rot.SetValue(1,2, cos(Y)*sin(R)+sin(Y)*sin(P)*cos(R));

	Rot.SetValue(2,0, sin(P));
	Rot.SetValue(2,1,-cos(P)*sin(R));
	Rot.SetValue(2,2, cos(P)*cos(R));

	Matrix Point(3,1);
	Point.SetValue(0,0,x);
	Point.SetValue(1,0,y);
	Point.SetValue(2,0,z);

	Point = Rot*Point;
	x = Point.GetValue(0,0);
	y = Point.GetValue(1,0);
	z = Point.GetValue(2,0);

	return *this;
}

double ARPoint::DotProduct(const ARPoint & rhs)
{
	return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
}

// -------------------------------------------------------

ARPosition PositionObject::GetTransformedPosition()
{
	dbg_print(ARDBG_VERBOSE,"Transform Position\n");
	ARPosition CombinedPosition = this->GetPosition();
	for (PositionObject * ThisPos = this->Next; ThisPos != NULL; ThisPos = ThisPos->Next)
	{
		CombinedPosition.Transform(ThisPos->GetPosition());
	}
	dbg_print(ARDBG_VERBOSE,"Transformed Position (%f %f %f)(%f %f %f)\n",CombinedPosition.Origin.x,CombinedPosition.Origin.y,CombinedPosition.Origin.z,CombinedPosition.Origin.y,CombinedPosition.Direction.x,CombinedPosition.Direction.y,CombinedPosition.Direction.z);
	return CombinedPosition;
}

bool PositionObject::FullChainPresent()
{
	bool AllPresent = true;
	for (PositionObject * ThisPos = this; ThisPos != NULL; ThisPos = ThisPos->Next)
	{
		if (!ThisPos->Present())
		{
			AllPresent = false;
			break;
		}
	}
	return AllPresent;
}

void PositionObject::Event(EventObject* event)
{
	if(event->GetEventType() == POE_SelectedDeltaPosition)
	{
		// if have renderobject
		if(GetParent() && GetParent()->Rend)
		{
			// if selected
			if(GetParent()->Rend->Selected())
			{

				PositionEventObject* e = dynamic_cast<PositionEventObject*>(event);
				ARPosition delta = e->GetDeltaPosition();
				ARPosition masked = ARPosition(
						ARPoint(
							delta.Origin.x,
							delta.Origin.y,
							delta.Origin.z ),
						ARPoint(
							GetPositionMask().Direction.x*delta.Direction.x,
							GetPositionMask().Direction.y*delta.Direction.y,
							GetPositionMask().Direction.z*delta.Direction.z));
				AdjustPosition(masked);

			}
		}
	}
	else if(event->GetEventType() == POE_SelectedPosition)
	{
		if(GetParent() && GetParent()->Rend)
		{
			// if selected
			if(GetParent()->Rend->Selected())
			{

				PositionEventObject* e = dynamic_cast<PositionEventObject*>(event);
				ARPosition delta = e->GetDeltaPosition();
				SetPosition(delta);

			}
		}
	}
}

ARPosition PositionObject::GetPositionMask()
{
	if(GetParent()->Rend)
		return GetParent()->Rend->MovementMask;
	return ARPosition();
}

void PositionObject::SetName(const char* Name)
{
	if (name)
		free(name);
	name = strdup(Name);
}

const char* PositionObject::GetName() const
{
	return name;
}

// -------------------------------------------------------

void PositionControllable::AdjustPosition(ARPosition WRFPosition)
{
	Pos += WRFPosition;
}

void PositionControllable::AdjustPosition(ARPosition CRFPosition,ARPosition CameraPosition)
{
	// Rotate vector from world to camera rotation
	Pos.Origin += CRFPosition.Origin.RotateRPY(CameraPosition.Direction.x,CameraPosition.Direction.y,CameraPosition.Direction.z);
	Pos.Direction += CRFPosition.Direction;
}

void PositionControllable::Event(EventObject* event)
{
	if(event->GetEventType() == POE_CameraDeltaPosition)
	{
		PositionEventObject* e = dynamic_cast<PositionEventObject*>(event);
		AdjustPosition(e->GetDeltaPosition(),e->GetCameraPosition());
	}
}

// -------------------------------------------------------

EventObject::EventObject(EventObjectType type)
{
	eventType = type;
	thread = false;
}

EventObject::EventObject(EventObjectType type, bool threaded)
{
	eventType = type;
	thread = threaded;
}

PickedEventObject::PickedEventObject(EventObjectType type,ARPoint mouse,RenderObject* picked, ARPosition cam): EventObject(type)
{
	Mouse = mouse;
	Picked = picked;
	Cam = cam;
}

PickedEventObject::PickedEventObject(EventObjectType type, ARPosition mask) : EventObject(type)
{
	Mask = mask;
	Picked = NULL;
}

PositionEventObject::PositionEventObject(EventObjectType type, ARPoint deltaOrigin, ARPoint deltaDirection, ARPosition camera) : EventObject(type)
{
	deltaPosition = ARPosition(deltaOrigin,deltaDirection);
	cameraPosition = camera;
}

PositionEventObject::PositionEventObject(EventObjectType type, ARPosition _deltaPosition, ARPosition camera) : EventObject(type) {

	deltaPosition = _deltaPosition;
	cameraPosition = camera;
}

EnvironmentEventObject::EnvironmentEventObject(EventObjectType type, int i) : EventObject(type)
{
	index = i;
}

RenderPositionEventObject::RenderPositionEventObject(EventObjectType type, ARPosition deltaPos, int i) : EventObject(type)
{
	Index = i;
	Pos = deltaPos;
}

MouseEventObject::MouseEventObject(EventObjectType type, ARPoint mouse, int button) : EventObject(type)
{
	Mouse = mouse;
	Button = button;
}

// -------------------------------------------------------

list<OutputObject *> ARDev::ActiveARList;
set<ARObject*> ARDev::EventHandlerList;
pthread_mutex_t ARDev::lock = PTHREAD_MUTEX_INITIALIZER;
//PositionObject * ARDev::LastPositionObject = NULL;
//#ifdef DEBUG
int ARDev::DebugLevel = ARDBG_INFO;
//#endif

ARDev::ARDev()
{
	if (pthread_mutex_init(&lock, NULL) < 0)
		dbg_print(ARDBG_ERR, "Unable to initialise ARDev mutex\n");

}

ARDev::~ARDev()
{
}


void ARDev::Lock()
{
	if (pthread_mutex_lock(&ARDev::lock) < 0)
		dbg_print(ARDBG_ERR, "Unable to lock ARDev mutex\n");
}

void ARDev::Unlock()
{
	if (pthread_mutex_unlock(&ARDev::lock) < 0)
		dbg_print(ARDBG_ERR, "Unable to unlock ARDev mutex\n");
}

int ARDev::Start(OutputObject * output, const char * Name)
{
	output->Name = strdup(Name);
	output->Alive = true;

	for (list<OutputObject *>::iterator i = ARDev::ActiveARList.begin(); i != ARDev::ActiveARList.end(); i++)
		if (*i == output) // already active
			return 0;
	ARDev::ActiveARList.push_back(output);
	if(output->spawnThread==true){
		// spawn thread to run the output in
		if (pthread_create( &output->thread, NULL, OutputObject::DummyMain, reinterpret_cast<void *> (output)) < 0)
			dbg_print(ARDBG_ERR, "Failed to create Thread\n");
	}
	return 0;
}

int ARDev::Stop(const char * Name)
{
	dbg_print(ARDBG_INFO,"Request to Stop ardev object %s\n",Name);
	bool found = false;
	// lock the main list
	ARDev::Lock();

	// locate the ARDev thread to stop
	for (list<OutputObject*>::iterator itr = ARDev::ActiveARList.begin(); itr != ARDev::ActiveARList.end(); ++itr)
	{
		if (strcmp((*itr)->Name, Name)==0)
		{
			dbg_print(ARDBG_INFO,"Stopping ardev object %s\n",Name);
			found = true;
			(*itr)->Lock();
			(*itr)->Alive = false;
			(*itr)->Unlock();
			//pthread_cancel(((*itr)->thread)); // we leave the thread to terminate itself but set alive = false so it does
			if ((*itr)->spawnThread)
				pthread_join(((*itr)->thread),NULL);
			//(*itr)->Output->Terminate(); // the thread needs to initialise its own gl object...and kill it
			(*itr)->thread = 0; // Let other things know the thread is no longer valid.
			dbg_print(ARDBG_VERBOSE,"Stopped ardev object %s\n",Name);
			ARDev::ActiveARList.remove(*itr);
			break;

			//delete *itr;
		}

	}

	// Removes events for removed objects (all)
	ARDev::CleanUpEvents();

	// unlock
	ARDev::Unlock();
	if (found)
		return 0;
	dbg_print(ARDBG_WARN,"Could not find ardev %s\n",Name);
	return -1;
}

int ARDev::Pause(const char * Name)
{
	dbg_print(ARDBG_INFO,"Request to Pause ardev object %s\n",Name);
	bool found = false;

	// lock the main list
	ARDev::Lock();

	// locate the ARDev thread to stop
	for (list<OutputObject*>::iterator itr = ARDev::ActiveARList.begin(); itr != ARDev::ActiveARList.end(); ++itr)
	{
		if (strcmp((*itr)->Name, Name)==0)
		{
			dbg_print(ARDBG_INFO,"Pausing ardev object %s\n",Name);
			found = true;
			(*itr)->Lock();
			(*itr)->Paused = true;
			(*itr)->Unlock();
			dbg_print(ARDBG_VERBOSE,"Paused ardev object %s\n",Name);
			break;
		}
	}

	// unlock
	ARDev::Unlock();
	if (found)
		return 0;
	dbg_print(ARDBG_WARN,"Could not find ardev %s\n",Name);
	return -1;
}

int ARDev::Resume(const char * Name)
{
	dbg_print(ARDBG_INFO,"Request to Resume ardev object %s\n",Name);
	bool found = false;

	// lock the main list
	ARDev::Lock();

	// locate the ARDev thread to stop
	for (list<OutputObject*>::iterator itr = ARDev::ActiveARList.begin(); itr != ARDev::ActiveARList.end(); ++itr)
	{
		if (strcmp((*itr)->Name, Name)==0)
		{
			dbg_print(ARDBG_INFO,"Resuming ardev object %s\n",Name);
			found = true;
			(*itr)->Lock();
			(*itr)->Paused = false;
			(*itr)->Unlock();
			dbg_print(ARDBG_VERBOSE,"Resumed ardev object %s\n",Name);
			break;
		}
	}

	// unlock
	ARDev::Unlock();
	if (found)
		return 0;
	dbg_print(ARDBG_WARN,"Could not find ardev %s\n",Name);
	return -1;
}

int ARDev::Add(RenderPair render, const char * Name)
{
	bool found = false;
//	dbg_print(ARDBG_INFO,"Finding ARDev to add Object to\n");

	// lock
	ARDev::Lock();

	// locate the ARDev thread(s) to add the renderobject to
	for (list<OutputObject*>::iterator itr = ARDev::ActiveARList.begin(); itr != ARDev::ActiveARList.end(); ++itr)
	{
		if (strcmp((*itr)->Name, Name)==0 || strcmp(Name,"all") == 0)
		{
			dbg_print(ARDBG_INFO,"Adding RenderObject to :%s\n",(*itr)->Name);
			found = true;
			(*itr)->Add(render);
		}
	}

	//unlock
	ARDev::Unlock();

	if (found == true)
		return 0;
	return -1;
}

int ARDev::Remove(RenderPair render, const char * Name)
{
	bool found = false;

	// lock
	ARDev::Lock();

	// locate the ARDev thread(s) to remove the renderobject to
	for (list<OutputObject*>::iterator itr = ARDev::ActiveARList.begin(); itr != ARDev::ActiveARList.end(); ++itr)
	{
		if (strcmp((*itr)->Name, Name)==0 || strcmp(Name,"all")==0)
		{
			found = true;
			(*itr)->Remove(render);
		}
	}

	//unlock
	ARDev::Unlock();

	if (found == true)
		return 0;
	return -1;
}

int ARDev::GetEnvIndex(const char * Name)
{
	// lock the main list
	ARDev::Lock();
	int i=0;
	for (list<OutputObject*>::iterator itr = ARDev::ActiveARList.begin(); itr != ARDev::ActiveARList.end(); ++itr, i++)
		if (strcmp((*itr)->Name, Name)==0)
			return i;

	return -1;
}

/// Events are passed down to ARObjects who have registered to receive events
void ARDev::PublishEvent(EventObject* event)
{
	pthread_t eventThread;
	//spawn thread to handle event
	if(event->Threaded())
		pthread_create(&eventThread,NULL,ARDev::EventWorker,event);
	else
		EventWorker(event);
}

/// Actually passes out events
void* ARDev::EventWorker(void* obj)
{
	EventObject* event = reinterpret_cast<EventObject*>(obj);
	for (set<ARObject*>::iterator itr = ARDev::EventHandlerList.begin(); itr != ARDev::EventHandlerList.end(); ++itr)
		(*itr)->Event(event);

	delete event;
	return NULL;
}

/// Objects register themselves for an event callback
void ARDev::RegisterForEvents(ARObject* obj)
{
	EventHandlerList.insert(obj);
}
/// Removes all objects registered in the event list
void ARDev::CleanUpEvents()
{
	ARDev::EventHandlerList.clear();
}
// -------------------------------------------------------
// Output Object
// -------------------------------------------------------

// Statics for shared OutputObjects
int OutputObject::currentEnv;
vector<string> OutputObject::envNames;
vector<CameraObject*> OutputObject::cameras;
vector<CaptureObject*> OutputObject::captures;
vector<PositionObject*> OutputObject::camera_positions;
int OutputObject::envFresh = false;

OutputObject::OutputObject() : TimerFPS(30)
{
	width=640;height=480;
	Paused = false;

	Camera = NULL;
	Capture = NULL;
	CameraPosition = NULL;

	OutputObject::currentEnv = 0;
	RegisterForEvents();

	selected = NULL;
	pickedObj = NULL;
	spawnThread = true;//sets the flag to true to spawn thread in ARDev::Start

	picked = false;
	setupTransformControls = false;

	TexWidth=OldTexWidth=0;
	TexHeight=OldTexHeight=0;
#ifdef HAVE_ANTTWEAKBAR
	tweakBar = NULL;
	TWMOUSEWHEEL = 0;
#endif
}

OutputObject::~OutputObject()
{
	
}

void OutputObject::Add(RenderPair & render)
{
	Lock();
	if(!render.isAdded)
	{
		RenderObjects.push_back(render);
		render.Added();
	}
	Unlock();
}

void OutputObject::Remove(RenderPair & render)
{
	Lock();
	if(render.isAdded)
	{
		RenderObjects.remove(render);
		render.Removed();
	}
	Unlock();
}

/// Add the given SecondaryOutputObject
void OutputObject::AddPost(FrameProcessObject * out)
{
	Lock();
	PostProcessObjects.push_back(out);
	Unlock();
}

/// Remove the given SecondaryOutputObject
void OutputObject::RemovePost(FrameProcessObject * out)
{
	Lock();
	PostProcessObjects.remove(out);
	Unlock();
}

/// Add the given PreProcessObject
void OutputObject::AddPre(FrameProcessObject * out)
{
	Lock();
	PreProcessObjects.push_back(out);
	Unlock();
}

/// Remove the given PreProcessObject
void OutputObject::RemovePre(FrameProcessObject * out)
{
	Lock();
	PreProcessObjects.remove(out);
	Unlock();
}

void OutputObject::AddEnvironment(const char * name)
{
	Lock();
	int envIndex = envNames.size();
	envNames.push_back(string(name));
#ifdef HAVE_ANTTWEAKBAR
	if (tweakBar != NULL)
		tweakBar->addEnvironment(envIndex,envNames[envIndex].c_str());
#endif
	Unlock();
}

void OutputObject::RemoveEnvironment(const char* name)
{
	Lock();
	vector<string>::iterator i = std::find(OutputObject::envNames.begin(), OutputObject::envNames.end(), string(name));
	if (i != OutputObject::envNames.end())
		OutputObject::envNames.erase(i);
	Unlock();
}

void * OutputObject::Main(){

	bool LoopOn = true;

	if(!Initialise(true))
	{
#ifdef HAVE_ANTTWEAKBAR
		TweakBar* newTweakBar = new TweakBar; // Now that the window is open, start anttweakbar
		Lock();
		tweakBar = newTweakBar;
		TWMOUSEWHEEL =0;
		// Add any environments which showed up before the gui was initialised.
		for (unsigned i = 0; i < envNames.size(); i++)
			tweakBar->addEnvironment(i,envNames[i].c_str());
		Unlock();
		tweakBar->removeDefaultEnv();
		tweakBar->generateDisplayMenu(RenderObjects);
#endif

		// Initialise the Output Module
		dbg_print(ARDBG_VERBOSE,"Calling Output Object Initialiser\n");

		// loop forever
		dbg_print(ARDBG_INFO,"Entering Main Loop for: %s\n", Name);

		while(LoopOn){
			loopFunc(LoopOn);
		}

#ifdef HAVE_ANTTWEAKBAR
		RemoveEnvironment(Name); // Remove self
		delete tweakBar; // Before we close the window, shut down the tweak bar
#endif

		Terminate();
	}
	else
	{
		while(LoopOn)
		{
			Lock();
			LoopOn = Alive;
			if (Paused)
			{
				Unlock();
				usleep(1000);
				continue;
			}
			Unlock();

			pthread_testcancel();
			usleep(1000);
		}
	}

	dbg_print(ARDBG_INFO,"Thread Exiting...\n");
	return NULL;
}

void OutputObject::loopFunc(bool& Loop){

	static bool first = true;
	bool LoopOn;
	static unsigned int Tick;
	static int GlError;

	if (first){
		Tick = 0;

		if (!(Loop)){
			LoopOn = true;
		}

		first = false;
	}

	if(Loop){
		LoopOn = Loop;
	}

	Lock();
	LoopOn = Alive;
	if (Paused)
	{
		Unlock();
		usleep(1000);
		return;
	}

	if(Loop){
		Loop = LoopOn;
	}

	Unlock();
	pthread_testcancel();

	++Tick;

	// start benchmarking from here
	FrameRate = TimerFPS.GetFrameRate();
	OtherTime = Timer.GetElapsedSeconds();

	dbg_print(ARDBG_VERBOSE,"Starting Tick %d\n",Tick);
	while((GlError=glGetError()) != GL_NO_ERROR)
		dbg_print(ARDBG_ERR,"Got Gl Error in past %d - %s\n",GlError,gluErrorString(GlError));

	// Get new Image
	dbg_print(ARDBG_VERBOSE,"Getting Frame from Capture Device\n");

	CaptureTime = 0;
	PreprocessTime = 0;

	Capture->Lock();
	bool camFresh = Capture->Fresh();
	const ARImage & im = Capture->GetFrame();
	dbg_print(ARDBG_VERBOSE,"Got Frame from Capture Device\n");
	if (camFresh || envFresh)
	{
		envFresh = false;
		CaptureTime = Timer.GetElapsedSeconds();

		// Load the new texture into openGL
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, Backdrop);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, im.x_size);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,im.x_size,im.y_size,im.ColourFormat,GL_UNSIGNED_BYTE,im.data);
		glDisable(GL_TEXTURE_2D);
		TextureTime = Timer.GetElapsedSeconds();

		// now process secondary output objects
		for (PreProcessList::iterator itr = PreProcessObjects.begin(); itr != PreProcessObjects.end(); ++itr)
		{
			dbg_print(ARDBG_VERBOSE,"Passing frame to preprocess object: %p\n",*itr);
			(*itr)->ProcessFrame(im);
		}
		PreprocessTime = Timer.GetElapsedSeconds();
	}
	Capture->Unlock();

	while((GlError=glGetError()) != GL_NO_ERROR)
	{
		dbg_print(ARDBG_ERR,"Got Gl Error filling texture %d - %s\n",GlError,gluErrorString(GlError));
		//assert(0);
	}

	dbg_print(ARDBG_VERBOSE,"Getting Camera Info\n");
	ARCamera & pos = Camera->GetCameraRef();

	// Render Output
	Lock();

	dbg_print(ARDBG_VERBOSE,"render frame\n");
	double Distance;
	double * pDistance = NULL;
	if (pos.CamType & AR_CAMTYPE_CONV_GL && pos.convergance!=NULL)
	{
		Distance = *pos.convergance;
		pDistance = &Distance;
	}

	RenderFrame(RenderObjects,im,pos,CameraPosition->GetTransformedPosition(),Tick, pDistance);
	RenderTime = Timer.GetElapsedSeconds();

	// now process secondary output objects
	if (!PostProcessObjects.empty())
	{
		// grab gl frame
		ARImage PostFrame;
		PostFrame.x_size = width;
		PostFrame.y_size = height;
		PostFrame.ByteDepth = 3;
		PostFrame.ColourFormat = GL_RGB;
		PostFrame.Allocate();
		glReadPixels(0,0,width, height, GL_RGB, GL_UNSIGNED_BYTE,static_cast<void*> (PostFrame.data));

		for (PostProcessList::iterator itr = PostProcessObjects.begin(); itr != PostProcessObjects.end(); ++itr)
		{
			(*itr)->ProcessFrame(PostFrame);
		}
	}
	PostprocessTime = Timer.GetElapsedSeconds();

	dbg_print(ARDBG_VERBOSE,"rendered frame\n");
	if (pDistance!=NULL)
		*pos.convergance = Distance;

	if(!picked)
		ShowFrame();
	else
		picked = false;

	while((GlError=glGetError()) != GL_NO_ERROR)
	{
		dbg_print(ARDBG_ERR,"Got Gl Error displaying frame %d - %s\n",GlError,gluErrorString(GlError));
	}

	Unlock();

	pthread_testcancel();

	double TotalTime = OtherTime + CaptureTime + TextureTime + PreprocessTime + RenderTime + PostprocessTime;
	dbg_print(ARDBG_VERBOSE,"Benchmarks: T: %4.3fs -> C:%04.2f T:%04.2f Pr:%04.2f R:%04.2f Po:%04.2f O:%04.2f\n",TotalTime, CaptureTime/TotalTime, TextureTime/TotalTime, PreprocessTime/TotalTime, RenderTime/TotalTime, PostprocessTime/TotalTime, OtherTime/TotalTime);

}

void OutputObject::RenderFrame(RenderList & List, const ARImage & Frame, const ARCamera & cam, const ARPosition & cam_pos, unsigned int Tick, double * Distance)
{
	const double NearClip = 0.1;
	const double FarClip = 100;

	// for Convergence calculation
	double MinDepth = FarClip;

	int TempErr;
	while((TempErr=glGetError()) != GL_NO_ERROR)
	{
		dbg_print(ARDBG_ERR,"Warning, GL error occured in past %d - %s\n",TempErr,gluErrorString(TempErr));
	}
	// clear
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// initilise for rendering background
	glViewport(0,0,width,height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Draw Backdrop with textures
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,Backdrop);
	glDisable(GL_LIGHTING); // Disable lighting for background

	//double Mod2Width = RoundPow2(static_cast<double> (width));
	//double Mod2Height = RoundPow2(static_cast<double> (height));
	double ScaleWidth = static_cast<double> (Frame.x_size)/TexWidth;
	double ScaleHeight = static_cast<double> (Frame.y_size)/TexHeight;

	glBegin(GL_QUADS);
	glTexCoord2f(0,ScaleHeight);
	glVertex2f(-1,-1);
	glTexCoord2f(0,0);
	glVertex2f(-1,1);
	glTexCoord2f(ScaleWidth,0);
	glVertex2f(1,1);
	glTexCoord2f(ScaleWidth,ScaleHeight);
	glVertex2f(1,-1);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	// clear depth buffer after the image is displayed
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);

	while((TempErr=glGetError()) != GL_NO_ERROR)
		dbg_print(ARDBG_ERR,"Got Gl Error rendering backdrop %d - %s\n",TempErr,gluErrorString(TempErr));

	// set up camera view
	glLoadIdentity();
	gluPerspective(RTOD(cam.y_fov),cam.aspect,NearClip,FarClip);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	ARPoint Origin = cam_pos.Origin;
	ARPoint Focus(1,0,0);
	ARPoint Up(0,0,1);
	Focus.RotateRPY(cam_pos.Direction.x,cam_pos.Direction.y,cam_pos.Direction.z);
	Focus+=Origin;
	Up.RotateRPY(cam_pos.Direction.x,cam_pos.Direction.y,cam_pos.Direction.z);
	//gluLookAt(Origin.x,Origin.y,Origin.z,Focus.x,Focus.y,Focus.z,Up.x,Up.y,Up.z);
	dbg_print(ARDBG_VERBOSE,"Camera Details: %f %f %f %f %f %f %f %f %f\n",cam_pos.Origin.x,cam_pos.Origin.y,cam_pos.Origin.z,Focus.x,Focus.y,Focus.z,Up.x,Up.y,Up.z);

	// Render the virtual items
	// 3 passes, one for each of the 'layers'
	for (int layer = 0; layer < 3; ++ layer)
	{
		switch (layer)
		{
			case 0:
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);
				glDisable (GL_BLEND);
				glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
				break;

			case 1:
				glDepthMask(GL_TRUE);
				glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
				break;

			case 2:
				glDepthMask(GL_FALSE);
				glEnable(GL_DEPTH_TEST);
				glEnable (GL_BLEND);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;

			default:
				dbg_print(ARDBG_ERR, "Should never get to case 3\n");
				assert(0);
		}

		dbg_print(ARDBG_VERBOSE,"Starting Rendering of list, layer = %d\n",layer);
		int i = 0;
		for (RenderList::iterator itr = List.begin(); itr != List.end(); ++itr)
		{
			// Check the state of the render object do we need to initialise it etc...
			if (!itr->Rend->Enabled())
				continue;
			if (!itr->Rend->Initialised())
			{
				itr->Rend->Initialise();
				if (!itr->Rend->Initialised())
					continue;
			}

			itr->Rend->Lock();

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(Origin.x,Origin.y,Origin.z,Focus.x,Focus.y,Focus.z,Up.x,Up.y,Up.z);

			ARPosition Offset;
			dbg_print(ARDBG_VERBOSE,"Getting Position to render object at\n");

			if (!(*itr).Pos->FullChainPresent())
			{
				itr->Rend->Unlock();
				continue;
			}

			if((*itr).Pos)
				Offset = (*itr).Pos->GetTransformedPosition();
			else
				Offset = ARPosition();

			dbg_print(ARDBG_VERBOSE,"Transform the GL modelview for rendering\n");
			// Now transform model view for rendering the object
			glTranslatef(Offset.Origin.x, Offset.Origin.y, Offset.Origin.z);
			glRotatef(RTOD(Offset.Direction.z),0,0,1);
			glRotatef(RTOD(Offset.Direction.y),0,1,0);
			glRotatef(RTOD(Offset.Direction.x),1,0,0);


			// Caluclate the minimum distance of intersect (if intersection exists)
			if (Distance != NULL && layer == 0)
			{
				ARPoint Direction(1,0,0);
				Direction.RotateRPY(Offset.Direction.x, Offset.Direction.y, Offset.Direction.z);
				Ray TraceOffset(Vector3D(Offset.Origin.x, Offset.Origin.y, Offset.Origin.z),
					Vector3D(Direction.x, Direction.y, Direction.z));
				Ray CameraRay(Vector3D(cam_pos.Origin.x, cam_pos.Origin.y, cam_pos.Origin.z),
					Vector3D(cam_pos.Direction.x, cam_pos.Direction.y, cam_pos.Direction.z));
				double Dist = (*itr).Rend->TraceDistance(TraceOffset,CameraRay,Offset.Direction);
				if (Dist >= NearClip && Dist < MinDepth)
					MinDepth = Dist;
			}

			// Call the objects render function
			dbg_print(ARDBG_VERBOSE,"Now actually call the render function for the object %d\n",++i);
			if (!(*itr).ThreadInitilised)
			{
				(*itr).Rend->ThreadInit();
				(*itr).ThreadInitilised = true;
			}
			if (layer == 0)
			{
				(*itr).Rend->RenderBase();
			}
			else if (layer == 1)
				(*itr).Rend->Render();
			else
			{
				(*itr).Rend->RenderTransparent();
				if((*itr).Rend->Selected())
					RenderSelectionTools(*itr);
			}



			while((TempErr=glGetError()) != GL_NO_ERROR)
				dbg_print(ARDBG_ERR,"Got Gl Error rendering item. %d - %s\n",TempErr,gluErrorString(TempErr));

			itr->Rend->Unlock();

		}
	}

	glDepthMask(GL_TRUE);

	char TextBuffer[256];

	if (ARDev::DebugLevel > 0)
	{
		snprintf(TextBuffer,255,"Camera Details: FPS %f\nPos: % 06.1f % 06.1f % 06.1f\nDir: % 06.3f % 06.3f % 06.3f\n Up: % 06.3f % 6.3f % 6.3f\n",
			FrameRate,
			Origin.x,Origin.y,Origin.z,
			Focus.x-Origin.x,Focus.y-Origin.y,Focus.z-Origin.z,
			Up.x,Up.y,Up.z);
	}
	else
	{
		snprintf(TextBuffer,255,"ARDev v%s\n(c) 2005 Toby Collett",VERSION);
	}

	DrawText(0,height-18,TextBuffer,1,1,1);

	// Now we extract the distance to the closest object
	if (Distance != NULL)
	{
		if (MinDepth < NearClip)
			*Distance = NearClip;
		else if (MinDepth < FarClip)
			*Distance = MinDepth;
		else
			// no intersection found, reset to some mid range value
			*Distance = 1.01;
	}

#ifdef HAVE_ANTTWEAKBAR
	TwDraw();
#endif

	while((TempErr=glGetError()) != GL_NO_ERROR)
		dbg_print(ARDBG_ERR,"Got Gl Error rendering debug text %d - %s\n",TempErr,gluErrorString(TempErr));
}

PickingColour RenderObject::picking = PickingColour(0,0,0);

void OutputObject::RenderSelectionTools(RenderPair render)
{
	glPushMatrix();
		glScalef(1.05f,1.05f,1.05f);
		int polygonMode[2];
		glLineWidth(1);
		glGetIntegerv(GL_POLYGON_MODE, polygonMode);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glColor3f(1,1,0);
		render.Rend->RenderBounding();
		glPolygonMode(GL_FRONT, polygonMode[0]);
		glPolygonMode(GL_BACK, polygonMode[1]);
	glPopMatrix();

	///setup transform objects if not already
	if(!setupTransformControls)
	{
		setupTransformControls = true;
		int i;
		for(i=0;i<6;i++)
		{
			dbg_print(ARDBG_INFO,"Creating a render pair for : Transform Controls\n");
			RenderObject & Render = *(new RenderTransformControls(i));
			Render.SetEnabled(false);

			PositionObject & Position = *(new PositionTransformControls());
			controls = new RenderPair(&Render, &Position,0,"TransformControls","origin");

			Position.SetParent(controls);
			Render.SetParent(controls);
			Unlock();
			Add(*controls);
			Lock();
		}
	}

	// setup mask
	PickedEventObject* p = new PickedEventObject(ROE_TransformMask,selected->MovementMask);
	ARDev::PublishEvent(p);
	// change position
	ARPosition current = selected->GetParent()->Pos->GetPosition();
	RenderPositionEventObject* r = new RenderPositionEventObject(RPE_TransformControls,current,0);
	ARDev::PublishEvent(r);
}

bool OutputObject::MouseInput(int x, int y, MouseButton button,ButtonState state)
{
	bool handled = false;
	switch(state)
	{
		case Pressed:
			switch(button)
			{
				case Left: //left click
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseButton(TW_MOUSE_PRESSED,TW_MOUSE_LEFT))
						handled = true;
#endif
					break;
				case Right: //right click
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseButton(TW_MOUSE_PRESSED,TW_MOUSE_RIGHT))
						handled = true;
#endif
					break;
				case Middle: //middle click
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseButton(TW_MOUSE_PRESSED,TW_MOUSE_MIDDLE))
						handled = true;
#endif
					break;
				case WheelUp: //wheel up
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseWheel(++TWMOUSEWHEEL))
						handled = true;
#endif
					break;
				case WheelDown: //wheel down
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseWheel(--TWMOUSEWHEEL))
						handled = true;
#endif
					break;
				default:
					break;
			}
			break;
		case Released:
			switch(button)
			{
				case Left: //left click
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseButton(TW_MOUSE_RELEASED,TW_MOUSE_LEFT))
						handled = true;
#endif
					break;
				case Right: //right click
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseButton(TW_MOUSE_RELEASED,TW_MOUSE_RIGHT))
						handled = true;
#endif
					break;
				case Middle: //middle click
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseButton(TW_MOUSE_RELEASED,TW_MOUSE_MIDDLE))
						handled = true;
#endif
					break;
				case WheelUp: //wheel up
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseWheel(++TWMOUSEWHEEL))
						handled = true;
#endif
					break;
				case WheelDown: //wheel down
#ifdef HAVE_ANTTWEAKBAR
					if(TwMouseWheel(--TWMOUSEWHEEL))
						handled = true;
#endif
					break;
				default:
					break;
			}
			break;
		case Moved:
#ifdef HAVE_ANTTWEAKBAR
			if(TwMouseMotion(x, y))
				handled = true;
#endif
			break;
	}
	return handled;
}

bool OutputObject::KeyInput(char keycode, ButtonState state)
{
	bool handled = false;
	switch(state)
	{
		case Pressed:
		{
			const int B=48; //base value for numbers 0-9
			switch(keycode)
			{
				case B+1:
				case B+2:
				case B+3:
				case B+4:
				case B+5:
				case B+6:
				case B+7:
				case B+8:
				case B+9:
				case B+0:
#ifdef HAVE_ANTTWEAKBAR
				if(TwKeyPressed(keycode,TW_KMOD_NONE))
					handled = true;
#endif
					break;
				default:
					break;
			}
			break;
		}
		case Released:
		{
			break;
		}
		default:
			break;
	}

	return handled;
}

void OutputObject::WindowResized(int width, int height)
{
	this->width = width;
	this->height = height;
#ifdef HAVE_ANTTWEAKBAR
	TwWindowSize(width,height);
#endif
}

RenderObject* OutputObject::PickingFrame(RenderList & List, const ARCamera & cam, const ARPosition & cam_pos,int x, int y)
{
	picked = true;
	const double NearClip = 0.1;
	const double FarClip = 100;
	unsigned char pixel[4];
	int viewport[4];

	int TempErr;
	while((TempErr=glGetError()) != GL_NO_ERROR)
	{
		dbg_print(ARDBG_ERR,"Warning, GL error occured in past %d - %s\n",TempErr,gluErrorString(TempErr));
	}
	// clear
	//glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// initilise for rendering background
	glViewport(0,0,width,height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// set up camera view
	gluPerspective(RTOD(cam.y_fov),cam.aspect,NearClip,FarClip);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	ARPoint Origin = cam_pos.Origin;
	ARPoint Focus(1,0,0);
	ARPoint Up(0,0,1);
	Focus.RotateRPY(cam_pos.Direction.x,cam_pos.Direction.y,cam_pos.Direction.z);
	Focus+=Origin;
	Up.RotateRPY(cam_pos.Direction.x,cam_pos.Direction.y,cam_pos.Direction.z);
	//gluLookAt(Origin.x,Origin.y,Origin.z,Focus.x,Focus.y,Focus.z,Up.x,Up.y,Up.z);
	dbg_print(ARDBG_VERBOSE,"Camera Details: %f %f %f %f %f %f %f %f %f\n",cam_pos.Origin.x,cam_pos.Origin.y,cam_pos.Origin.z,Focus.x,Focus.y,Focus.z,Up.x,Up.y,Up.z);

	dbg_print(ARDBG_VERBOSE,"Starting Picking of list\n");
	int i = 0;

	for (RenderList::iterator itr = List.begin(); itr != List.end(); ++itr)
	{
		// Check the state of the render object do we need to initialise it etc...
		if (!itr->Rend->Enabled())
			continue;
		if(!itr->Rend->Selectable())
			continue;
		if (!itr->Rend->Initialised())
		{
			itr->Rend->Initialise();
			if (!itr->Rend->Initialised())
				continue;
		}

		itr->Rend->Lock();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(Origin.x,Origin.y,Origin.z,Focus.x,Focus.y,Focus.z,Up.x,Up.y,Up.z);

		ARPosition Offset;
		dbg_print(ARDBG_VERBOSE,"Getting Position to render object at\n");

		if (!(*itr).Pos->FullChainPresent())
		{
			itr->Rend->Unlock();
			continue;
		}

		if((*itr).Pos)
			Offset = (*itr).Pos->GetTransformedPosition();
		else
			Offset = ARPosition();

		dbg_print(ARDBG_VERBOSE,"Transform the GL modelview for picking\n");
		// Now transform model view for rendering the object
		glTranslatef(Offset.Origin.x, Offset.Origin.y, Offset.Origin.z);
		glRotatef(RTOD(Offset.Direction.z),0,0,1);
		glRotatef(RTOD(Offset.Direction.y),0,1,0);
		glRotatef(RTOD(Offset.Direction.x),1,0,0);

		//get unique color
		PickingColour c = (*itr).Rend->GetPickingColour();
		glColor4ub(c.r,c.g,c.b,c.a);
		//printf("%s rendered with colour : %d %d %d\n",(*itr).Name,c.r,c.g,c.b );
		// Call the objects render function
		dbg_print(ARDBG_VERBOSE,"Now call the render bounding function for the object %d\n",++i);

		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);

		(*itr).Rend->RenderBounding();

		while((TempErr=glGetError()) != GL_NO_ERROR)
			dbg_print(ARDBG_ERR,"Got Gl Error rendering picking item. %d - %s\n",TempErr,gluErrorString(TempErr));

		itr->Rend->Unlock();
	}


	glGetIntegerv(GL_VIEWPORT, viewport);
	glReadPixels(x, viewport[3] - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	//printf("Hit: %d %d %d %d\n",pixel[0],pixel[1],pixel[2],pixel[3] );
//
	for (RenderList::iterator itr = List.begin(); itr != List.end(); ++itr)
	{
		PickingColour c = (*itr).Rend->GetPickingColour();
		if((*itr).Rend->Selectable())
		{
			if(c.r==pixel[0]&&c.g==pixel[1]&&c.b==pixel[2])
			{
				//printf("Found Match: %s.\n",(*itr).Name);
				return (*itr).Rend;
			}
		}

	}
	return NULL;
}

// Utility for gl text string output
void
OutputObject::DrawText(GLint x, GLint y, char* s, GLfloat r, GLfloat g, GLfloat b)
{
    int lines;
	int MaxX, MaxY, CurrentLineWidth;
    char* p;

	glDisable(GL_DEPTH_TEST);
	glColor3f(r,g,b);

    glMatrixMode(GL_PROJECTION);
     glPushMatrix();
     glLoadIdentity();
     glOrtho(0.0, width,
	    0.0, height, -1.0, 1.0);
     glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      glRasterPos2i(x, y);
		MaxX = CurrentLineWidth = 0;
		MaxY = 18;
      for(p = s, lines = 0; *p != '\0'; p++) {
	  if (*p == '\n') {
	      lines++;
		  if (CurrentLineWidth > MaxX)
			  MaxX = CurrentLineWidth;
		  CurrentLineWidth = 0;
		  MaxY = lines*15+15;
	  }
	  else
	  	CurrentLineWidth += glutBitmapWidth(GLUT_BITMAP_9_BY_15, *p);
      }

	  if (CurrentLineWidth > MaxX)
		  MaxX = CurrentLineWidth;
	  // now render background box;
	  glColor4f(0,0,0,0.5);
      glEnable (GL_BLEND);
	  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  glBegin(GL_QUADS);
	  glVertex3f(x,height,0);
	  glVertex3f(x+MaxX,height,0);
	  glVertex3f(x+MaxX,height-MaxY,0);
	  glVertex3f(x,height-MaxY,0);
	  glEnd();
	  glDisable(GL_BLEND);

	  // Then render the actual text
      glColor3f(r,g,b);
      for(p = s, lines = 0; *p != '\0'; p++) {
		  if (*p == '\n') {
		      lines++;
		      glRasterPos2i(x, y-(lines*15));
		  }
		  else
		  	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
    }


	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
}


const ARImage & OutputObject::GetFrame()
{
	static ARImage Temp;
	if (Temp.data == NULL)
	{
		Temp.x_size = width;
		Temp.y_size = height;
		Temp.data = new unsigned char[3 * width * height];
	}
	// grab gl frame
	glReadPixels(0,0,width, height, GL_RGB, GL_UNSIGNED_BYTE,static_cast<void*> (Temp.data));
	return Temp;

}

/// Notifies any objects who are registered as event handling objects
void OutputObject::PublishEvent(EventObject* event)
{
	dbg_print(ARDBG_VERBOSE,"Event Published\n");
	ARDev::PublishEvent(event); /// Gives event to all ARObjects that are registered to receive events
}

/// Change to an environment relative to the current environment
void OutputObject::RelativeEnvironment(int i)
{
	OutputObject::currentEnv += i;
	if(OutputObject::currentEnv < 0)
		OutputObject::currentEnv = 0;
	else if (OutputObject::currentEnv >= (int)OutputObject::cameras.size())
		OutputObject::currentEnv = OutputObject::cameras.size()-1;
	ChangeEnvironment();
}

/// Change to an environment in the absolute position
void OutputObject::AbsoluteEnvironment(int i)
{
	if(i >= (int)OutputObject::cameras.size() || i < 0)
		return;

	// environments start from 0
	OutputObject::currentEnv = i;
	dbg_print(ARDBG_VERBOSE,"ENV: %d\n",i);
	ChangeEnvironment();
}

/// Switch over environment objects
void OutputObject::ChangeEnvironment()
{
	Camera = OutputObject::cameras[OutputObject::currentEnv];
	Capture = OutputObject::captures[OutputObject::currentEnv];
	CameraPosition = OutputObject::camera_positions[OutputObject::currentEnv];
	envFresh = true;
}

void OutputObject::SetCloseCallback(void (*callback)(void*), void* data)
{
	this->callback = callback;
	callbackData = data;
}

// Input mouse coordinates
// Returns world coordinates
Vector3D OutputObject::UnProject(double x, double y)
{
	int viewport[4];
	double modelview[16];
	double projection[16];
	float winX, winY, winZ;
	double posX, posY, posZ;

	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glLoadIdentity();

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	//for(int i=0;i<16;i++)
	//{
		//cout << " " << modelview[i];
	//}
	//cout << endl;
	//for(int i=0;i<16;i++)
	//{
	//	//cout << " " << projection[i];
	//}
	//cout << endl;

	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

	gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);


	//glMatrixMode(GL_MODELVIEW);
	//glPopMatrix();

	return Vector3D(posX, posY, posZ);
}

// TODO
Vector3D OutputObject::Project(double x, double y, double z)
{
	int viewport[4];
	double modelview[16];
	double projection[16];
	double winX, winY, winZ;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	gluProject( x,y,z, modelview, projection, viewport, &winX, &winY, &winZ);

	return Vector3D(winX, winY, winZ);
}

RenderVector OutputObject::Intersect(ARPosition & cam_pos, ARPoint world)
{
	RenderVector Intersections;
	ARPoint Direction(1,0,0);
	//double Distance = 0;

	double Radius = 1.0; // refactor out

	for( RenderList::iterator render= RenderObjects.begin(); render != RenderObjects.end(); render++)
	{
		ARPosition object = (*render).Pos->GetTransformedPosition();
		ARPoint direction = world - cam_pos.Origin;
		Ray R = Ray( Vector3D(cam_pos.Origin.x, cam_pos.Origin.y, cam_pos.Origin.z) ,Vector3D(direction.x, direction.y, direction.z) );
		// Vector S is Ray Origin - Sphere origin
		Vector3D S = Vector3D(R.Origin.x - object.Origin.x, R.Origin.y - object.Origin.y,R.Origin.z - object.Origin.z);

		double SdotD = S.DotProduct(R.Direction);
		double SdotS = S.DotProduct(S);
		double DdotD = R.Direction.DotProduct(R.Direction);

		double Det=(Radius*Radius-SdotS)/DdotD + pow((SdotD/DdotD),2);

		if (Det > 0 || Det ==0)
			Intersections.push_back(&(*render));

	}
	return Intersections;
}

Vector3D OutputObject::PixelDirection(double x, double y)
{
	int viewport[4];
	double modelview[16];
	double projection[16];
	float winX, winY;
	double nearPosX, nearPosY, nearPosZ;
	double farPosX, farPosY, farPosZ;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );

	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;

	gluUnProject( winX, winY, 0, modelview, projection, viewport, &nearPosX, &nearPosY, &nearPosZ);
	gluUnProject( winX, winY, 1, modelview, projection, viewport, &farPosX, &farPosY, &farPosZ);

	Vector3D dir = Vector3D(farPosX - nearPosX, farPosY - nearPosY, farPosZ - nearPosZ);
	dir.Normalise();
	return dir;
}

RenderObject* OutputObject::PickObject(int x, int y)
{
	return PickingFrame(RenderObjects,Camera->GetCameraRef(),CameraPosition->GetTransformedPosition(),x,y);
}

void OutputObject::SelectPickedObject()
{
	if(pickedObj)
	{
		if (selected != pickedObj)
		{
			if(selected) // check for null
				selected->SetSelected(false); 	//deselect old selection
			selected = pickedObj; 				// select new
			dbg_print(ARDBG_INFO,"\tPicked: %s\n",pickedObj->GetParent()->Name);
			pickedObj->SetSelected(true); 		// set as selected

#ifdef HAVE_ANTTWEAKBAR
			tweakBar->SelectWindow(selected);
#endif
		}
	}
	else
	{
		dbg_print(ARDBG_INFO,"\tPicked Nothing\n");
		if(selected) // check for null
			selected->SetSelected(false); 	//deselect old selection
		selected = NULL; 				// select new
		pickedObj = NULL;
#ifdef HAVE_ANTTWEAKBAR
		tweakBar->SelectWindow(NULL);
#endif
	}
}

// Event handling
void OutputObject::Event(EventObject* event)
{
	if (event->GetEventType() == OE_CaptureConnect)
	{
		CaptureConnect();
	}

	// special multi-environment events
	if(OutputObject::cameras.size() > 1)
	{
		if (event->GetEventType() == EOE_RelativeEnvironment)
		{
			EnvironmentEventObject* e = dynamic_cast<EnvironmentEventObject*>(event);
			RelativeEnvironment(e->GetRelativeIndex());
		}
		else if (event->GetEventType() == EOE_AbsoluteEnvironment)
		{
			EnvironmentEventObject* e = dynamic_cast<EnvironmentEventObject*>(event);
			AbsoluteEnvironment(e->GetAbsoluteIndex());
		}

	}
}

ARCamera::ARCamera(ARPoint _Origin, ARPoint LookAt, ARPoint _Up, double _y_fov, double _aspect)
{
	Origin = _Origin;
	SetDirectionFromPoint(LookAt);
	y_fov = _y_fov;
	aspect = _aspect;
	Up=_Up;
	CamType = AR_CAMTYPE_SINGLE;
	convergance = NULL;
	separation = NULL;
}

ARCamera::ARCamera(const char * Filename) : R(3,3), T(3,1)
{
	CamType = AR_CAMTYPE_SINGLE;
	convergance = NULL;
	separation = NULL;

	std::ifstream fin(Filename);
	if (!fin.is_open())
	{
		dbg_print(ARDBG_ERR, "Could not open camera file: %s\n",Filename);
		return;
	}

//	dbg_print(ARDBG_VERBOSE, "Creating matricies\n");
//	R = Matrix(3,3);
//	T = Matrix(3,1);

	double v1,v2,v3,v4,v5,v6,v7,v8,v9;

	// Load Translation Matrix
	fin >> v1 >> v2 >> v3;
	T.SetValue(0,0,v1);
	T.SetValue(1,0,v2);
	T.SetValue(2,0,v3);

	// Load Rotation Matrix
	fin >> v1 >> v2 >>v3 >>v4 >>v5 >>v6 >>v7>>v8>>v9;
	R.SetValue(0,0,v1);
	R.SetValue(0,1,v2);
	R.SetValue(0,2,v3);
	R.SetValue(1,0,v4);
	R.SetValue(1,1,v5);
	R.SetValue(1,2,v6);
	R.SetValue(2,0,v7);
	R.SetValue(2,1,v8);
	R.SetValue(2,2,v9);

	fin >> FocalDistance >> sensor_width >> sensor_height;
	fin >> frame_width >> frame_height;
	fin >> sx >> sy;

	cout << "Scale: " << sx << " " << sy << endl;

	fin.close();

	// now we have loaded our parameters we need to calculate
	// some other for convenience later
	// rotation matrix inverse = transpose
	RInv = R.T();

	x_fov = 2 * atan2(sx*frame_width*sensor_width/2,FocalDistance);
	y_fov = 2 * atan2(sy*frame_height*sensor_height/2,FocalDistance);

	aspect = static_cast<double>(frame_width)/frame_height * sx/sy;
	cout << "Asp: " << aspect << " " << frame_width << " " << frame_height << endl;

	// Calculate the origin and direction vectors for the camera
	Origin = GetWRFDirFromCRF(ARPoint(0,0,0));
	ARPoint LookAt = GetWRFDirFromCRF(ARPoint(0,0,1)) - Origin;
	Up = GetWRFDirFromCRF(ARPoint(0,-1,0)) - Origin;

	ARPoint TempLook = LookAt;
	ARPoint TempUp = Up;

	printf("Calculate Z rotation: ");
	// get angle of projection to XY plane
	double RotZ = atan2(TempLook.y,TempLook.x);
	printf("%f\n",RotZ);
	TempUp.Print();
	TempUp.RotateZ(-RotZ);
	TempUp.Print();
	printf("\n");
	TempLook.Print();
	TempLook.RotateZ(-RotZ);
	TempLook.Print();
	printf("\n------------\n");

	printf("Calculate Y rotation: ");
	// get angle of projection to XZ plane
	double RotY = -atan2(TempLook.z,TempLook.x);
	printf("%f\n",RotY);
	TempUp.Print();
	TempUp.RotateY(-RotY);
	TempUp.Print();
	printf("\n");
	TempLook.Print();
	TempLook.RotateY(-RotY);
	TempLook.Print();
	printf("\n------------\n");

	printf("Calculate X rotation: ");
	// get angle of projection to YZ plane
	double RotX = -atan2(TempUp.y,TempUp.z);
	printf("%f\n",RotX);
	TempUp.Print();
	TempUp.RotateX(-RotX);
	TempUp.Print();
	printf("\n");
	TempLook.Print();
	TempLook.RotateX(-RotX);
	TempLook.Print();
	printf("\n------------\n");


	printf("Points from file: ");
	printf("\n");
	Origin.Print();
	printf("\n");
	LookAt.Print();
	printf("\n");
	Up.Print();
	printf("\n");

	Direction.z = -RotZ;
	Direction.y = -RotY;
	Direction.x = -RotX;

	ARPoint O = Origin;
	ARPoint F(1,0,0);
	ARPoint U(0,0,1);

	F.RotateRPY(Direction.x,Direction.y,Direction.z);
	U.RotateRPY(Direction.x,Direction.y,Direction.z);

	printf("%f %f %f\n", RTOD(Direction.x), RTOD(Direction.y), RTOD(Direction.z));


	printf("Recalculated: \n");
	F.Print();
	U.Print();
	printf("\n");
}


void ARCamera::SetDirectionFromPoint(ARPoint _In)
{
	double dx = -Origin.x + _In.x;
	double dy = -Origin.y + _In.y;
	double dz = -Origin.z + _In.z;

	double Magnitude = sqrt(dx*dx + dy*dy + dz*dz);
	if (Magnitude == 0)
		Direction = ARPoint(1,0,0);
	else
		Direction = ARPoint(dx/Magnitude, dy/Magnitude, dz/Magnitude);
}

ARPoint ARCamera::GetCRFPointFromPixel(double x, double y)
{
	ARPoint Ret;
	Ret.x = sx * (x - frame_width/2) * sensor_width;
	Ret.y = sy * (y - frame_height/2) * sensor_height;
	Ret.z = FocalDistance;
	return Ret;
}

ARPoint ARCamera::GetWRFDirFromCRF(ARPoint CRFPoint)
{
	Matrix P_CRF(3,1);
	P_CRF.SetValue(0,0,CRFPoint.x);
	P_CRF.SetValue(1,0,CRFPoint.y);
	P_CRF.SetValue(2,0,CRFPoint.z);

	//cout << "CRF=" << P_CRF << "RInv=" << RInv << "T=" << T << endl;

	Matrix Temp( RInv*(P_CRF - T));

	// change handedness of axes and conver to ARPoint
	return ARPoint(Temp.GetValue(0,0),Temp.GetValue(1,0),Temp.GetValue(2,0));
}

/*
ARPosition ARCamera::GetBPRay(ARPoint Point) ///< Convert a 2d pixel coordinate to a
{
	//calculate focalDistance and x_fov
	double focalDistance = 0.0;
	double x_fov = y_fov * aspect;
	focalDistance = (sensor_width/2.0)/tan(x_fov/2.0);

	//center of image
	double cx = imWidth/2.0;
	double cy = imHeight/2.0;

	//pixel in image centered coordinates
	double px = x - cx;
	double py = y - cy;

	Matrix CRF(3,1);
	CRF.SetValue(0,0,px);
	CRF.SetValue(0,1,py);
	CRF.SetValue(0,2,focalDistance);


	ARPoint result;
	result.x=px;
	result.y=py;
	result.z=focalDistance;

	// now we have a 3d point in the CRF we need to put it in the WRF
}


ARPoint ARCamera::GetRayFromImage(double x, double y, int imWidth, int imHeight) {
	//calculate focalDistance and x_fov
	double focalDistance = 0.0;
	double x_fov = y_fov * aspect;
	focalDistance = (sensor_width/2.0)/tan(x_fov/2.0);

	//center of image
	double cx = imWidth/2.0;
	double cy = imHeight/2.0;

	//pixel in image centered coordinates
	double px = x - cx;
	double py = y - cy;

	ARPoint result;
	result.x=focalDistance;
	result.y=-py;
	result.z=-px;

	return result;
}
*/


// Returns the shortest distance from a Ray to the object (-1 for no intersection)
// Default implementation does an intersection with a small sphere at the origin
double RenderObject::TraceDistance(const Ray & Offset, const Ray & R, const ARPoint & Rotation)
{
	Sphere S(Offset.Origin,Size);
	const vector<Vector3D> & Points = S.Intersect(R);
	if (Points.size() == 0)
		return -1;
	else if (Points.size() == 1)
			return (Points.back() - R.Origin).Length();
		double Dist1 = (Points.back() - R.Origin).Length();
		double Dist2 = (Points.front() - R.Origin).Length();

	return Dist1 < Dist2 ? Dist1 : Dist2;
}

void RenderObject::SetSelectable(const bool SelectState)
{
	Lock();
	selectable = SelectState;
	if(selectable && pickingColour.r == 255 && pickingColour.g == 255 && pickingColour.b == 255)
	{

		if(picking.r == 255)
		{
			picking.r = 0;

			if(picking.g == 255)
			{
				picking.g = 0;
				picking.b++;
			}
			else
				picking.g++;
		}
		else
			picking.r++;

		//printf("\tPicking Colour: %d %d %d\n",picking.r,picking.g,picking.b);
		pickingColour = PickingColour(picking.r,picking.g,picking.b);
	}
	Unlock();
}
