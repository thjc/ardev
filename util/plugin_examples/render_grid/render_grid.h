/* -- 2008-12-12 --
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

#ifndef _RENDER_GRID_H_
#define _RENDER_GRID_H_
#include <ardev/ardev.h>

#include <GL/gl.h>
#include <GL/glut.h>

#include <ardev/debug.h>
#include <ardev/exception.h>
#include <cm_objecthandler.h>
#include <cm_parameter_ardev.h>
#include <anttweakbar.h>

#undef signals
#include <libplayerc++/playerc++.h>

#include <deque>
#include <map>

using namespace PlayerCc;
using namespace std;

// Render object which does the rendering of the grid
class RenderGrid : public RenderObject
{
	public:
		/// Grid constructor, arguments passed in from RenderGridHandler
		RenderGrid(ARColour colour,float major, int intervals, int _size,bool _text, bool _circular) : RenderObject(colour)
		{
			Major = major;
			Intervals = intervals;
			size = _size;
			text = _text;
			circular = _circular;
			RegisterForEvents();
			SetSelectable(true);
			measuring = false;
			distance = 0;
			i=0;
			tweakBar = NULL;
		};
		/// Render the grid transparent
		void RenderTransparent();
		/// Render the grids bounding geometry
		void RenderBounding();

		/// These are callback functions for the GUI
#ifdef HAVE_ANTTWEAKBAR
		virtual void DisplaySetup(TweakBar* tweakBar, const char* Name);

		static void TW_CALL GetMajorTW(void *value,void *clientData)
		{
			*static_cast<float *>(value) = static_cast<RenderGrid *>(clientData)->Major;
		}
		static void TW_CALL SetMajorTW(const void *value,void *clientData)
		{
			static_cast<RenderGrid *>(clientData)->Major = (*static_cast<const float *>(value));
		}

		static void TW_CALL GetMinorTW(void *value,void *clientData)
		{
			*static_cast<float *>(value) = static_cast<RenderGrid *>(clientData)->Intervals;
		}
		static void TW_CALL SetMinorTW(const void *value,void *clientData)
		{
			static_cast<RenderGrid *>(clientData)->Intervals = (*static_cast<const float *>(value));
		}

		static void TW_CALL GetSizeTW(void *value,void *clientData)
		{
			*static_cast<int *>(value) = static_cast<RenderGrid *>(clientData)->size;
		}
		static void TW_CALL SetSizeTW(const void *value,void *clientData)
		{
			static_cast<RenderGrid *>(clientData)->size = (*static_cast<const int *>(value));
		}

#endif

		float Major;
		float Intervals;
		int size;
		float distance;

		bool circular;
		bool text; // render measurements // not working
		int i;
		
		TweakBar* tweakBar;

		// Measure markers
		ARPoint first,second; ///< markers for the measuring tape
		bool measuring; ///< indicates if you want to see and use the measuring tool

		///Event Handler
		virtual void Event(EventObject* event);
		ARPoint Transform(ARPoint mouse); ///< transform from mouse to world coordinates, on a plane
};

// configuration manager handler for loading and aride usage
class RenderGridHandler : public RenderObjectHandler
{
	public:
		RenderGridHandler();
		~RenderGridHandler();

		RenderObject & GetObject();
		void RemoveObject();

		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new RenderGridHandler);};

	protected:
		RenderGrid * obj;
		ARColourParameter Colour;
		IntParameter Major;
		IntParameter Minor;
		IntParameter Size;
		BooleanParameter Text;
		BooleanParameter Rect;
};

#endif	// _RENDER_GRID_H_
