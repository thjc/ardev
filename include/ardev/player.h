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
//
// File: ./include/ardev/player.h
// Created by: User <Email>
// Created on: Mon May 10 16:36:21 2004
//

#ifndef _ARDEV_PLAYER_H_
#define _ARDEV_PLAYER_H_

#include <ardev/ardevconfig.h>
#include <ardev/ardev.h>

#include <ardev/anttweakbar.h>

#ifdef HAVE_PLAYER

#include <geos_c.h>

#undef signals
#include <libplayerc++/playerc++.h>

#include <deque>
#include <map>

using namespace PlayerCc;
using namespace std;


/** \brief Base Player Object class
 *
 * If you wish to interact with the player client interface class then you should inherit form this class
 * and callt he AddChild method on the player client interface. This will in turn call the PlayerInit
 * function on the child. As player server connections are lost and reconnected the Platerinit and PlayerTerm
 * methods will be called for each registered child object.
 */
class PlayerObject
{
	public:
		PlayerObject() {};
		virtual ~PlayerObject() {};

		virtual void PlayerInit(PlayerClient * pci) = 0;
		virtual void PlayerTerm() = 0;
};


/** \brief Base Player Client object used to access a player servers
 *
 * Multiple player objects can connect through one player client. The player client object has the thread that is used to update the
 * Player data from that servers
 */
class PlayerClientInterface : public ARObject
{
	public:
		/// Constructor, takes the server name and port
		PlayerClientInterface(const char* name, int numServers = 1, const char ** PlayerServers=&DefServerName, const int* PlayerPorts=&DefServerPort);

		/// Destructor
		~PlayerClientInterface();

		/// Initialises the update thread and create client object
		int Initialise(bool Active = true);

		/// Terminate the update thread and clean up client object
		void Terminate();

		/// The Main update Thread
		void * Main();

		/// Return the PlayerClient Object
		PlayerClient * GetPlayerClient() {return Robot;};

		/// Add a child
		void AddChild(PlayerObject & NewChild);

		/// remove a child
		void RemoveChild(PlayerObject & Child);

		map<char*,int> GetPlayerServers()
		{
			map<char*,int> servers;
			for(int i=0;i< NumServers;i++)
				servers[PlayerServers[i]] = PlayerPorts[i];

			return servers;
		}

		/// process events
		virtual void Event(EventObject* event);
		static bool AutoConnect;
	protected:

		int NumServers;
		char ** PlayerServers;
		int* PlayerPorts;
		const char* Name;
		PlayerClient * Robot;
		list<PlayerObject *> Children;
		//bool Connected;

		static const char* DefServerName; // localhost
		static const int DefServerPort = 6665;
};



/** \brief Position Object interfacing to Player Position3d interface
 */
class PositionPlayer3d : public PositionObject, public PlayerObject
{
	public:
		/// Constructor, requires PlayerClientInterface
		PositionPlayer3d(PlayerClientInterface & pci);
		/// Destructor
		virtual ~PositionPlayer3d();

		/// Initialise()
		int Initialise(bool Active = true);

		/// Terminate (deletes proxy)
		void Terminate();

		/// Return the Current position
		ARPosition GetPosition();
		virtual void AdjustPosition(ARPosition pos){};

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

	protected:
		// PositionPlayer variables

		PlayerClientInterface * PlayerInterface; ///< PlayerClient object connection
		Position3dProxy * Position;	///< Position Proxy
		ARPosition Pos; ///< Current Position
};

/** \brief PositionObject inteface to player position interface
 */
class PositionPlayer : public PositionObject, public PlayerObject
{
	public:
		/// Constructor, requires PlayerClientInterface
		PositionPlayer(PlayerClientInterface & pci, int index);
		/// Destructor
		virtual ~PositionPlayer();
		/// Change player client
		void UpdateClient(PlayerClientInterface & pci);
		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();
		/// Get Current Position
		ARPosition GetPosition();
		virtual void AdjustPosition(ARPosition pos){};

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

	protected:
		// PositionPlayer variables

		PlayerClientInterface * PlayerInterface; ///< PlayerClient object connection
		Position2dProxy * Position; ///< Position Proxy
		ARPosition Pos; ///< Current Position
		int index;
};


/** class to capture camera position data from player ...
 * uses the ptz interface...
 * physical parameters are a 3d position of the pan tilt unit on the robot
 * and the physical posistion of the camera from the pan tilt center of rotation
 * also needed are the y fov (rad) and the aspect ratio
 */
class PositionPlayerPTZ : public PositionObject, public PlayerObject
{
	public:
		/// Constructor, Paramters as as per CameraObject with the additino of the Player server and port
		PositionPlayerPTZ(PlayerClientInterface & pci);
		/// Destructor
		virtual ~PositionPlayerPTZ();
		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// Get Current Position
		ARPosition GetPosition();
		virtual void AdjustPosition(ARPosition pos){};

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

