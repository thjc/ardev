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
#ifndef OUTPUT_X11_H
#define OUTPUT_X11_H

/***************************************************************************
 *            capture_file.h
 *
 *  Tue Apr 27 15:34:26 2004
 *  Copyright  2004  Toby Collett
 *  Email
 ****************************************************************************/

#include <ardev/ardev.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <pthread.h>
#include <X11/extensions/Xinerama.h>
#include <SDL/SDL.h>

#include <config.h>

/** \brief Class that creates a OpenGL rendering context and optionally an X11 window
 */
class OutputX11 : public OutputObject
{
	public:
		/// Create the output object with a window size of 0,0
		OutputX11(CaptureObject* cap, CameraObject * cam, PositionObject * cam_pos, int x=300,int y=300, const char * DisplayName=NULL, const char * Name=NULL,bool FullScreen=false);
		virtual ~OutputX11() {delete DisplayName;}; ///< Destructor
		int Initialise(bool); ///< Initialise the X window and gl context
		void Terminate(); ///< Terminate the X window
		void ShowFrame(); ///< Display the current Frame
		//void handleKeyPress( SDL_keysym *keysym ); ///<Handle sdl key events

		static OutputX11* GetSharedOutput(CaptureObject* cap, CameraObject * cam, PositionObject * cam_pos, const int _x, const int _y, const char * _DisplayName, const char * _Name, const bool FullScreen);
		static void RemoveSharedOutput(const char* name);

	protected:
		void ToggleFullScreen();
		void InitGL();
		virtual void CaptureConnect();

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
    	Atom wmDeleteMessage;

    	static OutputX11* Shared;
    	bool buttonDown;
    	bool looking;
    	bool moving;
    	int oldMouseX;
    	int oldMouseY;

    	void set_mwm_border(unsigned long flags);

    	bool init; // Used to prevent multiple windows opening when using a shared output
};

/* bit definitions for MwmHints.flags */
#define MWM_HINTS_FUNCTIONS (1L << 0)
#define MWM_HINTS_DECORATIONS (1L << 1)
#define MWM_HINTS_INPUT_MODE (1L << 2)
#define MWM_HINTS_STATUS (1L << 3)

/* bit definitions for MwmHints.decorations */
#define MWM_DECOR_ALL  (1L << 0)
#define MWM_DECOR_BORDER (1L << 1)
#define MWM_DECOR_RESIZEH (1L << 2)
#define MWM_DECOR_TITLE  (1L << 3)
#define MWM_DECOR_MENU  (1L << 4)
#define MWM_DECOR_MINIMIZE (1L << 5)
#define MWM_DECOR_MAXIMIZE (1L << 6)

#ifndef PROP_MOTIF_H
#define PROP_MOTIF_H
typedef struct
{
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long          inputMode;
    unsigned long status;

} PropMotifWmHints;

#define PROP_MOTIF_WM_HINTS_ELEMENTS 5
#endif // PROP_MOTIF_H

#endif
