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
#include <ardev/ardev.h>
#include <ardev/output_x11.h>
#include <libthjc/misc.h>
#include <ardev/debug.h>

#include <GL/gl.h>
#include <GL/glu.h>
/*#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <SDL/SDL_version.h>*/

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

#include <ardev/anttweakbar.h>

//pthread_mutex_t OutputX11::lock = PTHREAD_MUTEX_INITIALIZER;

OutputX11::OutputX11(CaptureObject* cap, CameraObject * cam, PositionObject * cam_pos, const int _x, const int _y, const char * _DisplayName, const char * _Name, const bool FullScreen) : OutputObject()
{
	if (_DisplayName)
		DisplayName = strdup(_DisplayName);
	else
		DisplayName=NULL;
	if (_Name)
		Name = strdup(_Name);
	else
		Name=NULL;
	width = _x;
	height = _y;
	Camera = cam;
	Capture = cap;
	CameraPosition = cam_pos;
	Fullscreen = !FullScreen;
	videoFlags = 0;

	buttonDown = false;
	looking = false;
	moving = false;
	oldMouseX = 0;
	oldMouseY = 0;

	init = false;
}

// sets up an OpenGL X window...once this is done we simply use the default
// Render frame and show frame to produce the output
int OutputX11::Initialise(bool Active)
{
	dbg_print(ARDBG_INFO,"Initialising Output Object\n");
	int err;
	// Lock mutex so we dont initilaise two windows at once
	Lock();

	// This prevents envrionments who are using the same output object from initialising it multiple times
	if(init)
	{
		Unlock();
		return 1;
	}
	// To prevent shared outputs being initialised multiple times

/*
    // this holds some info about our display
    const SDL_VideoInfo *videoInfo;

    // initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
	    fprintf( stderr, "Video initialization failed: %s\n",
		     SDL_GetError( ) );
	    exit( 1 );
	}

    // Fetch the video info
    videoInfo = SDL_GetVideoInfo( );

    if ( !videoInfo )
	{
	    fprintf( stderr, "Video query failed: %s\n",
		     SDL_GetError( ) );
	    exit( 1 );
	}

    // the flags to pass to SDL_SetVideoMode
    videoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL
    videoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering
    videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware

    // This checks to see if surfaces can be stored in memory
    if ( videoInfo->hw_available )
		videoFlags |= SDL_HWSURFACE;
    else
		videoFlags |= SDL_SWSURFACE;

    // This checks if hardware blits can be done
    if ( videoInfo->blit_hw )
		videoFlags |= SDL_HWACCEL;

	videoFlagsFull = videoFlags | SDL_FULLSCREEN;
    videoFlags |= SDL_RESIZABLE;       // Enable window resizing

    // Sets up OpenGL double buffering
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    // get a SDL surface
    surface = SDL_SetVideoMode( width, height, 24,
				videoFlags );

    // Verify there is a surface
    if ( !surface )
	{
	    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
	    exit( 1 );
	}

	if (!Fullscreen)
		ToggleFullScreen();
	else
		Fullscreen = false;
		*/
    /* initialize OpenGL */
    //initGL( );




	Window root;
	//char * dpyName = NULL;
	//GLboolean printInfo = GL_FALSE;

	if (!(dpy = XOpenDisplay(DisplayName)))
	{
		dbg_print(ARDBG_ERR,"Failed to open X Display %s\n",DisplayName==NULL?"<default>" : DisplayName);
		dbg_print(ARDBG_ERR,"Unknown display name: %s\n",DisplayName==NULL?"<default>" : DisplayName);
		return -1;
	}

	int x_org = 0;
	int y_org = 0;
/*	bool Xinerama = false;
	int width0 = 0;
	int height0 = 0;


	if ( XineramaIsActive( dpy ) )
	{
		int nscreens;
		XineramaScreenInfo * info = XineramaQueryScreens( dpy, &nscreens );

		dbg_print(ARDBG_INFO,"Xinerama is active\n");
		Xinerama = true;

		width0 = info[0].width;
		width0 = info[0].height;
		x_org = info[0].x_org;
		y_org = info[0].y_org;

		for(int i = 0; i< nscreens; i++ )
		{
			dbg_print(ARDBG_VERBOSE, "Screen %d: (%d) %d+%d+%dx%d\n", i, info[i].screen_number,
			info[i].x_org, info[i].y_org,
			info[i].width, info[i].height );
		}
	}
	else
	{
		dbg_print(ARDBG_INFO, "Xinerama is not active\n");
	}

     scr = DefaultScreenOfDisplay(dpy);
     printf( "DefaultScreenOfDisplay, %d x %d\n", WidthOfScreen(scr), HeightOfScreen(scr) );

     if (Fullscreen && Xinerama)
     {
     	width = width0;
     	height = height0;
     }
     else if (Fullscreen)
     {
     	width = WidthOfScreen(scr);
     	height = HeightOfScreen(scr);
     }*/

//	make_window(dpy, "ardev", 0,0,300,300,&win, &ctx);
	int attrib[] = {GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
	GLX_BLUE_SIZE, 1, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 1, None };
	int scrnum = DefaultScreen (dpy);
	XSetWindowAttributes attr;
	XVisualInfo *visinfo;
	root = RootWindow(dpy,scrnum);
	unsigned int mask;
	if(!(visinfo = glXChooseVisual(dpy, scrnum, attrib)))
	{
		dbg_print(ARDBG_ERR,"Failed to get XVisualInfo\n");
		return -1;
	}

	dbg_print(ARDBG_VERBOSE,"Creating GLX Context\n");
	ctx = glXCreateContext(dpy, visinfo, NULL, True);
	if (ctx == NULL)
	{
		dbg_print(ARDBG_ERR,"Error Creating glXContext ... NULL returned\n");
		return -1;
	}

	attr.background_pixel = 0;
	attr.border_pixel =0;
	attr.colormap = XCreateColormap( dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
/*	if (Fullscreen)
		attr.override_redirect = 1;*/
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	dbg_print(ARDBG_VERBOSE,"Creating X Window\n");
	win = XCreateWindow(dpy, root, x_org,y_org,width,height,0,visinfo->depth,
	InputOutput, visinfo->visual, mask, &attr);

	// register interest in the delete window message
	wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);

	if (!glXMakeCurrent(dpy,win,ctx))
	{
		dbg_print(ARDBG_ERR,"Error making glXContext current\n");
		return -1;
	}

	// set hints and properties
	XSizeHints sizehints;
	sizehints.x  =0;
	sizehints.y = 0;
	sizehints.width = width;
	sizehints.height = height;
	sizehints.flags = USSize | USPosition;
	dbg_print(ARDBG_VERBOSE,"Calling XSetNormalHints\n");
	XSetNormalHints(dpy, win , &sizehints);
	dbg_print(ARDBG_VERBOSE,"Calling XSetStandardProperties\n");
	XSetStandardProperties(dpy, win, "ardev", "ardev", None, (char **) NULL, 0,
	&sizehints);

	dbg_print(ARDBG_VERBOSE,"Freeing Visinfo\n");
	XFree(visinfo);
	while ((err = glGetError()))
		dbg_print(ARDBG_ERR, "Got Open GL Error at init start (error must have been prior to this call): %d %s\n",err,gluErrorString(err));

	dbg_print(ARDBG_VERBOSE,"Mapping Window\n");
	XMapWindow(dpy,win);
	dbg_print(ARDBG_VERBOSE,"Setting Current glx context\n");

	while ((err = glGetError()))
		dbg_print(ARDBG_ERR, "Got Open GL Error in init: %d %s\n",err,gluErrorString(err));

	if (!Fullscreen)
		ToggleFullScreen();
	else
		Fullscreen = false;

	InitGL();
	// to prevent multiple initialisation of the same object when sharing an output object
	init = true;

	initialised = true; // Standard ARObject initialised notification
	Unlock();

	return 0;
}

