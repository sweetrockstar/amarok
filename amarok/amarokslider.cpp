/***************************************************************************
                        amarokslider.cpp  -  description
                           -------------------
  begin                : Dec 15 2003
  copyright            : (C) 2003 by Mark Kretschmann
  email                : markey@web.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "amarokslider.h"

#include <qapplication.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qrangecontrol.h>
#include <qsize.h>
#include <qwidget.h>

#include <kglobal.h>

#define THICKNESS 7
#define MARGIN 3

//---<init>---
AmarokSlider::AmarokSlider( QWidget *parent, Qt::Orientation orient )
        : QWidget( parent, "AmarokSlider", Qt::WRepaintNoErase )
        , QRangeControl()
        , m_isPressed( false )
        , m_orientation( orient )

{
    if ( orient == Horizontal )
        setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                    QSizePolicy::Fixed ) );
    else
        setSizePolicy( QSizePolicy(  QSizePolicy::Fixed,
                                     QSizePolicy::Expanding ) );
}
//---</init>---


QSize AmarokSlider::minimumSizeHint() const
{
    return sizeHint();
}


void AmarokSlider::setValue( int val )
{
    if ( val != value() )
    {
        QRangeControl::setValue( val );
        update();
        emit valueChanged( val );
    }
}


QSize AmarokSlider::sizeHint() const
{
    constPolish();
    
    if ( m_orientation == Horizontal )
        return QSize( 1, THICKNESS + MARGIN ).expandedTo( QApplication::globalStrut() );
    else
        return QSize( THICKNESS + MARGIN, 1 ).expandedTo( QApplication::globalStrut() );
}


//---<events>---
void AmarokSlider::mouseMoveEvent( QMouseEvent *e )
{
    if ( m_isPressed )
    {
        if ( m_orientation == Qt::Horizontal )
            setValue( valueFromPosition( e->pos().x(), width() ) );
        else
            setValue( valueFromPosition( e->pos().y(), height() ) );
    }
}


void AmarokSlider::mousePressEvent( QMouseEvent *e )
{
    if ( m_orientation == Qt::Horizontal )
        setValue( valueFromPosition( e->x(), width() ) );
    else
        setValue( valueFromPosition( e->y(), height() ) );

    m_isPressed = true;
    emit sliderPressed();
}


void AmarokSlider::mouseReleaseEvent( QMouseEvent * )
{
    m_isPressed = false;
    emit sliderReleased();
}


void AmarokSlider::paintEvent( QPaintEvent * )
{
    int length = ( m_orientation == Qt::Horizontal ) ? width() : height();
    int pos = positionFromValue( value(), length-2-1 );
    
    QPixmap *pBufPixmap = new QPixmap( width(), height() );
    bitBlt( pBufPixmap, 0, 0, parentWidget()->paletteBackgroundPixmap(), x(), y(), width(), height() );

    QPainter p( pBufPixmap, this );
        
    if ( m_orientation == Qt::Vertical )
    {    
        p.translate( 0, height()-1 );
        p.rotate( -90 );    
        pos = length-2-1 - pos;
    }        
    
    p.translate( 0, MARGIN );
    p.drawRect( 0, 0, length-1, THICKNESS-1 );
    p.fillRect( 1, 1, pos,      THICKNESS-2-1, 
                QBrush( QColor( 0x00, 0x20, 0x90 ), QBrush::SolidPattern ) );
    p.translate( 0, -MARGIN );
                       
    //<Triangle Marker>
    QPointArray pa( 3 );
    pa.setPoint( 0, pos - 3, 1 );
    pa.setPoint( 1, pos + 3, 1 );
    pa.setPoint( 2, pos,     5 );
    p.setBrush( QBrush( paletteForegroundColor(), QBrush::SolidPattern ) );
    p.drawConvexPolygon( pa );
    //</Triangle Marker>
    
    p.end();
    bitBlt( this, 0, 0, pBufPixmap );
        
    delete pBufPixmap;
}


#include "amarokslider.moc"
