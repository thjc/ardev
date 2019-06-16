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
#include <ardev/capture.h>
#include <stdio.h>
#include <GL/glut.h>
#include <ardev/debug.h>

CaptureStage::CaptureStage(const char * _RobotName, CameraObject * camera, PositionObject * camera_position, PlayerClientInterface & _PlayerInterface)
{
	if (_RobotName)
		RobotName = strdup(_RobotName);
	else
		RobotName = "robot";
	Open = false;
	PlayerInterface = &_PlayerInterface;

	TheMap = -1;

	Frame.data = new unsigned char[480*300*3];
	if (Frame.data == NULL)
		Open = false;
	Frame.x_size = 480;
	Frame.y_size = 300;
	Frame.ColourFormat=GL_RGB;

	if (!(pCamera = camera))
	{
		dbg_print(ARDBG_ERR,"NULL Camera Object Specified\n");
		Open = false;
	}

	if (!(pCameraPosition = camera_position))
	{
		dbg_print(ARDBG_ERR,"NULL Camera Position Object Specified\n");
		Open = false;
	}


}

CaptureStage::~CaptureStage()
{
	delete RobotName;
}

int CaptureStage::GetMaxWidth()
{
	return Frame.x_size;
}

int CaptureStage::GetMaxHeight()
{
	return Frame.y_size;
}


int CaptureStage::Initialise(bool Active)
{
	if (!initialised)
	{
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void CaptureStage::Terminate()
{
	PlayerInterface->RemoveChild(*this);
}

void CaptureStage::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	//PlayerInterface->Lock();
	dbg_print(ARDBG_VERBOSE,"Creating Simulation Proxy for CaptureStage\n");
	Position = new SimulationProxy(pci,0);
	Map = new MapProxy(pci,0);
	Map->RequestMap();
	//PlayerInterface->Unlock();
	Open = true;
	Unlock();
}

void CaptureStage::PlayerTerm()
{
	Lock();
	//PlayerInterface->Lock();
	delete Position;
	Position = NULL;
	delete Map;
	Map = NULL;
	Open = false;
	//PlayerInterface->Unlock();
	Unlock();
}


void CaptureStage::drawMap()
{
	//printf("Creating map list\n");

	if (TheMap == -1)
	{
		GLfloat mat_specular_W[] = {1.0,1.0,1.0,1.0};
		GLfloat mat_shininess[]  = {50.0};

		TheMap = glGenLists(1);
		glNewList(TheMap, GL_COMPILE);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_W);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_specular_W);
		glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);

		const double height = 1.2;
		double resolution = Map->GetResolution();
		double half = resolution/2;
		double xmax = Map->GetWidth() * resolution + Map->GetOriginX();
		double ymax = Map->GetHeight() * resolution + Map->GetOriginY();
		double xmin = Map->GetOriginX();
		double ymin = Map->GetOriginY();

		//printf("%f %f %f %f %f %f\n",xmin,xmax,ymin,ymax,Map->GetOriginX(),Map->GetOriginY());

		// draw the ground plane
		glColor3f(0.6,0.6,0.6);
		glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		glVertex3f(xmin,ymin,0);
		glVertex3f(xmax,ymin,0);
		glVertex3f(xmax,ymax,0);
		glVertex3f(xmin,ymax,0);
		glEnd();

		// now the walls
		glColor3f(0.9,0.9,0.6);
		glBegin(GL_QUADS);
		glNormal3f(0,1,0);
		glVertex3f(xmin,ymin,0);
		glVertex3f(xmin,ymin,2*height);
		glVertex3f(xmax,ymin,2*height);
		glVertex3f(xmax,ymin,0);

		glNormal3f(-1,0,0);
		glVertex3f(xmax,ymin,0);
		glVertex3f(xmax,ymin,2*height);
		glVertex3f(xmax,ymax,2*height);
		glVertex3f(xmax,ymax,0);

		glNormal3f(0,-1,0);
		glVertex3f(xmax,ymax,0);
		glVertex3f(xmax,ymax,2*height);
		glVertex3f(xmin,ymax,2*height);
		glVertex3f(xmin,ymax,0);

		glNormal3f(1,0,0);
		glVertex3f(xmin,ymax,0);
		glVertex3f(xmin,ymax,2*height);
		glVertex3f(xmin,ymin,2*height);
		glVertex3f(xmin,ymin,0);
		glEnd();

		// now render the bitmap
		for (unsigned int y = 0 ; y < Map->GetHeight(); ++y)
		{
			for (unsigned int x = 0; x < Map->GetWidth(); ++x)
			{
				if(Map->GetCell(x,y) == 1)
				{
					glPushMatrix();
					double xpos = x*resolution + Map->GetOriginX();
					double ypos = y*resolution + Map->GetOriginY();
					glTranslatef(xpos,ypos,0);

					glColor3f(0,0.5,0);

					glBegin(GL_QUADS);
					glNormal3f(0,-1,0);
					glVertex3f(-half,-half,0);
					glVertex3f(-half,-half,height);
					glVertex3f(half,-half,height);
					glVertex3f(half,-half,0);

					glNormal3f(1,0,0);
					glVertex3f(half,-half,0);
					glVertex3f(half,-half,height);
					glVertex3f(half,half,height);
					glVertex3f(half,half,0);

					glNormal3f(0,1,0);
					glVertex3f(half,half,0);
					glVertex3f(half,half,height);
					glVertex3f(-half,half,height);
					glVertex3f(-half,half,0);

					glNormal3f(-1,0,0);
					glVertex3f(-half,half,0);
					glVertex3f(-half,half,height);
					glVertex3f(-half,-half,height);
					glVertex3f(-half,-half,0);

					glNormal3f(0,0,1);
					glVertex3f(-half,-half,height);
					glVertex3f(half,-half,height);
					glVertex3f(half,half,height);
					glVertex3f(-half,half,height);
					glEnd();
					glPopMatrix();
				}
			}
		}
		glColor3f(1,1,1);
		glEndList();
	}
	glCallList(TheMap);
}

