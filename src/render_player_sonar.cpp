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

RenderPlayerSonar::RenderPlayerSonar(PlayerClientInterface & pci, int Index)
{
	Sonar = NULL;
	PlayerInterface = &pci;
}


RenderPlayerSonar::~RenderPlayerSonar()
{
	delete Sonar;
}


int RenderPlayerSonar::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerSonar::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerSonar::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	printf("Player sonar init\n");
	Lock();
	printf("Locked\n");
	delete Sonar;
	Sonar = new SonarProxy(pci,0);
	printf("Requesting sonar geom\n");
	Sonar->RequestGeom();
	printf("Created\n");
	Unlock();
}

void RenderPlayerSonar::PlayerTerm()
{
	Lock();
	delete Sonar;	
	Sonar = NULL;
	Unlock();
}


void RenderPlayerSonar::RenderTransparent()
{
	if (Sonar == NULL)
		return;
		
	//PlayerInterface->Lock();
	if (Sonar->GetCount() != Sonar->GetPoseCount())
	{
		//PlayerInterface->Unlock();
		dbg_print(ARDBG_ERR,"Sonar range count(%d) != Pose count(%d)\n",Sonar->GetCount(),Sonar->GetPoseCount());
		return;
	}

	glColor4f(0,0.3,0.6,0.2);

	for (unsigned int i = 0 ; i<Sonar->GetCount(); i++)
	{
		glPushMatrix();
		
		double yaw = Sonar->GetPose(i).pyaw;
		double pitch = Sonar->GetPose(i).ppitch;
		double roll = Sonar->GetPose(i).proll;

		glTranslatef(Sonar->GetPose(i).px,Sonar->GetPose(i).py,Sonar->GetPose(i).pz);
		glRotatef(RTOD(yaw),RTOD(pitch),RTOD(roll),1);

		double ConeAngle = M_PI/18; // radius of cone
		double EndRadius = Sonar->GetScan(i)*atan(ConeAngle);
		glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0,0,0);
		
		for (double j = 0 ; j<=2*M_PI; j+=M_PI/180)
		{
			double y = EndRadius*cos(j);
			double z = EndRadius*sin(j);
			glVertex3f(Sonar->GetScan(i),y,z);
		}
		
/*		glVertex3f(geom_length+Sonar->ranges[i],-EndRadius,0);
		glVertex3f(geom_length+Sonar->ranges[i],EndRadius,0);*/
		glEnd();
		
		//printf("sonar pose %f %f %f\n",Sonar->poses[i][0],Sonar->poses[i][1],Sonar->poses[i][2]);
		
		//glVertex3f(Sonar->poses[i][0],-Sonar->poses[i][1],0);
		//glVertex3f(Sonar->poses[i][0]+Sonar->ranges[i]*cos(angle),-(Sonar->poses[i][1]+Sonar->ranges[i]*sin(angle)),0);
		
		//glVertex3f(0,500,0);
		//glVertex3f(4000*cos(M_PI*i/180.0),500,4000*sin(M_PI*i/180.0));
		glPopMatrix();
		
	}
//	Sonar->Unlock();
	
//	glEnd();
	
	
}
