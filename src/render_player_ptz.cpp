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
#include <GL/gl.h>
#include <GL/glut.h>

#include <math.h>

#include <ardev/debug.h>

RenderPlayerPTZ::RenderPlayerPTZ(PlayerClientInterface & pci, double _aspect, double _fov)
{
	aspect = _aspect;
	fov = _fov;
	Proxy = NULL;
	PlayerInterface = &pci;
}

RenderPlayerPTZ::~RenderPlayerPTZ()
{
	delete Proxy;
}

int RenderPlayerPTZ::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerPTZ::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerPTZ::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	delete Proxy;
	Proxy = new PtzProxy(pci,0);
	if (Proxy == NULL)
		dbg_print(ARDBG_ERR,"Error Creating Ptz Proxy\n");
	Unlock();
}

void RenderPlayerPTZ::PlayerTerm()
{
	Lock();
	delete Proxy;	
	Proxy = NULL;
	Unlock();
}

void RenderPlayerPTZ::RenderTransparent()
{
	if (Proxy == NULL)
		return;
		
	glColor4f(0.0,1.0,0.2,0.3);

//	Proxy->Lock();
	glPushMatrix();
	glRotatef(RTOD(Proxy->GetPan()),0,0,1);
	glRotatef(RTOD(Proxy->GetTilt()),0,1,0);
	
	double lfov;
	if (fov == 0)
		lfov = Proxy->GetZoom();
	else
		lfov = fov;
	
	double range = 1.0;
	double endwidth = range * tan(lfov)/2;
	double endheight = endwidth/aspect;
//	Proxy->Unlock();
	
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0,0,0);
	glVertex3f(range,endwidth,endheight);
	glVertex3f(range,-endwidth,endheight);
	glVertex3f(range,-endwidth,-endheight);
	glVertex3f(range,endwidth,-endheight);
	glVertex3f(range,endwidth,endheight);
	glEnd();
	
	glPopMatrix();
}

void RenderPlayerPTZ::Render()
{
	if (Proxy == NULL)
		return;
		
	glColor4f(0.0,1.0,0.2,1.0);

//	Proxy->Lock();
	glPushMatrix();
	glRotatef(RTOD(Proxy->GetPan()),0,0,1);
	glRotatef(RTOD(Proxy->GetTilt()),0,1,0);
	
	double lfov;
	if (fov == 0)
		lfov = Proxy->GetZoom();
	else
		lfov = fov;
	
	double range = 1.0;
	double endwidth = range * tan(lfov)/2;
	double endheight = endwidth/aspect;
//	Proxy->Unlock();
	
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(range,endwidth,endheight);
	glVertex3f(0,0,0);
	glVertex3f(range,-endwidth,endheight);
	glVertex3f(0,0,0);
	glVertex3f(range,-endwidth,-endheight);
	glVertex3f(0,0,0);
	glVertex3f(range,endwidth,-endheight);
	glEnd();
	
	glPopMatrix();
}
