/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_TRACKER_H
#define QWT_PLOT_TRACKER_H

#include "qwt_global.h"
#include "qwt_picker.h"

class QwtPlot;
class QwtPlotItem;
class QVariant;

class QBrush;
class QPen;

template< typename T > class QList;

class QWT_EXPORT QwtPlotTracker : public QwtPicker
{
    Q_OBJECT

    Q_PROPERTY( double borderRadius READ borderRadius WRITE setBorderRadius )
    Q_PROPERTY( QPen borderPen READ borderPen WRITE setBorderPen )
    Q_PROPERTY( QBrush backgroundBrush READ backgroundBrush WRITE setBackgroundBrush )

  public:
    enum TrackerAttribute
    {
        PositionInfo = 0x01,
        ItemInfo     = 0x02
    };

    Q_DECLARE_FLAGS( TrackerAttributes, TrackerAttribute )

    explicit QwtPlotTracker( QWidget* );
    virtual ~QwtPlotTracker();

    QwtPlot* plot();
    const QwtPlot* plot() const;

    QWidget* canvas();
    const QWidget* canvas() const;

    void setTrackerAttribute( TrackerAttribute, bool on = true );
    bool testTrackerAttribute( TrackerAttribute ) const;

    void setMaxColumns( uint numColums );
    uint maxColumns() const;

    void setBorderRadius( double );
    double borderRadius() const;

    void setBorderPen( const QPen& );
    QPen borderPen() const;

    void setBackgroundBrush( const QBrush& );
    QBrush backgroundBrush() const;

    virtual QRegion trackerMask() const QWT_OVERRIDE;
    virtual QwtText trackerText( const QPoint& ) const QWT_OVERRIDE;

    virtual void drawTracker( QPainter* ) const QWT_OVERRIDE;

    virtual int heightForWidth( const QwtPlotItem*,
        const QList< QVariant >&, int width ) const;

    virtual QSize minimumSize(
        const QwtPlotItem*, const QList< QVariant >& ) const;

    virtual QList< QVariant > trackerDataAt( const QwtPlotItem*, const QPointF& ) const;

  protected:
    virtual void drawTrackerData( QPainter*, const QwtPlotItem*,
        const QList< QVariant >&, const QRect& ) const;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
