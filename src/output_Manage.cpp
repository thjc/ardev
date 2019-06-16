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
 *  02-12-08:
 * 		Outputx11 changed to outputManage
 * 		some stuff removed
 */
#include <ardev/ardev.h>
#include <ardev/output_Manage.h>
#include <libthjc/misc.h>
#include <ardev/debug.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>
#include <X11/Xatom.h>

#include <pthread.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

//-----------------------------------------------------------------------------------------------------------------
OutputManage::OutputManage(CaptureObject* cap, CameraObject * cam, PositionObject * cam_pos, const int _x, const int _y, const char * _DisplayName, const bool FullScreen) : OutputObject()
{
	if (_DisplayName)
		DisplayName = strdup(_DisplayName);
	else
		DisplayName=NULL;
	width = _x;
	height = _y;
	Camera = cam;
	Capture = cap;
	CameraPosition = cam_pos;
	Fullscreen = !FullScreen;
	videoFlags = 0;

	init = false;
}
//-----------------------------------------------------------------------------------------------------------------
// sets up an OpenGL X window...once this is done we simply use the default
// Render frame and show frame to produce the output
int OutputManage::Initialise(bool Active)
{

	dbg_print(ARDBG_INFO,"Initialising Output Object\n");
	//int err;
	// Lock mutex so we dont initilaise two windows at once
	Lock();

	// This prevents envrionments who are using the same output object from initialising it multiple times
	if(init)
	{
		Unlock();
		return 1;
	}

	//Window root;

	//int x_org = 0;
	//int y_org = 0;

	InitGL();
	// to prevent multiple initialisation of of the same object when sharing an output object
	init = true;
	Unlock();

	return 0;
}
//-----------------------------------------------------------------------------------------------------------------
void OutputManage::InitGL()
{

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	GLfloat light_position[] = { 1,1,1,0 };
	GLfloat light_ambient[] = { 0.55,0.55,0.55,1 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	GLfloat mat_specular_W[] = {1.0,1.0,1.0,1.0};
	GLfloat mat_shininess[]  = {50.0};

	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular_W);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_specular_W);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &Backdrop);					// Create The Texture

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, Backdrop);

	// set texture parameters to improve performance
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	unsigned int MinTexWidth = Capture->GetMaxWidth();
	unsigned int MinTexHeight = Capture->GetMaxHeight();

	// musnt have got an image yet, wait a few seconds and try again
	if (MinTexWidth == 0 || MinTexHeight == 0)
	{
		for (int i = 0; i < 20; ++i)
		{
			sleep(1);
			MinTexWidth = Capture->GetMaxWidth();
			MinTexHeight = Capture->GetMaxHeight();
			if (MinTexWidth != 0 && MinTexHeight != 0)
				break;
		}
		if (MinTexWidth == 0 || MinTexHeight == 0)
		{
			dbg_print(ARDBG_ERR,"Error getting backdrop size\n");
			initialised = false;
			return;
		}
	}

	TexWidth = static_cast<unsigned int> (RoundPow2(static_cast<double> (MinTexWidth)));
	TexHeight = static_cast<unsigned int> (RoundPow2(static_cast<double> (MinTexHeight)));

	// GL doesnt like allocating textures smaller than this
	if (TexWidth < 2)
		TexWidth = 2;
	if (TexHeight < 2)
		TexHeight = 2;

	dbg_print(ARDBG_INFO,"Creating %dx%d backdrop texture\n",TexWidth, TexHeight);


	uint8_t * TempTexture = new uint8_t[3*TexWidth*TexHeight];
	// Test if we can fit the texture
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA8, TexWidth, TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, TempTexture);
	if (!glIsTexture(Backdrop))
	{
		dbg_print(ARDBG_ERR,"Error creating backdrop texture, not valid texid\n");
		initialised = false;
		return;
	}
	int ActualWidth = 0;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&ActualWidth);
	if (ActualWidth == 0)
	{
		dbg_print(ARDBG_ERR,"Error creating backdrop texture\n");
		initialised = false;
		return;
	}
	// now we actually generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, 3/*GL_RGBA8*/, TexWidth, TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, TempTexture);
	glDisable(GL_TEXTURE_2D);

	delete [] TempTexture;

}
//-----------------------------------------------------------------------------------------------------------------
void OutputManage::Terminate()
{
	dbg_print(ARDBG_INFO,"Terminating Output Object\n");
	//int err;
	Lock();
	init = false;
	Unlock();

}
//-----------------------------------------------------------------------------------------------------------------
void OutputManage::ShowFrame()
{
	return;//currently doing nothing from ShowFrame
}
//-----------------------------------------------------------------------------------------------------------------
void OutputManage::ToggleFullScreen()
{
    Fullscreen = !Fullscreen;
    if (Fullscreen)
    {
		printf("dims: %d x %d\n",width,height);
    }
    else
    {

    }

}

