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

RenderPlayerActArray::RenderPlayerActArray(PlayerClientInterface & pci, const ARColour & aColour, vector<int> ordering, int aIndex) : RenderObject(aColour)
{
	Proxy = NULL;
	PlayerInterface = &pci;
	Width = 0.05;//aWidth;
	Index = aIndex;
	Ordering = ordering;
}

RenderPlayerActArray::~RenderPlayerActArray()
{
	delete Proxy;
}

int RenderPlayerActArray::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerActArray::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerActArray::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	delete Proxy;
	Proxy = new ActArrayProxy(pci,Index);
	if (Proxy == NULL)
		dbg_print(ARDBG_ERR,"Error Creating ActArray Proxy: Index:%d\n", Index);
	Proxy->RequestGeometry();
	//ActArrayGeom = Proxy->GetActuatorGeom(0);
	Unlock();
}

void RenderPlayerActArray::PlayerTerm()
{
	Lock();
	dbg_print(ARDBG_VERBOSE, "Destroying actarray proxy\n");
	delete Proxy;
	Proxy = NULL;
	dbg_print(ARDBG_VERBOSE, "actarray Proxy destroyed\n");
	Unlock();
}

void RenderPlayerActArray::Render()
{
	if (Proxy == NULL)
		return;
	dbg_print(ARDBG_VERBOSE,"Rendering ActArray Using player interface\n");

	GLfloat mat_specular[] = { 0,0,1, 1.0 };
	GLfloat mat_diffuse[] = { 0,0,1, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	player_point_3d_t base_pos = Proxy->GetBasePos();
	player_orientation_3d_t base_orientation = Proxy->GetBaseOrientation();

	glTranslatef(base_pos.px,base_pos.py,base_pos.pz);
	glRotatef(RTOD(base_orientation.proll),1,0,0);
	glRotatef(RTOD(base_orientation.ppitch),0,1,0);
	glRotatef(RTOD(base_orientation.pyaw),0,0,1);

	ARPosition current = ARPosition( ARPoint(base_pos.px,base_pos.py,base_pos.pz),ARPoint(RTOD(base_orientation.proll),RTOD(base_orientation.ppitch),RTOD(base_orientation.pyaw)) );

	glColor3f(Colour.r,Colour.g,Colour.b);
	glLineWidth(1);

	PlayerInterface->Lock();
	Proxy->RequestGeometry();
	PlayerInterface->Unlock();
	glLineWidth(3);

	for (unsigned int ii = 0; ii < Proxy->GetCount(); ++ii)
	{
		int o = ii;
		if(Ordering.size()==Proxy->GetCount())
			o= Ordering[ii];

		player_actarray_actuator_t ii_data = Proxy->GetActuatorData(o);
		player_actarray_actuatorgeom_t ii_geom = Proxy->GetActuatorGeom(o);

		if(ii_geom.type == 1)//rotary
		{
			glRotatef(RTOD(ii_geom.orientation.proll),1,0,0);
			glRotatef(RTOD(ii_geom.orientation.ppitch),0,1,0);
			glRotatef(RTOD(ii_geom.orientation.pyaw),0,0,1);
			glRotatef(RTOD(ii_data.position),ii_geom.axis.px,ii_geom.axis.py,ii_geom.axis.pz);
			glBegin(GL_LINES);
			glVertex3f(0,0,0);
			glVertex3f(ii_geom.length,0,0);
			glEnd();
			glTranslatef(ii_geom.length,0,0);
			// Get accumulated position
			current += ARPosition(ARPoint(ii_geom.length,0,0),ARPoint(ii_geom.orientation.proll,ii_geom.orientation.ppitch,ii_geom.orientation.pyaw));
		}
		else if(ii_geom.type == 0)//linear
		{
			glBegin(GL_LINES);
			glVertex3f(0,0,0);
			glVertex3f(ii_data.position*ii_geom.axis.px,ii_data.position*ii_geom.axis.py,ii_data.position*ii_geom.axis.pz);
			glEnd();
			glTranslatef(ii_data.position*ii_geom.axis.px,ii_data.position*ii_geom.axis.py,ii_data.position*ii_geom.axis.pz);
			// Get accumulated position
			current += ARPosition(ARPoint(ii_data.position*ii_geom.axis.px,ii_data.position*ii_geom.axis.py,ii_data.position*ii_geom.axis.pz),ARPoint());
		}
		else
			dbg_print(ARDBG_ERR,"Non supported actuator type.\n");

		// publish event to update PositionActArray Index[ii]
		//printf("Act Array Position for Index %d: %f %f %f\n",o,current.Origin.x,current.Origin.y,current.Origin.z);
		RenderPositionEventObject* r = new RenderPositionEventObject(RPE_ActArray,current,ii);
		ARDev::PublishEvent(r);

		dbg_print(ARDBG_VERBOSE,"AA: %d length %f %s %f %s %f %f %f\n",o,ii_geom.length, ii_geom.type==1?"rotation":"size" ,ii_geom.type==1?RTOD(ii_data.position):ii_data.position, ii_geom.type==1?"about":"along",ii_geom.axis.px,ii_geom.axis.py,ii_geom.axis.pz );
	}
	glLineWidth(1);
}

void RenderPlayerActArray::RenderTransparent()
{
	if (Proxy == NULL)
		return;
	dbg_print(ARDBG_VERBOSE,"Rendering ActArray Using player interface\n");

}

