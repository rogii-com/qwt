/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_tracker.h"
#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_picker_machine.h"
#include "qwt_painter.h"
#include "qwt_math.h"

#include <qvariant.h>
#include <qlayoutitem.h>
#include <qpainter.h>

static inline QwtText qwtText( const QList< QVariant >& values )
{
    QwtText text;

    for ( int i = 0; i < values.size(); i++ )
    {
        const QVariant& v = values[i];

        if ( v.canConvert< QwtText >() )
            return qvariant_cast< QwtText >( v );

        if ( v.canConvert< QString >() )
            return qvariant_cast< QString >( v );
    }

    return QwtText();
}

static inline QwtGraphic qwtGraphic( const QList< QVariant >& values )
{
    for ( int i = 0; i < values.size(); i++ )
    {   
        const QVariant& v = values[i];

        if ( v.canConvert< QwtGraphic >() )
            return qvariant_cast< QwtGraphic >( v );
    }

    return QwtGraphic();
}

namespace
{
    class LayoutItem QWT_FINAL : public QLayoutItem
    {
      public:
        LayoutItem( const QwtPlotTracker* );

        virtual Qt::Orientations expandingDirections() const QWT_OVERRIDE;
        virtual QRect geometry() const QWT_OVERRIDE;
        virtual bool hasHeightForWidth() const QWT_OVERRIDE;
        virtual int heightForWidth( int ) const QWT_OVERRIDE;
        virtual bool isEmpty() const QWT_OVERRIDE;
        virtual QSize maximumSize() const QWT_OVERRIDE;
        virtual int minimumHeightForWidth( int ) const QWT_OVERRIDE;
        virtual QSize minimumSize() const QWT_OVERRIDE;
        virtual void setGeometry( const QRect& ) QWT_OVERRIDE;
        virtual QSize sizeHint() const QWT_OVERRIDE;

        void setTrackerData( const QList< QVariant >& );
        const QList< QVariant >& trackerData() const;

        void setPlotItem( const QwtPlotItem* );
        const QwtPlotItem* plotItem() const;

      private:
        const QwtPlotTracker* m_tracker;

        const QwtPlotItem* m_plotItem;
        QList< QVariant > m_trackerData;

        QRect m_rect;
    };

    LayoutItem::LayoutItem( const QwtPlotTracker* tracker )
        : m_tracker( tracker )
        , m_plotItem( NULL)
    {
    }

    inline void LayoutItem::setPlotItem( const QwtPlotItem* plotItem )
    {
        m_plotItem = plotItem;
    }

    inline const QwtPlotItem* LayoutItem::plotItem() const
    {
        return m_plotItem;
    }

    inline void LayoutItem::setTrackerData( const QList< QVariant >& trackerData )
    {
        m_trackerData = trackerData;
    }

    inline const QList< QVariant >& LayoutItem::trackerData() const
    {
        return m_trackerData;
    }

    Qt::Orientations LayoutItem::expandingDirections() const
    {
        return Qt::Horizontal;
    }

    bool LayoutItem::isEmpty() const
    {
        return false;
    }

    bool LayoutItem::hasHeightForWidth() const
    {
        return true;
    }

    int LayoutItem::minimumHeightForWidth( int width ) const
    {
        return m_tracker->heightForWidth( m_plotItem, m_trackerData, width );
    }

    int LayoutItem::heightForWidth( int width ) const
    {
        return m_tracker->heightForWidth( m_plotItem, m_trackerData, width );
    }

    QSize LayoutItem::maximumSize() const
    {
        return QSize( QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX );
    }

    QSize LayoutItem::minimumSize() const
    {
        return m_tracker->minimumSize( m_plotItem, m_trackerData );
    }

    QSize LayoutItem::sizeHint() const
    {
        return minimumSize();
    }

    void LayoutItem::setGeometry( const QRect& rect )
    {
        m_rect = rect;
    }

    QRect LayoutItem::geometry() const
    {
        return m_rect;
    }

    class Layout : public QwtDynGridLayout
    {
      public:
        Layout()
        {
            setMaxColumns( 2 );
            setSpacing( 0 );
            setContentsMargins( 0, 0, 0, 0 );
        }

        ~Layout()
        {
            clear();
        }

        void clear()
        {
            for ( int i = count() - 1; i >= 0; i-- )
                delete takeAt( i );
        }