	protected:
		PlayerClientInterface * PlayerInterface; ///< PlayerClient object connection
		PtzProxy * Position; ///< PTZ Proxy
		ARPosition Pos; ///< Current Position
};


/** \brief Renders information from the Player actarray interface
 */
class RenderPlayerActArray : public RenderObject, public PlayerObject
{
	public:
		/// Constructor for the Player Sonar object, uses supplied server and port
		RenderPlayerActArray(PlayerClientInterface & pci, const ARColour & aColour, vector<int> ordering, int Index = 0);
		/// Desctructor
		virtual ~RenderPlayerActArray();

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		/// Renders the actarray data in opengl (solid layer)
		void Render();

		/// Renders the actarray data in opengl (tranparent layer)
		void RenderTransparent();

	private:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		ActArrayProxy * Proxy; ///< The proxy
		int Index; ///< The player interface index
		vector<int> Ordering; ///< Order to render actuators

		double Width; ///< The width to render the actarray linkages
};

class PositionActArray: public PositionRenderable
{
	public:
		PositionActArray(ARPosition in, int index) :PositionRenderable(in,RPE_ActArray,index){};
};

/** \brief Renders information from the Player bumper interface
 */
class RenderPlayerBumper : public RenderObject, public PlayerObject
{
	public:
		/// Constructor for the Player Bumper object, uses supplied server and port
		RenderPlayerBumper(PlayerClientInterface & pci, int Index = 0, double Height = 0.1);
		/// Desctructor
		virtual ~RenderPlayerBumper();

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		/// Renders the bumper data in opengl
		void RenderTransparent();

	private:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		BumperProxy * Bumper; ///< The bumper proxy
		player_bumper_geom_t BumperGeom; ///< Storage for the bumper geom

		double Height; ///< The height of a bumper panel
};


/** \brief Renders information from the Player ir interface
 */
class RenderPlayerIr : public RenderObject, public PlayerObject
{
	public:
		/// Constructor for the Player Ir object, uses supplied server and port
		RenderPlayerIr(PlayerClientInterface & pci);
		/// Destructor
		virtual ~RenderPlayerIr();

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		void RenderTransparent(); ///< Render the IR data in openGL

	private:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		IrProxy * Ir; ///< The player IR Proxy
};

/** \brief Renders information from the Player laser interface
 */
class RenderPlayerLaser : public RenderObject, public PlayerObject
{
	public:
		/// Constructor for the Player Laser object, uses supplied player client object
		/// Colour sets the rendering colour
		/// RayInterval defines the scan interval to draw solid rays in the data (-1 for none)
		/// WithOutline defines wether to draw an outline around the laser data.
		RenderPlayerLaser(PlayerClientInterface & pci, const ARColour & aColour, int aRayInterval = 20, bool aWithOutline = true, int aIndex = 0);
		virtual ~RenderPlayerLaser(); ///< Destructor

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();


		void RenderTransparent(); ///< Render the laser object

		void Render(); ///< Render the laser object
		/// calculate ray trace distance
		virtual double TraceDistance(const Ray & Offset, const Ray & R, const ARPoint & Rotation);


	private:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		LaserProxy * Laser; ///< The LaserProxy Object

		double MaxRange;
		player_pose3d_t pose;

		bool WithOutline;
		int RayInterval;
		int Index;
};

/** \brief Renders information from the Player limb interface
 */
class RenderPlayerLimb : public RenderObject, public PlayerObject
{
	public:
		/// Constructor for the Player Sonar object, uses supplied server and port
		RenderPlayerLimb(PlayerClientInterface & pci, const ARColour & aColour, int Index = 0, double Radius = 0.05);
		/// Desctructor
		virtual ~RenderPlayerLimb();

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		/// Renders the actarray data in opengl (solid layer)
		void Render();

		/// Renders the actarray data in opengl (tranparent layer)
		void RenderTransparent();

	private:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		LimbProxy * Proxy; ///< The proxy
		int Index; ///< The player interface index
		player_limb_geom_req_t Geom; ///< Storage for the geom

		double Radius; ///< The width to render the actarray linkages
};

/** \brief Renders information from the Player localise interface
 *
 * Shows a rendering of the player localise interface
 */
class RenderPlayerLocalise : public RenderObject, public PlayerObject
{
	public:
		/// Constructs the object for rendering player maps
		RenderPlayerLocalise(PlayerClientInterface & pci);
		virtual ~RenderPlayerLocalise(); ///< Destructor

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		//void Render(); ///< Render the map in GL
		void RenderTransparent(); ///< Render the map object

	protected:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		LocalizeProxy * Proxy; ///< Player Map Proxy

};

