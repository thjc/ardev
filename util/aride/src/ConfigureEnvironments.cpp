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
 
 
#include "ConfigureEnvironments.h"
#include "mainwindow.h"
#include "cm_registry.h"
#include "cm_project.h"
#include "cm_objecthandler.h"
#include <ardev/debug.h>

#include <assert.h>

#include <QComboBox>
#include <QListWidget>
#include <QFileDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QMenu>
#include <QCursor>
#include <QAction>
#include <QPushButton>
#include <QTabWidget>
#include <QScrollArea>

ConfigureEnvironments::ConfigureEnvironments(QWidget* parent, const char* name, Qt::WFlags fl) 
{
	// load the dialog
	setupUi(this);

	// setup the connections
	QObject::connect(listEnvironments, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onEnvironmentClick(QListWidgetItem*)));

	QObject::connect(listPreProcess, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onPreProcessClick(QListWidgetItem*)));
	QObject::connect(listSecondary, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onSecondaryClick(QListWidgetItem*)));
	QObject::connect(listMisc, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onMiscClick(QListWidgetItem*)));

	QObject::connect(OutputType, SIGNAL(activated(const QString &)), this, SLOT(onOutputClick(const QString &)));
	QObject::connect(CameraType, SIGNAL(activated(const QString &)), this, SLOT(onCameraClick(const QString &)));
	QObject::connect(CaptureType, SIGNAL(activated(const QString &)), this, SLOT(onCaptureClick(const QString &)));

	QObject::connect(buttonAddPreProcess, SIGNAL(clicked(bool)), this, SLOT(onAddPreProcess()));
	QObject::connect(buttonAddMisc, SIGNAL(clicked(bool)), this, SLOT(onAddMisc()));
	QObject::connect(buttonAddSecondary, SIGNAL(clicked(bool)), this, SLOT(onAddSecondary()));

	QObject::connect(listEnvironments, SIGNAL(customContextMenuRequested ( const QPoint &)), this, SLOT(onEnvListRightClick(const QPoint &)));
	QObject::connect(listPreProcess, SIGNAL(customContextMenuRequested ( const QPoint &)), this, SLOT(onPreProcessListRightClick(const QPoint &)));
	QObject::connect(listSecondary, SIGNAL(customContextMenuRequested ( const QPoint &)), this, SLOT(onSecondaryListRightClick(const QPoint &)));
	QObject::connect(listMisc, SIGNAL(customContextMenuRequested ( const QPoint &)), this, SLOT(onMiscListRightClick(const QPoint &)));

	

	// rest of the constructor
	ActiveItem = NULL;
	
	OutputDetails = NULL;
	frameOutput->setWidgetResizable( true );

	PreProcessDetails = NULL;
	framePreProcess->setWidgetResizable( true );

	CaptureDetails = NULL;
	frameCapture->setWidgetResizable( true );

	CameraDetails = NULL;
	frameCamera->setWidgetResizable( true );
	
	SecondaryDetails = NULL;
	frameSecondary->setWidgetResizable( true );
	
	MiscDetails = NULL;
	frameMisc->setWidgetResizable( true );

	
	// now fill in the details of the dialog
	for(ObjectHandlerRegistryMap::const_iterator itr = ObjectHandlerRegistry.begin(); itr != ObjectHandlerRegistry.end(); ++itr)
	{
		switch (itr->second.Section)
		{
			case ARIDE_CAMERA:
				CameraType->addItem(itr->first);
				break;
			case ARIDE_CAPTURE:
				CaptureType->addItem(itr->first);
				break;
			case ARIDE_OUTPUT:
				OutputType->addItem(itr->first);
				break;
			case ARIDE_SECONDARYOUTPUT:
				SecondaryType->addItem(itr->first);
				break;
			case ARIDE_PREPROCESS:
				PreProcessType->addItem(itr->first);
				break;
			case ARIDE_MISC:
				MiscType->addItem(itr->first);
				break;
			default:
				break;
		}
	}	
	RefreshEnvironmentList();
	
}

ConfigureEnvironments::~ConfigureEnvironments()
{
}

void ConfigureEnvironments::RefreshEnvironmentList()
{
	QString Selected;
	QListWidgetItem * temp = listEnvironments->currentItem();
	if (temp)
		Selected = temp->text();
	
	// load environments
	listEnvironments->clear();
	for (list<aride_environment *>::iterator itr = CurrentProject->Environments.begin(); itr != CurrentProject->Environments.end(); ++itr)
	{
		listEnvironments->addItem((*itr)->GetName());
		listEnvironments->item(listEnvironments->count()-1)->setData(Qt::UserRole,(*itr)->GUID);
	}			
	QList<QListWidgetItem *> Result;
	Result = listEnvironments->findItems(Selected,Qt::MatchExactly|Qt::MatchCaseSensitive|Qt::MatchWrap);
	if (!Result.isEmpty())
		listEnvironments->setCurrentItem(Result.front());
	else if (listEnvironments->count() > 0)
		listEnvironments->setCurrentItem(0);
	onEnvironmentClick(listEnvironments->currentItem());
}

