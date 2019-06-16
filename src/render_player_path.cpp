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
#include <assert.h>

RenderPlayerPath::RenderPlayerPath(PlayerClientInterface & pci,ARColour & _Colour,int _Index) : RenderObject(_Colour)
{
	Position = NULL;
	
	alphaConfig = false;
	//Colour = _Colour;
	PlayerInterface = &pci;
	Index = _Index;
	
	HistoryLength = -1; // Display all readings by default.
	RegisterForEvents();
}

RenderPlayerPath::~RenderPlayerPath()
{
	delete Position;
}


int RenderPlayerPath::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerPath::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
	Points.clear();
}

void RenderPlayerPath::UpdateClient(PlayerClientInterface & pci)
{
	PlayerInterface = &pci;
}

void RenderPlayerPath::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	//PlayerInterface->Lock();
	delete Position;
	Position = new Position2dProxy(pci,Index);
	if (Position == NULL)
		dbg_print(ARDBG_ERR,"Error Creating Position Proxy\n");
	//PlayerInterface->Unlock();
	Unlock();
}

void RenderPlayerPath::PlayerTerm()
{
	Lock();
	//PlayerInterface->Lock();
	delete Position;	
	Position = NULL;
	Points.clear();
	//PlayerInterface->Unlock();	
	Unlock();
}


void RenderPlayerPath::Render()
{
	if (Position == NULL)
		return;
		
//	Position->Lock();
	double x,y,theta;
	x = Position->GetXPos();
	y = Position->GetYPos();
	theta = (Position->GetYaw());
	bool stall = Position->GetStall();
//	Position->Unlock();
	if (Points.empty() || Points.front().first.x != x || Points.front().first.y != y)
		if(x != 0 && y != 0 || !Points.empty())
			Points.push_front(std::pair<ARPoint, bool>(ARPoint(x,y,0), stall));
	
	if (Points.size() > HistoryLength && HistoryLength > 0)
		Points.pop_back();
	
	// draw the path
	glPushMatrix();
	glRotatef(-RTOD(theta),0,0,1);
	glLineWidth(3);
	if (Points.size() > 1)
	{
		glBegin(GL_LINE_STRIP);
		for (deque< std::pair<ARPoint, bool> >::const_iterator itr = Points.begin();itr != Points.end(); ++itr)
		{
			// check if this reading or the previous one are stalls
			deque< std::pair<ARPoint, bool> >::const_iterator itr2 = itr;
			if (itr2 != Points.begin())
				--itr2;
			if (itr->second || itr2->second)
			{
				glColor3f(1,0,0);
				glVertex3f(-(x-itr->first.x),-(y-itr->first.y), 0.002);
			}
			else
			{
				glColor3f(Colour.r,Colour.g,Colour.b);
				glVertex3f(-(x-itr->first.x),-(y-itr->first.y), 0);
			}
			
		}
		glEnd();
	}

	glPopMatrix();
	// draw arrow at the end of the path showing direction

	glLineWidth(6);
	glColor3f(0,0,0);
	glBegin(GL_LINES);
	glVertex3f(0.100,0,0.003);
	glVertex3f(0.800,0,0.003);
	glVertex3f(0.800,0,0.003);
	glVertex3f(0.500,0.200,0.003);
	glVertex3f(0.800,0,0.003);
	glVertex3f(0.500,-0.200,0.003);
	glEnd();	
	glLineWidth(3);
	if (stall)
		glColor3f(1,0,0);
	else
		glColor3f(Colour.r,Colour.g,Colour.b);
	glBegin(GL_LINES);
	glVertex3f(0.100,0,0.05);
	glVertex3f(0.800,0,0.05);
	glVertex3f(0.800,0,0.05);
	glVertex3f(0.500,0.200,0.05);
	glVertex3f(0.800,0,0.05);
	glVertex3f(0.500,-0.200,0.05);
	glEnd();	
	glLineWidth(1);
	
}


void RenderPlayerPath::RenderTransparent()
{
	if (Position == NULL)
		return;
		
//	Position->Lock();
	double x,y,theta;
	x = Position->GetXPos();
	y = Position->GetYPos();
	theta = (Position->GetYaw());
	bool stall = Position->GetStall();
//	Position->Unlock();
	if (Points.empty() || Points.front().first.x != x || Points.front().first.y != y)
		Points.push_front(std::pair<ARPoint, bool>(ARPoint(x,y,0), stall));
	
	if (Points.size() > HistoryLength && HistoryLength > 0)
		Points.pop_back();
	
	// draw the path
	glPushMatrix();
	glRotatef(-RTOD(theta),0,0,1);
	glLineWidth(100);
	if (Points.size() > 1)
	{
		glBegin(GL_LINE_STRIP);
		for (deque< std::pair<ARPoint, bool> >::const_iterator itr = Points.begin();itr != Points.end(); ++itr)
		{
			if (itr->second)
				glColor4f(1,0,0,0.25);
			else
				glColor4f(Colour.r,Colour.g,Colour.b,Colour.a);
			glVertex3f(-(x-itr->first.x),-(y-itr->first.y), 0);
		}
		glEnd();
	}
	glLineWidth(1);

	glPopMatrix();

}

void RenderPlayerPath::Event(EventObject* event)
{
	if (event->GetEventType() == ROE_Clear)
	{
		Points.clear();
	}
}

#ifdef HAVE_ANTTWEAKBAR
void RenderPlayerPath::DisplaySetup(TweakBar* tweakBar, const char* Name)
{

	static int guii=0;
	char uid[30];
	char args[500];
	snprintf(uid,30,"renderpath%d",++guii);
	snprintf(args,500," group='%s' min='-1' max=5000 step=1 label='History Length' help='Max number of nodes for the path to render, -1 for no max' ",Name);
	//TwAddVarCB(TweakBar::GetDisplayBar(), strdup(uid) ,TW_TYPE_INT8,SetHistoryTW,GetHistoryTW,this, strdup(args) );

}
#endif
