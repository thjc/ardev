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
#ifndef ARIDE_PLAYERHANDLERS_H
#define ARIDE_PLAYERHANDLERS_H

#include <ardev/player.h>

#include "cm_objecthandler.h"
#include "cm_parameter_ardev.h"

/* --------------------------------------------
    Player Object Handlers
   -------------------------------------------- */
class CaptureStage;
class Position3dPlayerHandler;
class PositionPlayerPTZHandler;

// --- PlayerClientInterfaceHandler Handler ---
class PlayerClientInterfaceHandler : public ArideObjectHandler<PlayerClientInterface>
{
	public:
		PlayerClientInterfaceHandler() :
			ServerNames("PlayerServer","Player Server Address","localhost"),
			ServerPorts("PlayerPort","Player Server Port","6665")
		{
			PlayerInterface=NULL;
			Parameters.push_back(&ServerNames);
			Parameters.push_back(&ServerPorts);
		};

		virtual ~PlayerClientInterfaceHandler() {delete PlayerInterface;};

		PlayerClientInterface & GetObject()
		{
			if (PlayerInterface == NULL)
			{
				QStringList servers = ServerNames.Value;
				QStringList ports = ServerPorts.Value;
				// If there are less values in 1 or the other, just copy the last value.
				for (int i = ports.size(); i < servers.size(); i++)
					ports.push_back(ports.last());
				for (int i = servers.size(); i < ports.size(); i++)
					servers.push_back(servers.last());
				int numServers = std::min(servers.size(), ports.size());
				char** rawServerNames = new char*[numServers];
				int* rawServerPorts = new int[numServers];
				for (int i = 0; i < numServers; i++)
				{
					QByteArray asciiName = servers[i].toAscii(); // Careful of lifetime
					rawServerNames[i] = strdup(asciiName.constData());
					rawServerPorts[i] = ports[i].toInt();
				}
				try
				{
					QByteArray asciiName = Name.Value.toAscii(); // Careful of lifetime
					PlayerInterface = new PlayerClientInterface(asciiName, numServers, (const char**)rawServerNames, rawServerPorts);
				}
				catch (...)
				{
					for (int i = 0; i < numServers; i++)
						free(rawServerNames[i]);
					delete [] rawServerNames;
					delete [] rawServerPorts;
					throw;
				}
				for (int i = 0; i < numServers; i++)
					free(rawServerNames[i]);
				delete [] rawServerNames;
				delete [] rawServerPorts;

				if (PlayerInterface == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			}
			CurrentProject->InitialiseObject(PlayerInterface);
			return *PlayerInterface;
		};
		void RemoveObject() { delete PlayerInterface; PlayerInterface = NULL;};

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new PlayerClientInterfaceHandler);};

	protected:
		ARListParameter ServerNames;
		ARListParameter ServerPorts;
		PlayerClientInterface * PlayerInterface;
};

// --- CapturePlayer Handler ---
class CapturePlayerHandler : public CaptureObjectHandler
{
	public:
		CapturePlayerHandler() :
			ServerName("PlayerServer","Player Server Address","localhost"),
			ServerPort("PlayerPort","Player Server Port","6665"),
			CameraIndex("CameraIndex","Player Camera Index","0")
		{
			Parameters.push_back(&ServerName);
			Parameters.push_back(&ServerPort);
			Parameters.push_back(&CameraIndex);
			Obj=NULL;
		};

		virtual ~CapturePlayerHandler() {delete Obj;};

		CaptureObject & GetObject()
		{
			if (Obj == NULL)
			{
				QByteArray asciiName = ServerName.Value.toAscii(); // Careful of lifetime
				Obj = new CapturePlayer(asciiName, ServerPort.Value, CameraIndex.Value);
				if (Obj == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			}
			CurrentProject->InitialiseObject(Obj);
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL;};

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new CapturePlayerHandler);};

	protected:
		StringParameter ServerName;
		IntParameter ServerPort;
		IntParameter CameraIndex;
		CapturePlayer * Obj;

};

// --- PlayerClientInterface Parameter ---
class PlayerClientInterfaceParameter : public ARObjectParameterBase
{
public:
		PlayerClientInterfaceParameter(QString _Name, QString _Description, QString _DefaultValue="") : ARObjectParameterBase(_Name,_Description,_DefaultValue) {Type="PlayerClientInterface";};
		virtual ~PlayerClientInterfaceParameter() {};

		PlayerClientInterfaceHandler * GetClass()
		{
			if (Value == "")
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			aride_object * temp = CurrentProject->GetObjectByGUID(Value);
			if (temp == NULL)
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			if (temp->Section == ARIDE_MISC && temp->Type == "PlayerClientInterface")
				return reinterpret_cast<PlayerClientInterfaceHandler *> (temp->Handler);
			throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
		};
};