void OutputX11::InitGL()
{
#ifdef HAVE_ANTTWEAKBAR
    TwWindowSize(width,height);
#endif
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

	unsigned int MinTexWidth;
	unsigned int MinTexHeight;

	// musn't have got an image yet, wait a few seconds and try again
	for (int i = 0; i < 20; ++i)
	{
		MinTexWidth = Capture->GetMaxWidth();
		MinTexHeight = Capture->GetMaxHeight();
		if (MinTexWidth != 0 && MinTexHeight != 0)
		{
			CaptureConnect();
			break;
		}
		sleep(0.25);
	}

}

void OutputX11::CaptureConnect()
{
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
}


void OutputX11::Terminate()
{
	dbg_print(ARDBG_INFO,"Terminating Output Object\n");
	int err;
	Lock();
	glXDestroyContext(dpy, ctx);
	while ((err = glGetError()))
		dbg_print(ARDBG_ERR, "Got Open GL Error in outputx11 terminate: %d %s\n",err,gluErrorString(err));

	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
//	SDL_Quit();

	init = false;
	Unlock();

}

void OutputX11::ShowFrame()
{
	dbg_print(ARDBG_VERBOSE,"Displaying GL Frame\n");
//	SDL_GL_SwapBuffers( );
	glXSwapBuffers(dpy,win);

	XEvent xev;

	// New method of checking for events required to get client messages (e.g. window close)
	//while (XCheckMaskEvent(dpy,StructureNotifyMask | ExposureMask | KeyPressMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask ,&xev))
	while (XEventsQueued(dpy, QueuedAfterFlush))
	{
		XNextEvent(dpy, &xev);

		switch (xev.type)
		{
			case KeyPress: // 2
				if(KeyInput(XLookupKeysym(&xev.xkey,0),Pressed))
					continue;
				switch(XLookupKeysym(&xev.xkey,0))
				{ // note: keysymdef.h for more keysyms
					case 'f':
					case 'F':
						dbg_print(ARDBG_VERBOSE,"we got a full screen request..\n");
						ToggleFullScreen();
						break;
					case 'q':
					case 'Q':
					case XK_Escape:
						dbg_print(ARDBG_VERBOSE,"we got a quit request..\n");
						callback(callbackData); // Call close callback
						break;
					case 'c':
					case 'C':
						dbg_print(ARDBG_VERBOSE,"we got a clear event..\n");
						PublishEvent(new EventObject(ROE_Clear));
						break;
					case 'w':
					case 'W':
					case XK_Up:
						dbg_print(ARDBG_VERBOSE,"pan up event.\n");
						PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(1,0,0).RotateZ(-CameraPosition->GetPosition().Direction.z),ARPoint(),ARPosition()));
						break;
					case 's':
					case 'S':
					case XK_Down:
						dbg_print(ARDBG_VERBOSE,"pan down event.\n");
						PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(-1,0,0).RotateZ(-CameraPosition->GetPosition().Direction.z),ARPoint(),ARPosition()));
						break;
					case 'a':
					case 'A':
					case XK_Left:
						dbg_print(ARDBG_VERBOSE,"pan left event.\n");
						PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(0,1,0),ARPoint(),CameraPosition->GetPosition()));
						break;
					case 'd':
					case 'D':
					case XK_Right:
						dbg_print(ARDBG_VERBOSE,"pan right event.\n");
						PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(0,-1,0),ARPoint(),CameraPosition->GetPosition()));
						break;
					case 'i':
					case 'I':
						dbg_print(ARDBG_VERBOSE,"zoom in event.\n");
						PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(0,0,-1),ARPoint(),ARPosition()));
						break;
					case 'o':
					case 'O':
						dbg_print(ARDBG_VERBOSE,"zoom out event.\n");
						PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(0,0,1),ARPoint(),ARPosition()));
						break;
					case ',':
					case '<':
						dbg_print(ARDBG_VERBOSE,"previous environment event.\n");
						PublishEvent(new EnvironmentEventObject(EOE_RelativeEnvironment,-1));
						break;
					case '.':
					case '>':
						dbg_print(ARDBG_VERBOSE,"next environment event.\n");
						PublishEvent(new EnvironmentEventObject(EOE_RelativeEnvironment,1));
						break;
					case '1':
						PublishEvent(new EnvironmentEventObject(EOE_AbsoluteEnvironment,0));
						break;
					case '2':
						PublishEvent(new EnvironmentEventObject(EOE_AbsoluteEnvironment,1));
						break;
					case '3':
						PublishEvent(new EnvironmentEventObject(EOE_AbsoluteEnvironment,2));
						break;
					case '4':
						PublishEvent(new EnvironmentEventObject(EOE_AbsoluteEnvironment,3));
						break;
					case XK_Control_L:
					case XK_Control_R:
						looking = true;
						break;
					case XK_Alt_L:
					case XK_Alt_R:
						moving = true;
						break;
				}
				break;
			case KeyRelease:
			{
				if(KeyInput(XLookupKeysym(&xev.xkey,0),Released))
					continue;
				
				switch(XLookupKeysym(&xev.xkey,0))
				{
					case XK_Control_L:
					case XK_Control_R:
						looking = false;
						break;
					case XK_Alt_L:
					case XK_Alt_R:
						moving = false;
						break;
				}
				
				break;
			}
			case ButtonPress:
			{	// Mouse button pressed
				XButtonEvent xbe = xev.xbutton;
				dbg_print(ARDBG_VERBOSE,"Button Pressed %d.\n",xbe.button);
				switch(xbe.button)
				{
					case 1: //left click
					{
						if (MouseInput(xbe.x,xbe.y,Left,Pressed))
							continue;

						//Picking
						pickedObj = PickObject(xbe.x,xbe.y);

						// Movement in a plane
						buttonDown = true;

						break;
					}
					case 2: //middle click
						if (MouseInput(xbe.x,xbe.y,Middle,Pressed))
							continue;
						break;
					case 3: //right click
						if (MouseInput(xbe.x,xbe.y,Right,Pressed))
							continue;
						if(selected) // check for null
							PublishEvent(new EventObject(ROE_ShowTransformControls));
						break;
					case 4: //wheel up
						if (MouseInput(xbe.x,xbe.y,WheelUp,Pressed))
							continue;
						PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(0,0,-1),ARPoint(),ARPosition())); // zoom in
						break;
					case 5: //wheel down
						if (MouseInput(xbe.x,xbe.y,WheelDown,Pressed))
							continue;
						PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(0,0,1),ARPoint(),ARPosition())); // zoom out
						break;
				}
				break;
			}
			case ButtonRelease:
			{
				XButtonEvent xbe = xev.xbutton;
				dbg_print(ARDBG_VERBOSE,"Button Released %d.\n",xbe.button);

				switch(xbe.button)
				{
					case 1: //left click
					{
						if (MouseInput(xbe.x,xbe.y,Left,Released))
							continue;
						string name,control ="";
						if(pickedObj)
						{
							name = string(pickedObj->GetParent()->Name);
							control = string("TransformControls");
						}

						// don't lose selection due to transform controls
						if(name.compare(control)!=0 || !(pickedObj))
							SelectPickedObject();

						if(!(selected))
							PublishEvent(new EventObject(ROE_HideTransformControls));
						buttonDown = false;

						if(pickedObj)
							PublishEvent(new PickedEventObject(POE_Transform,ARPoint(xbe.x,xbe.y,0),pickedObj,CameraPosition->GetTransformedPosition()));
						PublishEvent(new EventObject(ROE_ResetControls));
					}
				}
				break;
			}
			case MotionNotify:
			{
				XMotionEvent xme = xev.xmotion;
				if (MouseInput(xme.x,xme.y,NA,Moved))
					continue;
				
				if(looking)
				{
					int moveX = xme.x - oldMouseX;
					int moveY = oldMouseY - xme.y; // Y coordinates reversed
					double rotateYaw = moveX / (double)width * 6 * M_PI; // Screen width = 3 rotations
					double rotatePitch = moveY / (double)height * 4 * M_PI; // Screen height = 2 rotations
					PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(),ARPoint(0,rotatePitch,rotateYaw),CameraPosition->GetPosition()));
				}
				else if (moving)
				{
					ARPosition camPos = CameraPosition->GetTransformedPosition();
					// Transform camera by dragging around plane.
					Vector3D dir = PixelDirection(xme.x, xme.y);
					Vector3D onPlane = dir * (camPos.Origin.z / dir.z);
					Vector3D oldDir = PixelDirection(oldMouseX, oldMouseY);
					Vector3D oldOnPlane = oldDir * (camPos.Origin.z / oldDir.z);
					// Move the camera such that the position which used to be under the mouse still is
					Vector3D move = onPlane - oldOnPlane;
					ARPoint rotatedMove = ARPoint(move.x, move.y, 0).RotateYPR(-camPos.Direction.z, -camPos.Direction.y, -camPos.Direction.x);
					PublishEvent(new PositionEventObject(POE_CameraDeltaPosition,ARPoint(rotatedMove.x,rotatedMove.y,rotatedMove.z),ARPoint(),CameraPosition->GetPosition()));
				}
				else if(buttonDown)
				{
					if(pickedObj)
						PublishEvent(new PickedEventObject(POE_Transform,ARPoint(xme.x,xme.y,0),pickedObj,CameraPosition->GetTransformedPosition()));
				}
				
				oldMouseX = xme.x;
				oldMouseY = xme.y;
				
				//int X = xme.x;
				//int Y = xme.y;
				//dbg_print(ARDBG_VERBOSE,"Mouse: [%d,%d]\n",X,Y);  //not a good idea to enable, unless debugging mouse movement
				/*int margin = 60; // scroll border in pixels

				// horizontal scrolling
				if(X <= 0 + margin)
					PublishEvent(new PositionEventObject(POE_DeltaPosition,ARPoint(0,0,-0.5))); // left edge
				else if(X >= width-margin)
					PublishEvent(new PositionEventObject(POE_DeltaPosition,ARPoint(0,0,0.5))); // right edge

				// vertical scrolling
				if(Y <= 0 + margin)
					PublishEvent(new PositionEventObject(POE_DeltaPosition,ARPoint(-0.25,0,0))); // top edge
				else if(Y >= height-margin)
					PublishEvent(new PositionEventObject(POE_DeltaPosition,ARPoint(0.25,0,0))); // bottom edge
					*/
				break;
			}
			case Expose: // 12
				// do nothing
				break;
			case UnmapNotify: // 18
				// We should suspend rendering here
				break;
			case MapNotify: // 19
				// We should resume rendering here if we had previously suspended it
				break;
			case ReparentNotify: // 21
				// do nothing
				break;
			case ConfigureNotify: // 22
				WindowResized(xev.xconfigure.width, xev.xconfigure.height);
				dbg_print(ARDBG_VERBOSE,"Recieved resize event\n");
				break;
			case ClientMessage: // 33
				if (xev.xclient.data.l[0] == (unsigned)wmDeleteMessage)
				{
					dbg_print(ARDBG_VERBOSE,"Recieved close event\n");
					callback(callbackData); // Call quit callback
				}
				break;

			default:
				dbg_print(ARDBG_WARN,"Recieved unknown X event: %d\n",xev.type);
		}
	}

	// if background texture has changed update
	if(TexWidth != OldTexWidth || TexHeight != OldTexHeight)
	{
		OldTexWidth = TexWidth;
		OldTexHeight = TexHeight;

		dbg_print(ARDBG_INFO,"Creating %dx%d backdrop texture\n",TexWidth, TexHeight);

		glEnable(GL_TEXTURE_2D);

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
		delete [] TempTexture;
		
		// Get the last frame from the camera to sub in as data
		const ARImage & im = Capture->GetFrame();
		glPixelStorei(GL_UNPACK_ROW_LENGTH, im.x_size);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,im.x_size,im.y_size,im.ColourFormat,GL_UNSIGNED_BYTE,im.data);
		glDisable(GL_TEXTURE_2D);
	}
    // used to collect events
