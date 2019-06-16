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
#include <assert.h>

/**************************************************
 * Position 3d interface from Player
 *************************************************/

PositionPlayer3d::PositionPlayer3d(PlayerClientInterface & pci) : PositionObject()
{
	PlayerInterface = &pci;
	Position = NULL;
	
}


PositionPlayer3d::~PositionPlayer3d()
{
	if (Position)
		Terminate();
}

int PositionPlayer3d::Initialise(bool Active)
{
	if (!initialised)
	{
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void PositionPlayer3d::Terminate()
{
	PlayerInterface->RemoveChild(*this);
}

void PositionPlayer3d::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	//PlayerInterface->Lock();
	delete Position;
	Position = new Position3dProxy(pci,0);		
	//PlayerInterface->Unlock();
	Unlock();
}

void PositionPlayer3d::PlayerTerm()
{
	Lock();
	//PlayerInterface->Lock();
	delete Position;	
	Position = NULL;
	//PlayerInterface->Unlock();	
	Unlock();
}


ARPosition PositionPlayer3d::GetPosition()
{
	Lock();
	if (Position == NULL)
	{
		Unlock();
		return Pos;
	}
	Unlock();

//	Position->Lock();
	Pos.Origin = ARPoint(Position->GetXPos(), Position->GetYPos(), Position->GetZPos());
	Pos.Direction = ARPoint((Position->GetRoll()),(Position->GetPitch()),(Position->GetYaw()));
//	Position->Unlock();
	
	return Pos;
}

/**************************************************
 * Position PTZ interface from Player
 *************************************************/

PositionPlayerPTZ::PositionPlayerPTZ(PlayerClientInterface & pci) : PositionObject()
{
	PlayerInterface = &pci;
	Position = NULL;
}


PositionPlayerPTZ::~PositionPlayerPTZ()
{
	if (Position)
		Terminate();
}

int PositionPlayerPTZ::Initialise(bool Active)
{
	if (!initialised)
	{
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void PositionPlayerPTZ::Terminate()
{
	PlayerInterface->RemoveChild(*this);
}

void PositionPlayerPTZ::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	//PlayerInterface->Lock();
	delete Position;
	Position = new PtzProxy(pci,0);		
	//PlayerInterface->Unlock();
	Unlock();
}

void PositionPlayerPTZ::PlayerTerm()
{
	Lock();
	//PlayerInterface->Lock();
	delete Position;	
	Position = NULL;
	//PlayerInterface->Unlock();	
	Unlock();
}


ARPosition PositionPlayerPTZ::GetPosition()
{
	Lock();
	if (Position == NULL)
	{
		Unlock();
		return Pos;
	}
	Unlock();

//	Position->Lock();
	Pos.Origin = ARPoint(0,0,0);
	Pos.Direction = ARPoint(1,0,0);
	Pos.Direction.RotateYPR(Position->GetPan(),Position->GetTilt(),0);
//	Position->Unlock();
	
	return Pos;
}

/**************************************************
 * Position interface from Player
 *************************************************/

PositionPlayer::PositionPlayer(PlayerClientInterface & pci, int i) : PositionObject()
{
	PlayerInterface = &pci;
	Position = NULL;
	index = i;
}


PositionPlayer::~PositionPlayer()
{
	delete Position;
}


void PositionPlayer::UpdateClient(PlayerClientInterface & pci)
{
	PlayerInterface = &pci;
}

int PositionPlayer::Initialise(bool Active)
{
	if (!initialised)
	{
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void PositionPlayer::Terminate()
{
	PlayerInterface->RemoveChild(*this);
}

void PositionPlayer::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	//PlayerInterface->Lock();
	delete Position;
	Position = new Position2dProxy(pci,index);		
	//PlayerInterface->Unlock();
	Unlock();
}

void PositionPlayer::PlayerTerm()
{
	Lock();
	//PlayerInterface->Lock();
	delete Position;	
	Position = NULL;
	//PlayerInterface->Unlock();	
	Unlock();
}


ARPosition PositionPlayer::GetPosition()
{
	if (Position == NULL)
		return Pos;
	
//	Position->Lock();
	Pos.Origin = ARPoint(Position->GetXPos(), Position->GetYPos(), 0);
	Pos.Direction = ARPoint(0,0,(Position->GetYaw()));
//	Position->Unlock();

	
	return Pos;
}