// --- PlayerPosition3d Parameter ---
class PlayerPosition3dParameter : public ARObjectParameterBase
{
public:
		PlayerPosition3dParameter(QString _Name, QString _Description, QString _DefaultValue="") : ARObjectParameterBase(_Name,_Description,_DefaultValue) {Type="Position3dPlayer";};
		virtual ~PlayerPosition3dParameter() {};

		Position3dPlayerHandler * GetClass()
		{
			if (Value == "")
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			aride_object * temp = CurrentProject->GetObjectByGUID(Value);
			if (temp == NULL)
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			if (temp->Section == ARIDE_POSITION && temp->Type == "Position3dPlayer")
				return reinterpret_cast<Position3dPlayerHandler *> (temp->Handler);
			throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
		};
};


template<class Base>
class PlayerHandlerBase : public Base
{
	public:
		PlayerHandlerBase() : PlayerClient("PlayerClient","Player Client Object","")
		{
			Base::Parameters.push_back(&PlayerClient);
		};

		virtual ~PlayerHandlerBase() {};

	protected:
		PlayerClientInterfaceParameter PlayerClient;
};

template <class T, class Base>
class PlayerHandler : public PlayerHandlerBase<Base>
{
	public:
		PlayerHandler() {Obj=NULL;};
		~PlayerHandler() {delete Obj;};

		void RemoveObject() {delete Obj; Obj=NULL;};

	protected:
		T * Obj;
};

// --- Position Object Handlers ---
class PositionPlayerHandler : public PlayerHandlerBase<PositionObjectHandler>
{
	public:
		PositionPlayerHandler() :
			Index("Index","Player Driver Index","0")
		{
			Parameters.push_back(&Index);
			Obj=NULL;
		}
		~PositionPlayerHandler() {delete Obj;Obj=NULL;};

		PositionObject & GetObject()
		{
			if (Obj == NULL)
			{
				if ((Obj = new PositionPlayer(PlayerClient.GetClass()->GetObject(), Index.Value)) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			}

			Obj->UpdateClient(PlayerClient.GetClass()->GetObject()); // Player client may have been deleted.
			CurrentProject->InitialiseObject(Obj);
			return *Obj;
		}

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new PositionPlayerHandler);};

	protected:
		PositionPlayer * Obj;
		IntParameter Index;
};

class Position3dPlayerHandler : public PlayerHandlerBase<PositionObjectHandler>
{
	public:
		Position3dPlayerHandler() {Obj=NULL;};
		~Position3dPlayerHandler() {delete Obj;};

		PositionObject & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new PositionPlayer3d(PlayerClient.GetClass()->GetObject())) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			CurrentProject->InitialiseObject(Obj);

			return *Obj;
		}

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new Position3dPlayerHandler);};

	protected:
		PositionPlayer3d * Obj;

};

class PositionPlayerPTZHandler : public PlayerHandlerBase<PositionObjectHandler>
{
	public:
		PositionPlayerPTZHandler() {Obj=NULL;};
		~PositionPlayerPTZHandler() {delete Obj;};

		PositionObject & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new PositionPlayerPTZ(PlayerClient.GetClass()->GetObject())) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			CurrentProject->InitialiseObject(Obj);

			return *Obj;
		}

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new PositionPlayerPTZHandler);};

	protected:
		PositionPlayerPTZ * Obj;

};


// --- Render Object Handlers ---
class RenderPlayerActArrayHandler : public PlayerHandler<RenderPlayerActArray,RenderObjectHandler>
{
	public:
		RenderPlayerActArrayHandler() :
			Colour("Colour", "Colour","1 0 0 1"),
			Index("Index","Index","0"),
			Order("Order","Actuator Ordering","")
		{
			Obj=NULL;
			Parameters.push_back(&Colour);
			Parameters.push_back(&Index);
			Parameters.push_back(&Order);
		};
		~RenderPlayerActArrayHandler()
		{
			delete Obj;
		};


		RenderPlayerActArray & GetObject()
		{
			if (Obj == NULL)
			{
				QStringList orders = Order.Value;
				// If there are less values in 1 or the other, just copy the last value.
				for (int i = orders.size(); i < orders.size(); i++)
					orders.push_back(orders.last());

				vector<int> rawOrdering;
				for (int i = 0; i < orders.size(); i++)
				{
					rawOrdering.push_back(orders[i].toInt());
				}
				if ((Obj = new RenderPlayerActArray(PlayerClient.GetClass()->GetObject(), Colour.Value,rawOrdering, Index.Value)) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			}

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerActArrayHandler);};
	protected:
		ARColourParameter Colour;
		IntParameter Index;
		ARListParameter Order;
};

