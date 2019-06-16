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
#include <QPixmap>
#include <QPainter>
#include <QWheelEvent>
#include <QScrollArea>
 #include <QScrollBar>

#include "PixmapWidget.h"

PixmapWidget::PixmapWidget( QWidget *parent ) : QLabel( parent )
{
//	m_pm = new QPixmap( filename );
	zoomFactor = 1.0;
	zoomFocusX = 0;
	zoomFocusY = 0;
	haveImage = false;
	
	setMinimumSize( static_cast<int>(m_pm.width()*zoomFactor), static_cast<int>(m_pm.height()*zoomFactor) );
}

PixmapWidget::~PixmapWidget()
{
//	delete m_pm;
}

void PixmapWidget::setImage(const QImage & im)
{
	if ( !im.isNull() )
	{
		haveImage = true;
		m_pm = QPixmap::fromImage(im);
		repaint();
	}
}


void PixmapWidget::setZoomFactor( float f )
{
	int w, h;
	
	if( f == zoomFactor || !haveImage )
		return;
	
	float newFocusX = zoomFocusX / zoomFactor * f;
	float newFocusY = zoomFocusY / zoomFactor * f;
	int scrollX = int(newFocusX - zoomFocusX);
	int scrollY = int(newFocusY - zoomFocusY);
	
	QWidget *p = dynamic_cast<QWidget*>( parent() );
	QScrollBar* horizScroll = NULL;
	QScrollBar* vertScroll = NULL;
	int scrollPosX = 0;
	int scrollPosY = 0;
	if ( p )
	{
		QScrollArea *scrollArea = dynamic_cast<QScrollArea*>( p->parent() );
		if ( scrollArea )
		{
			horizScroll = scrollArea->horizontalScrollBar();
			vertScroll = scrollArea->verticalScrollBar();
			// Record the scroll bar positions before the widget is resized.
			scrollPosX = horizScroll->sliderPosition();
			scrollPosY = vertScroll->sliderPosition();
		}
	}

	zoomFactor = f;
	emit( zoomFactorChanged( zoomFactor ) );
	
	setUpdatesEnabled( false );

	w = static_cast<int> (m_pm.width()*zoomFactor);
	h = static_cast<int> (m_pm.height()*zoomFactor);
	setMinimumSize( w, h );
	
	if( p )
		resize( p->width(), p->height() );
	
	if ( horizScroll )
		horizScroll->setSliderPosition(scrollPosX + scrollX);
	if ( vertScroll )
		vertScroll->setSliderPosition(scrollPosY + scrollY);
	
	setUpdatesEnabled( true );
	repaint();
}

void PixmapWidget::paintEvent( QPaintEvent *event )
{
	int xoffset, yoffset;
	bool drawBorder = false;
	
	if( width() > m_pm.width()*zoomFactor )
	{
		xoffset = static_cast<int> ((width()-m_pm.width()*zoomFactor)/2);
		drawBorder = true;
	}
	else
	{
		xoffset = 0;
	}
	
	if( height() > m_pm.height()*zoomFactor )
	{
		yoffset = static_cast<int> ((height()-m_pm.height()*zoomFactor)/2);
		drawBorder = true;
	}
	else
	{
		yoffset = 0;
	}

	QPainter p( this );
	p.save();
	p.translate( xoffset, yoffset );
	p.scale( zoomFactor, zoomFactor );
	p.drawPixmap( 0, 0, m_pm );
	p.setPen( Qt::black );
	
	int i = 0;
	for (std::list<Point2D>::iterator itr = mPoints.begin(); itr != mPoints.end(); ++itr,++i)
	{
		QBrush Brush(qRgb(1,1,0));
		if (i < 9)
			Brush = QBrush(qRgb(1,0,1));
		else if (i < 18)
			Brush = QBrush(qRgb(0,1,1));
		
		p.setBrush(Brush);
		p.drawEllipse(static_cast<int> (itr->x),static_cast<int> (itr->y),10,10);
	}	
	
	p.restore();
	if( drawBorder )
	{
		p.setPen( Qt::black );
		p.drawRect( xoffset-1, yoffset-1, static_cast<int> (m_pm.width()*zoomFactor+1), static_cast<int> (m_pm.height()*zoomFactor+1) );
	}
}

void PixmapWidget::wheelEvent( QWheelEvent *event )
{
	float f;

	f = zoomFactor * (1.0 + 0.002*event->delta());
	if( f < 32.0/m_pm.width() )
		f = 32.0/m_pm.width();
	
	zoomFocusX = event->x();
	zoomFocusY = event->y();

	setZoomFactor( f );
}

void PixmapWidget::mousePressEvent( QMouseEvent* event )
{
	if ( !haveImage )
		return;
	
	int xoffset, yoffset;
	
	if( width() > m_pm.width()*zoomFactor )
		xoffset = static_cast<int> ((width()-m_pm.width()*zoomFactor)/2);
	else
		xoffset = 0;
	
	if( height() > m_pm.height()*zoomFactor )
		yoffset = static_cast<int> ((height()-m_pm.height()*zoomFactor)/2);
	else
		yoffset = 0;
	
	int transformedX = int((event->x() - xoffset) / zoomFactor);
	int transformedY = int((event->y() - yoffset) / zoomFactor);
	
	emit( clicked( transformedX, transformedY ) );
}
