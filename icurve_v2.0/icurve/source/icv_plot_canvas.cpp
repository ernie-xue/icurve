/* qt header */
#include <qapplication.h>
#include <qevent.h>
#include <QtGlobal>
#include <QLine>
#include <QPolygonF>
#include <QMessageBox>
#include <QColorDialog>
#include <math.h>
/* qwt header */
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_directpainter.h>
/* app header */
#include "icv_plot_canvas.h"
#include "icv_curve_property.h"

#define ICV_TOLERENCE_DISTANCE              (5)


IcvPlotCanvas::IcvPlotCanvas(IcvICurve *parent)
{
    if(NULL == parent )
        return ;

    canvas = static_cast<QwtPlotCanvas *>(parent->getPlot()->canvas());
    canvas->installEventFilter(this);
    canvas->setFocus();
    canvas->setCursor(Qt::ArrowCursor);
    canvas->setMouseTracking(true);

    initCurves();
    curSelectedCurve  = NULL;
    prevSelectedCurve = NULL;

    mainWin = parent;

    isEnableCursorMoveAction = true;
}

IcvPlotCanvas::~IcvPlotCanvas()
{

}


IcvICurve* IcvPlotCanvas::retrieveParent()
{
    return mainWin;
}


QwtPlotCanvas* IcvPlotCanvas::getCanvas()
{
    return canvas;
}


void IcvPlotCanvas::setCanvas(QwtPlotCanvas * cvs)
{
    canvas = cvs;
}


QList<IcvPlotCurve *> IcvPlotCanvas::getCurves()
{
    return curves;
}


void IcvPlotCanvas::setCurves(QList<IcvPlotCurve *> crvs)
{
    curves = crvs;
}


void IcvPlotCanvas::initCurves()
{
    return updateCurves();
}


void IcvPlotCanvas::updateCurves()
{
    QwtPlot *plot = canvas->plot();
    QwtPlotItemList items = plot->itemList();

    //curves.clear();
    for(qint16 i = 0; i < items.count(); i++)
    {
        if((items.value(i))->rtti() == QwtPlotItem::Rtti_PlotCurve)
        {
            IcvPlotCurve *curve = new IcvPlotCurve();
            curve->setCurve(static_cast<QwtPlotCurve *>(items.value(i)));
            curve->setCanvas(this);
            curves.append(curve);
        }
    }
}


void IcvPlotCanvas::lookforCurves()
{
    return updateCurves();
}


void IcvPlotCanvas::appendCurves(IcvPlotCurve *curve)
{
    curves.append(curve);
    return;
}


void IcvPlotCanvas::clearAllCurves()
{
    for(qint16 pos = 0; pos < curves.count(); pos++)
    {
        delete curves[pos];
    }

    curves.clear();
}


IcvPlotCurve* IcvPlotCanvas::getSelectedCurve()
{
    return curSelectedCurve;
}


void IcvPlotCanvas::onMouseLeftButtonClick(const QMouseEvent *event)
{
    /*release magnifier*/
    unlockMagnifier();
    /*enable cursor moving action*/
    isEnableCursorMoveAction = true;

    const QPoint pos = event->pos();
    /*pick the selected curve*/
    for(qint16 i = 0; i < curves.count(); i++)
    {
        QwtPlotCurve *curve = curves[i]->getCurve();
        double dist         = 0.0;
        curve->closestPoint(pos, &dist);

        if(dist < ICV_TOLERENCE_DISTANCE )
        {
            curSelectedCurve = curves[i];
            break;
        }
        else
        {
            curSelectedCurve = NULL;
        }
    }

    /*show markers*/
    if(NULL != curSelectedCurve)
    {
        curSelectedCurve->showMarkers();
        canvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint,false);
    }

    if(NULL != prevSelectedCurve)
    {
        prevSelectedCurve->hideMarkers();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);
    }

    prevSelectedCurve = curSelectedCurve;

    return ;
}


void IcvPlotCanvas::onMouseRightButtonClick(const QMouseEvent * event)
{
   lockMagnifier();

    const QPoint pos = event->pos();
    /*pick the selected curve*/
    for(qint16 i = 0; i < curves.count(); i++)
    {
        QwtPlotCurve *curve = curves[i]->getCurve();
        double dist         = 0.0;
        curve->closestPoint(pos, &dist);

        if(dist < ICV_TOLERENCE_DISTANCE )
        {
            curSelectedCurve = curves[i];
            break;
        }
        else
        {
            curSelectedCurve = NULL;
        }
    }

    /*show markers*/
    if(NULL != prevSelectedCurve)
    {
        prevSelectedCurve->hideMarkers();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);

    }

    if(NULL != curSelectedCurve)
    {
        curSelectedCurve->showMarkers();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);

        isEnableCursorMoveAction = false;
        crvSelPopMenu->exec(event->globalPos());
    }

    prevSelectedCurve = curSelectedCurve;

    return;
}