/** \brief Renders information from the Player map interface
 *
 * Shows a rendering of the player map interface
 */
class RenderPlayerMap : public RenderObject, public PlayerObject
{
	public:
		/// Constructs the object for rendering player maps
		RenderPlayerMap(PlayerClientInterface & pci, double _Height = 0);
		virtual ~RenderPlayerMap(); ///< Destructor

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();


		//void Render(); ///< Render the map in GL
		void RenderTransparent(); ///< Render the map object

	protected:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		MapProxy * Proxy; ///< Player Map Proxy
		double Height; ///< The height to render the map blocks at
		int GlList;
};

/** \brief Renders information from the Player map interface
 *
 * Shows a rendering of the player map interface
 */
class RenderPlayerVectorMap : public RenderObject, public PlayerObject
{
	public:
		/// Constructs the object for rendering player maps
		RenderPlayerVectorMap(PlayerClientInterface & pci, double _Height = 0, const ARColour& _Colour = ARColour(), set<int> renderLayers = set<int>(), int Index = 0);
		virtual ~RenderPlayerVectorMap(); ///< Destructor

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		void Render(); ///< Render the map in GL
		void RenderTransparent(); ///< Render the map object

	protected:
		void RenderGeom(const GEOSGeometry *g);///< Called recursivly to render standard geometries

		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		int Index;
		VectorMapProxy * Proxy; ///< Player Map Proxy
		double Height; ///< The height to render the map blocks at
		ARColour Colour;
		int GlList;

		set<int> RenderLayers;
};

/** \brief Renders information from the Player position interface
 *
 * This Renderer records the player odometry information and plots the robot past path
 */
class RenderPlayerPath : public RenderObject, public PlayerObject
{
	public:
		/// Constructs the object with the path colour (R,G,B) and player server and port
		RenderPlayerPath(PlayerClientInterface & pci,ARColour & _Colour,int _Index = 0);
		virtual ~RenderPlayerPath(); ///< Destructor

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();
		/// Change player client
		void UpdateClient(PlayerClientInterface & pci);

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		/// Get history length
		unsigned int GetHistoryLength() const {return HistoryLength;};
		/// set history length
		void SetHistoryLength(const unsigned int NewHistoryLength) {HistoryLength = NewHistoryLength;};

		void Render(); ///< Render the path in GL
		void RenderTransparent(); ///< Render the path object

		virtual void Event(EventObject* event); ///< Respond to events

#ifdef HAVE_ANTTWEAKBAR
		virtual void DisplaySetup(TweakBar* tweakBar, const char* Name);

		// callback for getting path history length
		static void TW_CALL GetHistoryTW(void *value,void *clientData)
		{
			*static_cast<int *>(value) = static_cast<RenderPlayerPath *>(clientData)->GetHistoryLength();
		}

		// callback for setting path history length
		static void TW_CALL SetHistoryTW(const void *value,void *clientData)
		{
			static_cast<RenderPlayerPath *>(clientData)->SetHistoryLength(*static_cast<const int *>(value));
		}
#endif

	protected:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		Position2dProxy * Position; ///< Player Position Proxy
		int Index;

		deque< std::pair<ARPoint, bool> > Points; ///< Past odometry points

		unsigned int HistoryLength; ///< The number of past readings to accumulate, set to -1 to display all readings

};

/** \brief Renders information from the Player PTZ interface
 *
 * Shows a rendering of the player PTZ interface
 */
class RenderPlayerPTZ : public RenderObject, public PlayerObject
{
	public:
		/// Constructs the object with the path colour (R,G,B) and player server and port
		RenderPlayerPTZ(PlayerClientInterface & pci,double _aspect=1.333, double _fov=0);
		virtual ~RenderPlayerPTZ(); ///< Destructor

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();


		void Render(); ///< Render the path in GL
		void RenderTransparent(); ///< Render the path object

	protected:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		PtzProxy * Proxy; ///< Player PTZ Proxy

		double aspect; ///< aspect ratio of camera
		double fov; ///< aspect ratio of camera

};

/** \brief Renders information from the Player sonar interface
 */
class RenderPlayerSonar : public RenderObject, public PlayerObject
{
	public:
		/// Constructor for the Player Sonar object, uses supplied server and port
		RenderPlayerSonar(PlayerClientInterface & pci, int Index = 0);
		/// Desctructor
		virtual ~RenderPlayerSonar();

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		/// Renders the sonar data in opengl
		void RenderTransparent();

	private:
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		SonarProxy * Sonar; ///< The sonar proxy
};

/** \brief Generates an artifical image frame from a simple opengl rendering of a stage map (specified as an 8 bit raw input file)
 *
 * Uses an existing CameraObject(usually a CameraPlayer) which gives the position of the camera.
 * Uses the fake localize driver for the location within the stage world and the Camera Object for the camera orientation relative to this
 */
