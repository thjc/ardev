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
#ifndef PIXMAPWIDGET_H
#define PIXMAPWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QString>

#include <list>

#include <libthjc/geometry.h>

class PixmapWidget : public QLabel
{
	Q_OBJECT
	
public:
	PixmapWidget(QWidget *parent=0 );
	~PixmapWidget();

	void setImage(const QImage & Im);
	void setPoints(std::list<Point2D> aPoints) {mPoints=aPoints;repaint();};
	void clearPoints() {mPoints.clear();repaint();};

public slots:
	void setZoomFactor( float );
	
signals:
	void zoomFactorChanged( float );
	void clicked( int, int );

protected:
	void paintEvent( QPaintEvent* );
	void wheelEvent( QWheelEvent* );
	void mousePressEvent( QMouseEvent* );

private:
	QPixmap m_pm;
	float zoomFactor;
	int zoomFocusX;
	int zoomFocusY;
	bool haveImage;

	std::list<Point2D> mPoints;
};

#endif // PIXMAPWIDGET_H