void IcvPlotCanvas::onMouseMove(const QMouseEvent * event)
{
    if(!isEnableCursorMoveAction)
        return ;

    /*pick the selected curve*/
    if(curves.isEmpty())
    {
        canvas->setCursor(Qt::ArrowCursor);
        return ;
    }


    const QPoint pos = event->pos();
    for(qint16 i = 0; i < curves.count(); i++)
    {
        QwtPlotCurve *curve = curves[i]->getCurve();
        double dist        = 0.0;
        curve->closestPoint(pos, &dist);

        if(dist <= ICV_TOLERENCE_DISTANCE )
        {
            canvas->setCursor(Qt::PointingHandCursor);
            curves[i]->setActivateState(ICV_CURVE_ACTIVATED);
            curSelectedCurve = curves[i];
            break;
        }
        else
        {
            canvas->setCursor(Qt::ArrowCursor);
            curSelectedCurve  = NULL;
        }
    }

    return ;
}


bool IcvPlotCanvas::eventFilter(QObject *object, QEvent *event)
{
    if((NULL == object) || (NULL == event) || (object != canvas))
        return false;

    switch(event->type())
    {
    case QEvent::MouseButtonPress:
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if(mouseEvent->button() == Qt::LeftButton)
            {
                onMouseLeftButtonClick(mouseEvent);
            }
            else if(mouseEvent->button() == Qt::RightButton)
            {
                onMouseRightButtonClick(mouseEvent);
            }
            break;
        }

    case QEvent::MouseMove:
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            onMouseMove(mouseEvent);
            break;
        }
    default:
        break;

    }

    return QObject::eventFilter(object,event);
}


bool IcvPlotCanvas::event(QEvent *eve)
{
    return QObject::event(eve);
}


void IcvPlotCanvas::deleteSelectCurve()
{
    lockMagnifier();

    if(NULL == curSelectedCurve)
        return ;

    QMessageBox msgBox(QMessageBox::Warning, tr("Warning"),
        "Curve will be deleted permanently, are you sure to proceeding?", 0, mainWin);
    msgBox.addButton(tr("Yes"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("No"),  QMessageBox::RejectRole);
    if (msgBox.exec() != QMessageBox::AcceptRole)
    {
        return;
    }

    /*remove from the curves' queue*/
    curves.removeAll(curSelectedCurve); 

    /*remove relative data from QList data repository*/
    QList <Command> *plotData = retrieveParent()->getPlotData();
    qint16 dataPos = curSelectedCurve->getDataPos();
    if(dataPos < plotData->count())
        plotData->removeAt(dataPos);

    /*deconstruct IcvPlotCurve object */
    delete curSelectedCurve;       
    /*memset,important!*/

    curSelectedCurve = NULL;           
    if(canvas !=NULL && canvas->plot() != NULL)
    {
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);
    }

    return ;
}


void IcvPlotCanvas::deleteCurve(IcvPlotCurve *crv)
{
    if(NULL == crv)
        return ;
   
    delete crv;             /*remove from qwtcavas*/
    curves.removeAll(crv);  /*remove from list of curves in the IcvCanvas  */

    return ;
}


void IcvPlotCanvas::setCurveColor()
{
    lockMagnifier();

    QColor color = QColorDialog::getColor(Qt::white,this);
    if(!color.isValid())
        return;

    curSelectedCurve->setColor(color);
    if(NULL != canvas && canvas->plot() != NULL)
    {
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);
    }

    return  ;
}


void IcvPlotCanvas::setCurveWidth(QAction *action)
{
    lockMagnifier();

    if(NULL == action)
        return;

    bool ok;
    qint16 width = action->data().toInt(&ok);
    if(!ok)
        return ;

    curSelectedCurve->setWidth(width);
    if(NULL != canvas && canvas->plot() != NULL)
    {
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);
    }

    action->setCheckable(true);
    action->setChecked(true);

    return ;
}


void IcvPlotCanvas::setCurveStyle(QAction *action)
{
    lockMagnifier();

    if(NULL == action)
        return;

    bool ok;
    Qt::PenStyle style = static_cast<Qt::PenStyle>(action->data().toInt(&ok));
    if(!ok)
        return ;

    curSelectedCurve->setStyle(style);
    if(NULL != canvas && canvas->plot() != NULL)
    {
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);
    }

    action->setCheckable(true);
    action->setChecked(true);

    return ;
}


