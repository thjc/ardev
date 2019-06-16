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
#include <QLineEdit>
#include <QString>
#include <QLabel>
#include <QTextStream>
#include <QGridLayout>
#include <QPushButton>

#include "cm_parameter.h"
#include "cm_parameter_ardev.h"

StringParameter::StringParameter(QString _Name, QString _Description, QString _DefaultValue) : Parameter(_Name,_Description,_DefaultValue) {id=0;Type="String";}

StringParameter::~StringParameter() {}

QWidget * StringParameter::CreateTypeWidget()
{
	QLineEdit * temp = new QLineEdit(toString());
	id = temp->winId();
	temp->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	connect(temp,SIGNAL(textChanged(const QString&)),this,SLOT(ValueChanged(const QString &)));
	return temp;
}

QString StringParameter::toString() {return Value;};
QString StringParameter::fromString(const QString & inString)
{
	QLineEdit * temp = reinterpret_cast<QLineEdit*> (QWidget::find(id));
	if (inString == Value)
		return Value;
	Value = inString;
	if (temp && inString != temp->text())
		temp->setText(inString);
	return Value;
}

void StringParameter::ValueChanged(const QString & NewValue) {fromString(NewValue);}

// Implementation of ARPointParamter
QString ARPointParameter::toString()
{
	QString Temp;
	QTextStream TempStream(&Temp);
	TempStream << Value.x << " " << Value.y << " " << Value.z;
	return Temp;
}

QString ARPointParameter::fromString(const QString & inString)
{
	QString Temp = inString;
	QTextStream inStream(&Temp);
	inStream >> Value.x >> Value.y >> Value.z;
	return toString();
}

QWidget * ARPointParameter::CreateTypeWidget()
{
	QGridLayout * layout = new QGridLayout;
	QWidget * ret = new QWidget;
	ret->setLayout(layout);

	layout->addWidget(new QLabel("x"),0,0);
	QLineEdit * xedit = new QLineEdit(QString().setNum(Value.x));
	layout->addWidget(xedit,0,1);
	layout->addWidget(new QLabel("y"),1,0);
	QLineEdit * yedit = new QLineEdit(QString().setNum(Value.y));
	layout->addWidget(yedit,1,1);
	layout->addWidget(new QLabel("z"),2,0);
	QLineEdit * zedit = new QLineEdit(QString().setNum(Value.z));
	layout->addWidget(zedit,2,1);
	connect(xedit,SIGNAL(textChanged(const QString&)),this,SLOT(xChange(const QString &)));
	connect(yedit,SIGNAL(textChanged(const QString&)),this,SLOT(yChange(const QString &)));
	connect(zedit,SIGNAL(textChanged(const QString&)),this,SLOT(zChange(const QString &)));
	return ret;
}

// Implementation of ARPositionParamter
QString ARPositionParameter::toString()
{
	QString Temp;
	QTextStream Stream(&Temp);
	Stream << Value.Origin.x << " " << Value.Origin.y << " " << Value.Origin.z << " ";
	Stream << RTOD(Value.Direction.x) << " " << RTOD(Value.Direction.y) << " " << RTOD(Value.Direction.z);
	return Temp;
}

QString ARPositionParameter::fromString(const QString & inString)
{
	QString Temp = inString;
	QTextStream inStream(&Temp);
	inStream >> Value.Origin.x >> Value.Origin.y >> Value.Origin.z;
	inStream >> Value.Direction.x >> Value.Direction.y >> Value.Direction.z;
	Value.Direction.x = DTOR(Value.Direction.x);
	Value.Direction.y = DTOR(Value.Direction.y);
	Value.Direction.z = DTOR(Value.Direction.z);
	return toString();
}

QWidget * ARPositionParameter::CreateTypeWidget()
{
	QGridLayout * layout = new QGridLayout;
	QWidget * ret = new QWidget;
	ret->setLayout(layout);

	QWidget * origin=new QWidget();
	layout->addWidget(new QLabel("Origin"),0,0);
	{
		QGridLayout * layout_inner = new QGridLayout;
		origin->setLayout(layout_inner);
		layout_inner->addWidget(new QLabel("x"),0,0);
		QLineEdit * xedit = new QLineEdit(QString().setNum(Value.Origin.x));
		layout_inner->addWidget(xedit,0,1);
		layout_inner->addWidget(new QLabel("y"),1,0);
		QLineEdit * yedit = new QLineEdit(QString().setNum(Value.Origin.y));
		layout_inner->addWidget(yedit,1,1);
		layout_inner->addWidget(new QLabel("z"),2,0);
		QLineEdit * zedit = new QLineEdit(QString().setNum(Value.Origin.z));
		layout_inner->addWidget(zedit,2,1);
		connect(xedit,SIGNAL(textChanged(const QString&)),this,SLOT(o_xChange(const QString &)));
		connect(yedit,SIGNAL(textChanged(const QString&)),this,SLOT(o_yChange(const QString &)));
		connect(zedit,SIGNAL(textChanged(const QString&)),this,SLOT(o_zChange(const QString &)));
	}
	layout->addWidget(origin,0,1);

	QWidget * rotation=new QWidget();
	layout->addWidget(new QLabel("Rotation"),1,0);
	{
		QGridLayout * layout_inner = new QGridLayout;
		rotation->setLayout(layout_inner);
		layout_inner->addWidget(new QLabel("rot x"),0,0);
		QLineEdit * xedit = new QLineEdit(QString().setNum(RTOD(Value.Direction.x)));
		layout_inner->addWidget(xedit,0,1);
		layout_inner->addWidget(new QLabel("rot y"),1,0);
		QLineEdit * yedit = new QLineEdit(QString().setNum(RTOD(Value.Direction.y)));
		layout_inner->addWidget(yedit,1,1);
		layout_inner->addWidget(new QLabel("rot z"),2,0);
		QLineEdit * zedit = new QLineEdit(QString().setNum(RTOD(Value.Direction.z)));
		layout_inner->addWidget(zedit,2,1);
		connect(xedit,SIGNAL(textChanged(const QString&)),this,SLOT(d_xChange(const QString &)));
		connect(yedit,SIGNAL(textChanged(const QString&)),this,SLOT(d_yChange(const QString &)));
		connect(zedit,SIGNAL(textChanged(const QString&)),this,SLOT(d_zChange(const QString &)));
	}
	layout->addWidget(rotation,1,1);

	return ret;
};