void ConfigureEnvironments::RefreshObjects()
{

	dbg_print(ARDBG_VERBOSE,"Refreshing environment objects\n");
	aride_environment * env = CurrentProject->GetEnvByGUID(listEnvironments->currentItem()->data(Qt::UserRole).toString());

	// then the mutiple object bits
	listPreProcess->clear();
	listMisc->clear();
	listSecondary->clear();
	delete MiscDetails ;
	MiscDetails = NULL;
	delete PreProcessDetails;
	PreProcessDetails = NULL;
	delete SecondaryDetails;
	SecondaryDetails = NULL;
	
	for (list<QString>::const_iterator itr= env->PreProcessObjects.begin(); itr != env->PreProcessObjects.end(); ++itr)
	{
		aride_object * obj = CurrentProject->GetObjectByGUID(*itr);
		assert(obj);
		listPreProcess->addItem(obj->GetName());
		listPreProcess->item(listPreProcess->count()-1)->setData(Qt::UserRole,obj->GUID);
	}
	if (listPreProcess->count() > 0)
	{
		listPreProcess->setCurrentItem(0);
		onPreProcessClick(listPreProcess->currentItem());
	}
	for (list<QString>::const_iterator itr = env->MiscObjects.begin(); itr != env->MiscObjects.end(); ++itr)
	{
		aride_object * obj = CurrentProject->GetObjectByGUID(*itr);
		assert(obj);
		listMisc->addItem(obj->GetName());
		listMisc->item(listMisc->count()-1)->setData(Qt::UserRole,obj->GUID);
	}
	if (listMisc->count() > 0)
	{
		listMisc->setCurrentItem(0);
		onMiscClick(listMisc->currentItem());
	}
	for (list<QString>::const_iterator itr = env->SecondaryObjects.begin(); itr != env->SecondaryObjects.end(); ++itr)
	{
		aride_object * obj = CurrentProject->GetObjectByGUID(*itr);
		assert(obj);
		listSecondary->addItem(obj->GetName());
		listSecondary->item(listSecondary->count()-1)->setData(Qt::UserRole,obj->GUID);
	}
	if (listSecondary->count() > 0)
	{
		listSecondary->setCurrentItem(0);
		onSecondaryClick(listSecondary->currentItem());
	}

	CurrentProject->RefreshObjects();

}

void ConfigureEnvironments::onEnvironmentClick(QListWidgetItem * item)
{
	if (item==NULL)
	{
		// no environment selected we shoudl disable the tabs, or possibly hide them
		envDetailTabs->hide();
		return;
	}
	else
	{
		envDetailTabs->show();		
	}
	
	QString GUID = item->data(Qt::UserRole).toString();
	aride_environment * env = CurrentProject->GetEnvByGUID(GUID);;
	assert(env);
	assert(env->EnvHandler);
	
	// now load up the objects the environment includes

	// Start with the Output object
	aride_object * out_object = CurrentProject->GetObjectByGUID(env->Out);
	if(out_object)
	{
		OutputType->setItemText(OutputType->currentIndex(),out_object->Type);
	}
	else
	{
		dbg_print(ARDBG_WARN,"Output object did not exist creating a new one of type %s\n",static_cast<const char *> (OutputType->currentText().toAscii()));
		env->Out = CurrentProject->CreateObjectNode(ARIDE_OUTPUT,OutputType->currentText())->GUID;
		dbg_print(ARDBG_WARN,"New output object GUID %s\n",static_cast<const char *> (env->Out.toAscii()));
	}
	onOutputClick(OutputType->currentText());
	
	// then the Capture object
	aride_object * cap_object = CurrentProject->GetObjectByGUID(env->Cap);
	if(cap_object)
		CaptureType->setItemText(CaptureType->currentIndex(),cap_object->Type);
	else
	{
		dbg_print(ARDBG_WARN,"Capture object did not exist creating a new one of type %s\n",static_cast<const char *> (CaptureType->currentText().toAscii()));
		env->Cap = CurrentProject->CreateObjectNode(ARIDE_CAPTURE,CaptureType->currentText())->GUID;
	}
	onCaptureClick(CaptureType->currentText());

	// then the Camera object
	aride_object * cam_object = CurrentProject->GetObjectByGUID(env->Cam);
	if(cam_object)
		CameraType->setItemText(CameraType->currentIndex(),cam_object->Type);
	else
	{
		dbg_print(ARDBG_WARN,"Camera object did not exist creating a new one of type %s\n",static_cast<const char *> (CameraType->currentText().toAscii()));
		env->Cam = CurrentProject->CreateObjectNode(ARIDE_CAMERA,CameraType->currentText())->GUID;
	}
	onCameraClick(CameraType->currentText());

	RefreshObjects();


}

