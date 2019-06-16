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

RenderPlayerLaser::RenderPlayerLaser(PlayerClientInterface & pci, const ARColour & aColour, int aRayInterval, bool aWithOutline, int aIndex) : RenderObject(aColour)
{
	MaxRange = 0;
	
	Laser = NULL;
	PlayerInterface = &pci;
	RayInterval = aRayInterval;
	WithOutline = aWithOutline;
	Index = aIndex;
}

RenderPlayerLaser::~RenderPlayerLaser()
{
	delete Laser;
}

int RenderPlayerLaser::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerLaser::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerLaser::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	delete Laser;
	Laser = new LaserProxy(pci,Index);
	if (Laser == NULL)
		dbg_print(ARDBG_ERR,"Error Creating Laser Proxy\n");
	Laser->RequestGeom();
	pose = Laser->GetPose();
	Unlock();
}

void RenderPlayerLaser::PlayerTerm()
{
	Lock();
	dbg_print(ARDBG_VERBOSE, "Destroying laser proxy\n");
	delete Laser;	
	Laser = NULL;
	dbg_print(ARDBG_VERBOSE, "Laser Proxy destroyed\n");
	Unlock();
}

void RenderPlayerLaser::Render()
{
	if (Laser == NULL)
		return;
	dbg_print(ARDBG_VERBOSE,"Rendering Laser Using player interface\n");

	
	GLfloat mat_specular[] = { 0,0,1, 1.0 };
	GLfloat mat_diffuse[] = { 0,0,1, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);		

	glTranslatef(pose.px,pose.py,pose.pz);
	glRotatef(RTOD(pose.pyaw),0,0,1);
	
//	glColor3f(0,0,1);
//	glutSolidCube(0.050);

	glColor3f(Colour.r,Colour.g,Colour.b);
	glLineWidth(1);


	if (WithOutline)
	{
		glBegin(GL_LINE_STRIP);
		glVertex3f(0,0,0);
	//	Laser->Lock();
		
		MaxRange = 0;
		
		dbg_print(ARDBG_VERBOSE, "Laser Range Count is %d\n", Laser->GetCount());
		for (unsigned int i = 0 ; i<Laser->GetCount(); ++i)
		{
			glVertex3f(Laser->GetPoint(i).px,Laser->GetPoint(i).py,0);
			MaxRange = PlayerCc::max(MaxRange, (*Laser)[i]);
		}
		glVertex3f(0,0,0);
		glEnd();
	}
	
	
//	Laser->Unlock();
}

void RenderPlayerLaser::RenderTransparent()
{
	if (Laser == NULL)
		return;
	dbg_print(ARDBG_VERBOSE,"Rendering Laser Using player interface\n");

//	Laser->Lock();
	MaxRange = 0;

	double alpha=Colour.a;
	
	glTranslatef(pose.px,pose.py,pose.pz);
	glRotatef(RTOD(pose.pyaw),0,0,1);

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(Colour.r,Colour.g,Colour.b,alpha);
	glVertex3f(0,0,0);

/*		glBegin(GL_LINE_STRIP);
		glVertex3f(0,0,0);
	//	Laser->Lock();
		
		MaxRange = 0;
		
		dbg_print(ARDBG_VERBOSE, "Laser Range Count is %d\n", Laser->GetCount());
		for (unsigned int i = 0 ; i<Laser->GetCount(); i+=RayInterval)
		{
			glVertex3f(Laser->GetPoint(i).px,Laser->GetPoint(i).py,0);
			glVertex3f(0,0,0);
			MaxRange = PlayerCc::max(MaxRange, (*Laser)[i]);
		}
		glVertex3f(0,0,0);
		glEnd();
	}*/
	
	for (unsigned int i = 0 ; i<Laser->GetCount(); i++)
	{
		if (RayInterval > 0 && i % RayInterval == 0)
		{
			alpha = alpha < Colour.a ? Colour.a + 0.15 : Colour.a - 0.10;
			glVertex3f(Laser->GetPoint(i).px,Laser->GetPoint(i).py,0);
			glVertex3f(0,0,0);
			glEnd();
			glColor4f(Colour.r,Colour.g,Colour.b,alpha);
			glBegin(GL_TRIANGLE_FAN);
			glVertex3f(0,0,0);
			
		}
		glVertex3f(Laser->GetPoint(i).px,Laser->GetPoint(i).py,0);
	}
	glEnd();
//	Laser->Unlock();
}

double RenderPlayerLaser::TraceDistance(const Ray & Offset, const Ray & R, const ARPoint & Rotation)
{
	ARPoint SegmentStart(1,0,0);
	ARPoint PlaneNormal(0,0,1);
	
	Ray R2(R.Origin - Offset.Origin, R.Direction);
	
	double Theta = (Laser->GetMaxAngle() - Laser->GetMinAngle())/2;	
	SegmentStart.RotateZ(Laser->GetMinAngle() + Theta);
	SegmentStart.RotateYPR(Rotation.x, Rotation.y, Rotation.z);
	PlaneNormal.RotateYPR(Rotation.x, Rotation.y, Rotation.z);
	SegmentStart = SegmentStart*MaxRange;
	
	Ray Normal(Vector3D(0,0,0),Vector3D(PlaneNormal.x,PlaneNormal.y,PlaneNormal.z));
	Plane P(Normal);
	const vector<Vector3D> & Points = P.Intersect(R2);
	if (Points.size() == 0)
		return -1;
	if (P.BoundCircleSegment(Points.front(), Vector3D(SegmentStart.x,SegmentStart.y,SegmentStart.z), Theta))
		return (Points.front()- R2.Origin).Length();
	return -1;
	
}