void IcvPlotCanvas::setCurveMarker(QAction *action)
{
    lockMagnifier();

    if(NULL == action)
        return;

    bool ok;
    QwtSymbol::Style symStyle = static_cast<QwtSymbol::Style>(action->data().toInt(&ok));
    if(!ok)
        return ;

    curSelectedCurve->setMarker(symStyle);
    if(NULL != canvas && canvas->plot() != NULL)
    {
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);
    }

    action->setCheckable(true);
    action->setChecked(true);

    return ;
}


void IcvPlotCanvas::setCurveMarkerSize(QAction *action)
{
    lockMagnifier();

    if(NULL == action)
        return;

    bool ok;
    qint16 size = action->data().toInt(&ok);
    if(!ok)
        return ;

    curSelectedCurve->setMarkerSize(size);
    if(NULL != canvas && canvas->plot() != NULL)
    {
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);
    }

    action->setCheckable(true);
    action->setChecked(true);

    return ;
}


void IcvPlotCanvas::setCurveGroupSize(QAction *action)
{
    lockMagnifier();

    if(NULL == action)
        return;
   
    bool isConvertSuccess;
    qint16 size = action->data().toInt(&isConvertSuccess);
    if(!isConvertSuccess)
        return;

    curSelectedCurve->setGroupSize(size);

    if(NULL != canvas && canvas->plot() != NULL)
    {
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,true);
        canvas->plot()->replot();
        canvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint,false);
    }

    action->setCheckable(true);
    action->setChecked(true);

    return;
}


void IcvPlotCanvas::setCurveProperty()
{
    lockMagnifier();

    if(NULL == propertySetAction)
        return ;

    IcvCurvePropertyDialog *propDiag = new IcvCurvePropertyDialog(curSelectedCurve,mainWin,Qt::Dialog);
    propDiag->resize(400,150);
    propDiag->show();

    return;
}


void IcvPlotCanvas::lockMagnifier()
{
    if(NULL != mainWin)
        mainWin->getMagnifier()->setEnabled(false);

    return ;
}


void IcvPlotCanvas::unlockMagnifier()
{
    if(NULL != mainWin)
        mainWin->getMagnifier()->setEnabled(true);

    return ;
}


void IcvPlotCanvas::createCurvePopMenu()
{
    QWidget * parent  = this;

    crvSelPopMenu = new QMenu(parent);
    crvSelPopMenu->addAction(delAction);
    crvSelPopMenu->addAction(colorSetAction);

    subCrvSelWidthMenu = new QMenu("width",parent);
    subCrvSelWidthMenu->addActions(widthActGrp->actions());
    crvSelPopMenu->addMenu(subCrvSelWidthMenu);

    subCrvSelStyleMenu = new QMenu("style",this);
    subCrvSelStyleMenu->addActions(styleActGrp->actions());
    crvSelPopMenu->addMenu(subCrvSelStyleMenu);

    subCrvSelMarkeStyleMenu = new QMenu("Marker",parent);
    subCrvSelMarkeStyleMenu->addActions(markerStyleActGrp->actions());
    crvSelPopMenu->addMenu(subCrvSelMarkeStyleMenu);

    subCrvSelMarkerSizeMenu = new QMenu("Marker size",parent);
    subCrvSelMarkerSizeMenu->addActions(markerSizeActGrp->actions());
    crvSelPopMenu->addMenu(subCrvSelMarkerSizeMenu);

    subCrvSelGroupSizeMenu = new QMenu("Group size",parent);
    subCrvSelGroupSizeMenu->addActions(groupSizeActGrp->actions());
    crvSelPopMenu->addMenu(subCrvSelGroupSizeMenu);

    crvSelPopMenu->addAction(propertySetAction);


    return ;
}


