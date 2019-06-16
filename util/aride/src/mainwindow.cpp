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
#include "mainwindow.h"
#include <ardev/debug.h>
#include <ardev/exception.h>
#include <assert.h>

#include <cm_objecthandler.h>
#include <cm_project.h>

#include "ConfigureEnvironments.h"
#include "ConfigureDisplaylist.h"

#include <QFileDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QGridLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QLayout>
#include <QMenu>
#include <QCursor>
#include <QAction>
#include <QPushButton>
#include <QMessageBox>
#include <QCloseEvent>

#include <list>
using namespace std;



MainWindow::MainWindow(QWidget* parent, const char* name, Qt::WFlags fl) : Ui_MainWindow()
{
	setupUi(this);
	Stopping = false;
	CurrentItem = NULL;
	DisplaylistDialog = NULL;
	EnvironmentsDialog = NULL;

	// Connect signals and slots
	QObject::connect(listObjects, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(ObjectClicked(QTreeWidgetItem*)));
	QObject::connect(listObjects, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(ObjectDoubleClicked(QTreeWidgetItem*)));
	QObject::connect(listObjects, SIGNAL(itemRightClicked(QTreeWidgetItem*,int)), this, SLOT(ObjectRightClicked(QTreeWidgetItem*)));
	QObject::connect(listObjects, SIGNAL(itemRenamed(QTreeWidgetItem*,int, const QString &)), this, SLOT(ObjectRenamed(QTreeWidgetItem*,int,const QString &)));

	QObject::connect(actionConfigure_Environments, SIGNAL(triggered()), this, SLOT(configureEnvironments()));
	QObject::connect(actionConfigure_Display_Sets, SIGNAL(triggered()), this, SLOT(configureDataSets()));

	QObject::connect(action_New, SIGNAL(triggered()), this, SLOT(fileNew()));
	QObject::connect(actionOpen, SIGNAL(triggered()), this, SLOT(fileOpen()));
	QObject::connect(actionSave, SIGNAL(triggered()), this, SLOT(fileSave()));
	QObject::connect(actionSave_As, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
	QObject::connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));

	QObject::connect(actionRun, SIGNAL(triggered()), this, SLOT(controlRun()));
	QObject::connect(actionStop, SIGNAL(triggered()), this, SLOT(controlStop()));

}


void MainWindow::Initialise(int argc, char ** argv)
{
	char * Filename = NULL;

	// if project file specified on command line load it
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i],"-d")==0) // check if debug level specification
			++i; // Skip the next parameter (debug level)
		else
			Filename = argv[i];
	}
	if (Filename)
	{
		try
		{
			CurrentProject.LoadFromFile(Filename);
			RefreshTrees();
		}
		catch(aride_exception e)
		{
			e.Print();
			//dbg_print(ARDBG_ERR,"Error loading file, error = %d:%s\n",e.Type,ExceptionString[e.Type]);
		}
	}
	else
	{
		CurrentProject.New();
		RefreshTrees();
	}
}

void MainWindow::fileNew()
{
	CurrentProject.New();
	RefreshTrees();
}

void MainWindow::fileOpen()
{
	QString Filename = QFileDialog::getOpenFileName(NULL,tr("Open ARIDE Project"),QString("./"),QString("aride project file (*.aride)"));
	try
	{
		QByteArray asciiFile = Filename.toAscii(); // Careful of lifetime
		CurrentProject.LoadFromFile(asciiFile);
	}
	catch (aride_exception & e)
	{
		QMessageBox::warning(this, "aride",ExceptionString[e.Type]);
	}
	RefreshTrees();
}

void MainWindow::fileSave()
{
	CurrentProject.SaveToFile();
}

void MainWindow::fileSaveAs()
{
	QString Filename = QFileDialog::getSaveFileName(NULL,"./","aride project file (*.aride)");
	if (Filename != "") // Save cancelled.
	{
		QByteArray asciiFile = Filename.toAscii(); // Careful of lifetime
		CurrentProject.SaveToFile(asciiFile);
	}
}