/*    SDL_Event event;

	    while ( SDL_PollEvent( &event ) )
		{
		    switch( event.type )
			{
			case SDL_ACTIVEEVENT:
			    // Something's happend with our focus
			    // If we lost focus or we are iconified, we
			    // shouldn't draw the screen
			    //
//			    if ( event.active.gain == 0 )
//				isActive = FALSE;
//			    else
//				isActive = TRUE;
			    break;
			case SDL_VIDEORESIZE:
			    /// handle resize event
			    surface = SDL_SetVideoMode( event.resize.w,
							event.resize.h,
							24, videoFlags );
			    if ( !surface )
				{
				    fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
				    exit( 1 );
				}
				width = event.resize.w;
				height = event.resize.h;
//			    resizeWindow( event.resize.w, event.resize.h );
			    break;
			case SDL_KEYDOWN:
			    // handle key presses
			    handleKeyPress( &event.key.keysym );
			    break;
			case SDL_QUIT:
			    // handle quit requests
			    //done = TRUE;
			    break;
			default:
			    break;
			}
		}    */

}

/* function to handle key press events */
/*void OutputX11::handleKeyPress( SDL_keysym *keysym )
{
    switch ( keysym->sym )
	{
	case SDLK_ESCAPE:
	    // ESC key was pressed
	    exit( 0 );
	    break;
	case SDLK_F1:
	    // F1 key was pressed
	    // this toggles fullscreen mode
	     //
	     ToggleFullScreen();

	    break;
	default:
	    break;
	}

    return;
}
*/

