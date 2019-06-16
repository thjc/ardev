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

RenderPlayerBumper::RenderPlayerBumper(PlayerClientInterface & pci, int Index, double _Height)
{
	Bumper = NULL;
	PlayerInterface = &pci;
	Height = _Height;
}


RenderPlayerBumper::~RenderPlayerBumper()
{
	delete Bumper;
}


int RenderPlayerBumper::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerBumper::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerBumper::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	delete Bumper;
	Bumper = new BumperProxy(pci,0);
	memset(&BumperGeom,0,sizeof(BumperGeom));
	Bumper->RequestBumperConfig();
	Unlock();
}

void RenderPlayerBumper::PlayerTerm()
{
	Lock();
	delete Bumper;	
	Bumper = NULL;
	Unlock();
}


#define INACTIVE_COLOUR 1,0,0,0.1
#define ACTIVE_COLOUR 0,1,0,0.5

void RenderPlayerBumper::RenderTransparent()
{
	if (Bumper == NULL)
		return;
	
//	Bumper->Lock();
	unsigned int BumpCount = Bumper->GetCount();
	
	if (Bumper->GetPoseCount() < Bumper->GetCount())
	{
		Unlock();
		dbg_print(ARDBG_WARN,"less bumper defs than bumpers");
		return;
	}

	for (unsigned int i = 0 ; i<BumpCount; i++)
	{
		// workaround for b21r until I modify the rflex driver to have two bumper banks
		if (i > 5)
			break;
		glPushMatrix();
		
		// first see fi the bumper is active
		if (Bumper->IsBumped(i))
			glColor4f(ACTIVE_COLOUR);
		else
			glColor4f(INACTIVE_COLOUR);
		
		// then draw the bumper
		player_bumper_define_t CurrentDef = Bumper->GetPose(i);

		double x = CurrentDef.pose.px;
		double y = CurrentDef.pose.py;
//		double z = CurrentDef.pose.pz;
		double yaw = CurrentDef.pose.pyaw;
//		double pitch = CurrentDef.pose.ppitch;
//		double roll = CurrentDef.pose.proll;
		double r = CurrentDef.radius;
		double l = CurrentDef.length;
		double alpha = l/r;
		
		double tdelta=atan2(y,x)-yaw;
		printf("%f %f %f %f\n",x,y,atan2(y,x),yaw);
		
		double x0 = x*cos(-alpha/2) + y*sin(-alpha/2);
		double y0 = -x*sin(-alpha/2) + y*cos(-alpha/2);

		int NumSegments = static_cast<int> (180.0*alpha/PI);
		if (NumSegments < 1)
			NumSegments = 1;
		
		glBegin(GL_QUAD_STRIP);
		glVertex3f(x0,y0,0);
		glVertex3f(x0,y0,Height);
		for (double beta = 0;beta < alpha;beta += M_PI/180)
		{
			double x1a = x*cos(beta-alpha/2) + y*sin(beta-alpha/2);
			double y1a = -x*sin(beta-alpha/2) + y*cos(beta-alpha/2);

			double x1 = x1a*cos(tdelta) + y1a*sin(tdelta);
			double y1 = -x1a*sin(tdelta) + y1a*cos(tdelta);


			glVertex3f(x1  , y1 , 0);
			glVertex3f(x1  , y1 , Height);
		}
		glEnd();	
		glPopMatrix();
		
	}
//	Bumper->Unlock();
}
