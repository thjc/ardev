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
#include <ardev/debug.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <ardev/ardev.h>
#include <ardev/capture.h>
using namespace std;

/// Constructor, takes the server name and port
CapturePlayer::CapturePlayer(const char * PlayerServer, int PlayerPort, int CameraIndex)
{
	fresh = false;
	Open = false;
	Server = strdup(PlayerServer);
	Port = PlayerPort;
	Index = CameraIndex;
}

/// Destructor
CapturePlayer::~CapturePlayer()
{
	// This is just in case, thread shouldnt be running if we are dsetroying the object
	// unless we are terminating the app and then well......
	if (thread || Robot)
		Terminate();
	delete Server;
}

/// Initialises the update thread and create client object
int CapturePlayer::Initialise(bool Active)
{
	dbg_print(ARDBG_INFO,"Player Client Interface Initilised\n");
	if (!Active)
		Pause();

	StartThread();
	return 0;
}

/// Tries to connect to player server
bool CapturePlayer::Connect()
{
	try
	{
		dbg_print(ARDBG_VERBOSE,"Creating connection to player server %s,%d\n",Server,Port);
		Robot = new PlayerClient(Server, Port);
		if (Robot == NULL)
		{
			dbg_print(ARDBG_ERR,"Unable to connect to the Player server on %s,%d\n",Server,Port);
			return false;
		}

		Robot->SetDataMode(PLAYER_DATAMODE_PULL);
		Robot->SetReplaceRule(true);

		dbg_print(ARDBG_VERBOSE,"Creating Camera Proxy index=%d\n",Index);
		Camera = new CameraProxy(Robot,Index);
		if (Camera == NULL)
		{
			dbg_print(ARDBG_ERR,"Unable to Open the camera proxy, index=%d\n",Index);
			return false;
		}

		Open = true;
		ARDev::PublishEvent(new EventObject(OE_CaptureConnect,true));
	}
	catch(...)
	{
		dbg_print(ARDBG_VERBOSE,"Unable to Open the camera proxy, index=%d\n",Index);
		return false;
	}
	return true;
}

/// Terminate the update thread and clean up client object
void CapturePlayer::Terminate()
{
	StopThread();

	if (!IsActive)
		Resume();

	while(thread != 0){} //wait for thread cleanup

	delete Robot;
	Robot = NULL;
}

/// The Main update Thread
void * CapturePlayer::Main()
{
	for (;;)
	{
		pthread_testcancel();
		Lock();

		if(!Open)
		{
			if(!Connect())
			{
				Unlock();
				sleep(1);
				continue;
			}
		}
		else
		{
	//		Camera->fresh = false;
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
			try
			{
				if (Robot->Peek())
				{
					Robot->Read();
					fresh = true;

					dbg_print(ARDBG_VERBOSE,"Got one round of player data\n");
				}
			}
			catch(PlayerError & e)
			{
				dbg_print(ARDBG_ERR,"Robot update failed.\n");
				//try to reconnect
				Open = Connect();
				Unlock();
				continue;
			}
		}

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
		Unlock();
		usleep(20000);
	}
	return NULL;
}

/// Get the Current Frame
const ARImage & CapturePlayer::GetFrame()
{
	//Lock();

	if (fresh)
	{
		if (Camera->GetCompression() != PLAYER_CAMERA_COMPRESS_RAW)
			Camera->Decompress();
		Frame.y_size = Camera->GetHeight();
		Frame.x_size= Camera->GetWidth();
		Frame.ByteDepth = Camera->GetDepth()/8;
		unsigned char * data = Frame.Allocate();
		Camera->GetImage(data);
		fresh = false;

	}
	//Unlock();
	return Frame;
}

void* CapturePlayer::DummyFrameWorker(void* cap)
{
	CapturePlayer* Cap = reinterpret_cast<CapturePlayer*>(cap);
	Cap->FrameWorker();
	pthread_exit(NULL);
}

void CapturePlayer::FrameWorker()
{
	if (Camera->GetCompression() != PLAYER_CAMERA_COMPRESS_RAW)
		Camera->Decompress();
	Frame.y_size = Camera->GetHeight();
	Frame.x_size= Camera->GetWidth();
	Frame.ByteDepth = Camera->GetDepth()/8;
	unsigned char * data = Frame.Allocate();

	if(fresh)
	{
		Lock();
		Camera->GetImage(data);
		fresh = false;
		Unlock();
	}
}

/// returns the maximum possible return frame width
int CapturePlayer::GetMaxWidth()
{
	if(!Open)
		return 0;

	int ret;
	Lock();
	ret = Camera->GetWidth();
	Unlock();
	return ret;
}

/// returns the maximum possible return frame height
int CapturePlayer::GetMaxHeight()
{
	if(!Open)
		return 0;

	int ret;
	Lock();
	ret = Camera->GetHeight();
	Unlock();
	return ret;
}
