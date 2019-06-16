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
#ifndef CM_DC1394_H_
#define CM_DC1394_H_

#include <ardev/ardevconfig.h>
#ifdef HAVE_DC1394
 

#include "cm_objecthandler.h"
#include "cm_parameter.h"
#include <ardev/capture.h>

void RegisterDC1394ObjectHandlers();

class CaptureDC1394Handler : public CaptureObjectHandler
{
	public:
		CaptureDC1394Handler();
		~CaptureDC1394Handler();
	
		CaptureDC1394 & GetObject();
		void RemoveObject();
	
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new CaptureDC1394Handler);};
	protected:
		CaptureDC1394 * Obj;
};

#endif
#endif /*CM_DC1394_H_*/