void ConfigureEnvironments::onMiscClick(QListWidgetItem * item)
{
	if (!item)
		return;
	aride_object * object = CurrentProject->GetObjectByGUID(item->data(Qt::UserRole).toString());
	onObjectClick(&MiscDetails,frameMisc,object);
}

void ConfigureEnvironments::onPreProcessClick(QListWidgetItem * item)
{
	if (!item)
		return;
	aride_object * object = CurrentProject->GetObjectByGUID(item->data(Qt::UserRole).toString());
	onObjectClick(&PreProcessDetails,framePreProcess,object);	
}

void ConfigureEnvironments::onSecondaryClick(QListWidgetItem * item)
{
	if (!item)
		return;
	aride_object * object = CurrentProject->GetObjectByGUID(item->data(Qt::UserRole).toString());
	onObjectClick(&SecondaryDetails,frameSecondary,object);	
}

void ConfigureEnvironments::onOutputClick(const QString & type)
{
	aride_environment * env = CurrentProject->GetEnvByGUID(listEnvironments->currentItem()->data(Qt::UserRole).toString());
	assert(env);
	aride_object * obj = CurrentProject->GetObjectByGUID(env->Out);
	if (obj)
	{
		obj->ChangeType(type);
		onObjectClick(&OutputDetails,frameOutput,obj);
		RefreshObjects();
	}
}

void ConfigureEnvironments::onCameraClick(const QString & type)
{
	aride_environment * env = CurrentProject->GetEnvByGUID(listEnvironments->currentItem()->data(Qt::UserRole).toString());
	assert(env);
	aride_object * obj = CurrentProject->GetObjectByGUID(env->Cam);
	if (obj)
	{
		obj->ChangeType(type);
		onObjectClick(&CameraDetails,frameCamera,obj);
		RefreshObjects();
	}
}

void ConfigureEnvironments::onCaptureClick(const QString & type)
{
	aride_environment * env = CurrentProject->GetEnvByGUID(listEnvironments->currentItem()->data(Qt::UserRole).toString());
	assert(env);
	aride_object * obj = CurrentProject->GetObjectByGUID(env->Cap);
	if (obj)
	{
		obj->ChangeType(type);
		onObjectClick(&CaptureDetails,frameCapture,obj);
		RefreshObjects();
	}
}

void ConfigureEnvironments::onAddMisc()
{
	aride_environment * env = CurrentProject->GetEnvByGUID(listEnvironments->currentItem()->data(Qt::UserRole).toString());
	aride_object * NewName = CurrentProject->CreateObjectNode(ARIDE_MISC,MiscType->currentText());
	if (NewName != NULL)
		env->MiscObjects.push_back(NewName->GUID);
	listMisc->addItem(NewName->GetName());
	listMisc->item(listMisc->count()-1)->setData(Qt::UserRole,NewName->GUID);
	listMisc->setCurrentItem(listMisc->item(listMisc->count()-1));
	RefreshObjects();	
}

void ConfigureEnvironments::onAddPreProcess()
{
	aride_environment * env = CurrentProject->GetEnvByGUID(listEnvironments->currentItem()->data(Qt::UserRole).toString());
	aride_object * NewName = CurrentProject->CreateObjectNode(ARIDE_PREPROCESS,PreProcessType->currentText());
	if (NewName != NULL)
		env->PreProcessObjects.push_back(NewName->GUID);
	listPreProcess->addItem(NewName->GetName());
	listPreProcess->item(listPreProcess->count()-1)->setData(Qt::UserRole,NewName->GUID);
	listPreProcess->setCurrentItem(listPreProcess->item(listPreProcess->count()-1));
	RefreshObjects();
}

