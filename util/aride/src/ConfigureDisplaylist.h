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
 
class QScrollArea;
class ObjectHandler;
class QFrame;
class aride_object;
class aride_displaylist_node;
class QTreeWidgetItem;
 
#ifndef _CONFIGUREDISPLAYLIST_H_
#define _CONFIGUREDISPLAYLIST_H_

#include "../ui/configuredisplaylist_base.h"

class ConfigureDisplaylist : public QDialog, public Ui_ConfigureDisplayList
{
	Q_OBJECT
	public:
		ConfigureDisplaylist(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
		virtual ~ConfigureDisplaylist();
		void ExternalNameChange(const QString & OldName, const QString & NewName);	
	protected:
		QWidget * Details;
		QScrollArea * DetailsView;	
		QTreeWidgetItem * ActiveItem;
			
		void BuildDisplayList(QTreeWidgetItem* ParentItem, aride_displaylist_node * ParentNode);
	
	public slots:
		void RefreshDisplayLists();
		void onObjectClick(QTreeWidgetItem * item);
		void onObjectRightClick(const QPoint & pos);
		
		// for handling the context menu
		void onAddDisplayList();
		void onAddObject(QAction *);
		void onRemoveDisplayList();
		void onRemoveObject();
};

#endif //_CONFIGUREDISPLAYLIST_H_