const ARImage & CaptureStage::GetFrame()
{
	dbg_print(ARDBG_VERBOSE,"Getting Frame from CaptureStage\n");
	if (!Open)
	{
		dbg_print(ARDBG_WARN,"Failed to generate frame\n");
		return Frame;
	}

	// clear gl buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// grab the camera details
	dbg_print(ARDBG_VERBOSE,"Grabbing camera details\n");
	pCamera->Lock();
	ARCamera cam = pCamera->GetCamera();
	pCamera->Unlock();
	pCameraPosition->Lock();
	ARPosition cam_pos = pCameraPosition->GetTransformedPosition();
	pCameraPosition->Unlock();

	// Add on the position of the robot to the camera...
	double x,y,a;
	dbg_print(ARDBG_VERBOSE,"Get the position of the robot from stage\n");
	Position->GetPose2d(const_cast<char*>(RobotName),x,y,a);
	ARPoint Origin = cam_pos.Origin;
	Origin.x += x;
	Origin.y += y;
	ARPoint Focus(1,0,0);
	ARPoint Up(0,0,1);
	Focus.RotateRPY(cam_pos.Direction.x,cam_pos.Direction.y,cam_pos.Direction.z-a);
	Focus+=Origin;
	Up.RotateRPY(cam_pos.Direction.x,cam_pos.Direction.y,cam_pos.Direction.z-a);
	dbg_print(ARDBG_VERBOSE,"Got the camera details\n");

	// set up viewpoint
	glViewport(0,0,Frame.x_size,Frame.y_size);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(RTOD(cam.y_fov),cam.aspect,0.1,20);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Origin.x,Origin.y,Origin.z,Focus.x,Focus.y,Focus.z,Up.x,Up.y,Up.z);
//	dbg_print(ARDBG_INFO,"Camera Details: %f %f %f %f %f %f %f %f %f\n",Origin.x,Origin.y,Origin.z,Focus.x,Focus.y,Focus.z,Up.x,Up.y,Up.z);
	//set up light
	GLfloat light_position[] = {x,y,3,0};
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	// now draw the map
	dbg_print(ARDBG_VERBOSE,"Generating captured frame\n");
	drawMap();

	// read image out of frame buffer
	glReadPixels(0,0,Frame.x_size, Frame.y_size, Frame.ColourFormat, GL_UNSIGNED_BYTE,static_cast<void*> (Frame.data));

	unsigned char * TempBuffer = new unsigned char[Frame.x_size * 3];
	if (TempBuffer)
	{
		for (unsigned int i = 0; i < Frame.y_size/2; ++i)
		{
			memcpy(TempBuffer,&Frame.data[i*Frame.x_size*3], Frame.x_size*3);
			memcpy(&Frame.data[i*Frame.x_size*3],&Frame.data[(Frame.y_size-i-1)*Frame.x_size*3], Frame.x_size*3);
			memcpy(&Frame.data[(Frame.y_size-i-1)*Frame.x_size*3],TempBuffer, Frame.x_size*3);

		}
	}
	else
		dbg_print(ARDBG_ERR,"failed to allocate memory");
	delete [] TempBuffer;

	// disable features used
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_MATERIAL);

	return Frame;
}
