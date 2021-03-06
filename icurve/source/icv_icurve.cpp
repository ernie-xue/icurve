﻿#include <Qt>
#include <QObject>
#include <QtGui>
#include <QDialog>  
#include <QMessageBox>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QDir>
#include <QIcon>
#include <QPair>
#include <QList>
#include <QSize>
#include <QRegExp>
#include <QComboBox>
#include <QDebug>
#include <QVariant>
#include <QMimeData>
#include <QSettings>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableWidget>
#include <QHeaderView>
#include <QPixmap>
#include <QtDebug>
#include <iostream>     // std::cout  
#include <functional>   // std::minus  
#include <numeric>      // std::accumulate  
#include <iostream>     // std::cout
#include <vector>       // std::vector, std::begin, std::end
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>    // std::for_each
#include <vector>       // std::vector
#include <qwt_legend.h>
#include <qwt_plot_renderer.h>
#include <qwt_legend_label.h>
#include <qwt_plot_item.h>
#include "icv_icurve.h"
#include "icv_curve_filter.h"
#include "icv_penstyle.h"
#include "icv_curve_property.h"
#include "icv_marker_property.h"
#include "icv_clipboard.h"
#include "icv_symbol.h"
#include "icv_curve_info.h"
#include "icv_axse_scale.h"
#include "icv_about.h"
#include "icv_skin.h"
#include "icv_data_plot.h"
#include "icv_textedit.h"
#include "icv_table_view.h"
#include "icv_table_model.h"

/*including tone index at head of the line*/
#define ICV_MAX_NUM_DIGITS_PERLINE           (11)  
#define ICV_PLOT_DATA_START_POS              (1)
#define ICV_MAX_LINE_NUM_BACKGROUD_PROCESS   (2000)
#define ICV_MAX_CURVE_NUM_BACKGROUND_PROCESS (50)
#define ICV_MAX_RECENT_FILE_NUM              (5)
#define ICV_MAX_ACCEPT_FILE_SIZE             (300000000)  /* 300M */
#define ICV_MAX_VIVID_COLOR_NUM              (20)

static  QString icvRgbColors[ICV_MAX_VIVID_COLOR_NUM]=
{
    "#FF0000", //red
    "#0000FF", //blue
    "#008000", //gree
    "#FF4500",
    "#D2691E",
    "#556B2F",
    "#9400D3",
    "#912CEE",
    "#68228B",
    "#4876FF",
    "#00BFFF",
    "#008B8B",
    "#7FFFAA",
    "#2E8B57",
    "#00FF00",
    "#FFD700",
    "#A52A2A",
    "#DAA520",
    "#2F4F4F",
    "#B03060"
};


IcvICurve::IcvICurve(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags)
{
    ui.setupUi(this);

    createExtraActions();
    createExtraMenus();

    initMainWinStyle(this);

    plot = new QwtPlot();
    initMainPlotter(plot);

    QVBoxLayout *centralLayout = new QVBoxLayout(this);
    centralLayout->addWidget(plot);
    centralLayout->setContentsMargins(10,10,30,10);

    QWidget *widget = new QWidget();
    widget->setLayout(centralLayout);
    setCentralWidget(widget);

    plotCanvas = new IcvPlotCanvas(this) ;
    plotCanvas->createCurvePopMenuAction();
    plotCanvas->createCurvePopMenu();

    analyProgressDialog  = NULL;
    differTool           = NULL;
    isDataAnalyCanceled  = false;

    /*{{{signals and slots*/
    connect(ui.actionOpen,           SIGNAL(triggered()),     this, SLOT(openFile()));
    connect(ui.actionNew,            SIGNAL(triggered()),     this, SLOT(newFile()));
    connect(ui.actionSaveAs,         SIGNAL(triggered()),     this, SLOT(saveAs()));
    connect(ui.actionExport,         SIGNAL(triggered()),     this, SLOT(exportData()));
    connect(ui.actionClose,          SIGNAL(triggered()),     this, SLOT(closePlot()));
    connect(ui.actionExit,           SIGNAL(triggered()),     this, SLOT(close()));
    /* edit menu */
    connect(ui.actionRefresh,        SIGNAL(triggered()),     this, SLOT(refreshPlot()));
    connect(ui.actionCut,            SIGNAL(triggered()),     this, SLOT(cutCurve()));
    connect(ui.actionCopy,           SIGNAL(triggered()),     this, SLOT(copyCurve()));
    connect(ui.actionPaste,          SIGNAL(triggered()),     this, SLOT(pasteCurve()));
    connect(ui.actionRemove,         SIGNAL(triggered()),     this, SLOT(removeCurves()));
    connect(ui.actionHide,           SIGNAL(triggered()),     this, SLOT(hideCurves()));
    connect(ui.actionShow,           SIGNAL(triggered()),     this, SLOT(showCurves()));
    connect(ui.actionDelete,         SIGNAL(triggered()),     this, SLOT(deleteCurves()));
    connect(ui.actionDeleteAll,      SIGNAL(triggered()),     this, SLOT(deleteCurvesAll()));
    connect(ui.actionFind,           SIGNAL(triggered()),     this, SLOT(findCurve()));
    connect(ui.actionShowAll,        SIGNAL(triggered()),     this, SLOT(showAllCurve()));
    connect(ui.actionSelectAll,      SIGNAL(triggered()),     this, SLOT(selectAllCurves()));  
    connect(ui.actionSelectInvert,   SIGNAL(triggered()),     this, SLOT(selectInvertCurves()));  
    /* curve menu */
    connect(ui.actionColor,          SIGNAL(triggered()),     this, SLOT(setCurveColor()));
    connect(ui.actionWidth,          SIGNAL(triggered()),     this, SLOT(setCurveWidth()));
    connect(ui.actionStyle,          SIGNAL(triggered()),     this, SLOT(setCurveStyle()));
    connect(ui.actionMarker,         SIGNAL(triggered()),     this, SLOT(setCurveMarker()));
    connect(ui.actionExpand,         SIGNAL(triggered()),     this, SLOT(expandCurve()));
    connect(ui.actionRepaint,        SIGNAL(triggered()),     this, SLOT(repaintCurve()));
    connect(ui.actionFilter,         SIGNAL(triggered()),     this, SLOT(filterCurve()));
    connect(ui.actionViewData,       SIGNAL(triggered()),     this, SLOT(viewCurveData()));
    connect(ui.actionViewStat,       SIGNAL(triggered()),     this, SLOT(viewCurveStat()));
    connect(ui.actionJumpToFile,     SIGNAL(triggered()),     this, SLOT(jumpToFilePos()));
    connect(ui.actionInfo,           SIGNAL(triggered()),     this, SLOT(showCurveInfo()));
    connect(ui.actionCurveProperties,SIGNAL(triggered()),     this, SLOT(setCurveProperties()));
    /* axse menu */
    connect(ui.actionAxseScale,      SIGNAL(triggered()),     this, SLOT(setAxseScale()));
    connect(ui.actionAxseTitle,      SIGNAL(triggered()),     this, SLOT(setAxseTitle()));
    connect(ui.actionAxseAlignment,  SIGNAL(triggered()),     this, SLOT(setAxseAlignment()));
    connect(ui.actionAxseRotation,   SIGNAL(triggered()),     this, SLOT(setAxseRotation()));
    /* insert menu */
    connect(ui.actionTitle,          SIGNAL(triggered()),     this, SLOT(insertTitle()));
    connect(ui.actionX_label,        SIGNAL(triggered()),     this, SLOT(insertXLabel()));
    connect(ui.actionY_label,        SIGNAL(triggered()),     this, SLOT(insertYLabel()));
    connect(ui.actionLegend,         SIGNAL(triggered()),     this, SLOT(insertLegend()));
    connect(ui.actionCurveName,      SIGNAL(triggered()),     this, SLOT(insertCurveName()));
    connect(ui.actionFooter,         SIGNAL(triggered()),     this, SLOT(insertFooter()));
    connect(ui.actionIndicator,      SIGNAL(triggered()),     this, SLOT(insertIndicator()));
    /* view menu */
    connect(ui.actionZoom,           SIGNAL(triggered(bool)), this, SLOT(enableZoomer(bool)));
    /* tool menu */
    connect(ui.actionHandMove,       SIGNAL(triggered(bool)), this, SLOT(enableHandMove(bool)));
    connect(ui.actionDiffer,         SIGNAL(triggered()),     this, SLOT(diffCurves()));
    connect(ui.actionCliParser,      SIGNAL(triggered()),     this, SLOT(parseCliData()));
    connect(ui.actionOneKeySetter,   SIGNAL(triggered()),     this, SLOT(oneKeySetPlot()));
    connect(ui.actionRegularExp,     SIGNAL(triggered()),     this, SLOT(setRegularExp()));
    
    /* help menu */
    connect(ui.actionAbout,          SIGNAL(triggered()),     this, SLOT(aboutIcurve()));
    /* others */
    connect(this, SIGNAL(analyDataProgress(qint32)), this, SLOT(updateAnalyProgressBar(qint32)));
    /*}}}*/

}

IcvICurve::~IcvICurve()
{

}

void IcvICurve::initMainWinStyle(QMainWindow *self)
{
    self->setWindowTitle("iCurve");
    self->setWindowIcon(QIcon(":/icurve/images/icurve.png"));
    self->setContentsMargins(0,0,0,0);
    self->statusBar()->showMessage("ready");
    IcvSkin skin;
    self->setStyleSheet(skin.GetSkinCss());
    QDesktopWidget* desktop = QApplication::desktop();
    move((desktop->width() - this->width())/2, (desktop->height() - this->height())/3);
    /* shortcuts */
    ui.actionOpen->setShortcut(QKeySequence::Open);
    ui.actionSaveAs->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    ui.actionPaste->setShortcut(QKeySequence::Paste); 
    ui.actionFilter->setShortcut(QKeySequence::Find);
    ui.actionSelectAll->setShortcut(QKeySequence::SelectAll);
    ui.actionSelectInvert->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_A);
    ui.actionShowAll->setShortcut(Qt::CTRL + Qt::Key_K);
    ui.actionRemove->setShortcut(QKeySequence::Delete);
    ui.actionDelete->setShortcut(Qt::CTRL + Qt::Key_Delete);
    ui.actionDeleteAll->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete);
    ui.actionRefresh->setShortcut(QKeySequence::Refresh);
    ui.actionExpand->setShortcut(Qt::CTRL + Qt::Key_G);
    ui.actionInfo->setShortcut(Qt::CTRL + Qt::Key_I);
    ui.actionCurveProperties->setShortcut(Qt::CTRL + Qt::Key_P);
    ui.actionDiffer->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_D);
    ui.actionCliParser->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_P);
    setAcceptDrops(true);
    return ;
}