// --- Position Object Handlers ---
class PositionActArrayHandler : public PositionObjectHandler
{
	public:
		PositionActArrayHandler();
		~PositionActArrayHandler();

		PositionObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new PositionActArrayHandler);};
	protected:
		PositionRenderable * obj;
		ARPositionParameter pos;
		IntParameter Index;
};


class RenderPlayerBumperHandler : public PlayerHandler<RenderPlayerBumper,RenderObjectHandler>
{
	public:
		RenderPlayerBumperHandler() :
			Index("Index","Interface Index","0"),
			Height("Height","Bumper Panel Height","0.1")
		{
			Obj=NULL;
			Parameters.push_back(&Index);
			Parameters.push_back(&Height);
		};
		~RenderPlayerBumperHandler()
		{
			delete Obj;
		};

		RenderPlayerBumper & GetObject()
		{
			if (Obj == NULL)
			{
				if ((Obj = new RenderPlayerBumper(PlayerClient.GetClass()->GetObject(),Index.Value, Height.Value)) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			}

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerBumperHandler);};

	protected:
		IntParameter Index;
		DoubleParameter Height;
};



class RenderPlayerIrHandler : public PlayerHandler<RenderPlayerIr,RenderObjectHandler>
{
	public:
		RenderPlayerIr & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new RenderPlayerIr(PlayerClient.GetClass()->GetObject())) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerIrHandler);};
};



class RenderPlayerLaserHandler : public PlayerHandler<RenderPlayerLaser,RenderObjectHandler>
{
	public:
		RenderPlayerLaserHandler() :
			Colour("Colour", "Colour","1 0 0 1"),
		RayInterval("RayInterval","Ray Interval","20"),
		WithOutline("WithOutline","With Outline","true"),
		Index("Index","Player Driver Index","0")
		{
			Obj=NULL;
			Parameters.push_back(&Colour);
			Parameters.push_back(&RayInterval);
			Parameters.push_back(&WithOutline);
			Parameters.push_back(&Index);
		};
		~RenderPlayerLaserHandler()
		{
			delete Obj;
		};


		RenderPlayerLaser & GetObject()
		{
			if (Obj == NULL)
			{
				if ((Obj = new RenderPlayerLaser(PlayerClient.GetClass()->GetObject(), Colour.Value, RayInterval.Value, WithOutline.Value, Index.Value)) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			}

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerLaserHandler);};
	protected:
		ARColourParameter Colour;
		IntParameter RayInterval;
		BooleanParameter WithOutline;
		IntParameter Index;
};

class RenderPlayerLimbHandler : public PlayerHandler<RenderPlayerLimb,RenderObjectHandler>
{
	public:
		RenderPlayerLimbHandler() :
			Colour("Colour", "Colour","1 0 0 1"),
			Index("Index","Index","0"),
			Radius("Radius","Radius","0.05")
		{
			Obj=NULL;
			Parameters.push_back(&Colour);
			Parameters.push_back(&Index);
			Parameters.push_back(&Radius);
		};
		~RenderPlayerLimbHandler()
		{
			delete Obj;
		};


		RenderPlayerLimb & GetObject()
		{
			if (Obj == NULL)
			{
				if ((Obj = new RenderPlayerLimb(PlayerClient.GetClass()->GetObject(), Colour.Value, Index.Value, Radius.Value)) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			}

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerLimbHandler);};
	protected:
		ARColourParameter Colour;
		IntParameter Index;
		DoubleParameter Radius;

};


class RenderPlayerLocaliseHandler : public PlayerHandler<RenderPlayerLocalise,RenderObjectHandler>
{
	public:
		RenderPlayerLocalise & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new RenderPlayerLocalise(PlayerClient.GetClass()->GetObject())) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerLocaliseHandler);};
};


class RenderPlayerMapHandler : public PlayerHandlerBase<RenderObjectHandler>
{
	public:
		RenderPlayerMapHandler() : Height("Height","Height of map blocks, 0 for 2d","0") {Obj=NULL;Parameters.push_back(&Height);};
		~RenderPlayerMapHandler() {delete Obj;};

		RenderPlayerMap & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new RenderPlayerMap(PlayerClient.GetClass()->GetObject(), Height.Value)) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerMapHandler);};
	protected:
		RenderPlayerMap * Obj;
		DoubleParameter Height;
};

