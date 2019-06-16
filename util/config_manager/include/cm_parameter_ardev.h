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
#include <qcombobox.h>
#include <qpointer.h>

#include "cm_parameter.h"
#include <ardev/exception.h>
#include <ardev/debug.h>

class aride_object;

// ARDev Object Types (ie Camera, Capture etc)

class ARObjectParameterBase : public StringParameter
{
	Q_OBJECT
	
	public:
		ARObjectParameterBase(QString _Name, QString _Description, QString _DefaultValue="") 
			: StringParameter(_Name,_Description,_DefaultValue) 
		{
			Section=-1;
			wid = NULL;
		};
		virtual ~ARObjectParameterBase() {};
	
		virtual QWidget * CreateTypeWidget() 
		{	
			dbg_print(ARDBG_VERBOSE,"creating new ardev_parameter with GUID: %s\n",static_cast<const char *> (Value.toAscii()));
			
			wid = new QComboBox();
			id = wid->winId();
			wid->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
			
			// fill in values
			for (list<aride_object *>::const_iterator itr = CurrentProject->Objects.begin(); itr != CurrentProject->Objects.end(); ++itr)
			{
				if ((*itr)->Section == Section || (Section == -1 && (*itr)->Type == Type))
				{
					wid->addItem((*itr)->GetName(),(*itr)->GUID);	
				}	
			}
			int index;
			if ((index = wid->findData(Value)) >= 0)
				wid->setCurrentIndex(index);
			else if ((index = wid->currentIndex()) >= 0)
				Value = wid->itemData(index).toString();
			QObject::connect(wid,SIGNAL(currentIndexChanged(int)),this,SLOT(ValueChanged(int)));
			
			return wid;
		}; 
		virtual QString toString() {return Value;};
		virtual QString fromString(const QString & inString) 
		{ 
			dbg_print(ARDBG_VERBOSE,"fromstring called on ardev_parameter with GUID: %s\n",static_cast<const char *> (inString.toAscii()));
			Value = inString;
			if (!wid)
			{
				dbg_print(ARDBG_VERBOSE,"No Widget yet\n");
				return Value;
			}
			if (wid->currentIndex() == wid->findData(Value))
			{
				dbg_print(ARDBG_VERBOSE,"Item already active\n");
				return Value;
			}
			int index;
			if ((index = wid->findData(Value)) >= 0)
			{
				dbg_print(ARDBG_VERBOSE,"Found item with GUID\n");
				wid->setCurrentIndex(index);
			}
			else if ((index = wid->currentIndex()) >= 0)
			{
				dbg_print(ARDBG_VERBOSE,"Didnt find GUID so setting to current Item\n");
				Value = wid->itemData(index).toString();
			}
			return Value;
		};
		
		virtual void update()
		{
			dbg_print(ARDBG_VERBOSE,"update called on ardev_parameter\n");
			if (!wid)
			{
				dbg_print(ARDBG_VERBOSE,"did not find widget to update\n");					
				return;
			}
			QString TempValue = Value; // need to preseve value accross the widget clear
			wid->clear();
			// fill in values
			for (list<aride_object *>::const_iterator itr = CurrentProject->Objects.begin(); itr != CurrentProject->Objects.end(); ++itr)
			{
				if ((*itr)->Section == Section || (Section == -1 && (*itr)->Type == Type))
				{
					wid->addItem((*itr)->GetName(),(*itr)->GUID);	
					dbg_print(ARDBG_VERBOSE,"Adding object to list: %s\n", static_cast<const char*> ((*itr)->GetName().toAscii()));
				}	
			}
			int index;
			dbg_print(ARDBG_VERBOSE,"Looking for Value: %s\n", static_cast<const char*> (Value.toAscii()));
			if ((index = wid->findData(TempValue)) >= 0)
				wid->setCurrentIndex(index);
			else if ((index = wid->currentIndex()) >= 0)
				Value = wid->itemData(index).toString();
		}		
	protected slots:
		virtual void ValueChanged(int index) 
		{
			assert(wid);
			dbg_print(ARDBG_VERBOSE, "ARObjectParameterBase Value changed to index %d\n",index);
			fromString(wid->itemData(index).toString());
		};
		
	
	protected:
		int Section;
		QPointer<QComboBox> wid;
};

template<class T, ArideSection SectionInit>
class ARObjectParameter : public ARObjectParameterBase
{
	public:
		ARObjectParameter(QString _Name, QString _Description, QString _DefaultValue="") : ARObjectParameterBase(_Name,_Description,_DefaultValue) {Section=SectionInit; Type=GetArideSectionTypeName(SectionInit);};
		~ARObjectParameter() {};
	
		T GetClass() 
		{
			if (Value == "")
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			aride_object * temp = CurrentProject->GetObjectByGUID(Value);
			if (temp == NULL)
				throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
			if (temp->Section == Section)
				return reinterpret_cast<T> (temp->Handler);
			throw aride_exception(ARDEV_CLASS_NOT_FOUND, __FILE__, __LINE__);
		};	

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