void ConfigureEnvironments::onAddSecondary()
{
	aride_environment * env = CurrentProject->GetEnvByGUID(listEnvironments->currentItem()->data(Qt::UserRole).toString());
	aride_object * NewName = CurrentProject->CreateObjectNode(ARIDE_SECONDARYOUTPUT,SecondaryType->currentText());
	if (NewName != NULL)
		env->SecondaryObjects.push_back(NewName->GUID);
	listSecondary->addItem(NewName->GetName());
	listSecondary->item(listSecondary->count()-1)->setData(Qt::UserRole,NewName->GUID);
	listSecondary->setCurrentItem(listSecondary->item(listSecondary->count()-1));
	RefreshObjects();
}

void ConfigureEnvironments::onEnvListRightClick(const QPoint & point)
{
	QMenu * Menu = new QMenu();
	ActiveItem = listEnvironments->itemAt(point);
	if (ActiveItem!=NULL)
		Menu->addAction(QString("Remove Environment"), this, SLOT(onRemoveEnvironment()));
	Menu->addAction("Add Environment", this, SLOT(onAddEnvironment()));
	Menu->exec(QCursor::pos());
	ActiveItem = NULL;
	delete Menu;	
}

void ConfigureEnvironments::onOtherListRightClick(QListWidgetItem * item, const QPoint & point)
{
	QMenu * Menu = new QMenu();
	if (item!=NULL)
		Menu->addAction("Remove Object", this, SLOT(onRemoveObject()));
	Menu->exec(QCursor::pos());
	ActiveItem = NULL;
	delete Menu;		
}

void ConfigureEnvironments::onMiscListRightClick(const QPoint & point)
{
	ActiveItem = listMisc->itemAt(point);
	onOtherListRightClick(ActiveItem,point);
}

void ConfigureEnvironments::onPreProcessListRightClick(const QPoint & point)
{
	ActiveItem = listPreProcess->itemAt(point);
	onOtherListRightClick(ActiveItem,point);
}

void ConfigureEnvironments::onSecondaryListRightClick(const QPoint & point)
{
	ActiveItem = listSecondary->itemAt(point);
	onOtherListRightClick(ActiveItem,point);
}

void ConfigureEnvironments::onAddEnvironment()
{
	CurrentProject->CreateEnvironmentNode();
	RefreshEnvironmentList();
}

void ConfigureEnvironments::onRemoveEnvironment()
{
	assert(ActiveItem);
	aride_environment * env = CurrentProject->GetEnvByGUID(ActiveItem->data(Qt::UserRole).toString());
	assert(env);
	CurrentProject->RemoveEnvironment(env);
	RefreshEnvironmentList();
	onEnvironmentClick(listEnvironments->currentItem());
}

void ConfigureEnvironments::onRemoveObject()
{
	assert(ActiveItem);
	aride_environment * env = CurrentProject->GetEnvByGUID(listEnvironments->currentItem()->data(Qt::UserRole).toString());
	aride_object * obj = CurrentProject->GetObjectByGUID(ActiveItem->data(Qt::UserRole).toString());
	assert(obj);
	assert(env);
	if (obj->Section == ARIDE_MISC)
		env->MiscObjects.remove(obj->GUID);
	else if (obj->Section == ARIDE_PREPROCESS)
		env->PreProcessObjects.remove(obj->GUID);
	if (obj->Section == ARIDE_SECONDARYOUTPUT)
		env->SecondaryObjects.remove(obj->GUID);
		
	CurrentProject->RemoveObject(obj);
	onEnvironmentClick(listEnvironments->currentItem());
}
	
void ConfigureEnvironments::onObjectClick(QWidget ** pDetails, QScrollArea * DetailsView, aride_object * Object)
{
	// now load the handlers for each of the environment items and display them

	delete *pDetails;
	*pDetails = new QFrame();
	if (*pDetails == NULL)
		throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	QWidget * Details = *pDetails;
	Details->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	
	DetailsView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	DetailsView->show();
	DetailsView->setWidget(Details);
	Details->show();
	
	QGridLayout * templ = new QGridLayout;
	delete Details->layout();
	Details->setLayout(templ);
	
	if (!Object)
		return;
	
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
		return;
	}

	// display each parameter
	int i=1;
	for (list<Parameter *>::iterator itr = Object->Handler->Parameters.begin(); itr != Object->Handler->Parameters.end(); ++itr, ++i)
	{
		temp = new QLabel((*itr)->Name);
		temp->show();
		templ->addWidget(temp,i,0);
		temp = new QLabel((*itr)->Type);
		temp->show();
		templ->addWidget(temp,i,1);
		QWidget * new_widget = (*itr)->CreateTypeWidget();
		QObject::connect(*itr, SIGNAL(requestUpdate()), this, SLOT(RefreshObjects()));
		new_widget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
		new_widget->show();
		templ->addWidget(new_widget,i,2);
	}
	templ->setRowStretch(i,1);
	
}
