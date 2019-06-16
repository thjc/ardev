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
 
#include <ardev/ardevconfig.h>
#ifdef HAVE_ARTOOLKITPLUS
 
#ifndef ARIDE_ARTOOLKITPLUSHANDLERS_H
#define ARIDE_ARTOOLKITPLUSHANDLERS_H

#include "cm_objecthandler.h"
#include "cm_parameter.h"
#include <ardev/artoolkitplus.h>

void RegisterARToolKitPlusObjectHandlers();


class ARToolKitPlusPreProcessHandler : public FrameProcessObjectHandler
{
	public:
		ARToolKitPlusPreProcessHandler();
		~ARToolKitPlusPreProcessHandler();
	
		ARToolKitPlusPreProcess & GetObject();
		void RemoveObject();
	
		static ObjectHandler * CreateHandler() {return reinterpret_cast<ObjectHandler * > (new ARToolKitPlusPreProcessHandler);};
	protected:
		ARToolKitPlusPreProcess * Obj;
	
		CameraObjectParameter Cam;
};

// --- ARToolKitPreProcess Parameter ---
class ARToolKitPlusPreProcessParameter : public ARObjectParameterBase
{
public: 
		ARToolKitPlusPreProcessParameter(QString _Name, QString _Description, QString _DefaultValue="") : ARObjectParameterBase(_Name,_Description,_DefaultValue) {Type="ARToolKitPlusPreProcess";};
		virtual ~ARToolKitPlusPreProcessParameter() {};

		ARToolKitPlusPreProcessHandler * GetClass() 
		{
			if (Value == "")
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			aride_object * temp = CurrentProject->GetObjectByGUID(Value);
			if (temp == NULL)
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			if (temp->Section == ARIDE_PREPROCESS && temp->Type == "ARToolKitPlusPreProcess")
				return reinterpret_cast<ARToolKitPlusPreProcessHandler *> (temp->Handler);
			throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
		};				
};

class ARToolKitPlusPositionHandler : public PositionObjectHandler
{
	public:
		ARToolKitPlusPositionHandler();
		~ARToolKitPlusPositionHandler();
	
		PositionObject & GetObject();
		void RemoveObject();
	
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new ARToolKitPlusPositionHandler);};
	protected:
		ARToolKitPlusPosition * Obj;
	
		IntParameter MarkerID;
		DoubleParameter Height;
		ARToolKitPlusPreProcessParameter Pre;
};

#endif
#endif 
