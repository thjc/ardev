

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
 * 		changed methods of outputx11 to be able to use this for FLTK
 */

#include <ardev/ardev.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <pthread.h>
#include <SDL/SDL.h>

#ifndef OUTPUT_MANAGE_H
#define OUTPUT_MANAGE_H

/***************************************************************************
 *            capture_file.h
 *
 *  Tue Apr 27 15:34:26 2004
 *  Copyright  2004  Toby Collett
 *  Email
 ****************************************************************************/


/** \brief Class that creates a OpenGL rendering context and optionally an X11 window
 */
class OutputManage : public OutputObject
{
	public:
		/// Create the output object with a window size of 0,0
		OutputManage(CaptureObject* cap, CameraObject * cam, PositionObject * cam_pos, int x=300,int y=300, const char * DisplayName=NULL, bool FullScreen=false);
		virtual ~OutputManage() {delete DisplayName;}; ///< Destructor
		int Initialise(bool); ///< Initialise the X window and gl context
		void Terminate(); ///< Terminate the X window
		void ShowFrame(); ///< Display the current Frame
		//void handleKeyPress( SDL_keysym *keysym ); ///<Handle sdl key events

	   	// this is part of a event workaround for events within output objects
	    virtual void Event(EventObject* event){};
	    
	    list<RenderPair> GetRenderList(){return RenderObjects;};
	    PositionObject* GetCameraPosition(){return CameraPosition;};

	protected:
		void ToggleFullScreen();
		void InitGL();


		char * DisplayName;
		Display *dpy; ///< The X Display
		Screen * scr; ///< xinerama screen
		Window win; ///< The X Window
		GLXContext ctx; ///< The GLContext
		bool Fullscreen; ///<do we want a full screen window
		SDL_Surface *surface; ///< Our SDl Surface
		/** Flags to pass to SDL_SetVideoMode */
    	int videoFlags;
    	int videoFlagsFull;
    	int OldWidth, OldHeight;

    	void set_mwm_border(unsigned long flags);

    	bool init; // Used to prevent multiple windows opening when using a shared output
};

#endif /*OUTPUT_MANAGE_H_*/