void OutputX11::ToggleFullScreen()
{
    //SDL_WM_ToggleFullScreen( surface );
    Fullscreen = !Fullscreen;
    if (Fullscreen)
    {
    	OldWidth = width;
    	OldHeight = height;
		bool Xinerama = false;
		int x_org = 0;
		int y_org = 0;


		if ( XineramaIsActive( dpy ) )
		{
			int nscreens;
			XineramaScreenInfo * info = XineramaQueryScreens( dpy, &nscreens );

			dbg_print(ARDBG_INFO,"Xinerama is active\n");
			Xinerama = true;

			width = info[0].width;
			height = info[0].height;
			x_org = info[0].x_org;
			y_org = info[0].y_org;

			for(int i = 0; i< nscreens; i++ )
			{
				dbg_print(ARDBG_VERBOSE, "Screen %d: (%d) %d+%d+%dx%d\n", i, info[i].screen_number,
				info[i].x_org, info[i].y_org,
				info[i].width, info[i].height );
			}
		}
		else
		{
			dbg_print(ARDBG_INFO, "Xinerama is not active\n");
			Screen * scr = DefaultScreenOfDisplay(dpy);
			Window rootwin = RootWindowOfScreen(scr);
			XWindowAttributes attribs;
			Status stat;
			stat = XGetWindowAttributes(dpy,rootwin,&attribs);
			width = attribs.width;
			height = attribs.height;


//    		printf( "DefaultScreenOfDisplay, %d x %d\n", WidthOfScreen(scr), HeightOfScreen(scr) );
    		//width = WidthOfScreen(scr);
    		//height = HeightOfScreen(scr);
		}
		printf("dims: %d x %d\n",width,height);
		//set_mwm_border(0);
		//XMoveResizeWindow(dpy,win,x_org,y_org,width,height);
		//XRaiseWindow(dpy, win);

		XEvent xev;
		Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
		Atom fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = win;
		xev.xclient.message_type = wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1;
		xev.xclient.data.l[1] = fullscreen;
		xev.xclient.data.l[2] = 0;

		XSendEvent(dpy, DefaultRootWindow(dpy), False,
		SubstructureNotifyMask, &xev);


    }
    else
    {
    	width = OldWidth;
    	height = OldHeight;
	XEvent xev;
	Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 0;
	xev.xclient.data.l[1] = fullscreen;
	xev.xclient.data.l[2] = 0;

	XSendEvent(dpy, DefaultRootWindow(dpy), False,
	SubstructureNotifyMask, &xev);
//		set_mwm_border(MWM_DECOR_ALL);
//		XMoveResizeWindow(dpy,win,0,0,width,height);
    }
#ifdef HAVE_ANTTWEAKBAR
    TwWindowSize(width,height);
#endif
}