#ifdef HAVE_GEOS
class RenderPlayerVectorMapHandler : public PlayerHandlerBase<RenderObjectHandler>
{
	public:
		RenderPlayerVectorMapHandler() : Height("Height","Height of map blocks, 0 for 2d","0"), Colour("Colour","Colour to render","1 1 1 1"),
		Index("Index","Player device Index","0") , RenderLayers("RenderLayers","Layers to render, -1 for ALL Layers","-1")
		{
			Obj=NULL;Parameters.push_back(&Height);
			Parameters.push_back(&Colour);
			Parameters.push_back(&RenderLayers);
			Parameters.push_back(&Index);
		};
		~RenderPlayerVectorMapHandler() {delete Obj;};

		RenderPlayerVectorMap & GetObject()
		{
			if (Obj == NULL)
			{
				set<int> layers;
				for (int i = 0; i < RenderLayers.Value.size(); i++)
					layers.insert(RenderLayers.Value[i].toInt());
				if ((Obj = new RenderPlayerVectorMap(PlayerClient.GetClass()->GetObject(), Height.Value, Colour.Value,layers,Index.Value)) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			}

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerVectorMapHandler);};
	protected:
		RenderPlayerVectorMap * Obj;
		DoubleParameter Height;
		ARColourParameter Colour;
		IntParameter Index;
		ARListParameter RenderLayers;
};

#endif

class RenderPlayerPathHandler : public PlayerHandlerBase<RenderObjectHandler>
{
	public:
		RenderPlayerPathHandler() : Colour("Colour","Path Colour","1 0 0 "), Index("Index", "Player Driver Index", "0")
		{
			Obj=NULL;
			Parameters.push_back(&Colour);
			Parameters.push_back(&Index);
		}
		~RenderPlayerPathHandler() {delete Obj;};

		RenderObject & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new RenderPlayerPath(PlayerClient.GetClass()->GetObject(),Colour.Value,Index.Value)) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			Obj->UpdateClient(PlayerClient.GetClass()->GetObject()); // Player client may have been deleted.
			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		}

		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerPathHandler);};

	protected:
		RenderPlayerPath * Obj;
		ARColourParameter Colour;
		IntParameter Index;
};

class RenderPlayerPTZHandler : public PlayerHandlerBase<RenderObjectHandler>
{
	public:
		RenderPlayerPTZHandler() :
			Aspect("Aspect","Aspect Ratio","1.3333"),
			FOV("FOV","FOV (deg). 0 to read from player","0")
		{Obj=NULL;Parameters.push_back(&Aspect);Parameters.push_back(&FOV);};
		~RenderPlayerPTZHandler() {delete Obj;};

		RenderObject & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new RenderPlayerPTZ(PlayerClient.GetClass()->GetObject(),Aspect.Value, FOV.Value*M_PI/180.0)) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		}

		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerPTZHandler);};

	protected:
		RenderPlayerPTZ * Obj;
		DoubleParameter Aspect;
		DoubleParameter FOV;
};

class RenderPlayerSonarHandler : public PlayerHandler<RenderPlayerSonar,RenderObjectHandler>
{
	public:
		RenderPlayerSonar & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new RenderPlayerSonar(PlayerClient.GetClass()->GetObject())) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		};
		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderPlayerSonarHandler);};

};

class RenderGraphics2DHandler : public RenderObjectHandler
{
	public:
		RenderGraphics2DHandler() {Obj=NULL;};
		~RenderGraphics2DHandler() {delete Obj;};

		RenderObject & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new Graphics2DRenderer) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		}

		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderGraphics2DHandler);};

	protected:
		Graphics2DRenderer * Obj;
};

class RenderGraphics3DHandler : public RenderObjectHandler
{
	public:
		RenderGraphics3DHandler() {Obj=NULL;};
		~RenderGraphics3DHandler() {delete Obj;};

		RenderObject & GetObject()
		{
			if (Obj == NULL)
				if ((Obj = new Graphics3DRenderer) == NULL)
					throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);

			CurrentProject->InitialiseObject(Obj);
			BaseObject = Obj;
			return *Obj;
		}

		void RemoveObject() { delete Obj; Obj = NULL; BaseObject = Obj;};
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderGraphics3DHandler);};

	protected:
		Graphics3DRenderer * Obj;
};

// --- Capture Object Handlers ---

class CaptureStageHandler : public PlayerHandlerBase<CaptureObjectHandler>
{
	public:
		CaptureStageHandler();
		~CaptureStageHandler();

		CaptureObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new CaptureStageHandler);};
	protected:
		CaptureStage * Capture;
		CameraObjectParameter *CameraObject;
		PositionObjectParameter *CameraPositionObject;
		StringParameter *RobotName;
};


// ----------------------------------------------

void RegisterPlayerObjectHandlers();

#endif