void IcvICurve::initMainPlotter(QWidget *plotWidget)
{
    QwtPlot *plot = static_cast<QwtPlot*>(plotWidget);
    plot->setCanvasBackground( Qt::white );
    plot->setAxisScale( QwtPlot::yLeft, -150.0, 150.0 );
    plot->setAxisScale( QwtPlot::xBottom, 0, 3000 );
    plot->setAxisLabelAlignment(QwtPlot::xBottom,Qt::AlignLeft);
    plot->setAxisLabelAlignment(QwtPlot::yLeft,Qt::AlignTop);
    plot->plotLayout()->setAlignCanvasToScales( false );
    plot->plotLayout()->setCanvasMargin(0);
    /* legendItem, default curve title is displayed */
    legendItem = new QwtPlotLegendItem();  
    legendItem->attach(plot);
    /* legend */
    legend = new QwtLegend;
    legend->setDefaultItemMode( QwtLegendData::Checkable );
    connect(legend, SIGNAL(checked(const QVariant &, bool, int)), this,
        SLOT(legendChecked( const QVariant &, bool)));
    /* grid */
    grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(Qt::DashLine));
    grid->setMinorPen(QPen(Qt::DashLine));  
    grid->attach( plot );
    /* magnifier */
    magnifier = new QwtPlotMagnifier(plot->canvas());
    magnifier->setEnabled(true);
    /* panner */
    panner = new QwtPlotPanner(plot->canvas());
    panner->setEnabled(true);
    /* picker */
    picker = new IcvPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                               QwtPlotPicker::CrossRubberBand, 
                               QwtPicker::AlwaysOn,
                               plot->canvas());
    /* zoomer */
    zoomer = new IcvPlotZoomer(plot->canvas());
    connect(zoomer, SIGNAL(zoomed(const QRectF&)), this, SLOT(zoomPlot(QRectF&)));
    return;
}

void IcvICurve::createExtraActions()
{
    /* create extra actions */
    for (qint16 cnt = 0; cnt < ICV_MAX_RECENT_FILE_NUM; cnt++)
    {
        QAction* action = new QAction(this);
        action->setVisible(false);
        connect(action, SIGNAL(triggered()),this, SLOT(openRecentFile()));
        recentFileActs.append(action);
    }
    return;
}

void IcvICurve::createExtraMenus()
{
    /* create extra menus*/
    for (qint16 cnt = 0; cnt < ICV_MAX_RECENT_FILE_NUM; cnt++)
        ui.menuFile->addAction(recentFileActs[cnt]);
    updateRecentFileActions();
    return;
}

void IcvICurve::openFile()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
            tr("Open file..."),
            fileInfo.absolutePath(),
            tr("Text files (*.txt *.log *.cap *.bak);;All Files (*)"));
    if(fileNames.isEmpty())
    {
        return; 
    }
    loadFile(fileNames);
    return;
}

void IcvICurve::newFile()
{
   IcvDataPlotDialog *dataPlotDlg = new IcvDataPlotDialog(this);
   dataPlotDlg->setWindowTitle("new data plot");
   dataPlotDlg->show();
   return;
}

void IcvICurve::plotBlockData(QString data)
{
    QTextStream stream(&data);
    qint16 posCurRepository = plotData.count();
    analyzeTextStream(stream, "block");
    for(qint16 pos = posCurRepository; pos < plotData.count(); pos++)
    {           
        QwtPlotCurve *qwtCurve = new QwtPlotCurve();
        qwtCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
        if(pos < ICV_MAX_VIVID_COLOR_NUM)
            qwtCurve->setPen(QColor(icvRgbColors[pos]), 1.0, Qt::SolidLine);
        else
            qwtCurve->setPen(QColor::fromHsl(rand()%360,rand()%256,rand()%100), 1.0, Qt::SolidLine);
        qwtCurve->setSamples(plotData[pos].getData().toVector());
        QwtSymbol *symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(Qt::yellow),
            QPen(Qt::red, 2), QSize(2, 2));
        qwtCurve->setSymbol(symbol);
        qwtCurve->setTitle(plotData.value(pos).getTitle());
        qwtCurve->setStyle(QwtPlotCurve::Lines);
        qwtCurve->attach(plot);

        IcvPlotCurve *plotCurve = new IcvPlotCurve;
        plotCurve->setCurve(qwtCurve);
        plotCurve->setCanvas(plotCanvas);
        plotCurve->setDataPos(pos);
        plotCurve->setCommand(plotData[pos]);
        plotCurve->setAttachedState(true);
        /* attach curves to plot canvas*/
        plotCanvas->appendCurves(plotCurve);
        /* 50 curves plotted, delay 50ms to handle the other events */
        if(0 == pos % 50)
            taskDelay(50);
    }
    plot->replot();
    return;
}

void IcvICurve::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;

    QString fileName = action->data().toString();
    /* save file info */
    fileInfo.setFile(fileName);
    /* load files */
    QStringList fileNames;
    fileNames.append(fileName);
    loadFile(fileNames);
    /* append file to recent file list */
    setCurrentFile(fileName);
    return;
}

void IcvICurve::loadFile(QStringList fileNames)
{
    /* before loading new data, get data postion for a new file */
    qint16 posCurRepository = plotData.count();
    for(qint16 fileCnt = 0; fileCnt < fileNames.count(); fileCnt++)
    {
        /* parse and analyze file content */
        if(ICU_OK != loadData(fileNames[fileCnt]))
            continue;
        /* append file to recent file list */
        setCurrentFile(fileNames[fileCnt]);
    }
    QProgressDialog *plotProgressDialog = NULL;
    if(plotData.count() > ICV_MAX_CURVE_NUM_BACKGROUND_PROCESS)
    {
        QString labelText = "total " + QString::number(plotData.count()) + " curves foud, plotting...";
        plotProgressDialog = createIcvProgressDiag(plot, posCurRepository, plotData.count(), 
            "plotting progress", labelText, QSize(300,100), true);
        plotProgressDialog->show();
    }
    /* when a new file exported, should not start from scratch */
    for(qint16 pos = posCurRepository; pos < plotData.count(); pos++)
    {           
        QwtPlotCurve *qwtCurve = new QwtPlotCurve();
        if(pos < ICV_MAX_VIVID_COLOR_NUM)
            qwtCurve->setPen(QColor(icvRgbColors[pos]), 1.0, Qt::SolidLine);
        else
            qwtCurve->setPen(QColor::fromHsl(rand()%360,rand()%256,rand()%150), 1.0, Qt::SolidLine);
        qwtCurve->setSamples(plotData[pos].getData().toVector());

        QwtSymbol *symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(Qt::yellow),
            QPen(Qt::red, 2), QSize(2, 2));
        qwtCurve->setSymbol(symbol);
        qwtCurve->setTitle(plotData.value(pos).getTitle());
        qwtCurve->setStyle(QwtPlotCurve::Lines);
        qwtCurve->attach(plot);

        IcvPlotCurve *plotCurve = new IcvPlotCurve;
        plotCurve->setCurve(qwtCurve);
        plotCurve->setCanvas(plotCanvas);
        plotCurve->setDataPos(pos);
        plotCurve->setCommand(plotData[pos]);
        plotCurve->setAttachedState(true);
        /* attach curves to plot canvas*/
        plotCanvas->appendCurves(plotCurve);
        /* plotting progress */
        if(plotProgressDialog != NULL)
        {
            plotProgressDialog->setValue(pos + 1);
            plotProgressDialog->repaint();  
        }
        /* 50 curves plotted, delay 50ms to handle the other events */
        if(0 == pos % 50)
            taskDelay(50);
    }
    plot->replot();
    if(plotProgressDialog != NULL)
        delete plotProgressDialog;
    updateStatusBar();
    return;
}

void IcvICurve::setCurrentFile(const QString &fileName)
{
    QSettings settings("icv_history.ini",QSettings::IniFormat);;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > ICV_MAX_RECENT_FILE_NUM)
    {
        files.removeLast();
    }

    settings.setValue("recentFileList", files);
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
        IcvICurve *mainWin = qobject_cast<IcvICurve *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
    return;
}

void IcvICurve::saveAs()
{
    QwtPlotRenderer renderer;
    QString plotTitle;
    if(plot != NULL)
        plotTitle = plot->title().text();
    else
        plotTitle = "";
    renderer.exportTo(plot, plotTitle);
    return;
}

void IcvICurve::exportData()
{
    QFile file;
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save File"), "icurve", tr("csv files(*.csv);;text files(*.txt);;excel(*.xsl *.xlsx)"));
    if (fileName.isNull())
    {
        return;         
    }
    file.setFileName(fileName);  
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {    
        QMessageBox::critical(NULL, "error", "unable to create file!");  
        file.close();    
        return;    
    }    

    QProgressDialog *progress = createIcvProgressDiag(this, 0, plotData.count(),
        "progress", "exporting...", QSize(300,100), true);
    progress->show();

    QString dataContent;
    QString dataTitle;
    dataTitle = tr("title,") + tr("file,") + tr("position,") + tr("brief,")+
                tr("name,")+tr("lineId,")+tr("direction,") + tr("group size,");
    qint32 maxToneNum = 0;
    for(qint32 row = 0; row < plotData.count(); row++)
    {
        IcvCommand cmd = plotData.value(row);
        dataContent += cmd.getTitle() + ",";
        dataContent += cmd.getFileName() + ",";
        dataContent += QString::number(cmd.getDataPosInFile()) + ",";
        dataContent += cmd.getBriefInfo().replace(QChar(','), QChar(' ')) + ",";
        dataContent += cmd.getName() + ",";
        dataContent += QString::number(cmd.getLineId()) + ",";
        dataContent += ((cmd.getDirection() == 0)? tr("US"):tr("DS")) + ",";
        dataContent += QString::number(cmd.getGroupSize()) + ",";
        QList<QPointF> data = plotData.value(row).getData();
        for(qint32 tone = 0; tone < data.count(); tone++)
            dataContent += QString::number(data.at(tone).y()) + " ,";
        dataContent += "\n";
        /* find maximum tones in all curves */
        if(maxToneNum < data.count())
            maxToneNum = data.count();
        progress->setValue(row+1);
    }   
    for(qint32 i = 0; i < maxToneNum; i++)
    {
        dataTitle += "tone " + QString::number(i) + ",";
    }
    dataTitle += "\n";

    QTextStream stream(&file);    
    stream << dataTitle << dataContent << endl;    
    stream.flush();    
    file.close();    
    delete progress;
    return;
}

void IcvICurve::closePlot()
{
    delete plot;
    plot = NULL;
    return;
}

void IcvICurve::insertTitle()
{
    bool ok;
    QString origTitle = plot->title().text();
    QString      text = QInputDialog::getText(this, tr("Input"), tr("Plot title:"),
                                              QLineEdit::Normal, origTitle, &ok);
    if (ok)
        plot->setTitle(text);
    return;
}

void IcvICurve::insertXLabel()
{
    bool ok;
    QString origLabel = plot->axisTitle(QwtPlot::xBottom).text();
    QString      text = QInputDialog::getText(this, tr("Input"),
        tr("X label:"), QLineEdit::Normal, origLabel, &ok);
    if (ok)
        plot->setAxisTitle(QwtPlot::xBottom,text);
    return;
}