void MainWindow::RefreshTrees()
{
	if (!CurrentProject.isOpen())
		return;

	listObjects->clear();

	// load environments
	// this is very inefficient, but I can fix that once the GUI stabalises
	listObjects->setColumnCount(2);
	for (list<aride_displaylist_node *>::iterator itr = CurrentProject.DisplayNodes.begin(); itr != CurrentProject.DisplayNodes.end(); ++itr)
	{
		if ((*itr)->Parent == NULL)
		{
//			printf("Got a parent node: %s\n",(*itr)->Name.ascii());
			QTreeWidgetItem * DisplayList = new QTreeWidgetItem(listObjects,QStringList((*itr)->Name));
			for (list<aride_displaylist_node *>::iterator itr2 = CurrentProject.DisplayNodes.begin(); itr2 != CurrentProject.DisplayNodes.end(); ++itr2)
			{
				if ((*itr2)->object == NULL)
					continue;
//				printf("check object, Type=: %d, parent = %p\n", (*itr2)->object->Section, (*itr2)->Parent);
				if ((*itr2)->object->Section == ARIDE_RENDER && (*itr2)->Parent != NULL)
				{
//					printf("Get a render node to add to a display list\n");
					aride_displaylist_node * grandparent = (*itr2);
					while (grandparent->Parent)
						grandparent = grandparent->Parent;
					if (grandparent == (*itr))
					{
						// plugins may not have handlers at this time
						if(!((*itr2)->object->Handler))
						{
							//
						}
						assert((*itr2)->object->Handler);
						aride_object * obj = CurrentProject.GetObjectByGUID((*itr2)->object->GUID);
						assert(obj);
						QTreeWidgetItem * TempItem = new QTreeWidgetItem(DisplayList,QStringList(obj->GetName()));
						TempItem->setData(0,Qt::UserRole,(*itr2)->object->GUID);
						if ((*itr2)->object->Handler->Enabled.Value)
						{
							TempItem->setText(1,"*");
						}
					}
				}
			}
			listObjects->setItemExpanded(DisplayList,true);
		}
	}
}

void MainWindow::ObjectDoubleClicked(QTreeWidgetItem * Item)
{
	if (Item == NULL)
		return;

	aride_object * obj = CurrentProject.GetObjectByGUID(Item->data(0,Qt::UserRole).toString());
	if (obj)
	{
		nonSlotObjectDoubleClicked(obj);
		RefreshTrees();
		return;
	}

	aride_displaylist_node * node = CurrentProject.GetDisplayNodeByGUID(Item->data(0,Qt::UserRole).toString());
	if (node)
	{
		nonSlotObjectDoubleClicked(node);
		RefreshTrees();
		return;
	}
}

void MainWindow::nonSlotObjectDoubleClicked(aride_object * obj)
{
	assert(obj);
	if (obj->Section != ARIDE_RENDER)
		return;
	assert(obj->Handler);
	bool Current = obj->Handler->Enabled.Value;
	obj->Handler->Enabled.fromString(Current ? "False" : "True");
	obj->Handler->SetEnabled(!Current);
}

void MainWindow::nonSlotObjectDoubleClicked(aride_displaylist_node * node)
{
	assert(node);
	aride_object * obj = CurrentProject.GetObjectByGUID(node->GUID);
	if (obj)
		nonSlotObjectDoubleClicked(obj);

	for (list<aride_displaylist_node *>::const_iterator itr = CurrentProject.DisplayNodes.begin(); itr != CurrentProject.DisplayNodes.end(); ++itr)
		if((*itr)->Parent == node)
			nonSlotObjectDoubleClicked(*itr);
}

