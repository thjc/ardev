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
#ifndef ARIDE_PARAMETER_ARDEV_H
#define ARIDE_PARAMETER_ARDEV_H

#include <qobject.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include "aride_parameter.h"
#include "mainwindow.h"
#include "aride_exception.h"

class aride_object;

// ARDev Object Types (ie Camera, Capture etc)

template<class T, ArideSection Section>
class ARObjectParameter : public StringParameter
{
	public:
		ARObjectParameter(QString _Name, QString _Description, QString _DefaultValue="") : StringParameter(_Name,_Description,_DefaultValue) {Type=GetArideSectionTypeName(Section);};
		~ARObjectParameter() {};
	
		T GetClass() 
		{
			if (Value == "")
				throw aride_exception(ARIDE_CLASS_NOT_FOUND, __FILE__, __LINE__);
			aride_object * temp = theMainWindow->GetObjectByName(Value);
			if (temp == NULL)
				throw aride_exception(ARIDE_CLASS_NOT_FOUND, __FILE__, __LINE__);
			if (temp->Section == Section)
				return reinterpret_cast<T> (temp->Handler);
			throw aride_exception(ARIDE_CLASS_NOT_FOUND, __FILE__, __LINE__);
		};	
		
		virtual void ExternalNameChange(const QString & OldName, const QString & NewName) 
		{
			if (Value == OldName)
				fromString(NewName);
		}
};

template <class T>
class ArideObjectHandler;

typedef ArideObjectHandler<CameraObject> CameraObjectHandler;
typedef ArideObjectHandler<CaptureObject> CaptureObjectHandler;
typedef ArideObjectHandler<OutputObject> OutputObjectHandler;
typedef ArideObjectHandler<PositionObject> PositionObjectHandler;
//typedef ArideObjectHandler<RenderObject> RenderObjectHandler;

class RenderObjectHandler;
/*typedef CameraObjectHandler;
typedef CaptureObjectHandler;
typedef OutputObjectHandler;
typedef PositionObjectHandler;*/

typedef ARObjectParameter<CameraObjectHandler*,ARIDE_CAMERA> CameraObjectParameter;
typedef ARObjectParameter<CaptureObjectHandler*,ARIDE_CAPTURE> CaptureObjectParameter;
typedef ARObjectParameter<OutputObjectHandler*,ARIDE_OUTPUT> OutputObjectParameter;
typedef ARObjectParameter<PositionObjectHandler*,ARIDE_POSITION> PositionObjectParameter;
typedef ARObjectParameter<RenderObjectHandler*,ARIDE_RENDER> RenderObjectParameter;


#endif