        void updateItems( const QwtPlotTracker* tracker, const QPointF& trackerPosition )
        {
            const QwtPlot* plot = tracker->plot();
            if ( plot == NULL )
            {
                clear();
                return;
            }

            int index = 0;

            double pos[ QwtAxis::AxisPositions ];

            {
                using namespace QwtAxis;

                const double x = trackerPosition.x();
                const double y = trackerPosition.y();

                pos[ YLeft ] = plot->canvasMap( YLeft ).invTransform( y );
                pos[ YRight ] = plot->canvasMap( YRight ).invTransform( y );
                pos[ XBottom ] = plot->canvasMap( XBottom ).invTransform( x );
                pos[ XTop ] = plot->canvasMap( XTop ).invTransform( x );
            }

            const QwtPlotItemList& items = plot->itemList();
            for ( QwtPlotItemIterator it = items.begin(); it != items.end(); ++it )
            {
                const QwtPlotItem* plotItem = *it;
                if ( plotItem->testItemAttribute( QwtPlotItem::Tracker ) )
                {
                    const QPointF trackerPos(
                        pos[ plotItem->xAxis() ], pos[ plotItem->yAxis() ] );

                    const QList< QVariant > data = plotItem->trackerData( 0, trackerPos );

                    if ( !data.isEmpty() )
                    {
                        LayoutItem* layoutItem = nullptr;
                        if ( index < count() )
                        {
                            layoutItem = static_cast< LayoutItem* >( itemAt( index ) );
                        }
                        else
                        {
                            layoutItem = new LayoutItem( tracker );
                            addItem( layoutItem );
                        }

                        layoutItem->setPlotItem( plotItem );
                        layoutItem->setTrackerData( data );

                        index++;
                    }
                }
            }

            for ( int i = count() - 1; i >= index; i-- )
                delete takeAt( i );
        }
    };
}

class QwtPlotTracker::PrivateData
{
  public:
    PrivateData()
        : borderRadius( 0 )
        , borderPen( Qt::NoPen )
        , itemMargin( 4 )
        , itemSpacing( 4 )
        , trackerAttributes( PositionInfo )
    {
    }

    double borderRadius;
    QPen borderPen;
    
    QBrush backgroundBrush;

    int itemMargin;
    int itemSpacing;

    TrackerAttributes trackerAttributes;
    Layout layout;
};

QwtPlotTracker::QwtPlotTracker( QWidget* canvas )
    : QwtPicker( canvas )
{
    m_data = new PrivateData;

    setStateMachine( new QwtPickerTrackerMachine() );
    setTrackerMode( AlwaysOn );
}

QwtPlotTracker::~QwtPlotTracker()
{
    delete m_data;
}

//! \return Observed plot canvas
QWidget* QwtPlotTracker::canvas()
{
    return parentWidget();
}

//! \return Observed plot canvas
const QWidget* QwtPlotTracker::canvas() const
{
    return parentWidget();
}

//! \return Plot widget, containing the observed plot canvas
QwtPlot* QwtPlotTracker::plot()
{
    QWidget* w = canvas();
    if ( w )
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >( w );
}

//! \return Plot widget, containing the observed plot canvas
const QwtPlot* QwtPlotTracker::plot() const
{
    const QWidget* w = canvas();
    if ( w )
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >( w );
}

void QwtPlotTracker::setMaxColumns( uint numColums )
{
    m_data->layout.setMaxColumns( numColums );
}

uint QwtPlotTracker::maxColumns() const
{
    return m_data->layout.maxColumns();
}

void QwtPlotTracker::setTrackerAttribute( TrackerAttribute attribute, bool on )
{
    if ( on )
        m_data->trackerAttributes |= attribute;
    else
        m_data->trackerAttributes &= ~attribute;
}

bool QwtPlotTracker::testTrackerAttribute( TrackerAttribute attribute ) const
{
    return m_data->trackerAttributes.testFlag( attribute );
}

void QwtPlotTracker::setBorderRadius( double radius )
{
    m_data->borderRadius = qwtMaxF( 0.0, radius );
}

double QwtPlotTracker::borderRadius() const
{
    return m_data->borderRadius;
}

void QwtPlotTracker::setBorderPen( const QPen& pen )
{
    m_data->borderPen = pen;
}

QPen QwtPlotTracker::borderPen() const
{
    return m_data->borderPen;
}

void QwtPlotTracker::setBackgroundBrush( const QBrush& brush )
{
    m_data->backgroundBrush = brush;
}

QBrush QwtPlotTracker::backgroundBrush() const
{
    return m_data->backgroundBrush;
}

