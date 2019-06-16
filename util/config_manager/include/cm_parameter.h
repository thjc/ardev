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
#ifndef ARIDE_PARAMETER_H
#define ARIDE_PARAMETER_H

#include <assert.h>

#include <ardev/ardev.h>
#include <ardev/debug.h>

#define signals protected
//#include <qobject.h>
#include <QLineEdit>
#include <QCheckBox>

#include "cm_project.h"
// Base Parameter Type

class Parameter : public QObject
{
	Q_OBJECT
	public:
		Parameter(QString _Name, QString _Description, QString _DefaultValue="") {Name = _Name;Description = _Description;DefaultValue=_DefaultValue;};
		virtual ~Parameter() {};

		QString Name;
		QString Type;
		QString Description;
		QString DefaultValue;

		virtual QString toString() = 0;
		virtual QString fromString(const QString & inString) = 0;

		virtual void update() {};

		virtual QWidget * CreateTypeWidget()=0;
	signals:
		void requestUpdate();
};

// Basic Types (string, int, double, float, boolean)
class StringParameter : public Parameter
{
	Q_OBJECT

	public:
		StringParameter(QString _Name, QString _Description, QString _DefaultValue="");
		virtual ~StringParameter();

		QString Value;
		virtual QWidget * CreateTypeWidget() ;
		virtual QString toString();
		virtual QString fromString(const QString & inString) ;
	protected slots:
		virtual void ValueChanged(const QString & NewValue);

	protected:
		WId id;

};

class NameParameter : public StringParameter
{
	Q_OBJECT

	public:
		NameParameter(QString _Name, QString _Description, QString _DefaultValue="") : StringParameter(_Name,_Description,_DefaultValue) {Type="Name";};
		virtual ~NameParameter() {};

	protected slots:
		virtual void ValueChanged(const QString & NewValue) {fromString(NewValue);emit requestUpdate();};



};

class IntParameter : public StringParameter
{
	public:
		IntParameter(QString _Name, QString _Description, QString _DefaultValue="") : StringParameter(_Name,_Description,_DefaultValue) {Type="int";};

		QString toString() {return QString().setNum(Value);};
		QString fromString(const QString & inString) {Value=inString.toInt(); return toString();};

		int Value;
};

class FloatParameter : public StringParameter
{
	public:
		FloatParameter(QString _Name, QString _Description, QString _DefaultValue="") : StringParameter(_Name,_Description,_DefaultValue) {Type="float";};


		QString toString() {return QString().setNum(Value);};
		QString fromString(const QString & inString) {Value=inString.toFloat(); return toString();};

		float Value;
};

class DoubleParameter : public StringParameter
{
	public:
		DoubleParameter(QString _Name, QString _Description, QString _DefaultValue="") : StringParameter(_Name,_Description,_DefaultValue) {Type="double";};

		QString toString() {return QString().setNum(Value);};
		QString fromString(const QString & inString) {Value=inString.toDouble(); return toString();};

		double Value;
};

class BooleanParameter : public Parameter
{
	Q_OBJECT

	public:
		BooleanParameter(QString _Name, QString _Description, QString _DefaultValue="") : Parameter(_Name,_Description,_DefaultValue)
		{Type="boolean";id = 0;Value=false;};
		virtual ~BooleanParameter() {};

		bool Value;
		QWidget * CreateTypeWidget()
		{
			QCheckBox * widget = new QCheckBox();
			id=widget->winId();
			widget->setChecked(Value);
			connect(widget,SIGNAL(toggled(bool)),this,SLOT(ValueChanged(bool)));
			return widget;
		};
		virtual QString toString() {return Value ? "True" : "False";};
		virtual QString fromString(const QString & inString)
		{
			QCheckBox * widget = reinterpret_cast<QCheckBox*> (QWidget::find(id));
			bool inValue = (inString == "True");
			if (inValue == Value)
				return toString();
			Value = inValue;
			if (widget)
				widget->setChecked(Value);
			return toString();
		};
	protected slots:
		void ValueChanged(bool State) {Value = State;};
	protected:
		WId id;
};

// ARDev Types (ie ARPoint...)

