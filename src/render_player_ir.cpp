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

RenderPlayerIr::RenderPlayerIr(PlayerClientInterface & pci)
{
	Ir = NULL;
	PlayerInterface = &pci;
}

RenderPlayerIr::~RenderPlayerIr()
{
	delete Ir;
}

int RenderPlayerIr::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerIr::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerIr::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	//PlayerInterface->Lock();
	delete Ir;
	Ir = new IrProxy(pci,0);
	if (Ir == NULL)
		dbg_print(ARDBG_ERR,"Error Creating Ir Proxy\n");
	Ir->RequestGeom();
	//PlayerInterface->Unlock();
	Unlock();
}

void RenderPlayerIr::PlayerTerm()
{
	Lock();
	//PlayerInterface->Lock();
	delete Ir;	
	Ir = NULL;
	//PlayerInterface->Unlock();	
	Unlock();
}

void RenderPlayerIr::RenderTransparent()
{
	if (Ir == NULL)
		return;
		
	glColor4f(0.4,0.0,0.6,0.6);


//	Ir->Lock();
	for (unsigned int i = 0 ; i<Ir->GetPoseCount() ; i++)
	{
		glPushMatrix();
		double x = Ir->GetPose(i).px;
		double y = Ir->GetPose(i).py;
		double z = Ir->GetPose(i).pz;
		double yaw = Ir->GetPose(i).pyaw;
		double pitch = Ir->GetPose(i).ppitch;
		double roll = Ir->GetPose(i).proll;

		glTranslatef(x,y,z);
		glRotatef(RTOD(yaw),RTOD(pitch),RTOD(roll),1);
			
		double range = (*Ir)[i];
		double endwidth = range * tan(M_PI/20);

		glBegin(GL_TRIANGLES);
		glVertex3f(0,0,0);
		glVertex3f(range,endwidth ,0);
		glVertex3f(range,-endwidth,0);
		glEnd();
		
		glPopMatrix();
	}
//	Ir->Unlock();
}
