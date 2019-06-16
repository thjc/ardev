#include <ardev/ardevconfig.h>

#include <algorithm>
#ifdef HAVE_ARIDE
	#include <cm_project.h>
	#ifdef HAVE_PLAYER
		#include <cm_playerhandlers.h>
		#include <libplayerc++/playerc++.h>
		#include <libplayerc++/playererror.h>
		using namespace PlayerCc;
	#endif
	#include <cm_objecthandler.h>
	#include <cm_artoolkithandlers.h>
	#include <cm_artoolkitplushandlers.h>

	#if HAVE_LIBLTDL
		#include <cm_pluginhandler.h>
	#endif
	#ifdef HAVE_OPENCV
		#include <cm_opencvblobhandlers.h>
	#endif
#else
	class MainWindow;
#endif

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <libplayerinterface/player.h>
#include <libplayercore/playercore.h>
#include <ardev/ardev.h>
#include <ardev/player.h>
#include <ardev/output_x11.h>
#include <ardev/opencv_blobtrack.h>
#include <ardev/artoolkitplus.h>
#include <ardev/capture.h>


const unsigned int MaxRobots = 8;


#include <vector>
using namespace std;
#include <GL/gl.h>
#include <GL/glut.h>




////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class ARDevPlugin : public Driver
{
  public:

    // Constructor; need that
    ARDevPlugin(ConfigFile* cf, int section);

    // Must implement the following player methods.
    virtual int Setup();
    virtual int Shutdown();

    // Thses methods are used to keep track of who has subscribed, because each clients data is handled separately.
    virtual int Subscribe(QueuePointer &queue, player_devaddr_t addr);
    virtual int Unsubscribe(QueuePointer &queue, player_devaddr_t addr);

    // This method will be invoked on each incoming message
    int ProcessMessage(QueuePointer & resp_queue,
                               player_msghdr * hdr,
                               void * data);

  private:


	CameraObject * cam;
	PositionObject * cam_pos;
	CaptureObject * cap;
	OutputObject * out;
	FrameProcessObject * pre;
    PositionObject * pos;

	// config variables
	char * OutputType;
	bool OutputX11Fullscreen;
	char * CaptureType;
	char * CameraType;
	char * CalibrationFile;
	char * PositionType;
	int MarkerID;
	double MarkerHeight;
	int DebugLevel;

	// for ARIDE configuration
	char * ARIDEConfigFile;
	char * ARIDERenderObject[MaxRobots];
    Graphics2DRenderer * Renderer2D[MaxRobots];
    Graphics3DRenderer * Renderer3D[MaxRobots];
	player_devaddr_t  dev_addr[MaxRobots];
	aride_project Project;

	std::list<QueuePointer> subscriberQueues; // Currently the only point in this is keep the reference counting accurate.
};

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
ARDevPlugin_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return((Driver*)(new ARDevPlugin(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void ARDevPlugin_Register(DriverTable* table)
{
  table->AddDriver("ardevplugin", ARDevPlugin_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
ARDevPlugin::ARDevPlugin(ConfigFile* cf, int section)
    : Driver(cf, section, false, 100) // need big ish queue with non replacing commands since each graphics item is likely to be a new command
{
	cam = NULL;
	cam_pos = NULL;
	cap = NULL;
	out = NULL;
	pre = NULL;
	pos = NULL;

	ARIDEConfigFile = NULL;
	memset(dev_addr,0,sizeof(dev_addr));
	memset(ARIDERenderObject,0,sizeof(ARIDERenderObject));
	memset(Renderer2D,0,sizeof(Renderer2D));
	memset(Renderer3D,0,sizeof(Renderer3D));

	// read settings from config file
	DebugLevel=cf->ReadInt(section,"debug_level",3);

	OutputType = strdup(cf->ReadString(section,"output","x11"));
	OutputX11Fullscreen = cf->ReadInt(section,"output_x11_fullscreen",0);
	CaptureType = strdup(cf->ReadString(section,"capture","dc1394"));
	CameraType = strdup(cf->ReadString(section,"camera","calib"));
	CalibrationFile = strdup(cf->ReadString(section,"camera_calib_file","calibration.calib"));
	PositionType = strdup(cf->ReadString(section,"position","opencv"));
	MarkerID = cf->ReadInt(section,"marker_id",0);
	MarkerHeight = cf->ReadFloat(section,"marker_height",0);

	// aride support
	ARIDEConfigFile = strdup(cf->ReadString(section,"aride_config_file",""));

	for (unsigned int i = 0; i < MaxRobots; ++i)
	{
		char Key[32];
		snprintf(Key,32,"robot%x",i);
		if(cf->ReadDeviceAddr(&(dev_addr[i]), section, "provides",
                      PLAYER_GRAPHICS2D_CODE, -1, Key) == 0)
		{
			if(this->AddInterface(dev_addr[i]) != 0)
			{
				this->SetError(-1);
				return;
			}
			ARIDERenderObject[i] = strdup(cf->ReadTupleString(section,"aride_render_object",i,""));
		}
		else if(cf->ReadDeviceAddr(&(dev_addr[i]), section, "provides",
                      PLAYER_GRAPHICS3D_CODE, -1, Key) == 0)
		{
			if(this->AddInterface(dev_addr[i]) != 0)
			{
				this->SetError(-1);
				return;
			}
			ARIDERenderObject[i] = strdup(cf->ReadTupleString(section,"aride_render_object",i,""));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int ARDevPlugin::Setup()
{
	puts("ARDevPlugin driver initialising");
	ARDev::DebugLevel = DebugLevel;

	// check if we are using an aride configuration file
	if (strcmp(ARIDEConfigFile,"")!=0)
	{
#ifdef HAVE_ARIDE
		// first we register our object and type handers
		RegisterDefaultObjectHandlers();
#ifdef HAVE_PLAYER
		RegisterPlayerObjectHandlers();
#endif
#ifdef HAVE_LIBAR
		RegisterARToolKitObjectHandlers();
#endif
#ifdef HAVE_OPENCV
		RegisterOpenCVBlobObjectHandlers();
#endif
#ifdef HAVE_ARTOOLKITPLUS
		RegisterARToolKitPlusObjectHandlers();
#endif
#if HAVE_LIBLTDL
		RegisterPluginObjectHandlers();
#endif

		// then create the application
		try
		{
			Project.LoadFromFile(ARIDEConfigFile);

			// then start the AR session
			Project.Run();

			// now locate the array of Render objects
			for (unsigned int i = 0; i < MaxRobots; ++i)
			{
				if (ARIDERenderObject[i]==NULL || ARIDERenderObject[i][0]=='\0')
					continue;
				aride_object * rend = Project.GetObjectByGUID(Project.GetGUIDByName(ARIDERenderObject[i]));
				if (NULL==rend)
				{
					printf("Unable to find object %s in ARIDE project\n", ARIDERenderObject[i]);
					continue;
				}
				if (NULL==rend->Handler)
				{
					printf("object %s has no handler\n", ARIDERenderObject[i]);
					continue;
				}
				dbg_print(ARDBG_INFO,"Locating render object %d (%s)\n",i,ARIDERenderObject[i]);
				printf("debug stuff %d %s %p\n",i,ARIDERenderObject[i],rend);
				if (dev_addr[i].interf == PLAYER_GRAPHICS2D_CODE)
					Renderer2D[i] = reinterpret_cast<Graphics2DRenderer *> (&reinterpret_cast<RenderGraphics2DHandler *> (rend->Handler)->GetObject());
				else if (dev_addr[i].interf == PLAYER_GRAPHICS3D_CODE)
					Renderer3D[i] = reinterpret_cast<Graphics3DRenderer *> (&reinterpret_cast<RenderGraphics3DHandler *> (rend->Handler)->GetObject());

				if (NULL==Renderer2D[i] && NULL==Renderer3D[i])
				{
					printf("Unable to get pointer for object %s from ARIDE project\n", ARIDERenderObject[i]);
					continue;
				}
				printf("found object successflly\n");
			}


		}
		catch (const aride_exception & e)
		{
			e.Print();
			fprintf(stderr,"Unhandled exception, Application Terminated\n");
			return 1;
		}
#ifdef HAVE_PLAYER
		catch (const PlayerCc::PlayerError & e)
		{
			cout << e;
			fprintf(stderr,"Unhandled exception, Application Terminated\n");
			return 1;
		}
#endif
		catch (...)
		{
			fprintf(stderr,"Unknown exception caught\n");
		}
#else
		printf("ARIDE support not built into player module, please reconfigure and rebuild ardev\n");
		return -1;
#endif

	}
	else
	{
		Renderer2D[0] = new Graphics2DRenderer;

		// First we initialise out camera object (and its position)
		if (strcmp(CameraType,"calib")==0)
		{
			ARCamera CameraProperties(CalibrationFile);
			cam = new CameraConstant(CameraProperties);
			ARPosition CameraOffsetConst(CameraProperties.Origin,CameraProperties.Direction);
			cam_pos = new PositionConstant(CameraOffsetConst);
			cam_pos->Initialise();
		}
		else
		{
			printf("Unknown Camera type: %s\n",CameraType);
			return -1;
		}

		// next initialise the capture object
		if (strcmp(CaptureType,"dc1394")==0)
		{
	#ifdef HAVE_DC1394
			cap = new CaptureDC1394();
			cap->Initialise();
	#else
			printf("Support for dc1394 not built\n");
			return -1;
	#endif
		}
		else
		{
			printf("Unknown Capture type: %s\n",CaptureType);
			return -1;
		}

		// create output object
		if (strcmp(OutputType,"x11")==0)
		{
			out = new OutputX11(cap,cam,cam_pos,800,600,":0","",OutputX11Fullscreen);
		}
		else
		{
			printf("Unknown Output type: %s\n",OutputType);
			return -1;
		}
		ARDev::Start(out,"overhead");

		// next create our tracker
		if (strcmp(PositionType,"opencv")==0)
		{
	#ifdef HAVE_OPENCV
			OpenCVBlobTrackPreProcess * pre_opencv = new OpenCVBlobTrackPreProcess(*cam,200,10000,false);
			pre_opencv->Initialise();
			out->AddPre(pre_opencv);

			BlobPair_t RobotBlob;
			RobotBlob.Hue1_Min = 165;
			RobotBlob.Hue1_Max = 175;
			RobotBlob.Hue2_Min = 95;
			RobotBlob.Hue2_Max = 105;
			RobotBlob.Height = 0.31;
			pos = new OpenCVBlobTrackPosition(*pre_opencv,RobotBlob);
			pos->Initialise();
			pre = pre_opencv;
	#else
			printf("OpenCV support not built\n");
			return -1;
	#endif
		}
		else if (strcmp(PositionType,"artoolkitplus")==0)
		{
	#ifdef HAVE_ARTOOLKITPLUS
			ARToolKitPlusPreProcess * artkp_pre;
			artkp_pre = new ARToolKitPlusPreProcess(*cam);
			artkp_pre->Initialise();
			printf("Initialised ARToolKitPlus preprocess, now add it to the output object\n");
			out->AddPre(artkp_pre);

			pos = new ARToolKitPlusPosition(*artkp_pre,MarkerID,MarkerHeight);
			pos->Initialise();
			printf("Initialised ARToolKitPlus position\n");
			pre = artkp_pre;
	#else
			printf("ARToolKitPlus support not built\n");
			return -1;
	#endif
		}	else
		{
			printf("Unknown Position type: %s\n",PositionType);
			return -1;
		}

		ARDev::Add(RenderPair(Renderer2D[0],pos),out->Name);
	}
	ARDev::DebugLevel = DebugLevel;
	puts("ARDevPlugin driver ready");

	return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int ARDevPlugin::Shutdown()
{
	puts("Shutting ARDevPlugin driver down");

	if (strcmp(ARIDEConfigFile,"")!=0)
	{
#ifdef HAVE_ARIDE
		Project.Stop();
		memset(Renderer2D,0,sizeof(Renderer2D));
		memset(Renderer3D,0,sizeof(Renderer3D));
#else
		printf("ARIDE support not compiled\n");
		return -1;
#endif
	}
	else
	{
		ARDev::Stop("overhead");

		delete cam;
		cam = NULL;
		delete cam_pos;
		cam_pos = NULL;
		delete cap;
		cap = NULL;
		delete pos;
		pos = NULL;
		delete out;
		out = NULL;
		delete pre;
		pre = NULL;

		for (unsigned int i = 0; i < MaxRobots; ++i)
		{
			delete Renderer2D[i];
			Renderer2D[i] = NULL;
			delete Renderer3D[i];
			Renderer3D[i] = NULL;
		}
	}

	puts("ARDevPlugin driver has been shutdown");

	return(0);
}

int ARDevPlugin::Subscribe(QueuePointer &queue, player_devaddr_t addr)
{
	if (Driver::Subscribe(addr) != 0)
		return -1;
	if (queue == NULL) // Always on driver.
		return 0;
	subscriberQueues.push_back(queue);
	for (unsigned int jj = 0; jj < MaxRobots; ++jj)
	{
		if (dev_addr[jj].host == addr.host && dev_addr[jj].robot == addr.robot &&
				dev_addr[jj].interf == addr.interf && dev_addr[jj].index == addr.index) // If we have the right robot.
		{
			if (Renderer2D[jj])
			{
				Renderer2D[jj]->Lock();
				if (Renderer2D[jj]->ClientElements.find(&(*queue)) != Renderer2D[jj]->ClientElements.end())
				{
					PLAYER_WARN("Client subscribed twice.");
					return -1;
				}
				vector<GraphicsElement> drawCommands;
				Renderer2D[jj]->ClientElements.insert(std::pair<MessageQueue*,vector<GraphicsElement> >(&(*queue), drawCommands));
				Renderer2D[jj]->Unlock();
			}
			else if (Renderer3D[jj])
			{
				Renderer3D[jj]->Lock();
				if (Renderer3D[jj]->ClientElements.find(&(*queue)) != Renderer3D[jj]->ClientElements.end())
				{
					PLAYER_WARN("Client subscribed twice.");
					return -1;
				}
				vector<player_graphics3d_cmd_draw_t *> drawCommands;
				Renderer3D[jj]->ClientElements.insert(std::pair<MessageQueue*,vector<player_graphics3d_cmd_draw_t *> >(&(*queue), drawCommands));
				Renderer3D[jj]->Unlock();
			}
		}
	}
	return 0;
}

int ARDevPlugin::Unsubscribe(QueuePointer &queue, player_devaddr_t addr)
{
	if (Driver::Unsubscribe(addr) != 0)
		return -1;
	if (queue == NULL)
			return 0;
	for (unsigned int jj = 0; jj < MaxRobots; ++jj)
	{
		if (dev_addr[jj].host == addr.host && dev_addr[jj].robot == addr.robot &&
				dev_addr[jj].interf == addr.interf && dev_addr[jj].index == addr.index) // If we have the right robot.
		{
			if (Renderer2D[jj])
			{
				Renderer2D[jj]->Lock();
				Graphics2DRenderer::ClientElementsMap::iterator found = Renderer2D[jj]->ClientElements.find(&(*queue));
				if (found != Renderer2D[jj]->ClientElements.end())
					Renderer2D[jj]->ClientElements.erase(found);
				else
					PLAYER_WARN("Client unsubscribed without subscribing.");
				Renderer2D[jj]->Unlock();
			}
			else if (Renderer3D[jj])
			{
				Renderer3D[jj]->Lock();
				Graphics3DRenderer::ClientElementsMap::iterator found = Renderer3D[jj]->ClientElements.find(&(*queue));
				if (found != Renderer3D[jj]->ClientElements.end())
					Renderer3D[jj]->ClientElements.erase(found);
				else
					PLAYER_WARN("Client unsubscribed without subscribing.");
				Renderer3D[jj]->Unlock();
			}
		}
	}
	std::list<QueuePointer>::iterator iter = std::find(subscriberQueues.begin(), subscriberQueues.end(), queue);
	if (iter == subscriberQueues.end())
		PLAYER_WARN("Client unsubscribed without subscribing.");
	subscriberQueues.erase(iter);
	return 0;
}

int ARDevPlugin::ProcessMessage(QueuePointer & resp_queue,
                                  player_msghdr * hdr,
                                  void * data)
{
	for (unsigned int jj = 0; jj < MaxRobots; ++jj)
	{
		// if its 2d then check for graphics2d commands
		if (Renderer2D[jj])
		{
			if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
		                           PLAYER_GRAPHICS2D_CMD_CLEAR,
		                           this->dev_addr[jj]))
			{
				Renderer2D[jj]->Lock();
				if (Renderer2D[jj]->ClientElements.find(&(*resp_queue)) == Renderer2D[jj]->ClientElements.end())
					PLAYER_ERROR("Client has not subscribed");
				else
					Renderer2D[jj]->ClientElements[&(*resp_queue)].clear();
				Renderer2D[jj]->Unlock();
				return 1;
			}

			if(	Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
		                           PLAYER_GRAPHICS2D_CMD_POINTS,
		                           this->dev_addr[jj]))
			{
				player_graphics2d_cmd_points_t * description = reinterpret_cast<player_graphics2d_cmd_points_t *> (data);
				GraphicsElement Temp;
				Temp.Type = hdr->subtype;
				Temp.r = static_cast<float> (description->color.red)/255.0;
				Temp.g = static_cast<float> (description->color.green)/255.0;
				Temp.b = static_cast<float> (description->color.blue)/255.0;
				Temp.a = 1-static_cast<float> (description->color.alpha)/255.0;
				for (unsigned int i = 0; i < description->points_count; ++i)
				{
					Temp.Points.push_back(description->points[i]);
				}
				Renderer2D[jj]->Lock();
				if (Renderer2D[jj]->ClientElements.find(&(*resp_queue)) == Renderer2D[jj]->ClientElements.end())
					PLAYER_ERROR("Client has not subscribed");
				else
					Renderer2D[jj]->ClientElements[&(*resp_queue)].push_back(Temp);
				Renderer2D[jj]->Unlock();
				return 1;
			}

			if(	Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
		                           PLAYER_GRAPHICS2D_CMD_POLYLINE,
		                           this->dev_addr[jj]))
			{
				player_graphics2d_cmd_polyline_t * description = reinterpret_cast<player_graphics2d_cmd_polyline_t *> (data);
				GraphicsElement Temp;
				Temp.Type = hdr->subtype;
				Temp.r = static_cast<float> (description->color.red)/255.0;
				Temp.g = static_cast<float> (description->color.green)/255.0;
				Temp.b = static_cast<float> (description->color.blue)/255.0;
				Temp.a = 1-static_cast<float> (description->color.alpha)/255.0;
				for (unsigned int i = 0; i < description->points_count; ++i)
				{
					Temp.Points.push_back(description->points[i]);
				}
				Renderer2D[jj]->Lock();
				if (Renderer2D[jj]->ClientElements.find(&(*resp_queue)) == Renderer2D[jj]->ClientElements.end())
					PLAYER_ERROR("Client has not subscribed");
				else
					Renderer2D[jj]->ClientElements[&(*resp_queue)].push_back(Temp);
				Renderer2D[jj]->Unlock();
				return 1;
			}

			if(	Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
		                           PLAYER_GRAPHICS2D_CMD_POLYGON,
		                           this->dev_addr[jj]))
			{
				player_graphics2d_cmd_polygon_t * description = reinterpret_cast<player_graphics2d_cmd_polygon_t *> (data);
				GraphicsElement Temp;
				Temp.Type = hdr->subtype;
				Temp.r = static_cast<float> (description->color.red)/255.0;
				Temp.g = static_cast<float> (description->color.green)/255.0;
				Temp.b = static_cast<float> (description->color.blue)/255.0;
				Temp.a = 1-static_cast<float> (description->color.alpha)/255.0;
				Temp.fr = static_cast<float> (description->fill_color.red)/255.0;
				Temp.fg = static_cast<float> (description->fill_color.green)/255.0;
				Temp.fb = static_cast<float> (description->fill_color.blue)/255.0;
				Temp.fa = 1-static_cast<float> (description->fill_color.alpha)/255.0;
				Temp.Filled = description->filled != 0;
				for (unsigned int i = 0; i < description->points_count; ++i)
				{
					Temp.Points.push_back(description->points[i]);
				}
				Renderer2D[jj]->Lock();
				if (Renderer2D[jj]->ClientElements.find(&(*resp_queue)) == Renderer2D[jj]->ClientElements.end())
					PLAYER_ERROR("Client has not subscribed");
				else
					Renderer2D[jj]->ClientElements[&(*resp_queue)].push_back(Temp);
				Renderer2D[jj]->Unlock();
				return 1;
			}
		}
		else if (Renderer3D[jj])
		{
			if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
		                           PLAYER_GRAPHICS3D_CMD_CLEAR,
		                           this->dev_addr[jj]))
			{
				Renderer3D[jj]->Lock();
				if (Renderer3D[jj]->ClientElements.find(&(*resp_queue)) == Renderer3D[jj]->ClientElements.end())
					PLAYER_ERROR("Client has not subscribed");
				else
					Renderer3D[jj]->Clear(&(*resp_queue));
				Renderer3D[jj]->Unlock();
				return 1;
			}

			if(	Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
		                           PLAYER_GRAPHICS3D_CMD_DRAW,
		                           this->dev_addr[jj]))
			{
				player_graphics3d_cmd_draw_t * gdata = new player_graphics3d_cmd_draw_t;
				memcpy(gdata, data, sizeof(player_graphics3d_cmd_draw_t));
				Renderer3D[jj]->Lock();
				if (Renderer3D[jj]->ClientElements.find(&(*resp_queue)) == Renderer3D[jj]->ClientElements.end())
					PLAYER_ERROR("Client has not subscribed");
				else
					Renderer3D[jj]->ClientElements[&(*resp_queue)].push_back(gdata);
				Renderer3D[jj]->Unlock();
				return 1;
			}
		}
	}
	return(-1);
}


////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
extern "C" {
  int player_driver_init(DriverTable* table)
  {
    puts("ARDevPlugin driver initializing");
    const char * Temp = "";
    ARDevInit(1,&Temp);
    ARDevPlugin_Register(table);
    puts("ARDevPlugin driver done");
    return(0);
  }
}

