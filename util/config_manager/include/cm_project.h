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

#ifndef ARIDE_PROJECT_H
#define ARIDE_PROJECT_H

#define CONFIG_FILE_VERSION "0.3"

#include <ardev/ardev.h>

#include <QDomNode>
#include <QDomElement>
#include <QDomDocument>

#include <stdlib.h>
#include <list>
#include <vector>
#include <deque>
using namespace std;

class QWidget;
class ObjectHandler;
class EnvironmentObjectHandler;

typedef enum ArideSection {ARIDE_UNKNOWN,ARIDE_CAMERA,ARIDE_CAPTURE,ARIDE_OUTPUT,ARIDE_SECONDARYOUTPUT,ARIDE_PREPROCESS,ARIDE_POSITION,ARIDE_RENDER,ARIDE_MISC,ARIDE_RENDERPAIR,ARIDE_ENVIRONMENTS,ARIDE_END} ArideSection;
const char * GetArideSectionTypeName(ArideSection Section);

class aride_object
{
	public:
		aride_object(const QDomElement & _Element, ObjectHandler * Handler=NULL);
		virtual ~aride_object();

		QString GUID;
		QString Parent;
		ArideSection Section; 	// ie camera, capture etc
		QString Type;		// ie CameraPlayer

		QDomElement Element;

		static int GetSection(const QString & TagName);

		void ChangeType(const QString & NewType);

		QString GetName() const;
		void SetName(const QString & Name);

		ObjectHandler * Handler;

		bool Missing; // plugins can be missing
};


class aride_project;

class aride_environment : public aride_object
{
	public:
		aride_environment(const QDomElement & _Element, aride_project* _Project);
		virtual ~aride_environment();

		EnvironmentObjectHandler * EnvHandler;

		QString Out;
		QString Cam;
		QString Cap;
		list<QString> PreProcessObjects;
		list<QString> SecondaryObjects;
		list<QString> MiscObjects;

		void Start();
		void Pause();
		void Stop();
		bool Paused;

		aride_project* Project;
};


class aride_displaylist_node
{
	public:
		aride_displaylist_node(const QDomElement & _Element, aride_displaylist_node * _Parent, aride_object * _Object);
		~aride_displaylist_node();

		aride_displaylist_node * Parent;

		QDomElement Element;
		aride_object * object;
		QString GUID;
		QString Name;

		QString GetName() {return Name;};
		void SetName(const QString & _Name) {Name = _Name;};
};


class aride_project
{
	public:
		aride_project();
		~aride_project();

		int LoadFromFile(const char * Filename);
		int SaveToFile(const char * Filename=NULL);
		int New();
		int Close();

		bool isOpen();
		bool isChanged();
		bool hasName();

		aride_object * CreateObjectNode(ArideSection Section, const QString & Type, aride_object * parent=NULL);
		int CreateEnvironmentNode();
		aride_displaylist_node * CreateDisplayList(QDomNode &n);
		aride_displaylist_node * CreateDisplayList();
		aride_displaylist_node * CreateDisplayListNode(aride_displaylist_node * parent, QDomNode &n);
		aride_displaylist_node * CreateDisplayListNode(const ArideSection Section, const QString & Type,const QString & Parent);
		void RemoveDisplayListNode(QString Name);
		void RemoveDisplayListNode(aride_displaylist_node * node);
		int RemoveObject(aride_object * obj);
		int RemoveEnvironment(aride_environment * env);

		RenderPair * CreateRenderPair(aride_displaylist_node * node);
		void CreateTransformChain(aride_displaylist_node * node);

		void Run();
		void Pause();
		void Stop();

		QString GetGUIDByName(const QString & GUID);
		aride_object * GetObjectByGUID(const QString & GUID);
		aride_environment * GetEnvByGUID(const QString & GUID);
		aride_displaylist_node * GetDisplayNodeByGUID(const QString & GUID);
		aride_object * GetFirstChild(const aride_object * obj);

		void RemoveObjects();
		ARObject * InitialiseObject(ARObject *);
		bool CheckName(const QString & Name); ///< returns true if the name is valid (ie not already used)
		QString MakeUniqueName(const QString & Type); /// < generates a unique naem for the object

		bool CheckGUID(const QString & GUID); ///< returns true if the GUID is unique
		QString MakeUniqueGUID(); /// < generates a unique GUID for the object

		void RefreshObjects();

		list<aride_object *> Objects;
		list<aride_environment *> Environments;
		list<aride_displaylist_node *> DisplayNodes;
		list<RenderPair *> RenderPairs;
	protected:
		deque<ARObject *> ActiveObjects;

		QDomDocument doc;
		char * StoredFilename;
		bool Named; // Whether the user has selected a name
		bool Changed;
		bool Open;
};

extern aride_project * CurrentProject;

#endif