// Implementation of ARColourParamter
QString ARColourParameter::toString()
{
	QString Temp;
	QTextStream TempStream(&Temp);
	TempStream << Value.r << " " << Value.g << " " << Value.b << " " << Value.a;
	return Temp;
}

QString ARColourParameter::fromString(const QString & inString)
{
	QString Temp = inString;
	QTextStream inStream(&Temp);
	inStream >> Value.r >> Value.g >> Value.b >> Value.a;
	return toString();
}

QWidget * ARColourParameter::CreateTypeWidget()
{
	QGridLayout * layout = new QGridLayout;
	QWidget * ret = new QWidget;
	ret->setLayout(layout);

	layout->addWidget(new QLabel("r"),0,0);
	QLineEdit * redit = new QLineEdit(QString().setNum(Value.r));
	layout->addWidget(redit,0,1);
	layout->addWidget(new QLabel("g"),1,0);
	QLineEdit * gedit = new QLineEdit(QString().setNum(Value.g));
	layout->addWidget(gedit,1,1);
	layout->addWidget(new QLabel("b"),2,0);
	QLineEdit * bedit = new QLineEdit(QString().setNum(Value.b));
	layout->addWidget(bedit,2,1);
	layout->addWidget(new QLabel("a"),3,0);
	QLineEdit * aedit = new QLineEdit(QString().setNum(Value.a));
	layout->addWidget(aedit,3,1);
	connect(redit,SIGNAL(textChanged(const QString&)),this,SLOT(rChange(const QString &)));
	connect(gedit,SIGNAL(textChanged(const QString&)),this,SLOT(gChange(const QString &)));
	connect(bedit,SIGNAL(textChanged(const QString&)),this,SLOT(bChange(const QString &)));
	connect(aedit,SIGNAL(textChanged(const QString&)),this,SLOT(aChange(const QString &)));
	return ret;
}

// Implementation of ARListParamterItem
void ARListParameterItem::ValueChanged(const QString & NewValue)
{
	parent->change(index, NewValue);
}

// Implementation of ARListParamter
ARListParameter::~ARListParameter()
{
	for (unsigned i = 0; i < items.size(); i++)
		delete items[i];
}

QString ARListParameter::toString()
{
	QString Temp;
	QTextStream TempStream(&Temp);
	for (int i = 0; i < Value.size(); i++)
	{
		if (i != 0)
			TempStream << " ";
		TempStream << Value[i];
	}
	return Temp;
}

QString ARListParameter::fromString(const QString & inString)
{
	Value = inString.split(' ');
	// Update gui? update items?
	return toString();
}

QWidget * ARListParameter::CreateTypeWidget()
{
	layout = new QGridLayout;
	QWidget * ret = new QWidget;
	ret->setLayout(layout);

	for (int i = 0; i < Value.size(); i++)
	{
		QLineEdit * field = new QLineEdit(Value[i]);
		layout->addWidget(field,i,0);
		items.push_back(new ARListParameterItem(this,i,field));
	}
	// Add an extra one for people to add new items.
	QLineEdit * field = new QLineEdit("");
	layout->addWidget(field,Value.size(),0);
	items.push_back(new ARListParameterItem(this,Value.size(),field));

	return ret;
}

void ARListParameter::change(unsigned index, const QString & inString)
{
	assert(index <= (unsigned)Value.size());
	if (index == (unsigned)Value.size()) // A new item has been added.
	{
		Value.push_back(inString);
		// Add a new field for more items.
		QLineEdit * field = new QLineEdit("");
		layout->addWidget(field,Value.size(),0);
		items.push_back(new ARListParameterItem(this,Value.size(),field));
	}
	else
	{
		Value[index] = inString;
		// Remove any empty items from the end of the list.
		bool removed = false;
		while (Value.size() && Value.last() == "")
		{
			Value.pop_back();
			delete items.back();
			items.pop_back();
			removed = true;
		}
		if (removed) // If list items have been removed, set focus to empty item
			items.back()->setFocus();
	}
}