class ARPointParameter : public Parameter
{
	Q_OBJECT

	public:
		ARPointParameter(QString _Name, QString _Description, QString _DefaultValue="") : Parameter(_Name,_Description,_DefaultValue) {Type="arpoint";};
		virtual ~ARPointParameter() {};

		virtual QWidget * CreateTypeWidget();

		virtual QString toString();
		virtual QString fromString(const QString & inString);

		ARPoint Value;

	protected slots:
		void xChange(const QString & inString) {Value.x = inString.toDouble();};
		void yChange(const QString & inString) {Value.y = inString.toDouble();};
		void zChange(const QString & inString) {Value.z = inString.toDouble();};
};

class ARPositionParameter : public Parameter
{
	Q_OBJECT

	public:
		ARPositionParameter(QString _Name, QString _Description, QString _DefaultValue="") : Parameter(_Name,_Description,_DefaultValue) {Type="arposition";};
		virtual ~ARPositionParameter() {};

		virtual QWidget * CreateTypeWidget();

		virtual QString toString();
		virtual QString fromString(const QString & inString);

		ARPosition Value;

	protected slots:
		void o_xChange(const QString & inString) {Value.Origin.x = inString.toDouble();};
		void o_yChange(const QString & inString) {Value.Origin.y = inString.toDouble();};
		void o_zChange(const QString & inString) {Value.Origin.z = inString.toDouble();};
		void d_xChange(const QString & inString) {Value.Direction.x = DTOR(inString.toDouble());};
		void d_yChange(const QString & inString) {Value.Direction.y = DTOR(inString.toDouble());};
		void d_zChange(const QString & inString) {Value.Direction.z = DTOR(inString.toDouble());};
};


class ARColourParameter : public Parameter
{
	Q_OBJECT

	public:
		ARColourParameter(QString _Name, QString _Description, QString _DefaultValue="") : Parameter(_Name,_Description,_DefaultValue) {Type="arcolour";};
		virtual ~ARColourParameter() {};

		virtual QWidget * CreateTypeWidget();

		virtual QString toString();
		virtual QString fromString(const QString & inString);

		ARColour Value;

	protected slots:
		void rChange(const QString & inString) {Value.r = inString.toDouble();};
		void gChange(const QString & inString) {Value.g = inString.toDouble();};
		void bChange(const QString & inString) {Value.b = inString.toDouble();};
		void aChange(const QString & inString) {Value.a = inString.toDouble();};
};

class ARListParameter;

class ARListParameterItem : public QObject
{
	Q_OBJECT

	public:
		ARListParameterItem(ARListParameter* parent, unsigned index, QLineEdit * widget) : parent(parent), index(index), widget(widget)
		{
			connect(widget,SIGNAL(textChanged(const QString&)),this,SLOT(ValueChanged(const QString &)));
		}
		ARListParameterItem& operator=(const ARListParameterItem& other)
		{
			parent = other.parent;
			index = other.index;
			widget = other.widget;
			connect(widget,SIGNAL(textChanged(const QString&)),this,SLOT(ValueChanged(const QString &)));
			return *this;
		}
		~ARListParameterItem()
		{
			widget->deleteLater(); // Avoid segfaults which can result from deleting in an event handler
		}
		void setFocus()
		{
			widget->setFocus();
		}

	protected slots:
		void ValueChanged(const QString & NewValue);

	private:
		ARListParameter* parent;
		unsigned index;
		QLineEdit * widget;
};

class QGridLayout;

class ARListParameter : public Parameter
{
	Q_OBJECT

	public:
		ARListParameter(QString _Name, QString _Description, QString _DefaultValue="") : Parameter(_Name,_Description,_DefaultValue)
		{
			Type="arlist";
			layout = NULL;
		}
		virtual ~ARListParameter();

		virtual QWidget * CreateTypeWidget();

		virtual QString toString();
		virtual QString fromString(const QString & inString);

		void change(unsigned index, const QString & inString);

		QStringList Value;

	//protected slots:
	//	void add();

	private:
		std::vector<ARListParameterItem*> items;
		QGridLayout * layout;
};


#endif