void QwtPlotTracker::drawTracker( QPainter* painter ) const
{
    const Layout& layout = m_data->layout;

    const QRect rect = layout.geometry();
    if ( rect.isEmpty() )
    {
        // don't draw a legend when having no content
        return;
    }

    if ( m_data->borderPen != Qt::NoPen
        || m_data->backgroundBrush.style() != Qt::NoBrush )
    {
        // expanding rect by borderPen/borderRadius. TODO ...

        painter->save();

        painter->setPen( m_data->borderPen );
        painter->setBrush( m_data->backgroundBrush );

        if ( m_data->borderRadius == 0 )
        {
            QwtPainter::drawRect( painter, rect );
        }
        else
        {
            painter->setRenderHint( QPainter::Antialiasing, true );
            painter->drawRoundedRect( rect,
                m_data->borderRadius, m_data->borderRadius );
        }

        painter->restore();
    }

    for ( int i = 0; i < layout.count(); i++ )
    {
        const LayoutItem* layoutItem =
            static_cast< LayoutItem* >( layout.itemAt( i ) );

        painter->save();

        drawTrackerData( painter, layoutItem->plotItem(),
            layoutItem->trackerData(), layoutItem->geometry() );

        painter->restore();
    }
}

void QwtPlotTracker::drawTrackerData( QPainter* painter,
    const QwtPlotItem* plotItem, const QList< QVariant >& data,
    const QRect& rect ) const
{
    Q_UNUSED( plotItem );

    const int m = m_data->itemMargin;

    const QRect r = rect.adjusted( m, m, -m, -m );

    painter->setClipRect( r, Qt::IntersectClip );

    int titleOff = 0;

    const QwtGraphic graphic = qwtGraphic( data );
    if ( !graphic.isEmpty() )
    {
        QRectF iconRect( r.topLeft(), graphic.defaultSize() );

        iconRect.moveCenter(
            QPoint( iconRect.center().x(), rect.center().y() ) );

        graphic.render( painter, iconRect, Qt::KeepAspectRatio );

        titleOff += iconRect.width() + m_data->itemSpacing;
    }

    const QwtText text = qwtText( data );
    if ( !text.isEmpty() )
    {
        painter->setPen( trackerPen() );
        painter->setFont( trackerFont() );

        const QRect textRect = r.adjusted( titleOff, 0, 0, 0 );
        text.draw( painter, textRect );
    }
}

QRegion QwtPlotTracker::trackerMask() const
{
    Layout& layout = m_data->layout;

    layout.updateItems( this, trackerPosition() );

    QRect rect = trackerRect( m_data->layout.sizeHint() );
    layout.setGeometry( rect );

    // expanding rect by borderPen/borderRadius. TODO ...
    return rect;
}

QwtText QwtPlotTracker::trackerText( const QPoint& pos ) const
{
    return QwtPicker::trackerText( pos );
}

int QwtPlotTracker::heightForWidth( const QwtPlotItem* plotItem,
    const QList< QVariant >& data, int width ) const
{
    Q_UNUSED( plotItem );

    width -= 2 * m_data->itemMargin;

    const QwtGraphic graphic = qwtGraphic( data );
    const QwtText text = qwtText( data );

    if ( text.isEmpty() )
        return graphic.height();

    if ( graphic.width() > 0 )
        width -= graphic.width() + m_data->itemSpacing;

    int h = text.heightForWidth( width, trackerFont() );
    h += 2 * m_data->itemMargin;

    return qMax( graphic.height(), h );
}

QSize QwtPlotTracker::minimumSize( const QwtPlotItem* plotItem,
    const QList< QVariant >& data ) const
{
    Q_UNUSED( plotItem );

    QSize size( 2 * m_data->itemMargin, 2 * m_data->itemMargin );

    const QwtGraphic graphic = qwtGraphic( data );
    const QwtText text = qwtText( data );

    int w = 0;
    int h = 0;

    if ( !graphic.isNull() )
    {
        w = graphic.width();
        h = graphic.height();
    }

    if ( !text.isEmpty() )
    {
        const QSizeF sz = text.textSize( trackerFont() );

        w += qwtCeil( sz.width() );
        h = qMax( h, qwtCeil( sz.height() ) );
    }

    if ( graphic.width() > 0 && !text.isEmpty() )
        w += m_data->itemSpacing;

    size += QSize( w, h );
    return size;
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_plot_tracker.cpp"
#endif