void IcvICurve::insertYLabel()
{
    bool ok;
    QString origLabel = plot->axisTitle(QwtPlot::yLeft).text();
    QString      text = QInputDialog::getText(this, tr("Input"), tr("Y label:"),
                                              QLineEdit::Normal, origLabel, &ok);
    if (ok)
        plot->setAxisTitle(QwtPlot::yLeft,text);
    return ;
}

void IcvICurve::insertLegend()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        ui.actionLegend->setChecked(false);
        return ;
    }
    if(true == ui.actionLegend->isChecked())
    {
        legend = new QwtLegend(plot);
        legend->setDefaultItemMode( QwtLegendData::Checkable );
        connect(legend, SIGNAL(checked(const QVariant &, bool, int)), this,
            SLOT(legendChecked( const QVariant &, bool)));

        plot->insertLegend(legend, QwtPlot::RightLegend );
        ui.actionLegend->setChecked(true);
    }
    else
    {
        plot->insertLegend(NULL);
        /* qwtplot will delete legend internally */
        legend = NULL;
        ui.actionLegend->setChecked(false);
    }
    return;
}

void IcvICurve::insertCurveName()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        ui.actionCurveName->setChecked(false);
        return ;
    }
    if(true == ui.actionCurveName->isChecked())
    {
        legendItem->attach(plot);
        ui.actionCurveName->setChecked(true);
    }
    else
    {
        legendItem->detach();
        ui.actionCurveName->setChecked(false);
    }
    plot->replot();
    return;
}

void IcvICurve::insertFooter()
{
    bool ok;
    QString origTitle = plot->footer().text();
    QString text = QInputDialog::getText(this, tr("Input"),
        tr("Footer"), QLineEdit::Normal, origTitle, &ok);
    if (ok)
        plot->setFooter(text);
    return;
}

void IcvICurve::insertIndicator()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        ui.actionIndicator->setChecked(false);
        return ;
    }
    QList<IcvPlotCurve *>selectedCurves = plotCanvas->getSelectedCurve();
    QList<IcvPlotCurve *>curves = !selectedCurves.isEmpty()? selectedCurves : allCurves;
    if(true == ui.actionIndicator->isChecked())
    {
        for(qint16 cnt = 0; cnt < curves.count(); cnt++)
        {
            if(!curves.at(cnt)->isAttached())
                continue;
            QList<QwtPlotMarker *>indicators = curves.at(cnt)->getIndicators();
            if(!indicators.isEmpty())
                continue;
            /* if not indicator attached to the curve, create and attach*/
            QwtPlotCurve *qwtCurve = curves.at(cnt)->getCurve();
            QList<QPointF> posList;
            for(qint16 posX = 0; posX < curves.at(cnt)->getCurve()->dataSize(); posX++)
            {
                if(qwtCurve->sample(posX).ry() < qwtCurve->maxYValue()&&
                   qwtCurve->sample(posX).ry() > qwtCurve->minYValue())
                {
                    QPointF pos;
                    pos.setX(posX);
                    pos.setY(qwtCurve->sample(posX).ry());
                    posList.append(pos);
                }
            }
            if(posList.isEmpty())
                continue;
            QPointF randPos = posList.at(rand() % posList.count());
            QString promt = curves.at(cnt)->getCommand().getPromt();
            QString title = curves.at(cnt)->getCurve()->title().text();
            QString file  = curves.at(cnt)->getCommand().getFileName();
            QwtPlotMarker *marker = new QwtPlotMarker();
            marker->setRenderHint( QwtPlotItem::RenderAntialiased, true );
            marker->setItemAttribute( QwtPlotItem::Legend, false );
            marker->setSymbol(new IcvSymbol(IcvSymbol::Arrow, qwtCurve->pen().color()));
            marker->setValue(randPos);
            QwtText label = QwtText(promt + "." + title + "@" + (file.length() > 10 ? file.left(10) + "..." : file));
            label.setFont(QFont("Times", 10, QFont::Bold));
            label.setColor(qwtCurve->pen().color());
            marker->setLabel(label);
            marker->setLabelAlignment( Qt::AlignRight | Qt::AlignTop);
            marker->attach(plot);
            QList<QwtPlotMarker *>markers;
            markers.append(marker);
            curves.at(cnt)->setIndicator(markers);
            ui.actionIndicator->setChecked(true);
        }
    }
    else
    {
        for(qint16 cnt = 0; cnt < curves.count(); cnt++)
        {
            curves.at(cnt)->deleteIndicator();
        }
        ui.actionIndicator->setChecked(false);
    }
    plot->replot();
    return;
}

void IcvICurve::setCurveColor()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *>selectedCurve = plotCanvas->getSelectedCurve();
    if(0 == selectedCurve.count())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return;
    }
    QColor color = QColorDialog::getColor(Qt::white,this);
    if(!color.isValid())
        return;

    for(qint16 cnt = 0; cnt < selectedCurve.count(); cnt++)
    {
        selectedCurve.at(cnt)->setColor(color);
    }
    plot->replot();
    return;
}

void IcvICurve::setCurveWidth()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> selectedCurve = plotCanvas->getSelectedCurve();
    if(0 == selectedCurve.count())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    bool ok;
    qint16 width = QInputDialog::getInt(this, tr("Input"),tr("Group size:"), 1, 1, 20, 1, &ok);
    for(qint16 cnt = 0; cnt < selectedCurve.count(); cnt++)
    {
        selectedCurve.at(cnt)->setWidth(width);
    }
    return;
}

void IcvICurve::setCurveStyle()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return;
    }
    QList<IcvPlotCurve *> selectedCurve = plotCanvas->getSelectedCurve();
    if(0 == selectedCurve.count())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return;
    }

    QDialog *curveStyleDlg = new QDialog(this);
    QLabel *titleLabel = new QLabel(curveStyleDlg);
    titleLabel->setText("style:");
    QComboBox *styleComboBox = new QComboBox(curveStyleDlg);
    styleComboBox->setObjectName("styleComboBox");
    styleComboBox->setItemDelegate(new IcvPenStyleDelegate((QObject *)styleComboBox));
    styleComboBox->addItem(tr("Solid"),     Qt::SolidLine);
    styleComboBox->addItem(tr("Dash"),      Qt::DashDotLine);
    styleComboBox->addItem(tr("Dot"),       Qt::DotLine);
    styleComboBox->addItem(tr("DashDot"),   Qt::DashDotLine);
    styleComboBox->addItem(tr("DashDotDot"),Qt::DashDotDotLine);
    QDialogButtonBox *btnBox = new QDialogButtonBox(curveStyleDlg);
    btnBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(btnBox,SIGNAL(accepted()),curveStyleDlg,SLOT(accept()));
    connect(btnBox,SIGNAL(rejected()),curveStyleDlg,SLOT(reject()));

    QGridLayout *gridLayout  = new QGridLayout(curveStyleDlg);  
    gridLayout->addWidget(titleLabel,0,0);  
    gridLayout->addWidget(styleComboBox,0,1);  
    gridLayout->addWidget(btnBox,1,1);  

    qint16 retcode = (qint16)curveStyleDlg->exec();
    if(QDialog::Accepted == retcode)
    {
        QComboBox *child = curveStyleDlg->findChild<QComboBox *>("styleComboBox");
        Qt::PenStyle style = static_cast<Qt::PenStyle>(child->currentIndex() + 1);

        for(qint16 cnt = 0; cnt < selectedCurve.count(); cnt++)
        {
            selectedCurve.at(cnt)->setStyle(style);
        }
    }
    plot->replot();
    delete curveStyleDlg;
    curveStyleDlg = NULL;
    return;
}

void IcvICurve::setCurveMarker()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> selectedCurve = plotCanvas->getSelectedCurve();
    if(0 == selectedCurve.count())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }
    IcvMarkerPropertyDialog *markrPropDlg = new IcvMarkerPropertyDialog(this);
    qint16 retcode = (qint16)markrPropDlg->exec();
    if(QDialog::Accepted == retcode)
    {
        QComboBox *childTypeComboBox = markrPropDlg->findChild<QComboBox *>("typeComboBox");
        QwtSymbol::Style style = static_cast<QwtSymbol::Style>(childTypeComboBox->currentIndex());
        for(qint16 cnt = 0; cnt < selectedCurve.count(); cnt++)
        {
            selectedCurve.at(cnt)->setMarker(style);

            QLineEdit *childSizeLineEdit = markrPropDlg->findChild<QLineEdit *>("sizeLineEdit");
            QString   size = childSizeLineEdit->text();
            if(size != "")
            {
                bool ok = false;
                selectedCurve.at(cnt)->setMarkerSize(size.toInt(&ok));
            }
            QBrush brush = markrPropDlg->getMarkerBrush();
            QPen   pen   = markrPropDlg->getMarkerPen();
            selectedCurve.at(cnt)->setMarkerColor(brush,pen);
        }
        plot->replot();
    }
    delete markrPropDlg;
    return;
}

void IcvICurve::refreshPlot()
{
    setAxseEyeSpan();
    plot->replot();
    plot->updateLegend();
    updateStatusBar();
    return;
}

void IcvICurve::cutCurve()
{
    return ;
}
 
void IcvICurve::copyCurve()
{
    if(NULL == plotCanvas)
        return;
    QList<IcvPlotCurve*> curves = plotCanvas->getSelectedCurve();
    IcvClipBoard  clipBoard;
    for(qint16 cnt = 0; cnt < curves.count(); cnt++)
    {
        clipBoard.data.append(plotData.at(curves.at(cnt)->getDataPos()));
    }
    QVariant data;
    QList<QPointF> plotdata = plotData[0].getData();
    return;
}

void IcvICurve::pasteCurve()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString           str = clipboard->text();  
    plotBlockData(str);
    return;
}

