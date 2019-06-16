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
 
#include "ConfigureDisplaylist.h"
#include "mainwindow.h"
#include "cm_project.h"
#include "cm_parameter.h"
#include "cm_objecthandler.h"
#include "cm_registry.h"
#include <ardev/debug.h>

#include <assert.h>

#include <QComboBox>
#include <QTreeWidget>
#include <QFileDialog>
#include <QListView>
#include <QTextEdit>
#include <QLabel>
#include <QSizePolicy>
#include <QScrollArea>
#include <QLayout>
#include <QGridLayout>
#include <QMenu>
#include <QCursor>
#include <QAction>
#include <QPushButton>
#include <QTabWidget>

ConfigureDisplaylist::ConfigureDisplaylist(QWidget* parent, const char* name, Qt::WFlags fl)
{
	// Load the dialog UI
	setupUi(this);
	frameRenderItem->setWidgetResizable( true );

	// set up connections
	QObject::connect(listRenderItems, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onObjectClick(QTreeWidgetItem*)));
	QObject::connect(listRenderItems, SIGNAL(customContextMenuRequested ( const QPoint &)), this, SLOT(onObjectRightClick(const QPoint &)));
	
	// the rest of the constructor	
	ActiveItem = NULL;
	Details = NULL;
	
	RefreshDisplayLists();

}

ConfigureDisplaylist::~ConfigureDisplaylist()
{
}

void ConfigureDisplaylist::RefreshDisplayLists()
{
	dbg_print(ARDBG_VERBOSE,"Refreshing Display List\n");
	listRenderItems->clear();
	for (list<aride_displaylist_node *>::iterator itr = CurrentProject->DisplayNodes.begin(); itr != CurrentProject->DisplayNodes.end(); ++itr)
	{
		if ((*itr)->Parent == NULL)
		{
			QTreeWidgetItem * DisplayListItem = new QTreeWidgetItem(listRenderItems,QStringList((*itr)->Name));
			DisplayListItem->setData(0,Qt::UserRole,(*itr)->GUID);
			BuildDisplayList(DisplayListItem,*itr);
			listRenderItems->setItemExpanded(DisplayListItem,true);
		}
	}
}


void ConfigureDisplaylist::BuildDisplayList(QTreeWidgetItem* ParentItem, aride_displaylist_node * ParentNode)
{
	dbg_print(ARDBG_VERBOSE,"Build Display List\n");

 	for (list<aride_displaylist_node *>::iterator itr = CurrentProject->DisplayNodes.begin(); itr != CurrentProject->DisplayNodes.end(); ++itr)
	{
		if ((*itr)->Parent == ParentNode)
		{
			dbg_print(ARDBG_VERBOSE,"Adding new Item %s\n",static_cast<const char *> ((*itr)->GUID.toAscii()));	
			aride_object * obj = CurrentProject->GetObjectByGUID((*itr)->GUID);
			assert(obj);
			QTreeWidgetItem * DisplayListItem = new QTreeWidgetItem(ParentItem,QStringList(obj->GetName()));
			DisplayListItem->setData(0,Qt::UserRole,obj->GUID);
			BuildDisplayList(DisplayListItem,*itr);
			DisplayListItem->treeWidget()->setItemExpanded(DisplayListItem,true);
		}
	}
}

void ConfigureDisplaylist::onObjectClick(QTreeWidgetItem * item)
{
	if (item == NULL)
		return;
	dbg_print(ARDBG_VERBOSE,"Getting object that was clicked on\n");
	aride_object * Object = CurrentProject->GetObjectByGUID(item->data(0,Qt::UserRole).toString());
	dbg_print(ARDBG_VERBOSE,"Got object that was clicked on\n");

	delete Details;
	Details = new QWidget();
	if (Details == NULL)
		throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	Details->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	dbg_print(ARDBG_VERBOSE,"Created Details\n");
	frameRenderItem->show();
	frameRenderItem->setWidget(Details);
	Details->show();
	dbg_print(ARDBG_VERBOSE,"Displayed details Details\n");

	QGridLayout * templ = new QGridLayout;
	delete Details->layout();
	Details->setLayout(templ);
	dbg_print(ARDBG_VERBOSE,"Set details layout\n");

	if (Object==NULL)
	{
		return;
	}
	
	// display header
	QLabel * temp = new QLabel("Object Type");
	temp->show();
	templ->addWidget(temp,0,0);
	temp = new QLabel(Object->Type);
	temp->show();
	templ->addWidget(temp,0,1,1,-1);

	// Check if we have a handler
	if(Object->Handler == NULL)
	{
		temp = new QLabel("No Object Handler Available for class",Details);
		temp->show();
		templ->addWidget(temp,1,0,3,0);
		return;
	}

	dbg_print(ARDBG_VERBOSE,"Load the parameters\n");
	// display each parameter
	int i =1;
	for (list<Parameter *>::iterator itr = Object->Handler->Parameters.begin(); itr != Object->Handler->Parameters.end(); ++itr, ++i)
	{
		dbg_print(ARDBG_VERBOSE,"Load paramter %d\n", i);
		temp = new QLabel((*itr)->Name);
		dbg_print(ARDBG_VERBOSE,"Have set name so now show %d\n", i);
		temp->show();
		dbg_print(ARDBG_VERBOSE,"Have set name so now add %d\n", i);
		templ->addWidget(temp,i,0);
		temp = new QLabel((*itr)->Type);
		dbg_print(ARDBG_VERBOSE,"Show paramter widget %d\n", i);
		temp->show();
		dbg_print(ARDBG_VERBOSE,"add paramter widget %d\n", i);
		templ->addWidget(temp,i,1);
		QWidget * new_widget = (*itr)->CreateTypeWidget();
		QObject::connect(*itr, SIGNAL(requestUpdate()), this, SLOT(RefreshDisplayLists()));
		new_widget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
		new_widget->show();
		templ->addWidget(new_widget,i,2);
	}	
	templ->setRowStretch(i,1);	
}


