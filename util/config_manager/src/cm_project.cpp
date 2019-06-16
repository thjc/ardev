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
#include "cm_project.h"
#include <ardev/exception.h>
#include "cm_registry.h"
#include "cm_objecthandler.h"
#include "cm_parameter.h"
#include "cm_parameter_ardev.h"
#include <ardev/debug.h>

#include <string.h>
#include <assert.h>

#include <QFile>
#include <QDomElement>
#include <QDomNode>
#include <QDomDocument>
#include <QTextStream>

#include <algorithm>

const char * GetArideSectionTypeName(ArideSection Section)
{
	switch (Section)
	{
		case ARIDE_UNKNOWN:
			return "arunknown";
		case ARIDE_CAMERA:
			return "arcamera";
		case ARIDE_CAPTURE:
			return "arcapture";
		case ARIDE_OUTPUT:
			return "aroutput";
		case ARIDE_SECONDARYOUTPUT:
			return "arsecondaryoutput";
		case ARIDE_PREPROCESS:
			return "arpreprocess";
		case ARIDE_POSITION:
			return "arposition";
		case ARIDE_RENDER:
			return "arrender";
		case ARIDE_MISC:
			return "armisc";
		case ARIDE_RENDERPAIR:
			return "arrenderpair";
		case ARIDE_ENVIRONMENTS:
			return "arenvironment";
		default:
			return "";
	}
}


aride_object::aride_object(const QDomElement & _Element, ObjectHandler * _Handler)
{
	Missing = false;
	Handler=NULL;
	Element = _Element;
	Section = static_cast<ArideSection> (GetSection(Element.tagName()));
	GUID = Element.attribute("GUID","");
	if (!CurrentProject->CheckGUID(GUID))
		throw(aride_exception(ARDEV_BAD_GUID,__FUNCTION__,__LINE__));
	Type = Element.attribute("Type","");
	QByteArray asciiType = Type.toAscii();
	Parent = Element.attribute("Parent","");
	if (_Handler == NULL)
	{
		if (ObjectHandlerRegistry[asciiType].CreateHandler)
		{
			if ((Handler = ObjectHandlerRegistry[asciiType].CreateHandler()) == NULL)
				throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
		}
		else
		{
			dbg_print(ARDBG_WARN,"Unable to find handler for object type %s in %s\n",static_cast<const char *> (asciiType),__FUNCTION__);
			if ((Handler = new PluginHandler) == NULL)
				throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
			Missing = true;
		}
	}
	else
		Handler = _Handler;
	for (list<Parameter *>::iterator itr = Handler->Parameters.begin(); itr != Handler->Parameters.end(); ++itr)
	{

		QString Temp = Element.attribute((*itr)->Name,(*itr)->DefaultValue);
		if(!Missing)
			(*itr)->fromString(Temp);
		else
			(*itr)->fromString("Missing "+Temp+" plugin.");

	}
}

aride_object::~aride_object()
{
	delete Handler;
}