void IcvICurve::performCurveFiltering(qint16 filterType, QString keyword, qint16 inAllState)
{
    /* parse keyword for lineId */
    QStringList keywordRange;
    if(keyword.contains(QRegExp("^[0-9]+$")))
    {
        keywordRange.push_back(keyword);
    }
    else if(keyword.contains(QRegExp("([0-9]+,){1,}[0-9]?")))
    {
        keywordRange =  keyword.split(",");
    }
    else if(keyword.contains(QRegExp("^[0-9]+:[0-9]+$")))
    {
        QStringList line = keyword.split(":");
        if(line.count() < 2)
            return;
        bool ok = false;
        qint16  lineStart = line.at(0).toInt(&ok);
        qint16  lineEnd   = line.at(1).toInt(&ok);
        for(qint16 i = lineStart; i < lineEnd; i++)
        {
            keywordRange.push_back(QString::number(i));
        }
    }
    QList<IcvPlotCurve *> curves = plotCanvas->getCurves();
    QProgressDialog *progress = createIcvProgressDiag(plot, 0, curves.count(),
        "filter progress", "filtering...", QSize(300,100), true);
    progress->show();

    qint16 foundCnt = 0;
    for(qint16 cnt = 0; cnt < curves.count(); cnt++)
    {
        qint16 dataPos = curves.at(cnt)->getDataPos();
        IcvCommand cmd = plotData[dataPos];
        bool   isMatch = false;
        switch(filterType)
        {
        case ICV_BY_COMPLETECOMAND:
            {
                QString completeComand = cmd.getName() + " " + QString::number(cmd.getLineId()) +
                    " " + QString::number(cmd.getDirection());
                isMatch = (completeComand.compare(keyword, Qt::CaseInsensitive) == 0)? true : false;
            }
            break;
        case ICV_BY_COMANDNAME:
            isMatch = (cmd.getName().compare(keyword, Qt::CaseInsensitive)== 0)? true : false;
            break;
        case ICV_BY_LINEID:
            isMatch = (0 != keywordRange.count(QString::number(cmd.getLineId())))? true : false;
            break;
        case ICV_BY_DIRECTION:
            {
                QString strDir = (cmd.getDirection() == 1) ? "DS":"US";
                isMatch = (keyword.compare(strDir, Qt::CaseInsensitive) == 0)? true : false;
            }
            break;
        case ICV_BY_PROMT:
            isMatch = (cmd.getPromt().compare(keyword, Qt::CaseInsensitive)== 0)? true : false;
            break;
        case ICV_BY_POS:
            isMatch = (0 != keywordRange.count(QString::number(cmd.getDataPosInFile())))? true : false;
            break;
        default:
            isMatch = false;
        }
        if(!isMatch)   
        {
            curves.at(cnt)->removeCurve();
            foundCnt++;
        }
        else
        {
            if(Qt::Checked == inAllState)
                curves.at(cnt)->showCurve();
        }
        progress->setValue(cnt);
        /* every 50 curves processed, delay 50ms to handle the other events */
        if(0 == cnt % 50)
            taskDelay(50);
    } 
    /*clear memory*/
    delete progress;
    if(0 == foundCnt) 
    {
        QMessageBox::information(this,tr("Info"), tr("No curves filtered!"));
        return;
    }
    plot->replot();
    return;
}

void IcvICurve::filterCurve()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }

    IcvCurveFilterDialog *filterDlg = new IcvCurveFilterDialog(this);
    filterDlg->setWindowTitle("Filter curves");
    if(filterDlg->exec() != QDialog::Accepted)
        return;

    qint16  filterType = filterDlg->getLookupType();
    QString keyword    = filterDlg->getKeyword();
    qint16 inAllState = (Qt::CheckState)filterDlg->getInAllCheckState();
    delete  filterDlg;

    performCurveFiltering(filterType, keyword, inAllState);
    return; 
}

void IcvICurve::filterCurvePreview(qint16 type, QString keyword,qint16 inAllState)
{
    performCurveFiltering(type, keyword, inAllState);
    return;
}

void IcvICurve::recoverCurveVisible()
{
    QList<IcvPlotCurve*> curves = plotCanvas->getCurves();
    for(qint16 cnt = 0; cnt < curves.count(); cnt++)
    {
        curves.at(cnt)->showCurve();
    }
    plot->replot();
    return;
}

void IcvICurve::findCurve()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }

    IcvCurveFilterDialog *filterDlg = new IcvCurveFilterDialog(this);
    filterDlg->setWindowTitle("Find curves");
    qint16 width  = filterDlg->geometry().width();
    qint16 height = filterDlg->geometry().height();
    filterDlg->setFixedSize(width,height);
    if(filterDlg->exec() != QDialog::Accepted)
        return;

    QString    keyword = filterDlg->lineEdit->text();
    qint16  filterType = filterDlg->getLookupType();
    delete filterDlg;
    filterDlg = NULL;
    /* parse keyword for lineId */
    QStringList lineId;
    if(keyword.contains(QRegExp("^[0-9]+$")))
    {
        lineId.push_back(keyword);
    }
    else if(keyword.contains(QRegExp("([0-9]+,){1,}[0-9]?")))
    {
        lineId =  keyword.split(",");
    }
    else if(keyword.contains(QRegExp("^[0-9]+:[0-9]+$")))
    {
        QStringList line = keyword.split(":");
        if(line.count() < 2)
            return;
        bool ok = false;
        qint16  lineStart = line.at(0).toInt(&ok);
        qint16  lineEnd   = line.at(1).toInt(&ok);
        for(qint16 i = lineStart; i <= lineEnd; i++)
        {
            lineId.push_back(QString::number(i));
        }
    }

    QList<IcvPlotCurve *> curves = plotCanvas->getCurves();
    QProgressDialog *progress = createIcvProgressDiag(plot, 0, curves.count(),
        "progress", "searching...", QSize(300,100), true);
    progress->show();
    qint16 matchCnt = 0;
    QList<IcvPlotCurve *> crvMatch;
    for(qint16 cnt = 0; cnt < curves.count(); cnt++)
    {
        qint16 dataPos = curves.at(cnt)->getDataPos();
        IcvCommand cmd = plotData[dataPos];
        bool isMatch  = false;
        if(ICV_BY_COMPLETECOMAND == filterType)
        {
            QString completeComand = cmd.getName() + " " + QString::number(cmd.getLineId()) +
                " " + QString::number(cmd.getDirection());
            isMatch = (completeComand.compare(keyword, Qt::CaseInsensitive) == 0)?
                true : false;
        }
        else if (ICV_BY_COMANDNAME == filterType)
        {
            isMatch = (cmd.getName().compare(keyword, Qt::CaseInsensitive)== 0)?
                true : false;
        }
        else if (ICV_BY_LINEID == filterType)
        {
            isMatch = (0 != lineId.count(QString::number(cmd.getLineId())))? true : false;
        }
        else if (ICV_BY_DIRECTION == filterType)
        {
            QString strDir = (cmd.getDirection() == 1) ? "DS":"US";
            isMatch = (keyword.compare(strDir, Qt::CaseInsensitive) == 0)? 
                true : false;
        }
        if(isMatch)   
        {
            crvMatch.push_back(curves.at(cnt));
            matchCnt++;
        }
        /* every 50 curves processed, delay 50ms to handle the other events */
        if(0 == cnt % 50)
            taskDelay(50);
    } 
    /*clear memory*/
    delete progress;
    if(0 == matchCnt) 
    {
        QMessageBox::information(this,tr("Info"), tr("No curves filtered!"));
        return;
    }
    plotCanvas->highlightCurve(crvMatch);
    plotCanvas->setCurSelectCurves(crvMatch);
    plot->updateLegend();
    plot->replot();
    return;
}


void IcvICurve::deleteCurves()
{
    if(NULL == plotCanvas)
        return ;
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> selCurve = plotCanvas->getSelectedCurve();
    if(selCurve.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    QList<IcvPlotCurve*> curves = plotCanvas->getSelectedCurve();
    plotCanvas->deleteCurves(curves);
    plot->replot();
    updateStatusBar();
    return;
}

void IcvICurve::deleteCurvesAll()
{
    if(NULL == plotCanvas)
        return ;
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }

    plotCanvas->deleteCurves(allCurves);
    plot->replot();
    updateStatusBar();
    return;
}

void IcvICurve::removeCurves()
{
    if(NULL == plotCanvas)
        return ;
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> selCurve = plotCanvas->getSelectedCurve();
    if(selCurve.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    plotCanvas->removeSelectCurves();
    plot->replot();
    updateStatusBar();
    return;
}

void IcvICurve::hideCurves()
{
    if(NULL == plotCanvas)
        return ;
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> selCurve = plotCanvas->getSelectedCurve();
    if(selCurve.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    plotCanvas->hideSelectCurves();
    plot->replot();
    updateStatusBar();
    return;
}

void IcvICurve::showCurves()
{
    if(NULL == plotCanvas)
        return ;
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> selCurve = plotCanvas->getSelectedCurve();
    if(selCurve.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    plotCanvas->showSelectCurves();
    plot->replot();
    updateStatusBar();
    return;
}


void IcvICurve::showAllCurve()
{
    if(NULL == plotCanvas)
        return;

    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curves plotted, maybe file not opened."));
        return ;
    }

    QMessageBox msgBox(QMessageBox::Information, tr("info"), "Really to show all?", 0, this);
    msgBox.addButton(tr("Yes"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("No"),  QMessageBox::RejectRole);
    if (msgBox.exec() != QMessageBox::AcceptRole)
        return;

    qint16 maxCnt = allCurves.count();
    QProgressDialog *progress = createIcvProgressDiag(plot, 0, maxCnt, "progress", "displaying...", QSize(300,100), true);
    progress->show();
    for(qint16 pos = 0; pos < maxCnt; pos++)
    {
        allCurves.at(pos)->showCurve();
        progress->setValue(pos);
        if( 0 == pos%50)
            taskDelay(50);
    }
    plot->replot();
    delete progress;
    updateStatusBar();
    return;
}

void IcvICurve::selectAllCurves()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCanvasCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return;
    }
    plotCanvas->highlightCurve(allCurves);
    plotCanvas->setCurSelectCurves(allCurves);
    plot->replot();
    return;
}


void IcvICurve::selectInvertCurves()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> curve = plotCanvas->getSelectedCurve();
    for(qint16 cnt = 0; cnt < curve.count(); cnt++)
    {
        curve.at(cnt)->hideMarkers();
        allCurves.removeAll(curve.at(cnt));
    }
    QList<IcvPlotCurve *> reversedCurves = allCurves;
    plotCanvas->highlightCurve(reversedCurves);
    plotCanvas->setCurSelectCurves(reversedCurves);
    plot->replot();
    return;
}

void IcvICurve::expandCurve()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return;
    }
    QList<IcvPlotCurve *> curve = plotCanvas->getSelectedCurve();
    if(curve.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return;
    }
    bool ok;
    int groupSize = QInputDialog::getInt(this, tr("Input"),tr("Group size:"), 
                                         4, 1, 32, 2, &ok);
    if (!ok)
        return ;

    for(qint16 cnt = 0; cnt < curve.count(); cnt++)
    {
        curve.at(cnt)->setGroupSize(groupSize);
    }
    plot->replot();
    return;
}

void IcvICurve::repaintCurve()
{
    QList<IcvPlotCurve *> allCurves = plotCanvas->getCanvasCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return;
    }

    QList<IcvPlotCurve *>selectedCurves = plotCanvas->getSelectedCurve();
    QList<IcvPlotCurve *>curves = !selectedCurves.isEmpty()? selectedCurves : allCurves;
    for(qint16 cnt = 0; cnt < curves.count(); cnt++)
    {
        if(cnt < ICV_MAX_VIVID_COLOR_NUM)
             curves.at(cnt)->setColor(QColor(icvRgbColors[cnt]));
        else
            curves.at(cnt)->setColor(QColor::fromHsl(rand()%360,rand()%256,rand()%100));
    }
    plot->replot();
    return;
}


void IcvICurve::legendChecked( const QVariant &itemInfo, bool on)
{
    QwtPlotItem *plotItem = plot->infoToItem(itemInfo);
    if(NULL == plotItem)
        return ;

    plotItem->setVisible( on );
    QwtLegend *lgd = dynamic_cast<QwtLegend *>(legend);
    QList<QWidget *> legendWidgets = lgd->legendWidgets(plot->itemToInfo(plotItem));
    if (legendWidgets.size() == 1)
    {
        QwtLegendLabel *legendLabel = dynamic_cast<QwtLegendLabel *>(legendWidgets[0]);
        if (legendLabel)
            legendLabel->setChecked( on );
    }
    plot->replot();
    return ;
}