void ConfigureDisplaylist::onObjectRightClick(const QPoint & p)
{
	QTreeWidgetItem * item = listRenderItems->itemAt(p);
	aride_object * Object;
	if (item)
	{
		ActiveItem = item;
		Object = CurrentProject->GetObjectByGUID(item->data(0,Qt::UserRole).toString());
	}
	else
	{
		ActiveItem = NULL;
		Object = NULL;	
	}

	
	QMenu * Menu = new QMenu();
	// we clicked on the root of a display set
	if (item != NULL && Object == NULL)
		Menu->addAction("Remove Display List", this, SLOT(onRemoveDisplayList()));
	else if (item != NULL && Object != NULL)
		Menu->addAction("Remove Object", this, SLOT(onRemoveObject()));

	Menu->addAction("Add Display List", this, SLOT(onAddDisplayList()));

	
	QMenu * MiscMenu = NULL;
	QMenu * RenderMenu = NULL;
	QMenu * PositionMenu = NULL;

	if (item != NULL && Object == NULL)
		MiscMenu = new QMenu("Add Misc Object");	
	if (item != NULL && Object != NULL && Object->Section == ARIDE_POSITION)
		RenderMenu = new QMenu("Add Render Object");
	if (item != NULL && (Object==NULL || Object->Section == ARIDE_POSITION))
		PositionMenu = new QMenu("Add Position Object");

	int Counter = ARIDE_END;
	for(ObjectHandlerRegistryMap::iterator itr = ObjectHandlerRegistry.begin(); itr != ObjectHandlerRegistry.end(); ++itr)
	{
		Counter++;
		QMenu * target;
		switch (itr->second.Section)
		{
			case ARIDE_POSITION:
				target = PositionMenu;
				break;
			case ARIDE_RENDER:
				target = RenderMenu;
				break;
			case ARIDE_MISC:
				target = MiscMenu;
				break;
			default:
				continue;
		}
		if (target)
			QObject::connect(target->addAction(itr->first), SIGNAL(triggered(QAction*)), this, SLOT(onAddObject(QAction*)));
	}
		
	if (PositionMenu)
		Menu->addMenu(PositionMenu);
	if(RenderMenu)
		Menu->addMenu(RenderMenu);
	if(MiscMenu)
		Menu->addMenu(MiscMenu);
	
	QAction * Selected = Menu->exec(QCursor::pos());
	if (ActiveItem)
		onAddObject(Selected);
	ActiveItem = NULL;
	
	delete Menu;
}

void ConfigureDisplaylist::onAddDisplayList()
{
	aride_displaylist_node * NewList = CurrentProject->CreateDisplayList();
	assert(NewList != NULL);
	RefreshDisplayLists();
	QTreeWidgetItem * Newitem = listRenderItems->findItems(NewList->GetName(),Qt::MatchExactly|Qt::MatchCaseSensitive|Qt::MatchWrap).front();
	listRenderItems->setCurrentItem(Newitem);
	onObjectClick(Newitem);
}

void ConfigureDisplaylist::onAddObject(QAction * Action)
{
	assert(ActiveItem);
	if (!Action)
		return;
	assert(Action);
	for(ObjectHandlerRegistryMap::iterator itr = ObjectHandlerRegistry.begin(); itr != ObjectHandlerRegistry.end(); ++itr)
	{
		if (itr->first == Action->text())
		{
			dbg_print(ARDBG_INFO,"Adding Object, Type=%s\n",static_cast<const char *> (QString(itr->first).toAscii()));
			aride_displaylist_node * NewObject = CurrentProject->CreateDisplayListNode(itr->second.Section,QString(itr->first),ActiveItem->data(0,Qt::UserRole).toString());
			dbg_print(ARDBG_INFO,"Adding Object, Name=%s\n",static_cast<const char *> (NewObject->GUID.toAscii()));
			RefreshDisplayLists();
			dbg_print(ARDBG_VERBOSE,"Find new Item so we can set it as active\n");
			QList<QTreeWidgetItem *> Newitems = listRenderItems->findItems(NewObject->GetName(),Qt::MatchExactly|Qt::MatchCaseSensitive|Qt::MatchWrap);
			if (!Newitems.isEmpty())
			{
				dbg_print(ARDBG_VERBOSE,"Set new Item active\n");
				listRenderItems->setCurrentItem(Newitems.front());
				onObjectClick(Newitems.front());
			}
			return;
		}
	}
}

void ConfigureDisplaylist::onRemoveDisplayList()
{
	assert(ActiveItem);
	CurrentProject->RemoveDisplayListNode(ActiveItem->data(0,Qt::UserRole).toString());
	RefreshDisplayLists();
}

void ConfigureDisplaylist::onRemoveObject()
{
	assert(ActiveItem);
	CurrentProject->RemoveDisplayListNode(ActiveItem->data(0,Qt::UserRole).toString());
	RefreshDisplayLists();	
}