void MainWindow::ObjectRightClicked(QTreeWidgetItem * Item)
{
	// disabled for now since this should be done through the other config windows
	// eventualyl a subset could be added back here?
	return;

	if (Item == NULL)
		return;

	QMenu * Menu = new QMenu();

	delete Menu;
}

void MainWindow::AddObject(int id)
{
	int Counter = ARIDE_END;
	for(ObjectHandlerRegistryMap::iterator itr = ObjectHandlerRegistry.begin(); itr != ObjectHandlerRegistry.end(); ++itr)
	{
		Counter++;
		if (Counter == id)
		{
			dbg_print(ARDBG_INFO,"Adding Object, Type=%s\n",static_cast<const char*> (QString(itr->first).toAscii()));
			CurrentProject.CreateObjectNode(itr->second.Section,QString(itr->first));
			return;
		}
	}
}

void MainWindow::AddEnvironment(int id)
{
	CurrentProject.CreateEnvironmentNode();
}

void MainWindow::RemoveObject(int id)
{

	if (CurrentItem == NULL)
		return;
	if (CurrentItem->type() == ARIDE_LIST_VIEW_ITEM_ENVIRONMENT)
	{
		CurrentProject.RemoveEnvironment(reinterpret_cast<ARListViewItem_Environment *> (CurrentItem)->object);
	}
	else if (CurrentItem->type() == ARIDE_LIST_VIEW_ITEM_OBJECT)
	{
		CurrentProject.RemoveObject(reinterpret_cast<ARListViewItem_Object *> (CurrentItem)->object);
	}
}

void MainWindow::AddSubObject(int id)
{
	int Counter = ARIDE_END;
	for(ObjectHandlerRegistryMap::iterator itr = ObjectHandlerRegistry.begin(); itr != ObjectHandlerRegistry.end(); ++itr)
	{
		Counter++;
		if (Counter == id)
		{
			dbg_print(ARDBG_INFO,"Adding Object, Type=%s\n",static_cast<const char *> (QString(itr->first).toAscii()));
			CurrentProject.CreateObjectNode(itr->second.Section,QString(itr->first),reinterpret_cast<ARListViewItem_Object *> (CurrentItem)->object);
			return;
		}
	}
}


void MainWindow::controlRunPauseAction_toggled(bool State)
{
	if (Stopping)
		return;

	if (State)
		CurrentProject.Run();
	else
		CurrentProject.Pause();
}

void MainWindow::controlStop()
{
	Stopping = true;

	CurrentProject.Stop();

	Stopping = false;
}

void MainWindow::controlRun()
{
	CurrentProject.Run();
}

void MainWindow::configureEnvironments()
{
	ConfigureEnvironments Dialog(0);
	EnvironmentsDialog = &Dialog;
	Dialog.exec();
	EnvironmentsDialog = NULL;
}

void MainWindow::configureDataSets()
{
	ConfigureDisplaylist Dialog(0);
	DisplaylistDialog = &Dialog;
	Dialog.exec();
	DisplaylistDialog = NULL;
	RefreshTrees();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	if (!CurrentProject.isChanged())
		return; // Project has no changes, close without prompt.

	int result = QMessageBox::information( this, "aride",
		"Do you want to save before exiting?",
		"&Save", "&Don't Save", "&Cancel",
		0, // Enter = button 0
		2); // Esc = button 2
	switch(result)
	{
	case 0: // save
		if (!CurrentProject.hasName()) // If no name specified yet, ask user
		{
			QString filename = QFileDialog::getSaveFileName(NULL,"./","aride project file (*.aride)");
			if (filename == "")
				event->ignore(); // If save-as dialog canceled, don't close program
			else
			{
				QByteArray asciiFile = filename.toAscii(); // Careful of lifetime
				CurrentProject.SaveToFile(asciiFile);
			}
		}
		else
			CurrentProject.SaveToFile();
		break;
	case 1: // don't save
		break;
	case 2: // cancel
		event->ignore();
		break;
	}
}