class CaptureStage : public CaptureObject, public PlayerObject
{
	public:
		//CaptureStage() {pCamera = NULL; Position = NULL; Open=false; TheMap = -1;};
		/// \brief Create a CaptureStage object from the given map on the given server, CameraObject and Player server details for the stage connection
		CaptureStage(const char * RobotName, CameraObject * camera, PositionObject * CameraPosition, PlayerClientInterface & _PlayerInterface);
		/// Destructor
		virtual ~CaptureStage();

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		/// Get the Current Frame
		virtual const ARImage & GetFrame();
		/// returns the maximum possible return frame width
		int GetMaxWidth();
		/// returns the maximum possible return frame height
		int GetMaxHeight();

	protected:
		/// Connection is open and valid
		bool Open;
		ARImage Frame; ///< Current Frame
		CameraObject * pCamera; ///< Pointer to the CameraObject
		PositionObject * pCameraPosition; ///< Pointer to the Camera's Position Object
		int TheMap; ///< The OpenGL Render list for the Map
		void drawMap(); ///< Create OpenGL Render list for the map
		const char * RobotName;

		// player client objects
		PlayerClientInterface * PlayerInterface; ///< The player client proxy
		SimulationProxy * Position; ///< Simulation Proxy for getting robots true position
		MapProxy * Map; ///< Map Proxy
};


/** \brief Grabs a frame from a player camera device
 *
 * This is particularly relevant for capturing frames across a network using the cameracompress player driver
 *
 */
class CapturePlayer : public CaptureObject
{
	public:
		/// \brief Create a CapturePlayer object connected to the given playerclient, it is recommended that the playerclient is
		/// set to pull new mode
		CapturePlayer(const char * PlayerServer="localhost", int PlayerPort=6665, int CameraIndex = 0);
		/// Destructor
		virtual ~CapturePlayer();

		/// The Main update Thread
		void * Main();

		/// Initialise()
		int Initialise(bool Active = true);
		/// Terminate (deletes proxy)
		void Terminate();

		/// Connect to player server
		bool Connect();

		/// initialise the player proxy
		void PlayerInit(PlayerClient * pci);
		/// terminate the player proxy
		void PlayerTerm();

		/// Get the Current Frame
		virtual const ARImage & GetFrame();

		static void* DummyFrameWorker(void* cap);
		void FrameWorker();
		/// returns the maximum possible return frame width
		int GetMaxWidth();
		/// returns the maximum possible return frame height
		int GetMaxHeight();
		
		virtual bool Fresh() {return fresh;};

	protected:
		/// Connection is open and valid
		bool Open;
		ARImage Frame; ///< Current Frame
		bool fresh;

		const char* Server;
		int Port;
		int Index;

		// player client objects
		PlayerClient * Robot; ///< The player client proxy
		CameraProxy * Camera; ///< TruthProxy
};

////////////////////////////////////////////////////////////////////////////////
// The classes needed for the graphics2d render object
////////////////////////////////////////////////////////////////////////////////
class GraphicsElement
{
	public:
		GraphicsElement() {Type = 0;Filled = false;};

		uint32_t Type;
		double r,g,b,a;
		double fr,fg,fb,fa;
		bool Filled;
		vector<player_point_2d_t> Points;
};

class MessageQueue;

class Graphics2DRenderer : public RenderObject
{
	public:
		Graphics2DRenderer() {RegisterForEvents();colourConfig=true;};
		~Graphics2DRenderer() {;};

		void Render();
		void RenderTransparent();

		virtual void Event(EventObject* event); ///< Respond to events

		typedef map< MessageQueue*, vector<GraphicsElement> > ClientElementsMap;
		ClientElementsMap ClientElements;


};

class Graphics3DRenderer : public RenderObject
{
	public:

		typedef map< MessageQueue*, vector<player_graphics3d_cmd_draw_t *> > ClientElementsMap;

		Graphics3DRenderer() {RegisterForEvents(); colourConfig=true;};
		~Graphics3DRenderer()
		{
			for (ClientElementsMap::iterator clientItr = ClientElements.begin(); clientItr != ClientElements.end(); ++clientItr)
				Clear(clientItr->first);
		}

		void Clear(MessageQueue* client)
		{
			for (vector<player_graphics3d_cmd_draw_t*>::iterator itr = ClientElements[client].begin(); itr != ClientElements[client].end(); ++itr)
			{
				delete *itr;
			}
			ClientElements[client].clear();
		}

		void RenderOne(player_graphics3d_cmd_draw_t & item);

		void Render();
		void RenderTransparent();

		virtual void Event(EventObject* event); ///< Respond to events

		ClientElementsMap ClientElements;


};

#endif  // HAVE_PLAYER
#endif	// _ARDEV_PLAYER_H_
