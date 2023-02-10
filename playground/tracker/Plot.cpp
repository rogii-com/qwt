/*****************************************************************************
* Qwt Examples - Copyright (C) 2002 Uwe Rathmann
* This file may be used under the terms of the 3-clause BSD License
*****************************************************************************/

#include "Plot.h"

#include <QwtPickerMachine>
#include <QwtPlotCanvas>
#include <QwtPlotGrid>
#include <QwtPlotTextLabel>
#include <QwtPlotCurve>
#include <QwtPlotLayout>
#include <QwtScaleWidget>
#include <QwtPlotTracker>
#include <QwtSymbol>
#include <QwtSyntheticPointData>
#include <QwtMath>

static const char* colors[] =
{
    "DarkRed", "DarkSeaGreen", "LightSalmon", "DarkBlue", "LightGoldenrodYellow",
    "DodgerBlue", "Limegreen", "DarkCyan", "Goldenrod", "Aquamarine", "DarkOrchid",
    "ForestGreen", "DarkMagenta", "Fuchsia", "LightCoral", "DarkTurquoise", "Gainsboro",
    "Azure", "Bisque", "DarkOrange", "BlanchedAlmond", "Blue", "BlueViolet", "Brown",
    "Burlywood", "DarkOliveGreen", "CadetBlue", "Chartreuse", "Aqua", "DarkKhaki",
    "Chocolate", "Coral", "CornflowerBlue", "Cornsilk", "Crimson", "Cyan", "DarkGreen",
    "DeepPink", "DeepSkyBlue", "DodgerBlue", "FireBrick", "LightGray", "FloralWhite",
    "GhostWhite", "LightSeaGreen", "LightSlateGray", "Gold", "DarkSalmon", "AntiqueWhite",
    "LightPink", "Green", "LightBlue", "LightCyan", "GreenYellow", "Grey", "Honeydew",
    "DarkGoldenrod", "LightSkyBlue", "HotPink", "Lavender", "IndianRed", "Indigo",
    "Ivory", "Khaki", "LavenderBlush", "LawnGreen", "LemonChiffon", "LightGreen",
    "LightGrey", "LightYellow", "Lime", "Beige", "Linen"
};

namespace
{
    class CurveData : public QwtSyntheticPointData
    {
      public:
        CurveData( double offset, double amplitude )
            : QwtSyntheticPointData( 50, QwtInterval( 0.0, 10.0 ) )
            , m_offset( offset )
            , m_amplitude( amplitude )
        {
        }

        virtual double y( double x ) const QWT_OVERRIDE
        {
            return qFastSin( x + m_offset ) * m_amplitude;
        }

      private:
        const double m_offset;
        const double m_amplitude;
    };

    class Curve : public QwtPlotCurve
    {
      public:
        Curve( const QString& colorName, double offset, double amplitude )
        {
            setTitle( colorName );

            const QColor color( colorName );

            setPen( color, 1 ),
            setRenderHint( QwtPlotItem::RenderAntialiased, true );

            QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse,
                QBrush( Qt::white ), QPen( color, 2 ), QSize( 2, 2 ) );
            setSymbol( symbol );

            setData( new CurveData( offset, amplitude ) );
        }

        virtual QwtText trackerInfoAt( int, const QPointF& pos ) const QWT_OVERRIDE
        {
            const QRectF br = boundingRect();
            if ( br.width() <= 0.0 )
                return QwtText();

            const double x = pos.x();

            if ( x < br.left() || x > br.right() )
                return QwtText();

            double y;

            const int index = adjacentPoint( Qt::Horizontal, x );

            if ( index == -1 )
            {
                const QPointF last = sample( dataSize() - 1 );

                if ( pos.x() != last.x() )
                    return QwtText();

                y = last.y();
            }
            else
            {
                const QLineF line( sample( index - 1 ), sample( index ) );
                y = line.pointAt( ( x - line.p1().x() ) / line.dx() ).y();
            }

            if ( qAbs( y ) < 10e-4 )
                y = 0.0;

            return QString::number( y, 'f', 4 );
        }
    };

    class Tracker : public QwtPlotTracker
    {
      public:
        Tracker( QWidget* canvas )
            : QwtPlotTracker( canvas )
        {
            const QPen pen( "MediumOrchid" );

            setRubberBand( VLineRubberBand );
            setRubberBandPen( pen );

            setBorderPen( pen );
            setBorderRadius( 10 );

            QColor bg( Qt::white );
            bg.setAlpha( 150 );

            setBackgroundBrush( bg );
            setMaxColumns( 3 );
        }
    };

}

Plot::Plot( QWidget* parent )
    : QwtPlot( parent)
{
    setPalette( Qt::black );

    // we want to have the axis scales like a frame around the canvas
    plotLayout()->setAlignCanvasToScales( true );
    for ( int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++ )
        axisWidget( axisPos )->setMargin( 0 );

    setAxisAutoScale( QwtPlot::yLeft, true );
    setAxisAutoScale( QwtPlot::xBottom, true );

    QwtPlotCanvas* canvas = new QwtPlotCanvas();
    canvas->setAutoFillBackground( false );
    canvas->setFrameStyle( QFrame::NoFrame );
    setCanvas( canvas );

    // a title
    QwtText title( "Tracker Demo" );
    title.setColor( Qt::white );
    title.setRenderFlags( Qt::AlignHCenter | Qt::AlignTop );

    QFont font;
    font.setBold( true );
    title.setFont( font );

    QwtPlotTextLabel* titleItem = new QwtPlotTextLabel();
    titleItem->setText( title );
    titleItem->attach( this );

    // grid

    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setMajorPen( Qt::white, 0, Qt::DotLine );
    grid->setMinorPen( Qt::gray, 0, Qt::DotLine );
    grid->attach( this );

    // curves

    for ( int i = 0; i < 17; i++ )
    {
        const int index = i % ( sizeof( colors ) / sizeof( colors[0] ) );

        Curve* curve = new Curve( colors[index],
            ( qwtRand() % 100 ) / 20.0, qwtRand() % 100 );
        curve->attach( this );
    }

    // tracker
    (void) new Tracker( this->canvas() );
}
