#ifndef ICV_PLOT_CANVAS_H
#define ICV_PLOT_CANVAS_H

#include <qobject.h>
#include <QtGlobal>
#include <QMenu>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>

#include "icv_icurve.h"
#include "icv_plot_curve.h"

class QPoint;
class QPointF;
class QCustomEvent;
class QwtPlot;
class QPolygonF;
class QwtPlotCurve;
class IcvICurve;

class IcvPlotCanvas : public QWidget
{
    Q_OBJECT

public:
    IcvPlotCanvas(IcvICurve *parent);
    ~IcvPlotCanvas();

    IcvICurve* retrieveParent();
    void setMainWin(IcvICurve *icurve);

    QwtPlotCanvas *getCanvas();
    void setCanvas(QwtPlotCanvas *cvs);

    QList<IcvPlotCurve *> getCurves();
    void setCurves(QList<IcvPlotCurve *> crvs);
    void initCurves();
    void updateCurves();
    void lookforCurves();
    void appendCurves(IcvPlotCurve *curve);
    void clearCurves();
    IcvPlotCurve* getSelectedCurve();

    void createCurvePopMenu();
    void createCurvePopMenuAction();

    virtual bool eventFilter( QObject *, QEvent * );
    virtual bool event( QEvent * );


private:
    void onMouseLeftButtonClick(const QMouseEvent *event);
    void onMouseRightButtonClick(const QMouseEvent *event);
    void onMouseMove(const QMouseEvent *event);


private:
    QwtPlotCanvas *canvas;
    IcvICurve  *mainWin;
    QList<IcvPlotCurve *> curves;
    IcvPlotCurve *curSelectedCurve;
    IcvPlotCurve *prevSelectedCurve;

    /*menus*/
    QMenu *crvSelPopMenu;
    QMenu *subCrvSelWidthMenu;
    QMenu *subCrvSelStyleMenu;
    QMenu *subCrvSelMarkeStyleMenu;
    QMenu *subCrvSelMarkerSizeMenu;

    /*actions*/
    QAction *cutAction;
    QAction *copyAction;
    QAction *delAction;
    QAction *colorSetAction;
    QAction *propertySetAction;

    /*action groups*/
    QActionGroup *widthActGrp;
    QActionGroup *styleActGrp;
    QActionGroup *markerStyleActGrp;
    QActionGroup *markerSizeActGrp;


private slots:
    void setCurveColor();
    void setCurveWidth(QAction *action);
    void setCurveStyle(QAction *action);
    void setCurveMarker(QAction *action);
    void setCurveMarkerSize(QAction *action);
    void deleteCurve();

};

#endif