void IcvICurve::showCurveInfo()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> curve = plotCanvas->getSelectedCurve();
    if(curve.isEmpty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }
    /* more than one curves selected, dispayed as tableview */
    IcvCommand  cmd =  plotData.at(curve.at(0)->getDataPos());
    QStandardItemModel *model=new QStandardItemModel();
    QStringList horizonHeader;
    horizonHeader<<"name"<<"shell"<<"direction"<<"group Size"<<"brief"<<"file"<<"position";
    model->setHorizontalHeaderLabels(horizonHeader);

    QStringList vertiHeader;
    QStandardItem *newItem = NULL;
    for(qint16 row = 0; row < curve.count(); row++)
    {
        vertiHeader << QString::number(row);
        IcvCommand cmd = curve.at(row)->getCommand();
        /* title */
        newItem = new QStandardItem(cmd.getTitle());
        model->setItem(row ,0, newItem);
        /* shell */
        newItem = new QStandardItem(cmd.getPromt());
        model->setItem(row ,1, newItem);
        /* dir */
        newItem = new QStandardItem((cmd.getDirection() == 0)? "US": "DS"); 
        model->setItem(row ,2, newItem);
        /* group size */
        newItem = new QStandardItem(QString::number(cmd.getGroupSize()));
        model->setItem(row ,3, newItem);
        /* brief info */
        newItem = new QStandardItem(cmd.getBriefInfo());
        model->setItem(row ,4, newItem);
        /* the other */
        newItem = new QStandardItem(cmd.getFileName());
        model->setItem(row ,5, newItem);
        newItem = new QStandardItem(QString::number(cmd.getDataPosInFile()));
        model->setItem(row ,6, newItem);
    }
    model->setVerticalHeaderLabels(vertiHeader);
    /* display */
    IcvTableView *tbl = new IcvTableView();
    tbl->setModel(model);
    tbl->setResizeMode(0, QHeaderView::ResizeToContents);
    tbl->setResizeMode(1, QHeaderView::ResizeToContents);
    tbl->setResizeMode(2, QHeaderView::ResizeToContents);
    tbl->setResizeMode(3, QHeaderView::ResizeToContents);
    tbl->setColumnWidth(4, 200);
    tbl->setResizeMode(5, QHeaderView::ResizeToContents);
    tbl->setResizeMode(6, QHeaderView::ResizeToContents);

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("brief info");
    QHBoxLayout *layout = new QHBoxLayout(dlg);
    layout->addWidget(tbl);
    layout->setAlignment(Qt::AlignTop);
    dlg->setLayout(layout);
    dlg->resize(900,600);
    dlg->show();
    return;
}

void IcvICurve::setCurveProperties()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> curve = plotCanvas->getSelectedCurve();
    if(curve.isEmpty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    IcvCurvePropertyDialog *propDiag = new IcvCurvePropertyDialog(curve, this, Qt::Dialog);
    propDiag->exec();
    return;
}

void IcvICurve::jumpToFilePos()
{
    IcvTextEdit *editor = new IcvTextEdit(this);
    int toLineNumber = 100;
    editor->load("C:\\LZC\\wkspace\\samples\\log\\00.txt"); 
    editor->show();
    return;
}

void IcvICurve::viewCurveData()
{
   QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
   if(allCurves.empty())
   {
       QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
       return ;
   }
   QList<IcvPlotCurve *> curve = plotCanvas->getSelectedCurve();
   if(curve.isEmpty())
   {
       QMessageBox::information(this,tr("Info"),tr("No curve selected."));
       return ;
   }

   IcvCommand  cmd =  plotData.at(curve.at(0)->getDataPos());
   QStandardItemModel *model=new QStandardItemModel();
   QStringList horizonHeader;
   horizonHeader<<"number"<<"curve"<<"data";
   model->setHorizontalHeaderLabels(horizonHeader);

   QStringList vertiHeader;
   QStandardItem *newItem = NULL;
   for(qint16 row = 0; row < curve.count(); row++)
   {
       newItem = new QStandardItem(QString::number(row));
       model->setItem(row ,0, newItem);

       IcvCommand cmd = curve.at(row)->getCommand();
       QString curveIdentity;
       curveIdentity = cmd.getTitle() + " at line " +  QString::number(cmd.getDataPosInFile()) +
           " of " + cmd.getFileName();
       newItem = new QStandardItem(curveIdentity);
       model->setItem(row ,1, newItem);
       /* plotting data */
       qint16 posInRepo = curve.at(row)->getDataPos();
       QList<QPointF> data = plotData.value(posInRepo).getData();
       QString dataStr="";
       for(qint32 tone = 0; tone < data.count(); tone++)
       {
           dataStr += QString::number(data.at(tone).y())+",";
       }
       dataStr.remove(QRegExp(",$"));
       newItem = new QStandardItem(dataStr);
       model->setItem(row ,2, newItem);
   }
   /* display */
   IcvTableView *tbl = new IcvTableView();
   tbl->setModel(model);
   tbl->setResizeMode(0, QHeaderView::ResizeToContents);
   tbl->setColumnWidth(1, 200);
   tbl->setColumnWidth(2, 300);  
   QDialog *dlg = new QDialog(this);
   dlg->setWindowTitle("curve data");
   QHBoxLayout *layout = new QHBoxLayout(dlg);
   layout->addWidget(tbl);
   layout->setAlignment(Qt::AlignTop);
   dlg->setLayout(layout);
   dlg->resize(900,600);
   dlg->show();
   return;
}

void IcvICurve:: viewCurveStat()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return ;
    }
    QList<IcvPlotCurve *> curve = plotCanvas->getSelectedCurve();
    if(curve.isEmpty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    QStandardItemModel *model=new QStandardItemModel();
    QStringList horizonHeader;
    horizonHeader<<"number"<<"curve"<<"maximum"<<"minimum"<<"average"<<"variance";
    model->setHorizontalHeaderLabels(horizonHeader);
    QStringList vertiHeader;
    QStandardItem *newItem = NULL;
    for(qint16 row = 0; row < curve.count(); row++)
    {
        newItem = new QStandardItem(QString::number(row));
        model->setItem(row ,0, newItem);

        IcvCommand cmd = curve.at(row)->getCommand();
        QString curveIdentity;
        curveIdentity = cmd.getTitle() + " at line " +  QString::number(cmd.getDataPosInFile()) +
            " of " + cmd.getFileName();
        newItem = new QStandardItem(curveIdentity);
        model->setItem(row, 1, newItem);

        /* statistics of plotting data */
        QList<QPointF> data =  cmd.getData();
        std::vector<qreal> crvRy;
        for(qint16 count = 0; count < data.count(); count++)
        {
            crvRy.push_back(data.value(count).ry());
        }
        /* max in plot data */
        double maxRy = *std::max_element(crvRy.begin(), crvRy.end());
        newItem = new QStandardItem(QString::number(maxRy));
        model->setItem(row, 2, newItem);
        /* min in plot data */
        double minRy = *std::min_element(crvRy.begin(), crvRy.end());
        newItem = new QStandardItem(QString::number(minRy));
        model->setItem(row, 3, newItem);
        /* mean of plot data */
        double sum = std::accumulate(crvRy.begin(), crvRy.end(), 0.0);  
        double mean =  sum/crvRy.size(); 
        QString meanStr;
        meanStr.sprintf("%.2f", mean);
        newItem = new QStandardItem(meanStr);
        model->setItem(row, 4, newItem);
        /* variance of plot data */
        double accum  = 0.0;  
        for(qint32 i = 0; i < crvRy.size();i++)
        {  
            accum  += (crvRy[i]-mean)*(crvRy[i]-mean);  
        }
        double stdev = sqrt(accum/(crvRy.size()-1)); 
        QString stdDevStr;
        stdDevStr.sprintf("%.2f", stdev);
        newItem = new QStandardItem(stdDevStr);
        model->setItem(row, 5, newItem);
    }
    /* display */
    IcvTableView *tbl = new IcvTableView();
    tbl->setModel(model);
    tbl->setResizeMode(0, QHeaderView::ResizeToContents);
    tbl->setColumnWidth(1, 200);
    tbl->setResizeMode(2, QHeaderView::ResizeToContents);
    tbl->setResizeMode(3, QHeaderView::ResizeToContents);
    tbl->setResizeMode(4, QHeaderView::ResizeToContents);
    tbl->setResizeMode(5, QHeaderView::ResizeToContents);
    tbl->setResizeMode(6, QHeaderView::ResizeToContents);
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("curve statistics");
    QHBoxLayout *layout = new QHBoxLayout(dlg);
    layout->addWidget(tbl);
    layout->setAlignment(Qt::AlignTop);
    dlg->setLayout(layout);
    dlg->resize(900,600);
    dlg->show();
    return;
}

QwtPlotZoomer* IcvICurve::getZoomer()
{
    return zoomer;
}

QwtPlotPicker* IcvICurve::getPicker()
{
    return picker;
}

void IcvICurve::enableZoomer( bool checked)
{
    bool enable = checked;
    zoomer->setEnabled(enable);
    plotCanvas->setZoomState(enable);
    /* disable magnifier and panner */
    magnifier->setEnabled(!enable);
    panner->setEnabled(!enable);
    if(checked)
        plotCanvas->getCanvas()->setCursor(Qt::CrossCursor);
    else if(ui.actionHandMove->isChecked())
        plotCanvas->getCanvas()->setCursor(Qt::OpenHandCursor);
    else
        plotCanvas->getCanvas()->setCursor(Qt::ArrowCursor);
    ui.actionZoom->setCheckable(true);
    ui.actionZoom->setChecked(checked);
    plot->replot();
    return;
}

void IcvICurve::enableHandMove( bool checked)
{
    /* magnifier and panner are compliant with hand-move */
    magnifier->setEnabled(checked);
    panner->setEnabled(checked);
    /* zoomer is only enabled*/
    if(!checked && true == ui.actionZoom->isChecked())
        zoomer->setEnabled(true);
    else
        zoomer->setEnabled(false);

    if(checked)
        plotCanvas->getCanvas()->setCursor(Qt::OpenHandCursor);
    else if(ui.actionZoom->isChecked())
        plotCanvas->getCanvas()->setCursor(Qt::CrossCursor);
    else
        plotCanvas->getCanvas()->setCursor(Qt::ArrowCursor);

    ui.actionHandMove->setCheckable(true);
    ui.actionHandMove->setChecked(checked);
    plot->replot();
    return;
}

bool IcvICurve::isHandMoveChecked()
{
    return ui.actionHandMove->isChecked();
}

void IcvICurve::zoomPlot(const QRectF &rect)
{
    //plotCanvas->setZoomState(true);
    return;
}

void IcvICurve::diffCurves()
{
    QList<IcvPlotCurve *> curves = plotCanvas->getSelectedCurve();
    differTool = new IcvCurveDiffer(curves, this);
    differTool->setDiffCurves(curves);
    differTool->show();
    return;
}

