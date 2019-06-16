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
 
#ifndef _CONFIGUREENVIRONMENTS_H_
#define _CONFIGUREENVIRONMENTS_H_

#include "../ui/configureenvironments_base.h"

class QScrollArea;
class ObjectHandler;
class QFrame;
class QWidget;
class aride_object;


class ConfigureEnvironments : public QDialog, public Ui_ConfigureEnvironments
{
	Q_OBJECT
	public:
		ConfigureEnvironments(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
		~ConfigureEnvironments();
	
		void RefreshEnvironmentList();
	public slots:
		void RefreshObjects();

		void onEnvironmentClick(QListWidgetItem * item);
		void onMiscClick(QListWidgetItem * item);
		void onPreProcessClick(QListWidgetItem * item);
		void onSecondaryClick(QListWidgetItem * item);
		void onOutputClick(const QString & type);
		void onCameraClick(const QString & type);
		void onCaptureClick(const QString & type);
		void onAddMisc();
		void onAddPreProcess();
		void onAddSecondary();
		
		void onEnvListRightClick(const QPoint & point);
		void onOtherListRightClick(QListWidgetItem * item, const QPoint & point);
		void onMiscListRightClick(const QPoint & point);
		void onPreProcessListRightClick(const QPoint & point);
		void onSecondaryListRightClick(const QPoint & point);
		
		// slots for the context menus
		void onAddEnvironment();
		void onRemoveEnvironment();
		void onRemoveObject();
		
	protected:
		QWidget * OutputDetails;
		//QScrollArea * OutputDetailsView;	

		QWidget * PreProcessDetails;
		QScrollArea * PreProcessDetailsView;	

		QWidget * CaptureDetails;
		QScrollArea * CaptureDetailsView;	

		QWidget * CameraDetails;
		QScrollArea * CameraDetailsView;	

		QWidget * SecondaryDetails;
		QScrollArea * SecondaryDetailsView;	
		
		QWidget * MiscDetails;
		QScrollArea * MiscDetailsView;	
		
		void onObjectClick(QWidget ** Details, QScrollArea * DetailsView, aride_object * object);
		QListWidgetItem * ActiveItem;
};

#endif //_CONFIGUREENVIRONMENTS_H_
