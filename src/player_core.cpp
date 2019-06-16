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
#include <ardev/player.h>

#include <pthread.h>

#include <ardev/debug.h>

const char* PlayerClientInterface::DefServerName = "localhost";
bool PlayerClientInterface::AutoConnect;

/// Constructor, takes the server name and port
PlayerClientInterface::PlayerClientInterface(const char* name, int _NumServers, const char ** _PlayerServers, const int* _PlayerPorts)
{
	NumServers = _NumServers;
	PlayerServers = new char*[NumServers];
	PlayerPorts = new int[NumServers];
	for (int i = 0; i < NumServers; i++)
	{
		PlayerServers[i] = strdup(_PlayerServers[i]);
		PlayerPorts[i] = _PlayerPorts[i];
	}
	Robot = NULL;
	AutoConnect = true;
	Name = strdup(name);

	RegisterForEvents();

	//Robot->SetDataMode(PLAYER_DATAMODE_PULL_NEW);
}

/// Destructor
PlayerClientInterface::~PlayerClientInterface()
{

	// This is just in case, thread shouldnt be running if we are dsetroying the object
	// unless we are terminating the app and then well......
	if (thread || Robot)
		Terminate();

	for (int i = 0; i < NumServers; i++)
		free(PlayerServers[i]);
	delete [] PlayerServers;
	delete [] PlayerPorts;
	delete Name;
}

/// Initialises the update thread and create client object
int PlayerClientInterface::Initialise(bool Active)
{
	dbg_print(ARDBG_INFO,"Player Client Interface Initilised\n");
	if (!Active)
		Pause();

	StartThread();
	return 0;
}

/// Terminate the update thread and clean up client object
void PlayerClientInterface::Terminate()
{
	StopThread();

	if (!IsActive)
		Resume();

	delete Robot;
	Robot = NULL;
}

/// The Main update Thread
void * PlayerClientInterface::Main()
{
	Lock();
	for (;;)
	{
		Unlock();
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
		pthread_testcancel();
		usleep(1000);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
		Lock();
		try
		{
			if (!Robot)
			{
				// Wait a little before connecting. Get broken sockets otherwise
				usleep(250000);
				if(PlayerClientInterface::AutoConnect)
				{
					PlayerClient * NewRobot = NULL;
					for (int i = 0; i < NumServers; i++)
					{
						Unlock();
						try
						{
							NewRobot = new PlayerClient(PlayerServers[i], PlayerPorts[i]);
						}
						catch (const PlayerError& x)
						{
							NewRobot = NULL;
						}
						Lock();
						if (NewRobot)
							break;
					}
					dbg_print(ARDBG_VERBOSE,"Attempting to connect to player server\n");
					if (NewRobot)
					{
						Robot = NewRobot;
						Robot->SetDataMode(PLAYER_DATAMODE_PULL);
						//Robot->SetReplaceRule(true); // Don't assume replace, might be graphics or similar
						dbg_print(ARDBG_VERBOSE, "Connection Attempt Succeeded\n");
						for (list<PlayerObject *>::iterator itr = Children.begin(); itr != Children.end(); ++itr)
							(*itr)->PlayerInit(Robot);
					}
					else
					{
						dbg_print(ARDBG_VERBOSE, "Connection Attempt Failed\n");
						sleep(1);
						continue;
					}
				}
			}

			if (Robot)
			{
				try
				{
					Robot->ReadIfWaiting();
					dbg_print(ARDBG_VERBOSE,"PlayerClient got one round of data\n");
				}
				catch (const PlayerError &e)
				{
					dbg_print(ARDBG_VERBOSE,"Robot update failed, disconnecting\n");
					for (list<PlayerObject *>::iterator itr = Children.begin(); itr != Children.end(); ++itr)
						(*itr)->PlayerTerm();
					delete Robot;
					Robot = NULL;

					dbg_print(ARDBG_VERBOSE,"Disconnect Complete\n");
				}
			}
		}
		catch (const PlayerError & e)
		{
			dbg_print(ARDBG_ERR,"Unhandled player exception: %s in %s\n", e.GetErrorStr().c_str(), e.GetErrorFun().c_str());
			for (list<PlayerObject *>::iterator itr = Children.begin(); itr != Children.end(); ++itr)
				(*itr)->PlayerTerm();
			delete Robot;
			Robot = NULL;
		}
		catch(...)
		{
			fprintf(stderr,"Unknown exception caught\n");
		}

	}
	return NULL;
}


/// Add a child
void PlayerClientInterface::AddChild(PlayerObject & NewChild)
{
	dbg_print(ARDBG_VERBOSE,"adding child\n");
	Lock();
	if (Robot)
	{
		dbg_print(ARDBG_VERBOSE,"intialise child\n");
		NewChild.PlayerInit(Robot);
	}
	Children.push_back(&NewChild);
	Unlock();
}


/// Remove a child
void PlayerClientInterface::RemoveChild(PlayerObject & Child)
{
	dbg_print(ARDBG_VERBOSE,"removing child\n");
	Lock();
	Children.remove(&Child);
	if (Robot)
		Child.PlayerTerm();
	Unlock();
}

void PlayerClientInterface::Event(EventObject* event)
{
	if (event->GetEventType() == POE_DisconnectPlayerInterfaces)
	{
		if(Robot)
		{
			Lock();
			for (list<PlayerObject *>::iterator itr = Children.begin(); itr != Children.end(); ++itr)
				(*itr)->PlayerTerm();
			delete Robot;
			Robot = NULL;

			dbg_print(ARDBG_VERBOSE,"Disconnect Complete\n");
			Unlock();
		}
	}
	else if(event->GetEventType() == POE_AutoOnPlayerInterfaces)
	{
		PlayerClientInterface::AutoConnect = true;
	}
	else if(event->GetEventType() == POE_AutoOffPlayerInterfaces)
	{
		PlayerClientInterface::AutoConnect = false;
	}
}