void IcvICurve::parseCliData()
{
    IcvCliParserDialog *cliParser =  new IcvCliParserDialog(this);
    cliParser->show();
    return;
}

void IcvICurve::oneKeySetPlot()
{
    QList<IcvPlotCurve *> curves = plotCanvas->getCanvasCurves();
    if(curves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        return;
    }

    QStringList crvsTitle;
    crvsTitle.push_back(curves.at(0)->getCommand().getName());
    for(qint16 cnt = 1; cnt < curves.count(); cnt++)
    {
        qint16 tn = 0;
        for(tn = 0; tn < crvsTitle.count(); tn++)
        {
            if(crvsTitle.at(tn) == curves.at(cnt)->getCommand().getName())
                break;
        }
        if(tn == crvsTitle.count())
            crvsTitle.push_back(curves.at(cnt)->getCommand().getName());
    }
    QString title;
    if(1 == crvsTitle.count())
        title = crvsTitle.at(0);
    else
    {
        for(qint16 tn = 0; tn < crvsTitle.count() - 1; tn++)
            title += crvsTitle.at(tn) + ",";
        title += crvsTitle.at(crvsTitle.count() - 1);
    }
    plot->setTitle(title.remove("get"));

    QString labelY;
    if(crvsTitle.count() > 1)
        labelY = "Y(unit)";
    else
    {
        if(!crvsTitle.at(0).compare("getTxPsd", Qt::CaseInsensitive))
            labelY = "txPsd(dbm/hz)";
        else if(!crvsTitle.at(0).compare("getSnr", Qt::CaseInsensitive))
            labelY = "SNR(db)";
        else if(!crvsTitle.at(0).compare("getQln", Qt::CaseInsensitive))
            labelY = "Qln(dbm/hz)";
        else if(!crvsTitle.at(0).compare("getHlog", Qt::CaseInsensitive))
            labelY = "Hlog(db)";
        else if(!crvsTitle.at(0).compare("getNoiseMargin", Qt::CaseInsensitive))
            labelY = "Noise margin(db)";
        else if(!crvsTitle.at(0).compare("getBitAlloc", Qt::CaseInsensitive))
            labelY = "BitAlloc(bit)";
        else if(!crvsTitle.at(0).compare("getAln", Qt::CaseInsensitive))
            labelY = "Aln(dbm/hz)";
        else if(!crvsTitle.at(0).compare("getRmcBitAlloc", Qt::CaseInsensitive))
            labelY = "RmcBitAlloc(bit)";
    }
    plot->setAxisTitle(QwtPlot::yLeft,labelY);
    plot->setAxisTitle(QwtPlot::xBottom,"tone");
    plot->setFooter("4.3125 kHz/tone");
    plot->replot();
    return;
}

void IcvICurve::setRegularExp()
{
     IcvRegExp *regExp = new IcvRegExp(this, Qt::Dialog);
     regExp->show();
     return;
}

void IcvICurve::setAxseScale()
{
    IcvAxseSCaleDialog *axseScaleDlg = new IcvAxseSCaleDialog(this);

    axseScaleDlg->setWindowTitle("Axse scale dialog");
    if(axseScaleDlg->exec() != QDialog::Accepted)
        return ;

    bool   ok    = false;
    double minX  = axseScaleDlg->lineEditMinX->text().toDouble(&ok);
    double maxX  = axseScaleDlg->lineEditMaxX->text().toDouble(&ok);
    double minY  = axseScaleDlg->lineEditMinY->text().toDouble(&ok);
    double maxY  = axseScaleDlg->lineEditMaxY->text().toDouble(&ok);
    plot->setAxisScale(QwtPlot::xBottom, minX, maxX, 0);
    plot->setAxisScale(QwtPlot::yLeft, minY, maxY, 0);
    plot->replot();
    return;
}

void IcvICurve::setAxseTitle()
{
   /* the same as insert x/y label */
    QDialog *axseTitleDlg = new QDialog(this);
    QLabel *labelX = new QLabel("Axse X:");
    QLineEdit *textEditX = new QLineEdit(axseTitleDlg);
    textEditX->setObjectName(QString(tr("textEditX")));
    textEditX->resize(40,20); 

    QLabel *labelY = new QLabel("Axse Y:");
    QLineEdit *textEditY = new QLineEdit(axseTitleDlg);
    textEditY->setObjectName(tr("textEditY"));
    textEditY->resize(40,20);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(axseTitleDlg);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), axseTitleDlg, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), axseTitleDlg, SLOT(reject()));

    QGridLayout *layout = new QGridLayout(axseTitleDlg);
    layout->addWidget(labelX,0,0,1,1);
    layout->addWidget(textEditX,0,1,1,1);
    layout->addWidget(labelY,1,0,1,1);
    layout->addWidget(textEditY,1,1,1,1);
    layout->addWidget(buttonBox,2,1);
    layout->setColumnStretch(0,1);
    layout->setColumnStretch(1,3);
    layout->setRowStretch(0,1);
    layout->setRowStretch(1,1);
    layout->setRowMinimumHeight(1,20);
    /*alignment*/
    layout->setAlignment(Qt::AlignTop|Qt::AlignRight);
    axseTitleDlg->setLayout(layout);
    axseTitleDlg->resize(250,40);

    qint16 retcode = (qint16)axseTitleDlg->exec();
    if(QDialog::Accepted == retcode)
    {
        QLineEdit *child = NULL;

        child = axseTitleDlg->findChild<QLineEdit *>("textEditX");
        QString titleX = child->text();
        plot->setAxisTitle(QwtPlot::xBottom,titleX);

        child = axseTitleDlg->findChild<QLineEdit *>("textEditY");
        QString titleY = child->text();
        plot->setAxisTitle(QwtPlot::yLeft,titleY);
    }
    delete axseTitleDlg;
    return;
}

void IcvICurve::setAxseAlignment()
{
    QDialog *axseAlignDlg = new QDialog(this);
    axseAlignDlg->setWindowTitle("Axse aliagment");

    QLabel *labelHor = new QLabel("Horizontal");
    QLabel *labelVer = new QLabel("Vertical");

    QLabel *labelX = new QLabel("X:");
    QComboBox *alignComboHX = new QComboBox(axseAlignDlg);
    alignComboHX->setObjectName("alignComboHX");
    alignComboHX->addItem(tr("Left"));
    alignComboHX->addItem(tr("Right"));
    alignComboHX->addItem(tr("Center"));

    QComboBox *alignComboVX = new QComboBox(axseAlignDlg);
    alignComboVX->setObjectName("alignComboVX");
    alignComboVX->addItem(tr("Top"));
    alignComboVX->addItem(tr("Bottom"));
    alignComboVX->addItem(tr("Center"));

    QLabel *labelY = new QLabel("Y:");
    QComboBox *alignComboHY = new QComboBox(axseAlignDlg);
    alignComboHY->setObjectName("alignComboHY");
    alignComboHY->addItem(tr("Left"));
    alignComboHY->addItem(tr("Right"));
    alignComboHY->addItem(tr("Center"));

    QComboBox *alignComboVY = new QComboBox(axseAlignDlg);
    alignComboVY->setObjectName("alignComboVY");
    alignComboVY->addItem(tr("Top"));
    alignComboVY->addItem(tr("Bottom"));
    alignComboVY->addItem(tr("Center"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(axseAlignDlg);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), axseAlignDlg, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), axseAlignDlg, SLOT(reject()));

    QGridLayout *layout = new QGridLayout(axseAlignDlg);
    layout->addWidget(labelHor,     0, 1, 1, 1);
    layout->addWidget(labelVer,     0, 2, 1, 1);
    layout->addWidget(labelX,       1, 0, 1, 1);
    layout->addWidget(alignComboHX, 1, 1, 1, 1);
    layout->addWidget(alignComboVX, 1, 2, 1, 1);
    layout->addWidget(labelY,       2, 0, 1, 1);
    layout->addWidget(alignComboHY, 2, 1, 1, 1);
    layout->addWidget(alignComboVY, 2, 2, 1, 1);
    /* ok and cancel button span 2 column */
    layout->addWidget(buttonBox,    3, 1, 1, 2); 
    /*alignment*/
    layout->setAlignment(Qt::AlignTop|Qt::AlignRight);
    axseAlignDlg->setLayout(layout);
    axseAlignDlg->setFixedSize(200,120);

    qint16 retcode = (qint16)axseAlignDlg->exec();
    if(QDialog::Accepted == retcode)
    {
        QComboBox *childH = NULL;
        QComboBox *childV = NULL;
        bool ok;
        childH = axseAlignDlg->findChild<QComboBox *>("alignComboHX");
        childV = axseAlignDlg->findChild<QComboBox *>("alignComboVX");
        Qt::Alignment alignX = childH->currentText().toInt(&ok) |
                               childV->currentText().toInt(&ok) ;
        plot->setAxisLabelAlignment(QwtPlot::xBottom,alignX);

        childH = axseAlignDlg->findChild<QComboBox *>("alignComboHY");
        childV = axseAlignDlg->findChild<QComboBox *>("alignComboVY");
        Qt::Alignment alignY = childH->currentText().toInt(&ok) |
                               childV->currentText().toInt(&ok) ;
        plot->setAxisLabelAlignment(QwtPlot::yLeft,alignY);

    }
    delete axseAlignDlg;
    return;
}

void IcvICurve::setAxseRotation()
{
    QDialog *axseRotationDlg = new QDialog(this);

    QLabel *labelX = new QLabel("Angle for X:");
    QLineEdit *textEditX = new QLineEdit(axseRotationDlg);
    textEditX->setObjectName(QString(tr("textEditX"))); 

    QLabel *labelY = new QLabel("Angle for Y:");
    QLineEdit *textEditY = new QLineEdit(axseRotationDlg);
    textEditY->setObjectName(tr("textEditY"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(axseRotationDlg);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), axseRotationDlg, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), axseRotationDlg, SLOT(reject()));

    QGridLayout *layout = new QGridLayout(axseRotationDlg);
    layout->addWidget(labelX,    0, 0, 1, 1);
    layout->addWidget(textEditX, 0, 1, 1, 1);
    layout->addWidget(labelY,    1, 0, 1, 1);
    layout->addWidget(textEditY, 1, 1, 1, 1);
    layout->addWidget(buttonBox,2,1);
    layout->setColumnStretch(0,1);
    layout->setColumnStretch(1,3);
    layout->setRowStretch(0,1);
    layout->setRowStretch(1,2);
    layout->setRowMinimumHeight(1,20);
    /*alignment*/
    layout->setAlignment(Qt::AlignTop|Qt::AlignRight);
    axseRotationDlg->setLayout(layout);

    qint16 retcode = (qint16)axseRotationDlg->exec();
    if(QDialog::Accepted == retcode)
    {
        QLineEdit *child = NULL;
        bool ok;

        child = axseRotationDlg->findChild<QLineEdit *>("textEditX");
        double angleX = child->text().toDouble(&ok);
        plot->setAxisLabelRotation(QwtPlot::xBottom,angleX);

        child = axseRotationDlg->findChild<QLineEdit *>("textEditY");
        double angleY = child->text().toDouble(&ok);
        plot->setAxisLabelRotation(QwtPlot::yLeft,angleY);
    }
    delete axseRotationDlg;
    return;
}

