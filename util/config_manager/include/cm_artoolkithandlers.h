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
#ifdef HAVE_LIBAR
 
#ifndef ARIDE_ARTOOLKITHANDLERS_H
#define ARIDE_ARTOOLKITHANDLERS_H

#include "cm_objecthandler.h"
#include "cm_parameter.h"
#include <ardev/artoolkit.h>

void RegisterARToolKitObjectHandlers();


class ARToolKitPreProcessHandler : public PreProcessObjectHandler
{
	public:
		ARToolKitPreProcessHandler();
		~ARToolKitPreProcessHandler();
	
		ARToolKitPreProcess & GetObject();
		void RemoveObject();
	
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new ARToolKitPreProcessHandler);};
	protected:
		ARToolKitPreProcess * Obj;
	
		IntParameter Threshold;
		CameraObjectParameter Cam;
};

// --- ARToolKitPreProcess Parameter ---
class ARToolKitPreProcessParameter : public StringParameter
{
public: 
		ARToolKitPreProcessParameter(QString _Name, QString _Description, QString _DefaultValue="") : StringParameter(_Name,_Description,_DefaultValue) {Type="ARToolKitPreProcess";};
		virtual ~ARToolKitPreProcessParameter() {};

		ARToolKitPreProcessHandler * GetClass() 
		{
			if (Value == "")
				throw aride_exception(ARIDE_CLASS_NOT_FOUND, __FILE__, __LINE__);
			aride_object * temp = theMainWindow->GetObjectByName(Value);
			if (temp == NULL)
				throw aride_exception(ARIDE_CLASS_NOT_FOUND, __FILE__, __LINE__);
			if (temp->Section == ARIDE_PREPROCESS && temp->Type == "ARToolKitPreProcess")
				return reinterpret_cast<ARToolKitPreProcessHandler *> (temp->Handler);
			throw aride_exception(ARIDE_CLASS_NOT_FOUND, __FILE__, __LINE__);
		};				
};

class ARToolKitPositionHandler : public PositionObjectHandler
{
	public:
		ARToolKitPositionHandler();
		~ARToolKitPositionHandler();
	
		PositionObject & GetObject();
		void RemoveObject();
	
		static ObjectHandler * CreateHandler() {return static_cast<ObjectHandler * > (new ARToolKitPositionHandler);};
	protected:
		ARToolKitPosition * Obj;
	
		DoubleParameter PattWidth;
		StringParameter PattFile;
		ARToolKitPreProcessParameter Pre;
};

#endif
#endif 