/*
 * Specify which Motif window manager border decorations to put on a
 * top-level window.  For example, you can specify that a window is not
 * resizabe, or omit the titlebar, or completely remove all decorations.
 * Input:  dpy - the X display
 *         w - the X window
 *         flags - bitwise-OR of the MWM_DECOR_xxx symbols in
X11/Xm/MwmUtil.h
 *                 indicating what decoration elements to enable.  Zero
would
 *                 be no decoration.
 */
void OutputX11::set_mwm_border(unsigned long flags)
{
   PropMotifWmHints motif_hints;
   Atom prop, proptype;

   /* setup the property */
   motif_hints.flags = MWM_HINTS_DECORATIONS;
   motif_hints.decorations = flags;

   /* get the atom for the property */
   prop = XInternAtom( dpy, "_MOTIF_WM_HINTS", True );
   if (!prop) {
      /* something went wrong! */
      return;
   }

   /* not sure this is correct, seems to work, XA_WM_HINTS didn't work */
   proptype = prop;

   XChangeProperty( dpy, win,                         /* display, window */
                    prop, proptype,                 /* property, type */
                    32,                             /* format: 32-bit datums
*/
                    PropModeReplace,                /* mode */
                    (unsigned char *) &motif_hints, /* data */
                    PROP_MOTIF_WM_HINTS_ELEMENTS    /* nelements */
                  );
}

OutputX11* OutputX11::Shared = 0;

void OutputX11::RemoveSharedOutput(const char* name)
{
	if(OutputX11::Shared==0)
		return;
	
	OutputX11::Shared->RemoveEnvironment(name);

	if (OutputObject::envNames.size() == 0)
	{
		delete OutputX11::Shared;
		OutputX11::Shared = 0;
	}

	return;
}

OutputX11* OutputX11::GetSharedOutput(CaptureObject* cap, CameraObject * cam, PositionObject * cam_pos, const int _x, const int _y, const char * _DisplayName, const char * _Name, const bool FullScreen)
{
	if(OutputX11::Shared==0)
	{
		//Clear lists
		OutputObject::cameras.clear();
		OutputObject::camera_positions.clear();
		OutputObject::captures.clear();
		//Create new singleton object
		OutputX11::Shared = new OutputX11(cap,cam,cam_pos,_x, _y,_DisplayName,_Name,FullScreen);
	}
	
	OutputX11::Shared->AddEnvironment(_Name);

	// Store environment objects, so we can switch them later
	OutputObject::cameras.push_back(cam);
	OutputObject::camera_positions.push_back(cam_pos);
	OutputObject::captures.push_back(cap);

	return OutputX11::Shared;
}