void IcvICurve::setAxseProperties()
{
    return;
}

void IcvICurve::setAxseEyeSpan()
{
    QList<IcvPlotCurve*> allCurves = plotCanvas->getCurves();
    if(allCurves.empty())
    {
        QMessageBox::information(this,tr("Info"),tr("No curve in canvas."));
        ui.actionIndicator->setChecked(false);
        return ;
    }
    QList<IcvPlotCurve *>selectedCurves = plotCanvas->getSelectedCurve();
    QList<IcvPlotCurve *>curves = !selectedCurves.isEmpty()? selectedCurves : allCurves;

    std::vector<qreal> curvesMaxRx;
    std::vector<qreal> curvesMaxRy;
    std::vector<qreal> curvesMinRx;
    std::vector<qreal> curvesMinRy;
    for(qint16 row = 0; row < curves.count(); row++)
    {
        if(!curves.at(row)->isAttached())
            continue;
        IcvCommand cmd = curves.at(row)->getCommand();
        QList<QPointF> data =  cmd.getData();
        std::vector<qreal> crvRx;
        std::vector<qreal> crvRy;
        for(qint16 count = 0; count < data.count(); count++)
        {
            crvRx.push_back(data.value(count).rx());
            crvRy.push_back(data.value(count).ry());
        }
        /* find maximum of one curve */
        double maxRxCurve = *std::max_element(crvRx.begin(), crvRx.end());
        double maxRyCurve = *std::max_element(crvRy.begin(), crvRy.end());
        curvesMaxRx.push_back(maxRxCurve);
        curvesMaxRy.push_back(maxRyCurve);
        /* find minimum of one curve */
        double minRxCurve = *std::min_element(crvRx.begin(), crvRx.end());
        double minRyCurve = *std::min_element(crvRy.begin(), crvRy.end());
        curvesMinRx.push_back(minRxCurve);
        curvesMinRy.push_back(minRyCurve);
    }

    if(curvesMaxRx.size() == 0 || curvesMinRx.size() == 0 || 
        curvesMinRx.size() == 0 || curvesMinRy.size() == 0)
        return;
    double maxRx = *std::max_element(curvesMaxRx.begin(), curvesMaxRx.end());
    double maxRy = *std::max_element(curvesMaxRy.begin(), curvesMaxRy.end());
    double minRx = *std::min_element(curvesMinRx.begin(), curvesMinRx.end());
    double minRy = *std::min_element(curvesMinRy.begin(), curvesMinRy.end());
    plot->setAxisScale(QwtPlot::xBottom, minRx - ICV_EYESCAN_MARGIN, maxRx + ICV_EYESCAN_MARGIN);
    plot->setAxisScale(QwtPlot::yLeft,   minRy - ICV_EYESCAN_MARGIN, maxRy + ICV_EYESCAN_MARGIN);
    return;
}

void IcvICurve::aboutIcurve()
{
    IcvAboutDialog *dlg = new IcvAboutDialog(this);
    dlg->show();
    return;
}

ICU_RET_STATUS IcvICurve::loadData(const QString &filename)
{
    QFile file(filename);
    if(!file.exists())
    {
        return ICU_FILE_NOT_EXIST;
    }
    if(!file.open(QFile::ReadOnly ))
    {
        return ICU_FILE_READ_ERROR;
    }
    if(ICU_OK != analyzeFile(file))
    {
        return ICU_PLOT_DATA_FORMAT_ERROR;
    }

    return ICU_OK;
}

ICU_RET_STATUS IcvICurve::analyzeFile(QFile &file)
{
    QTextStream dataTextStream(&file);
    QFileInfo fileInfo(file);
    if(fileInfo.size() > ICV_MAX_ACCEPT_FILE_SIZE)
    {
        QString info = fileInfo.fileName() + " larger than 300M, rejected!";
        QMessageBox::critical(this,"Error",info);
        return ICU_OK;
    }

    return analyzeTextStream(dataTextStream, fileInfo.fileName());
}

ICU_RET_STATUS IcvICurve::analyzeTextStream(QTextStream &textStream, QString textName)
{
    /* scanning progress */
    QDialog *dialogInfo = new QDialog(this);
    QLabel *labelInfo = new QLabel("Scanning " + textName + " ...");
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(labelInfo);
    dialogInfo->setLayout(layout);
    dialogInfo->setFixedSize(300,50);
    dialogInfo->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialogInfo->setModal(true);
    dialogInfo->show();

    qint32 nLine = 0;
    qint16 dataScopeMode = ICV_DATA_SCOPE_BCM;
    while(!textStream.atEnd())
    {
        QString dataLine = textStream.readLine();
        QRegExp reg(QString("show xdsl scstatus-segment (Bitload|Qln|SNR|GainAlloc|Hlog|LinImg|LinReal) (adsl|vdsl)_\\d+/\\d+/\\d+\\s?$"));
        reg.setCaseSensitivity(Qt::CaseInsensitive);
        if(dataLine.contains(reg) && (dataScopeMode == ICV_DATA_SCOPE_BCM))
            dataScopeMode = ICV_DATA_SCOPE_CLI;

        nLine++;
        /* delay 50ms to handle other event,preventing high cpu usage */
        if(0 == (nLine % 20000))  
        {
            QElapsedTimer timer;
            timer.start();
            while(timer.elapsed() < 50)
            {
                QCoreApplication::processEvents();
            }
        }
    }
    qint32  totalLineNum = nLine;
    delete dialogInfo;

    if(ICV_DATA_SCOPE_CLI == dataScopeMode)
    {
        analyzeCliTextStream(textStream, textName, totalLineNum);
        return ICU_OK;
    }

    if(totalLineNum > ICV_MAX_LINE_NUM_BACKGROUD_PROCESS)
    {
        analyProgressDialog = createIcvProgressDiag(this, 0, totalLineNum, QString("analyzing "+ textName + " ..."), 
                                                    tr(""), QSize(300, 100),true);
        analyProgressDialog->show();
        analyProgressDialog->repaint();
        connect(analyProgressDialog, SIGNAL(canceled()), this, SLOT(cancelAnalyProgressBar()));
    }

    IcvCommand          cmd;
    QList< QString>     titlePattern;
    QRegExp             cmdRegExp;
    isDataAnalyCanceled = false;
    qint32 line         = 0;
    cmd.reset();
    textStream.seek(0);
    titlePattern = cmd.getTitlePattern();
    while(!textStream.atEnd() && !isDataAnalyCanceled)
    {
        line++;
        QString dataLine = textStream.readLine();
        bool isMatch     = false;
        for(qint16 i = 0; i < titlePattern.count(); i++)
        {
            if(!titlePattern.isEmpty())
            {
                cmdRegExp.setPattern(titlePattern.at(i));
                cmdRegExp.setCaseSensitivity(Qt::CaseInsensitive);
                isMatch = dataLine.contains(cmdRegExp);
                if(isMatch)
                    break;
            }
        }
        if(isMatch)
        {
            /* if the last command is not empty, terminate it */
            if(cmd.getData().count() > 0)              
                plotData.push_back(cmd);   
            /* begin to set new command */
            QStringList caps = cmdRegExp.capturedTexts();
            QString promt    = caps[1];
            QString name     = caps[2];
            bool ok = false;
            qint16 lineId    = caps[3].toInt(&ok);
            qint16 direction = caps[4].toInt(&ok);
            /* preparing the new command */
            cmd.reset();
            cmd.setPrompt(promt);
            cmd.setName(name);
            cmd.setLineId(lineId);
            cmd.setDirection(direction);
            cmd.setDataPosInFile(line);
            cmd.setBriefInfo(dataLine);
            cmd.setFileName(textName);
            /* set cmd matched state */
            cmd.setState(CMD_TITLE_MATCHED);
            continue;
        }

        if(cmd.getState() < CMD_TITLE_MATCHED)
            continue;
        /* some case like: >rfc getqln 0 0  Line  3 DS QLN (dBm/Hz, grouped by 8 tones),
           so continue to parsing the left characters in the same line */
        if(cmd.getState() < CMD_GROUPSIZE_MATCHED)
        {
            /* math groupsize, if found, loop continue */
            if(cmd.matchGroupSize(dataLine))
            {
                cmd.setState(CMD_GROUPSIZE_MATCHED);
                continue;
            }
        }

        if(cmd.getState() <= CMD_PLOTDATA_MATCHED)
        {
            /* spectial format pre-processing */
            if(dataLine.contains(cmd.getSpecReplace().first))
                dataLine = dataLine.replace(cmd.getSpecReplace().first,cmd.getSpecReplace().second);

            QRegExp regExpr;
            regExpr.setCaseSensitivity(Qt::CaseInsensitive);
            regExpr.setPattern(cmd.getDataPattern());
            if(-1 == regExpr.indexIn(dataLine))
                continue;

            qint16 ret = assembleData(dataLine,&cmd);
            if(ret != ICU_OK)
            {
                QString error = textName + " at line " + QString::number(line) + ":data format incorrect.";
                QMessageBox::critical(this,"ERROR",error);
                continue; 
            }
        }
 
        if((NULL != analyProgressDialog) && (line%100 == 0))
            emit analyDataProgress(line);
        /* delay 500ms to handle other event,preventing high cpu usage */
        if(0 == (line % 2000))
        {
            QElapsedTimer timer;
            timer.start();  
            while(timer.elapsed() < 50)  
            {
                QCoreApplication::processEvents();  
            }
        }
    }
    /* no more new command found when at file end, save the current data */
    if(cmd.getData().count() > 0)     
    {
        cmd.setState(CMD_CLOSED);
        plotData.push_back(cmd);   
    }
    /* importing data action halted, data should also be cleared */
    if(isDataAnalyCanceled)
        plotData.clear();
    if((NULL != analyProgressDialog))
    {
        delete analyProgressDialog;
        analyProgressDialog = NULL;
    }
    return ICU_OK;
}