void IcvPlotCanvas::createCurvePopMenuAction()
{
    QWidget * parent  = this;
    QAction *action = NULL;

    /*delete the curve*/
    delAction = new QAction(QIcon(":/images/delete.png"),tr("Delete"),parent);
    delAction->setShortcut(QKeySequence::Delete);
    delAction->setStatusTip("curve delete");
    connect(delAction,SIGNAL(triggered()),this,SLOT(deleteSelectCurve()));

    /*color the curve*/
    colorSetAction = new QAction(tr("Color..."),parent);
    colorSetAction->setStatusTip("curve set color");
    connect(colorSetAction,SIGNAL(triggered()),this,SLOT(setCurveColor()));

    /*{{{set width action*/
    widthActGrp = new QActionGroup(parent);
    action      = new QAction(widthActGrp);
    action->setText(QString::number(0.5));
    action->setData(0.5);
    for(qint8 i = 1; i< 11; i++)  
    {
        action = new QAction(widthActGrp);
        action->setText(QString::number(i));
        action->setData(i);
    }

    connect(widthActGrp,SIGNAL(triggered(QAction *)),this,SLOT(setCurveWidth(QAction *)));

    /*{{{curve subCrvSelStyleMenu action*/
    QList < QPair<QString,int> > styleList;
    styleList.append(qMakePair(tr("Solid"),1));
    styleList.append(qMakePair(tr("Dash"),2));
    styleList.append(qMakePair(tr("Dot"),3));
    styleList.append(qMakePair(tr("DashDot"),4));
    styleList.append(qMakePair(tr("DashDotDot"),5));

    styleActGrp = new QActionGroup(parent);
    for(qint16 i = 0; i < styleList.count(); i++)
    {
        action = new QAction(styleActGrp);
        action->setText((styleList.at(i).first));
        action->setData((styleList.at(i).second));

    }
    connect(styleActGrp,SIGNAL(triggered(QAction *)),this,SLOT(setCurveStyle(QAction *)));
    /*}}}*/

    /*{{{curve marker action*/
    QList< QPair<QString,int> > markerList;
    markerList.append(qMakePair(tr("None"),-1));
    markerList.append(qMakePair(tr("Ellipse"),0));
    markerList.append(qMakePair(tr("Rect"),1));
    markerList.append(qMakePair(tr("Diamond"),2));
    markerList.append(qMakePair(tr("Triangle"),3));
    markerList.append(qMakePair(tr("DTriangle"),4));
    markerList.append(qMakePair(tr("UTriangle"),5));
    markerList.append(qMakePair(tr("LTriangle"),6));
    markerList.append(qMakePair(tr("RTriangle"),7));
    markerList.append(qMakePair(tr("Cross(+)"),8));
    markerList.append(qMakePair(tr("XCross(x)"),9));
    markerList.append(qMakePair(tr("HLine"),10));
    markerList.append(qMakePair(tr("VLine"),11));
    markerList.append(qMakePair(tr("Star1"),12));
    markerList.append(qMakePair(tr("Star2"),13));
    markerList.append(qMakePair(tr("Hexagon"),14));

    markerStyleActGrp = new QActionGroup(parent);
    QList <QPair<QString,int> >::iterator m;
    for(m = markerList.begin(); m < markerList.end(); m++)
    {
        action = new QAction(markerStyleActGrp);
        action->setText((m->first));
        action->setData((m->second));
    }
    connect(markerStyleActGrp,SIGNAL(triggered(QAction *)),this,SLOT(setCurveMarker(QAction *)));
    /*}}}*/


    /*{{{set curve marker size action*/
    QList< QPair<QString,int> > markerSize;
    markerSize.append(qMakePair(tr("2"),2));
    markerSize.append(qMakePair(tr("4"),4));
    markerSize.append(qMakePair(tr("5"),5));
    markerSize.append(qMakePair(tr("6"),6));
    markerSize.append(qMakePair(tr("7"),7));
    markerSize.append(qMakePair(tr("8"),8));
    markerSize.append(qMakePair(tr("10"),10));
    markerSize.append(qMakePair(tr("12"),12));
    markerSize.append(qMakePair(tr("18"),18));
    markerSize.append(qMakePair(tr("24"),24));
    markerSize.append(qMakePair(tr("48"),48));

    markerSizeActGrp = new QActionGroup(parent);
    QList <QPair<QString,int> >::iterator t;
    for(t = markerSize.begin(); t < markerSize.end(); t++)
    {
        action = new QAction(markerSizeActGrp);
        action->setText((t->first));
        action->setData((t->second));
    }
    connect(markerSizeActGrp,SIGNAL(triggered(QAction *)),this,SLOT(setCurveMarkerSize(QAction *)));
    /*}}}*/


    /*{{{set curve group size, to expand curve*/
    QList< QPair<QString,int> > groupSize;
    groupSize.append(qMakePair(tr("2"), 2));
    groupSize.append(qMakePair(tr("4"), 4));
    groupSize.append(qMakePair(tr("6"), 6));
    groupSize.append(qMakePair(tr("8"), 8));
    groupSize.append(qMakePair(tr("10"),10));

    groupSizeActGrp = new QActionGroup(parent);
    QList <QPair<QString,int> >::iterator gsize;
    for(gsize = groupSize.begin(); gsize < groupSize.end(); gsize++)
    {
        action = new QAction(groupSizeActGrp);
        action->setText((gsize->first));
        action->setData((gsize->second));
    }
    connect(groupSizeActGrp, SIGNAL(triggered(QAction *)),this,SLOT(setCurveGroupSize(QAction *)));

    propertySetAction = new QAction(tr("Properties..."),this);
    propertySetAction->setStatusTip("set curve properties");
    connect(propertySetAction,SIGNAL(triggered()),this,SLOT(setCurveProperty()));

    return ;
}


