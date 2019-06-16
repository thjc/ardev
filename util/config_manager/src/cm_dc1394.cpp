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
#include "cm_dc1394.h"

#ifdef HAVE_DC1394

/* -----------------------------------------
    ARToolKitPlusPreProcess Handler Methods
   ----------------------------------------- */

CaptureDC1394Handler::CaptureDC1394Handler() 
{
	Obj = NULL;
}

CaptureDC1394Handler::~CaptureDC1394Handler()
{
	delete Obj;
}
	
CaptureDC1394 & CaptureDC1394Handler::GetObject()
{
	if (Obj == NULL)
	{
		Obj = new CaptureDC1394;
		if (Obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	CurrentProject->InitialiseObject(Obj);
	return *Obj;
}

void CaptureDC1394Handler::RemoveObject()
{
	delete Obj;
	Obj = NULL;
}

#define REG(X,Y,Z) dbg_print(ARDBG_INFO,"\t%s\n",X);ObjectHandlerRegistry[X] = ObjectRegistryEntry(Y,Z);
void RegisterDC1394ObjectHandlers()
{
	dbg_print(ARDBG_INFO,"Registering DC1394 Object Handlers\n");
	REG("CaptureDC1394",CaptureDC1394Handler::CreateHandler,ARIDE_CAPTURE);
}

#endif