ICU_RET_STATUS IcvICurve::analyzeCliTextStream(QTextStream &textStream, QString textName, qint32 totalLine)
{
 #if 0
   if(totalLine > ICV_MAX_LINE_NUM_BACKGROUD_PROCESS)
    {
        analyProgressDialog = createIcvProgressDiag(this, 0, totalLine, QString("analyzing "+ textName + " ..."), 
                                                    tr(""), QSize(300, 100),true);
        analyProgressDialog->show();
        analyProgressDialog->repaint();
        connect(analyProgressDialog, SIGNAL(canceled()), this, SLOT(cancelAnalyProgressBar()));
    }

    IcvCommand cmd;
    IcvCommand prevCmd;
    cmd.reset();
    cmd.setDataScopeMode(ICV_DATA_SCOPE_CLI);
    prevCmd.reset();
    prevCmd.setDataScopeMode(ICV_DATA_SCOPE_CLI);
    textStream.seek(0);
    isDataAnalyCanceled = false;
    qint32 line = 0;
    while(!textStream.atEnd() && !isDataAnalyCanceled)
    {
        line++;
        QString dataLine = textStream.readLine();
        qint16       pos = 0;
        QString curCmdName("NULL");
        QString   curPromt("NULL");
        QString     lineId("NULL");
        QString  direction("NULL");

        QRegExp cmdRegExp;
        cmd.initTitlePattern();
        cmdRegExp.setPattern(cmd.getTitlePattern());
        cmdRegExp.setCaseSensitivity(Qt::CaseInsensitive);
        bool isMatch = dataLine.contains(cmdRegExp);
        if(isMatch)
        {
            /* if new command found but data of the last command is not empty, 
               terminate it */
            prevCmd = cmd;
            /* begin to set new command */
            QStringList caps = cmdRegExp.capturedTexts();
            QString name  = caps[1];
            bool       ok = false;
            qint16 port  = caps[2].toInt(&ok);; 
            /* preparing the new command */
            cmd.reset();
            cmd.setPrompt("ZXCLI");
            cmd.setName(name);
            cmd.setLineId(port);
            cmd.setDataPosInFile(line);
            cmd.setBriefInfo(dataLine);
            cmd.setFileName(textName);
            /* set cmd matched state */
            cmd.setState(CMD_TITLE_MATCHED);
            continue;
        }

        if(cmd.getState() >= CMD_TITLE_MATCHED && cmd.getState() < CMD_GROUPSIZE_MATCHED)
        {
            /* math groupsize, if found, loop continue */
            if(cmd.matchGroupSize(dataLine))
            {
                cmd.setState(CMD_GROUPSIZE_MATCHED);
                continue;
            }
        }

        if(dataLine.contains(QRegExp("upstream|downstream")))
        {
            if(dataLine.contains("upstream"))
            {
                cmd.setDirection(0);
            }
            else
            {
                if(!cmd.getData().isEmpty())
                {
                    /* if the last command is not empty, terminate it */
                    prevCmd = cmd;
                    plotData.push_back(prevCmd);
                    prevCmd.reset();
                    /* set downstream as copy of upstream except the data. */
                    cmd.setDirection(1);
                    cmd.clearData();
                }
            }
            continue;
        }

        /* is data pattern matched? */
        QRegExp regExpr(cmd.getDataPattern());
        regExpr.setCaseSensitivity(Qt::CaseInsensitive);
        if(!dataLine.contains(regExpr))
            continue;

        /* {{{command already started, try to parse data */
        if(prevCmd.getData().count() > 0)              
        {
            /* reset previous command first*/
            plotData.push_back(prevCmd);   
            prevCmd.reset();
        }
        cmd.setState(CMD_PLOTDATA_MATCHED);
        qint16 ret = assembleCliData(&cmd, dataLine);
        if(ret == ICU_PLOT_DATA_FORMAT_ERROR)
        {
            QString error = textName + " at line " + QString::number(line) \
                + ":data format incorrect.";
            QMessageBox::critical(this,"ERROR",error);

            return ICU_PLOT_DATA_FORMAT_ERROR; 
        }

        if((NULL != analyProgressDialog) && (line%100 == 0))
            emit analyDataProgress(line);

        /* delay 500ms to handle other event,preventing high cpu usage */
        if(0 == (line % 2000))
        {
            QElapsedTimer timer;
            timer.start();  
            while(timer.elapsed() < 50)  
            {
                QCoreApplication::processEvents();  
            }
        }
    }
    /* no more new command found when at file end, save the current data */
    if(cmd.getData().count() > 0)     
    {
        cmd.setState(CMD_CLOSED);
        plotData.push_back(cmd); 
    }
    /* importing data action halted, data should also be cleared */
    if(isDataAnalyCanceled)
        plotData.clear();
    if((NULL != analyProgressDialog))
    {
        delete analyProgressDialog;
        analyProgressDialog = NULL;
    }
#endif
    return ICU_OK;
}

ICU_RET_STATUS IcvICurve::assembleData(QString dataLine, IcvCommand *cmd)
{
    QStringList splitList;
    splitList = dataLine.split(QRegExp("\\s+|\\,"),QString::SkipEmptyParts);
    /* filter some type of date like: 11, -11, -11.1 */
    QStringList digList;
    digList = splitList.filter(QRegExp("^\\d+$|^\\d+\\.\\d+$|^-\\d+\\.\\d+$"));
    if(digList.isEmpty())
        return ICU_OK;

    if(ICU_OK != appendCommandData(cmd, digList))
        return ICU_ERROR;

    if(digList.count() < ICV_MAX_NUM_DIGITS_PERLINE)
    {
        cmd->setState(CMD_CLOSED);
        return ICU_OK;
    }
    return ICU_OK;
}

ICU_RET_STATUS IcvICurve::assembleCliData(IcvCommand *cmd, QString dataLine)
{
    QStringList digList = parzeCliHex(cmd, dataLine);
    return appendCliCommandData(cmd, digList);
}

QStringList IcvICurve::parzeCliHex(IcvCommand *cmd, QString dataLine)
{
    QStringList hexs;
    QStringList dataList;
    bool ok = false;

    hexs = dataLine.split(" ", QString::SkipEmptyParts);
    if(cmd->getName().contains(QRegExp("GainAlloc|Hlog|LinImg|LinReal")))
    {
        /* segment 2-2  */
        for(qint16 i = 0; i < hexs.count(); i+=2)
        {
            QString str = QString::number((hexs.at(i) + hexs.at(i + 1)).toInt(&ok, 16));
            float df = str.toFloat(&ok);
            QString strDf;
            if(cmd->getName().contains(QRegExp("GainAlloc")))
                df = RFC_GAIN_CONV(df);
            else
                df = RFC_HLOG_CONV(df);
            strDf.sprintf("%6.1f",df);
            dataList.append(strDf);
        }
    }
    else if(cmd->getName().contains(QRegExp("Qln|SNR")))  
    {
        /* segment 1-1  */
        for(qint16 i = 0; i < hexs.count(); i++)
        {
            QString str = QString::number(hexs.at(i).toInt(&ok, 16));
            float df = str.toFloat(&ok);
            QString strDf;
            if(cmd->getName().contains(QRegExp("Qln")))
                df = RFC_QLN_CONV(df);
            else
                df = RFC_SNR_CONV(df);
            strDf.sprintf("%6.1f",df);
            dataList.append(strDf);
        }
    }
    else if(cmd->getName().contains(QRegExp("Bitload")))
    {
        /* segment 0.5-0.5  */
        for(qint16 i = 0; i < hexs.count(); i++)
        {
            QString strf;
            QString strs;
            strf.sprintf("%2d", (hexs.at(i).left(1).toInt(&ok,16)));
            strs.sprintf("%2d", (hexs.at(i).right(1).toInt(&ok,16)));
            dataList.append(strf);
            dataList.append(strs);
        }
    }
    return dataList;
}

ICU_RET_STATUS IcvICurve::appendCommandData(IcvCommand *cmd, QStringList data)
{
    bool ok = false;
    QList<QPointF> points; 
    QPointF        point;
    qint16         tone = 0;

    if(0 == cmd->getData().count())
        tone = data.at(0).toInt(&ok);  
    else
        tone = cmd->getData().count();
    for(qint16 i = ICV_PLOT_DATA_START_POS; i < data.count(); i++)
    {
        qreal dataItem = data.at(i).toFloat(&ok);
        if(false == ok)
            return ICU_PLOT_DATA_FORMAT_ERROR;
        tone++;
        point.setX(tone);
        point.setY(dataItem);
        points.append(point);
    }
    cmd->setData(points,true);
    return ICU_OK;
}

ICU_RET_STATUS IcvICurve::appendCliCommandData(IcvCommand *cmd, QStringList data)
{
    bool ok = false;
    QList<QPointF> points; 
    QPointF        point;
    qint16         tone = cmd->getData().count();
    for(qint16 i = 0; i < data.count(); i++)
    {
        qreal dataItem = data.at(i).toFloat(&ok);
        if(false == ok)
            return ICU_PLOT_DATA_FORMAT_ERROR;
        tone++;
        point.setX(tone);
        point.setY(dataItem);
        points.append(point);
    }
    cmd->setData(points,true);
    return ICU_OK;
}

QList <IcvCommand>* IcvICurve::getPlotData()
{
    return &plotData;
}

void IcvICurve::resetDifferTool()
{
    differTool = NULL;
    return;
}

void IcvICurve::updateAnalyProgressBar(qint32 progress)
{
    if(analyProgressDialog != NULL)
    {
        analyProgressDialog->setValue(progress);
        analyProgressDialog->setLabelText(QString("%1/%2").arg(progress).arg(analyProgressDialog->maximum()));
        analyProgressDialog->repaint();
    }
    return ;
}

void IcvICurve::cancelAnalyProgressBar()
{
    isDataAnalyCanceled = true;
    return;
}

void IcvICurve::updateRecentFileActions()
{
    QSettings settings("icv_history.ini",QSettings::IniFormat);;
    QStringList files = settings.value("recentFileList").toStringList();
    qint16 numRecentFiles = qMin(files.size(), (int)ICV_MAX_RECENT_FILE_NUM);

    for (qint16 i = 0; i < numRecentFiles; i++) 
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (qint16 j = numRecentFiles; j < ICV_MAX_RECENT_FILE_NUM; ++j)
    {
        recentFileActs[j]->setVisible(false);
    }
    return;
}


void IcvICurve::taskDelay(qint32 mseconds)
{
    QElapsedTimer timer;
    timer.start();  
    while(timer.elapsed() < mseconds)  
    {
        QCoreApplication::processEvents();  
    }
    return;
}


QProgressDialog * IcvICurve::createIcvProgressDiag(QWidget *parent, int rangeMin, int rangeMax,
     QString winTitle, QString LabelText, QSize size, bool isModal)
{
    QProgressDialog *prgDiag = new QProgressDialog(parent);
    prgDiag->setRange(rangeMin, rangeMax);
    prgDiag->setWindowTitle(winTitle);
    prgDiag->setLabelText(LabelText);
    prgDiag->setFixedSize(size);
    prgDiag->setModal(isModal);
    return prgDiag;
}

void  IcvICurve::updateStatusBar()
{
    QString status;
    status += QString::number(plotData.count()) + tr(" in repository, ");
    status += QString::number(plotCanvas->getCanvasCurves().count()) + tr(" in canvas.");
    statusBar()->showMessage(status);
    return;
}

QwtPlot* IcvICurve::getPlot()
{
    return plot;
}

QwtPlotMagnifier* IcvICurve::getMagnifier()
{
    return magnifier;
}

void IcvICurve::paintEvent ( QPaintEvent * event )
{
    return QWidget::paintEvent(event);
}

void IcvICurve::mouseMoveEvent ( QMouseEvent * event )
{
    return QWidget::mouseMoveEvent(event);
}

void IcvICurve::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void IcvICurve::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls= event->mimeData()->urls();
    if(urls.isEmpty())
        return;
    foreach(QUrl url, urls)
    {
        QStringList fileNames;
        fileNames.append(url.toLocalFile());
        loadFile(fileNames);
    }
    return ;
}
