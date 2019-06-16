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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../ui/mainwindow_base.h"
#include <cm_project.h>

#include <QTreeWidget>
#include <QListWidgetItem>
#include <QMainWindow>

class ARObject;

class QGrid;
class QScrollView;
class ConfigureEnvironments;
class ConfigureDisplaylist;


//#include <Qlistview.h>

#include <deque>

using namespace std;

#define ARIDE_LIST_VIEW_ITEM_OBJECT 1001
#define ARIDE_LIST_VIEW_ITEM_ENVIRONMENT 1002

class ARListViewItem_Object : public QTreeWidgetItem
{
	public:
		ARListViewItem_Object(aride_object * _object, QTreeWidgetItem * parent, QString Text) : QTreeWidgetItem(parent,QStringList(Text))
		{
			object = _object;
		};
		aride_object * object;

		virtual int rtti() const {return ARIDE_LIST_VIEW_ITEM_OBJECT;};
};

class ARListViewItem_Environment : public QTreeWidgetItem
{
	public:
		ARListViewItem_Environment(aride_environment * _object, QTreeWidgetItem * parent, QString Text) : QTreeWidgetItem(parent,QStringList(Text))
		{
			object = _object;
		};
		aride_environment * object;

		int rtti() const {return ARIDE_LIST_VIEW_ITEM_ENVIRONMENT;};
};


class MainWindow : public QMainWindow, private Ui_MainWindow
{

public:
	MainWindow(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
	virtual ~MainWindow() {};
	void Initialise( int argc=0, char **argv=NULL);

	void RefreshTrees(); // Resfresh the tree of objects in the GUI.


	QTreeWidgetItem * CurrentItem;

	aride_project CurrentProject;

	void nonSlotObjectDoubleClicked(aride_object * obj);
	void nonSlotObjectDoubleClicked(aride_displaylist_node * node);
public slots:
	void ObjectDoubleClicked(QTreeWidgetItem * Item);
	void ObjectRightClicked(QTreeWidgetItem * Item);

	void controlRunPauseAction_toggled(bool State);
	void controlStop();
	void controlRun();

	void configureEnvironments();
	void configureDataSets();

	void AddObject(int id);
	void AddSubObject(int id);
	void AddEnvironment(int id);
	void RemoveObject(int id);

    virtual void fileNew();
    virtual void fileOpen();
    virtual void fileSave();
    virtual void fileSaveAs();

    virtual void closeEvent(QCloseEvent* event); // Override close event to allow the user to save.

protected:
	bool Stopping; // It seems if this is true, the project can not be paused.

	ConfigureDisplaylist * DisplaylistDialog;
	ConfigureEnvironments * EnvironmentsDialog;

protected slots:
//    virtual void languageChange();

private:
    Q_OBJECT


};

//extern MainWindow * theMainWindow;

#endif // MAINWINDOW_H
