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

RenderPlayerLimb::RenderPlayerLimb(PlayerClientInterface & pci, const ARColour & aColour, int aIndex, double aRadius) : RenderObject(aColour)
{
	Proxy = NULL;
	PlayerInterface = &pci;
	Radius = aRadius;
	Index = aIndex;

}

RenderPlayerLimb::~RenderPlayerLimb()
{
	delete Proxy;
}

int RenderPlayerLimb::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerLimb::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerLimb::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	delete Proxy;
	Proxy = new LimbProxy(pci,Index);
	if (Proxy == NULL)
		dbg_print(ARDBG_ERR,"Error Creating Limb Proxy: Index:%d\n", Index);
	Proxy->RequestGeometry();
	Geom = Proxy->GetGeom();

	Unlock();
}

void RenderPlayerLimb::PlayerTerm()
{
	Lock();
	dbg_print(ARDBG_VERBOSE, "Destroying Limb proxy\n");
	delete Proxy;	
	Proxy = NULL;
	dbg_print(ARDBG_VERBOSE, "Limb Proxy destroyed\n");
	Unlock();
}

void RenderPlayerLimb::Render()
{
	if (Proxy == NULL)
		return;
	dbg_print(ARDBG_VERBOSE,"Rendering Limb Using player interface\n");

	GLfloat mat_specular[] = { 0,0,1, 1.0 };
	GLfloat mat_diffuse[] = { 0,0,1, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);		

	
	//glTranslatef(Geom.basePos.px,Geom.basePos.py,Geom.basePos.pz);
	//glRotatef(RTOD(pose.pa),0,0,1);
	
//	glColor3f(0,0,1);
//	glutSolidCube(0.050);

	glColor3f(Colour.r,Colour.g,Colour.b);
	glLineWidth(1);

	player_limb_data_t ldata = Proxy->GetData();
	
	
	glBegin(GL_LINE_STRIP);
	glVertex3f(ldata.position.px,ldata.position.py,ldata.position.pz);
	glVertex3f(ldata.position.px+2*Radius*ldata.orientation.px,ldata.position.py+2*Radius*ldata.orientation.py,ldata.position.pz+2*Radius*ldata.orientation.pz);
	glEnd();
	
//	Laser->Unlock();
}

void RenderPlayerLimb::RenderTransparent()
{
	if (Proxy == NULL)
		return;
	dbg_print(ARDBG_VERBOSE,"Rendering Limb Using player interface\n");

	double alpha=0.19;
		
	//glTranslatef(Geom.basePos.px,Geom.basePos.py,Geom.basePos.pz);
	//glRotatef(RTOD(Geom.basePos.pa),0,0,1);

	player_limb_data_t ldata = Proxy->GetData();
	glTranslatef(ldata.position.px,ldata.position.py,ldata.position.pz);
	
	glColor4f(Colour.r,Colour.g,Colour.b,alpha);
	glutSolidSphere(Radius, 10, 10);
	


//	Laser->Unlock();
}