void aride_object::ChangeType(const QString & NewType)
{
	if (Type == NewType)
		return;

	Type = NewType;
	QByteArray asciiType = Type.toAscii();
	delete Handler;
	Handler = NULL;

	if (ObjectHandlerRegistry[asciiType].CreateHandler)
	{
		if ((Handler = ObjectHandlerRegistry[asciiType].CreateHandler()) == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	else
	{
		dbg_print(ARDBG_WARN,"Unable to find handler for object type %s in %s\n",static_cast<const char *> (asciiType),__FUNCTION__);
		return;
	}

	Element.setAttribute("Type",Type);

	// repropagate the parameters with values
	for (list<Parameter *>::iterator itr = Handler->Parameters.begin(); itr != Handler->Parameters.end(); ++itr)
	{
		QString Temp = Element.attribute((*itr)->Name,(*itr)->DefaultValue);
		(*itr)->fromString(Temp);
	}
}

QString aride_object::GetName() const
{
	if (Handler)
		return Handler->Name.Value;
	else
		return "";
}

void aride_object::SetName(const QString & Name)
{
	if (Handler)
		Handler->Name.fromString(Name);
}

int aride_object::GetSection(const QString & Tagname)
{
	if(Tagname == "camera")
		return ARIDE_CAMERA;
	else if(Tagname == "capture")
		return ARIDE_CAPTURE;
	else if(Tagname == "position")
		return ARIDE_POSITION;
	else if(Tagname == "output")
		return ARIDE_OUTPUT;
	else if(Tagname == "secondaryoutput")
		return ARIDE_SECONDARYOUTPUT;
	else if(Tagname == "preprocess")
		return ARIDE_PREPROCESS;
	else if(Tagname == "render")
		return ARIDE_RENDER;
	else if(Tagname == "misc")
		return ARIDE_MISC;
	else if (Tagname == "render_pair")
		return ARIDE_RENDERPAIR;
	else if (Tagname == "environment")
		return ARIDE_ENVIRONMENTS;
	else
		return ARIDE_UNKNOWN;
}


/* -------------------------------------------------------------------------
	aride_environment class implementation
   ------------------------------------------------------------------------- */

aride_environment::aride_environment(const QDomElement & _Element, aride_project* _Project) : aride_object(_Element,new EnvironmentObjectHandler)
{
	Paused = false;
	Element = _Element;
	Project = _Project;

	GUID = Element.attribute("GUID","");
	if (!CurrentProject->CheckGUID(GUID))
		throw(aride_exception(ARDEV_BAD_GUID,__FUNCTION__,__LINE__));

	SetName(Element.attribute("Name",""));

	EnvHandler = reinterpret_cast<EnvironmentObjectHandler *> (Handler);

	Cam = Element.attribute("Camera","");
	Cap = Element.attribute("Capture","");
	Out = Element.attribute("Output","");

	// now we should load the sub elements, ie preprocess,misc,secondary output etc

	for( QDomNode n = Element.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
		if (n.isElement() && n.toElement().tagName() == "item")
		{
			dbg_print(ARDBG_VERBOSE,"\t\tAdding Item %s to env\n",static_cast<const char *> (n.toElement().attribute("Name","").toAscii()));
			QString ItemGUID = n.toElement().attribute("GUID","");
			aride_object * obj = CurrentProject->GetObjectByGUID(ItemGUID);
			if (!obj)
			{
				dbg_print(ARDBG_WARN,"No object named (%s) found\n",static_cast<const char *> (ItemGUID.toAscii()));
				continue;
			}
			if (obj->Section == ARIDE_PREPROCESS)
				PreProcessObjects.push_back(ItemGUID);
			else if (obj->Section == ARIDE_SECONDARYOUTPUT)
				SecondaryObjects.push_back(ItemGUID);
			else if (obj->Section == ARIDE_MISC)
				MiscObjects.push_back(ItemGUID);
			else
				dbg_print(ARDBG_WARN,"Item (%s) has invalid section (%d) for env\n",static_cast<const char *> (ItemGUID.toAscii()),obj->Section);
		}
	}
}

aride_environment::~aride_environment()
{
	aride_object * obj;
	obj = CurrentProject->GetObjectByGUID(Cam);
	if (obj)
		CurrentProject->RemoveObject(obj);
	obj = CurrentProject->GetObjectByGUID(Cap);
	if (obj)
		CurrentProject->RemoveObject(obj);
	obj = CurrentProject->GetObjectByGUID(Out);
	if (obj)
		CurrentProject->RemoveObject(obj);
	for (list<QString>::const_iterator itr= PreProcessObjects.begin(); itr != PreProcessObjects.end(); ++itr)
	{
		obj = CurrentProject->GetObjectByGUID(*itr);
		if (obj)
			CurrentProject->RemoveObject(obj);
	}
	for (list<QString>::const_iterator itr= MiscObjects.begin(); itr != MiscObjects.end(); ++itr)
	{
		obj = CurrentProject->GetObjectByGUID(*itr);
		if (obj)
			CurrentProject->RemoveObject(obj);
	}
	for (list<QString>::const_iterator itr= SecondaryObjects.begin(); itr != SecondaryObjects.end(); ++itr)
	{
		obj = CurrentProject->GetObjectByGUID(*itr);
		if (obj)
			CurrentProject->RemoveObject(obj);
	}

}

void* DummyClose(void* data)
{
	reinterpret_cast<aride_project*>(data)->Stop();
	return NULL;
}

void OnClose(void* data)
{
	// Create another thread to do the actual close to avoid the case of a thread committing suicide
	// and failing to shut down anything else.
	pthread_t thread;
	int err;
	if ((err=pthread_create(&thread,NULL,DummyClose,data)))
	{
		dbg_print(ARDBG_ERR,"failed to create thread: %d\n",err);
	}
}

void aride_environment::Start()
{
	if (Handler == NULL)
		throw aride_exception(ARDEV_NO_HANDLER,__FUNCTION__,__LINE__);

	QByteArray asciiName = GetName().toAscii();
	if (Paused)
	{
		ARDev::Resume(asciiName);
		Paused = false;
		return;
	}
	dbg_print(ARDBG_ERR,"Starting Environment %s\n",static_cast<const char *> (asciiName));

	aride_object * out_object = CurrentProject->GetObjectByGUID(Out);
	assert(out_object);
	OutputObject & Output = reinterpret_cast <OutputObjectHandler*> (out_object->Handler)->GetObject();
	Output.SetCloseCallback(&OnClose, Project);


	for (list<QString>::const_iterator itr=PreProcessObjects.begin(); itr != PreProcessObjects.end(); ++itr)
	{
		aride_object * object = CurrentProject->GetObjectByGUID(*itr);
		assert(object);
		Output.AddPre(&reinterpret_cast <FrameProcessObjectHandler*> (object->Handler)->GetObject());
	}
	for (list<QString>::const_iterator itr=SecondaryObjects.begin(); itr != SecondaryObjects.end(); ++itr)
	{
		aride_object * object = CurrentProject->GetObjectByGUID(*itr);
		assert(object);
		Output.AddPost(&reinterpret_cast <FrameProcessObjectHandler*> (object->Handler)->GetObject());
	}

	ARDev::Start(&Output,asciiName);

	for (list<RenderPair *>::const_iterator itr = CurrentProject->RenderPairs.begin(); itr != CurrentProject->RenderPairs.end(); ++itr)
	{
		Output.Add(**itr);
	}
}

void aride_environment::Pause()
{
	QByteArray asciiName = GetName().toAscii(); // Careful of lifetime
	ARDev::Pause(asciiName);
	Paused = true;
}

void aride_environment::Stop()
{
	QByteArray asciiName = GetName().toAscii(); // Careful of lifetime
	dbg_print(ARDBG_VERBOSE,"Stopping Environment %s\n",static_cast<const char *> (asciiName));
	ARDev::Stop(asciiName);
	if (Handler == NULL)
		return;
	Paused = false;
}

/* -------------------------------------------------------------------------
	aride_displaylist_node class implementation
   ------------------------------------------------------------------------- */


aride_displaylist_node::aride_displaylist_node(const QDomElement & _Element, aride_displaylist_node * _Parent, aride_object * _Object)
{
	Element = _Element;
	Parent = _Parent;
	object = _Object;
}


aride_displaylist_node::~aride_displaylist_node()
{
}


/* -------------------------------------------------------------------------
	aride_project class implementation
   ------------------------------------------------------------------------- */
aride_project * CurrentProject;

aride_project::aride_project()
{
	StoredFilename=NULL;
	Named = false;
	Changed = false;
	Open = false;
	CurrentProject = this;
}

aride_project::~aride_project()
{
	free(StoredFilename);
	while (Objects.begin() != Objects.end())
	{
		delete Objects.front();
		Objects.pop_front();
	}
	while (Environments.begin() != Environments.end())
	{
		delete Environments.front();
		Environments.pop_front();
	}
	CurrentProject = NULL;
}

int aride_project::New()
{
	StoredFilename = strdup("newproject.aride");
	Named = false;
	Changed = false;
	Open = true;
    QDomElement root = doc.createElement( "aride_project" );
	root.setAttribute("version",CONFIG_FILE_VERSION);
    doc.appendChild( root );

    QDomElement objects = doc.createElement( "objects" );
    root.appendChild( objects );

	return 0;
}

int aride_project::Close()
{
	Stop();

	// Remove the Object Elements
	while (Objects.begin() != Objects.end())
	{
		delete Objects.front();
		Objects.pop_front();
	}

	// Remove the Environment Elements
	while (Environments.begin() != Environments.end())
	{
		delete Environments.front();
		Environments.pop_front();
	}

	// Remove the Environment Elements
	while (DisplayNodes.begin() != DisplayNodes.end())
	{
		delete DisplayNodes.front();
		DisplayNodes.pop_front();
	}

	free(StoredFilename);
	StoredFilename = NULL;
	Open = false;
	doc = QDomDocument();

	return 0;
}

bool aride_project::isOpen()
{
	return Open;
}

bool aride_project::isChanged()
{
	return Changed;
}

bool aride_project::hasName()
{
	return Named;
}

int aride_project::LoadFromFile(const char * Filename)
{
	dbg_print(ARDBG_INFO,"Loading Project File %s\n",Filename);
	if (Open)
	 Close();
	free(StoredFilename);
	StoredFilename = NULL;

	QFile file(Filename);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!file.isOpen())
		throw aride_exception(ARDEV_FILE_NOT_FOUND,__FUNCTION__,__LINE__);
	if(!doc.setContent(&file,false))
		throw aride_exception(ARDEV_XML_PARSE_FAILED,__FUNCTION__,__LINE__);

	if (doc.documentElement().toElement().attribute("version","0") != CONFIG_FILE_VERSION)
	{
		dbg_print(ARDBG_WARN,"Incorrect config file version (%s), should be (%s)\n",static_cast<const char *> (doc.documentElement().toElement().attribute("version""0").toAscii()),CONFIG_FILE_VERSION);
		//return 0;
	}

	QDomElement element = doc.documentElement();
	bool found = false;
	for( QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() )
	{
		if (node.nodeName() == "objects")
		{
			for( QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling() )
			{
				if (n.isElement())
				{
					found = true;
					dbg_print(ARDBG_INFO,"\tLoading Object %s\n",static_cast<const char *> (n.toElement().tagName().toAscii()));
					Objects.push_back(new aride_object(n.toElement()));
				}
			}
		}
	}
	
	if (!found)
	{
		dbg_print(ARDBG_INFO,"No Objects Element in project file\n");
		return 0;
	}

	// load the environments...and idsplay lists
	element = doc.documentElement().toElement();
	if (element.isNull() || element.tagName() != "aride_project")
	{
		dbg_print(ARDBG_INFO,"No aride_project Element in project file\n");
		return 0;
	}

    for( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
		if (n.isElement())
		{
			if (n.toElement().tagName() == "environment")
			{
				dbg_print(ARDBG_VERBOSE,"\tLoading Environment %s\n",static_cast<const char *> (n.toElement().attribute("Name","").toAscii()));
				Environments.push_back(new aride_environment(n.toElement(), this));
			}
			else if (n.toElement().tagName() == "display_list")
			{
				CreateDisplayList(n);
			}
			else if (n.toElement().tagName() == "objects")
			{
				// ignore it we have already processed objects
			}
			else
			{
				dbg_print(ARDBG_WARN,"\tUnknown Element in project file: %s\n",static_cast<const char *> (n.toElement().tagName().toAscii()));
			}
		}
	}

	StoredFilename = strdup(Filename);
	Changed = false;
	Named = true;
	Open = true;
	return 0;
}

int aride_project::SaveToFile(const char * Filename)
{
	if (Filename == NULL)
	{
		Filename = StoredFilename;
		if (Filename == NULL)
			throw aride_exception(ARDEV_NO_FILE_SPECIFIED,__FUNCTION__,__LINE__);
	}
	else
	{
		free(StoredFilename);
		StoredFilename = strdup(Filename);
	}

	dbg_print(ARDBG_INFO,"Saving Objects\n");
	for (list<aride_object *>::iterator itr = Objects.begin(); itr != Objects.end(); ++itr)
	{
		dbg_print(ARDBG_VERBOSE,"\tSaving Object: %s\n",static_cast<const char *> ((*itr)->GetName().toAscii()));
		(*itr)->Element.setAttribute("GUID",(*itr)->GUID);
		(*itr)->Element.setAttribute("Parent",(*itr)->Parent);
		if ((*itr)->Handler != NULL)
		{
			for (list<Parameter *>::iterator itr2 = (*itr)->Handler->Parameters.begin(); itr2 != (*itr)->Handler->Parameters.end(); ++itr2)
			{
				(*itr)->Element.setAttribute((*itr2)->Name,(*itr2)->toString());
			}
		}
	}

	dbg_print(ARDBG_INFO,"Saving Environments\n");
	for (list<aride_environment *>::iterator itr = Environments.begin(); itr != Environments.end(); ++itr)
	{
		dbg_print(ARDBG_VERBOSE,"\tSaving Environment: %s\n",static_cast<const char *> ((*itr)->GetName().toAscii()));
		(*itr)->Element.setAttribute("GUID",(*itr)->GUID);
		(*itr)->Element.setAttribute("Output",(*itr)->Out);
		(*itr)->Element.setAttribute("Camera",(*itr)->Cam);
		(*itr)->Element.setAttribute("Capture",(*itr)->Cap);
		if ((*itr)->Handler != NULL)
		{
			for (list<Parameter *>::iterator itr2 = (*itr)->Handler->Parameters.begin(); itr2 != (*itr)->Handler->Parameters.end(); ++itr2)
			{
				(*itr)->Element.setAttribute((*itr2)->Name,(*itr2)->toString());
			}
		}
		// clear out the environment 'items'
		while((*itr)->Element.hasChildNodes())
			(*itr)->Element.removeChild((*itr)->Element.firstChild());
		// now create nodes for the current items
		for (list<QString>::const_iterator itr2= (*itr)->PreProcessObjects.begin(); itr2 != (*itr)->PreProcessObjects.end(); ++itr2)
		{
			QDomElement item = doc.createElement( "item" );
			item.setAttribute("GUID",*itr2);
		    (*itr)->Element.appendChild( item );
		}
		for (list<QString>::const_iterator itr2 = (*itr)->MiscObjects.begin(); itr2 != (*itr)->MiscObjects.end(); ++itr2)
		{
			QDomElement item = doc.createElement( "item" );
			item.setAttribute("GUID",*itr2);
		    (*itr)->Element.appendChild( item );
		}
		for (list<QString>::const_iterator itr2 = (*itr)->SecondaryObjects.begin(); itr2 != (*itr)->SecondaryObjects.end(); ++itr2)
		{
			QDomElement item = doc.createElement( "item" );
			item.setAttribute("GUID",*itr2);
		    (*itr)->Element.appendChild( item );
		}
	}

	dbg_print(ARDBG_VERBOSE,"Opening Output File %s\n",Filename);
	QFile OutFile(Filename);
	if (!OutFile.open(QIODevice::WriteOnly))
		throw (aride_exception(ARDEV_FILE_OPEN_FAILED,__FUNCTION__,__LINE__));
	QTextStream OutStream(&OutFile);
	OutStream << doc.toString();
	OutFile.close();

	Changed = false;
	Named = true;
	return 0;
}

aride_object * aride_project::CreateObjectNode(ArideSection Section, const QString & Type, aride_object * parent)
{
	// objects element
	QDomElement element = doc.documentElement().namedItem("objects").toElement();
	if (element.isNull())
	{
		dbg_print(ARDBG_INFO,"No Object Element in project file\n");
		assert(0);
	}

	QString tagName;
	// first create the dom node
	switch (Section)
	{
		case ARIDE_CAMERA:
			tagName = "camera";
			break;
		case ARIDE_CAPTURE:
			tagName = "capture";
			break;
		case ARIDE_OUTPUT:
			tagName = "output";
			break;
		case ARIDE_SECONDARYOUTPUT:
			tagName = "secondaryoutput";
			break;
		case ARIDE_PREPROCESS:
			tagName = "preprocess";
			break;
		case ARIDE_POSITION:
			tagName = "position";
			break;
		case ARIDE_RENDER:
			tagName = "render";
			break;
		case ARIDE_MISC:
			tagName = "misc";
			break;
		default:
			assert(0);
	}
	QDomElement newElement = doc.createElement(tagName);
	element.appendChild(newElement);
	newElement.setAttribute("Type",Type);
	newElement.setAttribute("GUID",MakeUniqueGUID());
	newElement.setAttribute("Name",MakeUniqueName(Type));
	if (parent)
		newElement.setAttribute("Parent",parent->GUID);

	Changed = true;

	// now create the aride_object
	Objects.push_back(new aride_object(newElement));
	RefreshObjects();
	return Objects.back();
}

int aride_project::CreateEnvironmentNode()
{
	// objects element
	QDomElement element = doc.documentElement();
	if (element.isNull())
	{
		dbg_print(ARDBG_INFO,"No Parent Element in project file\n");
		return 0;
	}

	QDomElement newElement = doc.createElement("environment");
	newElement.setAttribute("Name",MakeUniqueName("Environment"));
	newElement.setAttribute("GUID",MakeUniqueGUID());

	element.appendChild(newElement);

	Changed = true;

	// now create the aride_object
	Environments.push_back(new aride_environment(newElement, this));
	return 0;
}

aride_displaylist_node * aride_project::CreateDisplayList(QDomNode &n)
{
	if (n.isElement())
	{
		aride_displaylist_node * NewDisplayNode = new aride_displaylist_node(n.toElement(),NULL,NULL);
		assert(NewDisplayNode);
		NewDisplayNode->GUID = n.toElement().attribute("GUID","");
		NewDisplayNode->SetName(n.toElement().attribute("Name",""));

	    for( QDomNode child = n.firstChild(); !child.isNull(); child = child.nextSibling() )
    		CreateDisplayListNode(NewDisplayNode,child);
		DisplayNodes.push_back(NewDisplayNode);
		return NewDisplayNode;
	}
	else
	{
		dbg_print(ARDBG_INFO,"Cannot CreateDisplayList, QDomNode is not element\n");
	}
	return NULL;
}

aride_displaylist_node * aride_project::CreateDisplayList()
{
	QDomElement element = doc.documentElement();
	if (element.isNull())
	{
		dbg_print(ARDBG_INFO,"No Parent Element in project file\n");
		return NULL;
	}

	QDomElement newElement = doc.createElement("display_list");
	newElement.setAttribute("Name",MakeUniqueName("DisplayList"));
	newElement.setAttribute("GUID",MakeUniqueGUID());
	element.appendChild(newElement);

	return CreateDisplayList(newElement);
}

aride_displaylist_node * aride_project::CreateDisplayListNode(aride_displaylist_node * parent, QDomNode &n)
{
	dbg_print(ARDBG_VERBOSE,"CreateDisplayListNode parent and new node\n");
	if (n.isElement())
	{
		if (n.toElement().tagName() == "item")
		{
			aride_object * object = GetObjectByGUID(n.toElement().attribute("GUID",""));
			if (!object)
			{
				dbg_print(ARDBG_WARN,"\tnode (%s) for display list (%s) not found",static_cast<const char *> (n.toElement().attribute("GUID","").toAscii()),static_cast<const char *> (n.toElement().attribute("root","").toAscii()));
				return NULL;
			}
			aride_displaylist_node * NewDisplayNode = new aride_displaylist_node(n.toElement(),parent,object);
			assert(NewDisplayNode);
			NewDisplayNode->GUID = n.toElement().attribute("GUID","");

		    for( QDomNode child = n.firstChild(); !child.isNull(); child = child.nextSibling() )
    			CreateDisplayListNode(NewDisplayNode,child);
			DisplayNodes.push_back(NewDisplayNode);

			Changed = true;

			return NewDisplayNode;
		}
		else
		{
			dbg_print(ARDBG_WARN,"\tUnknown Element in display_list section: %s\n",static_cast<const char *> (n.toElement().tagName().toAscii()));
		}
	}
	else
	{
		dbg_print(ARDBG_WARN,"new node is not element\n");
	}
	return NULL;
}

aride_displaylist_node * aride_project::CreateDisplayListNode(const ArideSection Section, const QString & Type,const QString & Parent)
{
	aride_displaylist_node * parentnode = GetDisplayNodeByGUID(Parent);
	if (parentnode == NULL)
	{
		dbg_print(ARDBG_WARN,"Could not find parent (%s) for new node\n",static_cast<const char *> (Parent.toAscii()));
		return NULL;
	}

	aride_object * NewObject = CreateObjectNode(Section, Type);
	if (NewObject == NULL)
	{
		dbg_print(ARDBG_ERR, "Unable to create object %d:%s\n",Section,static_cast<const char *> (Type.toAscii()));
		return NULL;
	}


	QDomElement newElement = doc.createElement("item");
	newElement.setAttribute("GUID",NewObject->GUID);
	parentnode->Element.appendChild(newElement);

	Changed = true;

	return CreateDisplayListNode(parentnode,newElement);
}

void aride_project::RemoveDisplayListNode(QString GUID)
{
	aride_displaylist_node * node = GetDisplayNodeByGUID(GUID);
	if (node == NULL)
	{
		dbg_print(ARDBG_WARN,"Could not find node (%s)\n",static_cast<const char *> (GUID.toAscii()));
		return;
	}
	else
		RemoveDisplayListNode(node);
}

void aride_project::RemoveDisplayListNode(aride_displaylist_node * node)
{
	assert(node);
	// first remove any children
	// have to be careful here as removing the children can kill the iterators
	bool FoundOne = false;
	do
	{
		FoundOne = false;
		for (list<aride_displaylist_node *>::iterator itr = DisplayNodes.begin(); itr != DisplayNodes.end(); ++itr)
		{
			if ((*itr)->Parent == node)
			{
				FoundOne = true;
				RemoveDisplayListNode(*itr);
				break;
			}
		}
	}
	while(FoundOne);

	// then remove the object we point to
	if (node->object)
		RemoveObject(node->object);
	// finally remove ourselves
	DisplayNodes.remove(node);
	node->Element.parentNode().removeChild(node->Element);
	delete node;

	Changed = true;
}




RenderPair * aride_project::CreateRenderPair(aride_displaylist_node * node)
{
	assert(node);
	assert(node->object);
	assert(node->object->Section != ARIDE_OUTPUT);
	assert(node->Parent);
	assert(node->Parent->object);
	assert(node->Parent->object->Section == ARIDE_POSITION);


	dbg_print(ARDBG_INFO,"Creating a render pair for : %s\n",static_cast<const char *> (node->GUID.toAscii()));

	RenderObject & Render = reinterpret_cast<RenderObjectHandler *> (node->object->Handler)->GetObject();
	Render.SetEnabled(reinterpret_cast<RenderObjectHandler *> (node->object->Handler)->Enabled.Value);

	PositionObject & Position = reinterpret_cast<PositionObjectHandler *> (node->Parent->object->Handler)->GetObject();

	QByteArray asciiObjName = node->object->GetName().toAscii(); // Careful of lifetime
	QByteArray asciiParentName = node->Parent->object->GetName().toAscii();
	RenderPair * ret = new RenderPair(&Render, &Position,0, static_cast<const char *> (asciiObjName),static_cast<const char *> (asciiParentName));

	Position.SetParent(ret);
	Render.SetParent(ret);

	return ret;
}

void aride_project::CreateTransformChain(aride_displaylist_node * node)
{
	assert(node);
	assert(node->object);
	assert(node->object->Section != ARIDE_OUTPUT);

	dbg_print(ARDBG_INFO,"Creating transform chain for : %s\n",static_cast<const char *> (node->GUID.toAscii()));

	PositionObject & Position = reinterpret_cast<PositionObjectHandler *> (node->object->Handler)->GetObject();

	QByteArray asciiName = node->object->GetName().toAscii(); // Careful of lifetime
	Position.SetName(static_cast<const char *> (asciiName));

	PositionObject * LastPos =  & Position;

	aride_displaylist_node * pnode = node->Parent;

	while(pnode && pnode->object && pnode->object->Section==ARIDE_POSITION)
	{
		PositionObject * ParentPosition = &reinterpret_cast<PositionObjectHandler *> (pnode->object->Handler)->GetObject();
		QByteArray asciiParentName = pnode->object->GetName().toAscii();
		ParentPosition->SetName(static_cast<const char *> (asciiParentName));
		LastPos->Next = ParentPosition;
		if (ParentPosition->Next)
			break;
		pnode = pnode->Parent;
		LastPos = ParentPosition;
	}

	return;
}

int aride_project::RemoveObject(aride_object * obj)
{
	if (obj == NULL)
		return 0;

	Objects.remove(obj);
	obj->Element.parentNode().removeChild(obj->Element);
	delete obj;

	Changed = true;

	return 0;
}

int aride_project::RemoveEnvironment(aride_environment * env)
{
	if (env == NULL)
		return 0;

	Environments.remove(env);
	env->Element.parentNode().removeChild(env->Element);
	delete env;

	Changed = true;

	return 0;
}


QString aride_project::GetGUIDByName(const QString & Name)
{
	dbg_print(ARDBG_VERBOSE,"Looking for object: %s\n",static_cast<const char *> (Name.toAscii()));
	for (list<aride_object *>::iterator itr = Objects.begin(); itr != Objects.end(); ++itr)
	{
		if ((*itr)->GetName() == Name)
			return (*itr)->GUID;
	}
	dbg_print(ARDBG_VERBOSE, "Object \"%s\" not found\n",static_cast<const char *> (Name.toAscii()) );
	return "";
}


aride_object * aride_project::GetObjectByGUID(const QString & GUID)
{
	dbg_print(ARDBG_VERBOSE,"Looking for object: %s\n",static_cast<const char *> (GUID.toAscii()));
	for (list<aride_object *>::iterator itr = Objects.begin(); itr != Objects.end(); ++itr)
	{
		if ((*itr)->GUID == GUID)
			return *itr;
	}
	dbg_print(ARDBG_VERBOSE, "Object \"%s\" not found\n",static_cast<const char *> (GUID.toAscii()));
	return NULL;
}

aride_environment * aride_project::GetEnvByGUID(const QString & GUID)
{
	dbg_print(ARDBG_VERBOSE,"Looking for object: %s\n",static_cast<const char *> (GUID.toAscii()));
	for (list<aride_environment *>::iterator itr = Environments.begin(); itr != Environments.end(); ++itr)
	{
		if ((*itr)->GUID == GUID)
			return *itr;
	}
	dbg_print(ARDBG_VERBOSE, "Environment \"%s\" not found\n",static_cast<const char *> (GUID.toAscii()));
	return NULL;
}

aride_displaylist_node * aride_project::GetDisplayNodeByGUID(const QString & GUID)
{
	dbg_print(ARDBG_VERBOSE,"Looking for object: %s\n",static_cast<const char *> (GUID.toAscii()));
	for (list<aride_displaylist_node *>::iterator itr = DisplayNodes.begin(); itr != DisplayNodes.end(); ++itr)
	{
		if ((*itr)->GUID == GUID)
			return *itr;
	}
	dbg_print(ARDBG_VERBOSE, "Display Node \"%s\" not found\n",static_cast<const char *> (GUID.toAscii()));
	return NULL;
}


aride_object * aride_project::GetFirstChild(const aride_object * obj)
{
	dbg_print(ARDBG_INFO,"Looking for object child of: %s\n",static_cast<const char *> (obj->GUID.toAscii()));
	for (list<aride_object *>::const_iterator itr = Objects.begin(); itr != Objects.end(); ++itr)
	{
		if (GetObjectByGUID((*itr)->Parent) == obj)
		{
			return *itr;
		}
	}
	return NULL;
}

void aride_project::RemoveObjects()
{
	dbg_print(ARDBG_INFO,"Cleaning up objects\n");
	for (list<aride_object *>::iterator itr = Objects.begin(); itr != Objects.end(); ++itr)
	{
		dbg_print(ARDBG_INFO,"Remove Object: %s\n", static_cast<const char *> ((*itr)->GUID.toAscii()));
		(*itr)->Handler->RemoveObject();
	}
}

ARObject * aride_project::InitialiseObject(ARObject * obj)
{
	for (deque<ARObject *>::const_iterator itr = ActiveObjects.begin(); itr != ActiveObjects.end(); ++itr)
			if (*itr == obj)
				return obj;
	dbg_print(ARDBG_INFO,"Initialising Object %p\n",obj);
	assert(obj);
	obj->Initialise();
	ActiveObjects.push_front(obj);

	return obj;
}

/// returns true if the name is valid (ie not already used)
bool aride_project::CheckName(const QString & Name)
{
	for (list<aride_object *>::iterator itr = Objects.begin(); itr != Objects.end(); ++itr)
		if ((*itr)->GetName() == Name)
			return false;
	for (list<aride_environment *>::iterator itr = Environments.begin(); itr != Environments.end(); ++itr)
		if ((*itr)->GetName() == Name)
			return false;
	for (list<aride_displaylist_node *>::iterator itr = DisplayNodes.begin(); itr != DisplayNodes.end(); ++itr)
		if ((*itr)->GetName() == Name)
			return false;
	return true;
}

/// generates a unique name for the object
QString aride_project::MakeUniqueName(const QString & Type)
{
	QString TestName;
	for (int i = 0; ; ++i)
	{
		TestName = Type + QString().setNum(i);
		if (CheckName(TestName))
			return TestName;
	}
	return "Bad Name";
}


/// returns true if the name is valid (ie not already used)
bool aride_project::CheckGUID(const QString & GUID)
{
	for (list<aride_object *>::iterator itr = Objects.begin(); itr != Objects.end(); ++itr)
		if ((*itr)->GUID == GUID)
			return false;
	for (list<aride_environment *>::iterator itr = Environments.begin(); itr != Environments.end(); ++itr)
		if ((*itr)->GUID == GUID)
			return false;
	for (list<aride_displaylist_node *>::iterator itr = DisplayNodes.begin(); itr != DisplayNodes.end(); ++itr)
		if ((*itr)->GUID == GUID)
			return false;
	return true;
}

/// generates a unique name for the object
QString aride_project::MakeUniqueGUID()
{
	QString TestGUID;
	for (int i = 0; ; ++i)
	{
		TestGUID = "GUID::" + QString().setNum(i);
		if (CheckGUID(TestGUID))
			return TestGUID;
	}
	return "Bad GUID";
}


void aride_project::Run()
{
	// create the render pairs
	for (list<aride_displaylist_node *>::iterator itr2 = DisplayNodes.begin(); itr2 != DisplayNodes.end(); ++itr2)
	{
		if ((*itr2)->object == NULL)
			continue;
		if ((*itr2)->object->Section == ARIDE_RENDER && !(*itr2)->object->Missing) // Don't add missing plugins
		{
			RenderPair * rp = CreateRenderPair(*itr2);
			if (rp)
				RenderPairs.push_back(rp);
		}
		// setup transform chains for all position objects
		if ((*itr2)->object->Section == ARIDE_POSITION && !(*itr2)->object->Missing) // Don't add missing plugins
		{
			dbg_print(ARDBG_VERBOSE,"Transform: %s\n",static_cast<const char *> ((*itr2)->object->GUID.toAscii()));
			CreateTransformChain(*itr2);
		}
	}
	
	for (list<aride_environment *>::iterator itr = Environments.begin(); itr != Environments.end(); ++itr)
	{
		(*itr)->Start();
	}
}

void aride_project::Pause()
{
	for (list<aride_environment *>::iterator itr = Environments.begin(); itr != Environments.end(); ++itr)
	{
		(*itr)->Pause();
	}
}

struct term : public unary_function<ARObject *, void>
{
  void operator() (ARObject * x) { x->Terminate(); }
};

void aride_project::Stop()
{
	for (list<aride_environment *>::iterator itr = Environments.begin(); itr != Environments.end(); ++itr)
	{
		(*itr)->Stop();
	}
	for_each(ActiveObjects.begin(), ActiveObjects.end(), term());
	ActiveObjects.clear();
	RemoveObjects(); // Detach objects from handlers.
	for (list<RenderPair *>::const_iterator itr = RenderPairs.begin(); itr != RenderPairs.end(); ++itr)
		delete *itr;
	RenderPairs.clear();
}


void aride_project::RefreshObjects()
{
	dbg_print(ARDBG_VERBOSE,"Project is refreshing all objects\n");
	for (list<aride_object *>::const_iterator itr = Objects.begin(); itr != Objects.end(); ++itr)
	{
		if ((*itr)->Handler)
			(*itr)->Handler->update();
	}
}
