#include <Qt>
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

#include <qwt_legend.h>
#include <qwt_plot_renderer.h>
#include <qwt_legend_label.h>
#include <qwt_plot_item.h>

#include "icv_icurve.h"
#include "icv_curve_filter.h"
#include "icv_penstyle.h"
#include "icv_marker_property.h"


/*including tone index at head of the line*/
#define ICV_MAX_NUM_DIGITS_PERLINE          (11)  

#define ICV_PLOT_DATA_START_POS             (1)
#define ICV_MAX_LINE_NUM_BACKGROUD_PROCESS  (2000)



IcvICurve::IcvICurve(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags)
{
    ui.setupUi(this);

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
    isDataAnalyCanceled  = false;

    /*{{{signals and slots*/
    connect(ui.actionOpen,       SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui.actionSaveAs,     SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui.actionClose,      SIGNAL(triggered()), this, SLOT(closePlot()));
    connect(ui.actionExit,       SIGNAL(triggered()), this, SLOT(close()));
    
    /*curve menu*/
    connect(ui.actionColor,      SIGNAL(triggered()), this, SLOT(setCurveColor()));
    connect(ui.actionWidth,      SIGNAL(triggered()), this, SLOT(setCurveWidth()));
    connect(ui.actionStyle,      SIGNAL(triggered()), this, SLOT(setCurveStyle()));
    connect(ui.actionMarker,     SIGNAL(triggered()), this, SLOT(setCurveMarker()));
    connect(ui.actionExpand,     SIGNAL(triggered()), this, SLOT(expandCurve()));
    connect(ui.actionFilter,     SIGNAL(triggered()), this, SLOT(filterCurve()));
    connect(ui.actionSearch,     SIGNAL(triggered()), this, SLOT(searchCurve()));
    connect(ui.actionShowAll,    SIGNAL(triggered()), this, SLOT(showAllCurve()));

    /*insert menu*/
    connect(ui.actionX_label,    SIGNAL(triggered()), this, SLOT(insertXLabel()));
    connect(ui.actionY_label,    SIGNAL(triggered()), this, SLOT(insertYLabel()));
    connect(ui.actionLegend,     SIGNAL(triggered()), this, SLOT(insertLegend()));
    connect(ui.actionCurveName,  SIGNAL(triggered()), this, SLOT(insertCurveName()));
    connect(ui.actionFooter,     SIGNAL(triggered()), this, SLOT(insertFooter()));

    connect(this, SIGNAL(analyDataProgress(qint16)), this, SLOT(updateAnalyProgressBar(qint16)));

    /*}}}*/

}


IcvICurve::~IcvICurve()
{

}


void IcvICurve::initMainWinStyle(QMainWindow *self)
{
    self->setWindowTitle("iCurve");
    self->setContentsMargins(0,0,0,0);

    QPalette mainWinPalette;
    mainWinPalette.setColor(QPalette::Background,Qt::lightGray);
    self->setPalette(mainWinPalette);

    return ;
}


void IcvICurve::initMainPlotter(QWidget *plotWidget)
{
    QwtPlot *plot = static_cast<QwtPlot*>(plotWidget);

    plot->setCanvasBackground( Qt::white );
    plot->setAxisScale( QwtPlot::yLeft, -200.0, 200.0 );
    plot->setAxisScale( QwtPlot::xBottom, 0, 5000 );

    plot->setAxisLabelAlignment(QwtPlot::xBottom,Qt::AlignLeft);
    plot->setAxisLabelAlignment(QwtPlot::yLeft,Qt::AlignTop);

    plot->plotLayout()->setAlignCanvasToScales( false );
    plot->plotLayout()->setCanvasMargin(0);

    /*default curve title is displayed, */
    legendItem = new QwtPlotLegendItem();  
    legendItem->attach(plot);

    legend = new QwtLegend;
    legend->setDefaultItemMode( QwtLegendData::Checkable );
    connect(legend, SIGNAL(checked(const QVariant &, bool, int)), this,
        SLOT(legendChecked( const QVariant &, bool)));

    grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(Qt::DashLine));
    grid->setMinorPen(QPen(Qt::DashLine));  
    grid->attach( plot );


    (void) new QwtPlotPanner( plot->canvas());
    magnifier = new QwtPlotMagnifier(plot->canvas());

    return ;
}


void IcvICurve::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open file..."),
            fileInfo.absolutePath(),
            tr("Text files (*.txt);;All Files (*)"));

    if(fileName.isEmpty())
    {
        return ; 
    }

    if(ICU_OK == loadData(fileName))
    {
        for(qint16 pos = 0; pos < plotData.count(); pos++)
        {
            
            QwtPlotCurve *qwtCurve = new QwtPlotCurve();
            qwtCurve->setPen(QColor::fromHsl(rand()%360,rand()%256,rand()%200), 1.0, Qt::SolidLine);
            qwtCurve->setSamples(plotData[pos].getData().toVector());

            QwtSymbol *symbol = new QwtSymbol( QwtSymbol::NoSymbol,
                QBrush(Qt::yellow), QPen(Qt::red, 2), QSize(8, 8) );
            qwtCurve->setSymbol(symbol);
            qwtCurve->setTitle(plotData.value(pos).getCommandTitle());
            qwtCurve->setStyle(QwtPlotCurve::Lines);
            qwtCurve->attach(plot);

            IcvPlotCurve *plotCurve = new IcvPlotCurve;
            plotCurve->setCurve(qwtCurve);
            plotCurve->setCanvas(plotCanvas);
            plotCurve->setDataPos(pos);

            plotCanvas->appendCurves(plotCurve);

            plot->setAxisScale( QwtPlot::yLeft, 0, 70 );
            plot->setAxisScale( QwtPlot::xBottom, 0.0, 3000 );
            plot->replot();
        }
    }

    return ;
}


void IcvICurve::saveAs()
{
    QwtPlotRenderer renderer;
    renderer.exportTo(plot, "legends.pdf");
}


void IcvICurve::closePlot()
{
    delete plot;
    return ;
}


void IcvICurve::insertTitle()
{
    bool ok;
    QString origTitle = plot->title().text();
    QString text = QInputDialog::getText(this, tr("Input"),
        tr("Plot title:"), QLineEdit::Normal, origTitle, &ok);
    if (ok)
        plot->setTitle(text);

    return ;
}


void IcvICurve::insertXLabel()
{
    bool ok;
    QString origLabel = plot->axisTitle(QwtPlot::xBottom).text();
    QString text = QInputDialog::getText(this, tr("Input"),
        tr("X lable:"), QLineEdit::Normal, origLabel, &ok);
    if (ok)
        plot->setAxisTitle(QwtPlot::xBottom,text);

    return;
}


void IcvICurve::insertYLabel()
{
    bool ok;
    QString origLabel = plot->axisTitle(QwtPlot::yLeft).text();
    QString text = QInputDialog::getText(this, tr("Input"),
        tr("Y lable:"), QLineEdit::Normal, origLabel, &ok);
    if (ok)
        plot->setAxisTitle(QwtPlot::yLeft,text);

    return ;
}


void IcvICurve::insertLegend()
{
    if(true == ui.actionLegend->isChecked())
    {
        plot->insertLegend(legend, QwtPlot::RightLegend );
        ui.actionLegend->setChecked(true);
    }
    else
    {
        plot->insertLegend(NULL);
        ui.actionLegend->setChecked(false);
    }

    return ;
}


void IcvICurve::insertCurveName()
{
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
    return ;
}


void IcvICurve::insertFooter()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Input"),
        tr("Footer"), QLineEdit::Normal, "", &ok);
    if (ok)
        plot->setFooter(text);

    return ;
}


void IcvICurve::setCurveColor()
{
    IcvPlotCurve *selectedCurve = plotCanvas->getSelectedCurve();
    if(NULL == selectedCurve)
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    QColor color = QColorDialog::getColor(Qt::white,this);
    if(!color.isValid())
        return;

    selectedCurve->setColor(color);
    plot->replot();

    return  ;
}


void IcvICurve::setCurveWidth()
{
    IcvPlotCurve *selectedCurve = plotCanvas->getSelectedCurve();
    if(NULL == selectedCurve)
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    bool   ok;
    qint16 width = QInputDialog::getInt(this, tr("Input"),tr("Group size:"), 
        1, 1, 20, 1, &ok);

    selectedCurve->setWidth(width);
    plot->replot();

    return  ;
}


void IcvICurve::setCurveStyle()
{
    IcvPlotCurve *selectedCurve = plotCanvas->getSelectedCurve();
    if(NULL == selectedCurve)
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    QDialog *curveStyleDlg = new QDialog(this);

    QLabel *titleLabel = new QLabel(curveStyleDlg);
    titleLabel->setText("Select style:");

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
    gridLayout->setRowMinimumHeight(1,50);

    qint16 retcode = (qint16)curveStyleDlg->exec();
    if(QDialog::Accepted == retcode)
    {
        QComboBox *child = curveStyleDlg->findChild<QComboBox *>("styleComboBox");
        Qt::PenStyle style = static_cast<Qt::PenStyle>(child->currentIndex() + 1);
        selectedCurve->setStyle(style);

        plot->replot();
    }

    delete curveStyleDlg;

    return;
}


void IcvICurve::setCurveMarker()
{

    IcvPlotCurve *selectedCurve = plotCanvas->getSelectedCurve();
    if(NULL == selectedCurve)
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
        selectedCurve->setMarker(style);

        QLineEdit *childSizeLineEdit = markrPropDlg->findChild<QLineEdit *>("sizeLineEdit");
        QString   size = childSizeLineEdit->text();
        if(size != "")
        {
            bool ok = false;
            selectedCurve->setMarkerSize(size.toInt(&ok));
        }

        QBrush brush = markrPropDlg->getMarkerBrush();
        QPen   pen   = markrPropDlg->getMarkerPen();
        selectedCurve->setMarkerColor(brush,pen);

        plot->replot();
    }

    delete markrPropDlg;

    return;
}


void IcvICurve::filterCurve()
{
    IcvCurveFilterDialog *filterDlg = new IcvCurveFilterDialog(this);
    filterDlg->setWindowTitle("Filter curves");
    if(filterDlg->exec() != QDialog::Accepted)
        return ;

    QString keyword = filterDlg->lineEdit->text();
    qint16  filterType = filterDlg->getFilterType();
   
    QList<IcvPlotCurve *> curves = plotCanvas->getCurves();
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
            QString strLineId = QString::number(cmd.getLineId());
            isMatch = (keyword == strLineId) ? true : false;
        }
        else if (ICV_BY_DIRECTION == filterType)
        {
            QString strDir = (cmd.getDirection() == 1) ? "DS":"US";
            isMatch = (keyword.compare(strDir, Qt::CaseInsensitive) == 0)? 
                true : false;
        }

        if(false == isMatch)   
        {
            plotCanvas->deleteCurve(curves.at(cnt));
        }

    } 

    plot->updateLegend();
    plot->replot();

    /*clear memory*/
    delete filterDlg;
    
    return ;
}


void IcvICurve::searchCurve()
{

    IcvCurveFilterDialog *filterDlg = new IcvCurveFilterDialog(this);
    filterDlg->setWindowTitle("Find curves");
    if(filterDlg->exec() != QDialog::Accepted)
        return ;

    QString keyword = filterDlg->lineEdit->text();
    qint16  filterType = filterDlg->getFilterType();

    QList<IcvPlotCurve *> curves = plotCanvas->getCurves();
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
            QString strLineId = QString::number(cmd.getLineId());
            isMatch = (keyword == strLineId) ? true : false;
        }
        else if (ICV_BY_DIRECTION == filterType)
        {
            QString strDir = (cmd.getDirection() == 1) ? "DS":"US";
            isMatch = (keyword.compare(strDir, Qt::CaseInsensitive) == 0)? 
                true : false;
        }

        if(true == isMatch)   
        {
            plotCanvas->highlightCurve(curves.at(cnt));
        }

    } 

    plot->replot();

    /*clear memory*/
    delete filterDlg;

    return ;
}


void IcvICurve::showAllCurve()
{
    if(NULL == plotCanvas)
        return ;

    plotCanvas->clearAllCurves();
    for(qint16 pos = 0; pos < plotData.count(); pos++)
    {

        QwtPlotCurve *qwtCurve = new QwtPlotCurve();
        qwtCurve->setPen(QColor::fromHsl(rand()%360,rand()%256,rand()%200), 1.0, Qt::SolidLine);
        qwtCurve->setSamples(plotData[pos].getData().toVector());

        QwtSymbol *symbol = new QwtSymbol( QwtSymbol::NoSymbol,
            QBrush(Qt::yellow), QPen(Qt::red, 2), QSize(8, 8));
        qwtCurve->setSymbol( symbol );
        qwtCurve->setTitle(plotData.value(pos).getCommandTitle());
        qwtCurve->attach(plot);

        IcvPlotCurve *plotCurve = new IcvPlotCurve;
        plotCurve->setCurve(qwtCurve);
        plotCurve->setCanvas(plotCanvas);
        plotCurve->setDataPos(pos);

        plotCanvas->appendCurves(plotCurve);

        plot->setAxisScale( QwtPlot::yLeft, 0, 70 );
        plot->setAxisScale( QwtPlot::xBottom, 0.0, 3000 );
        plot->replot();
    }

    return;
}


void IcvICurve::expandCurve()
{

    IcvPlotCurve *curve = plotCanvas->getSelectedCurve();
    if(NULL == curve)
    {
        QMessageBox::information(this,tr("Info"),tr("No curve selected."));
        return ;
    }

    bool ok;
    int groupSize = QInputDialog::getInt(this, tr("Input"),tr("Group size:"), 
                                         4, 1, 8, 2, &ok);
    if (ok)
    {
        curve->setGroupSize(groupSize);
    }
    
    plot->replot();

    return;
}


void IcvICurve::legendChecked( const QVariant &itemInfo, bool on)
{

    QwtPlotItem *plotItem = plot->infoToItem(itemInfo);

    if (plotItem)
    {
        plotItem->setVisible( on );
        QwtLegend *lgd = dynamic_cast<QwtLegend *>(legend);
        QList<QWidget *> legendWidgets = lgd->legendWidgets(plot->itemToInfo(plotItem));
        if (legendWidgets.size() == 1)
        {
            QwtLegendLabel *legendLabel =
                dynamic_cast<QwtLegendLabel *>(legendWidgets[0]);

            if (legendLabel)
                legendLabel->setChecked( on );
        }

        plot->replot();
    }

    return ;
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

    if(ICU_OK != analyzeData(file))
    {
        return ICU_PLOT_DATA_FORMAT_ERROR;
    }

    return ICU_OK;
}


ICU_RET_STATUS IcvICurve::analyzeData(QFile &file)
{
    qint32  cout = 0;
    bool    ok   = true;
    qint16  line = 0;
    qint16  totalLineNum = 0;
    QRegExp regExp; 
    IcvCommand cmd;
    IcvCommand prevCmd;

    QTextStream dataTextStream(&file);
    totalLineNum = 0;
    while (!dataTextStream.atEnd()) 
    {
        QString line = dataTextStream.readLine();
        totalLineNum++; 
    }


    if(totalLineNum > ICV_MAX_LINE_NUM_BACKGROUD_PROCESS)
    {
        analyProgressDialog = new QProgressDialog(plot);
        analyProgressDialog->setModal(true);
        analyProgressDialog->setRange(0, totalLineNum);
        analyProgressDialog->setWindowTitle("File analyzing...");
        /*display immediately*/
        analyProgressDialog->show();
        analyProgressDialog->repaint();
        
        connect(analyProgressDialog, SIGNAL(canceled()), this, SLOT(cancelAnalyProgressBar()));
    }

    dataTextStream.seek(0);
    isDataAnalyCanceled   = false;
    QStringList cmdFamily = cmd.getFamily();
    line = 0;
    while(!dataTextStream.atEnd() && !isDataAnalyCanceled)
    {
        line++;
        QString dataLine = dataTextStream.readLine();
        qint16 pos       = 0;
        QString curCmdName("NULL");
        for(qint16 i = 0; i< cmdFamily.count(); i++)
        {
            regExp.setPattern(cmdFamily.value(i));
            regExp.setCaseSensitivity(Qt::CaseInsensitive);
            pos = regExp.indexIn(dataLine);
            if(-1 != pos)
            {
                curCmdName = cmdFamily.value(i);
                break;
            }
        }

        if(curCmdName != "NULL") /*try to get parameters of command*/
        {
            QString pattern(curCmdName + "\\s+([0-9]|1[0-9])\\s+([0-1])");		
            regExp.setPattern(pattern);
            pos = regExp.indexIn(dataLine);
            if(-1 != pos)
            {
               /*if a new command found,and the last command is not empty,
                  set the last command complete.
                */
                prevCmd = cmd;

                cmd.reset();
                cmd.setName(curCmdName);  

                QString port(regExp.capturedTexts().at(1));
                cmd.setLineId(port.toInt(&ok));

                QString dir(regExp.capturedTexts().at(2));
                cmd.setDirection(dir.toInt(&ok));

                cmd.setState(CMD_STARTED);
            }

        }

        if(CMD_STARTED == cmd.getState())
        {
            if(prevCmd.getData().count() > 0)      
            {
                plotData.push_back(prevCmd);   
                prevCmd.reset();
            }

            regExp.setPattern("\\s+:\\s+");
            if(-1 == regExp.indexIn(dataLine))
            {
                continue;
            }

            qint16 ret = assembleData(dataLine,&cmd);
            if(ret == ICU_PLOT_DATA_FORMAT_ERROR)
            {
                QString error = file.fileName() + " at line " + QString::number(line) \
                    + ":data format incorrect.";
                QMessageBox::critical(this,"ERROR",error);

                return ICU_PLOT_DATA_FORMAT_ERROR; 
            }
        }

        if(NULL != analyProgressDialog)
            emit analyDataProgress(line);
    }

    /*no more new command found when at file end, save the current data*/
    if(cmd.getData().count() > 0)     
    {
         cmd.setState(CMD_CLOSED);
         plotData.push_back(cmd);   
    }


    if(isDataAnalyCanceled && (NULL != analyProgressDialog))
    {
        delete analyProgressDialog;
        analyProgressDialog = NULL;
        plotData.clear();
    }


    return ICU_OK; 
}


ICU_RET_STATUS IcvICurve::assembleData(QString dataLine, IcvCommand *cmd)
{
    QStringList splitList;
    QStringList digList;

    splitList = dataLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    if(!splitList.contains(":") )  /*only ":"*/
        return ICU_PLOT_DATA_FORMAT_ERROR;

    if((cmd->getName() == "getTxPsd") && splitList.contains("---"))
    {
        splitList = splitList.replaceInStrings("---","-150.0");
    }

    digList = splitList.filter(QRegExp("^\\d+$|^\\d+\\.\\d+$|^-\\d+\\.\\d+$"));

    if(digList.count() > ICV_MAX_NUM_DIGITS_PERLINE ||(digList.count() <=0))
    {
        return ICU_PLOT_DATA_FORMAT_ERROR;
    }

    if(!digList.at(0).at(0).isDigit())
    {
        return ICU_PLOT_DATA_FORMAT_ERROR;
    }

    bool ok = false;
    qint16 toneIndex = digList.at(0).toInt(&ok);
    if(false == ok)
        return ICU_PLOT_DATA_FORMAT_ERROR;

    QList<QPointF> points; 
    QPointF        point;
    qint16         tone = cmd->getData().count();
    for(qint16 i = ICV_PLOT_DATA_START_POS; i < digList.count(); i++)
    {
        qreal dataItem = digList.at(i).toFloat(&ok);
        if(false == ok)
            return ICU_PLOT_DATA_FORMAT_ERROR;

        tone++;
        point.setX(tone);
        point.setY(dataItem);
        points.append(point);
    }

    cmd->setData(points,true);

    if(digList.count() < ICV_MAX_NUM_DIGITS_PERLINE)
        cmd->setState(CMD_CLOSED);

    return ICU_OK;
}


QList <IcvCommand>* IcvICurve::getPlotData()
{
    return &plotData;
}


void IcvICurve::updateAnalyProgressBar(qint16 progress)
{
    if(analyProgressDialog != NULL)
    {
        analyProgressDialog->setValue(progress);
        analyProgressDialog->repaint();
    }

    return ;
}


void IcvICurve::cancelAnalyProgressBar()
{
    isDataAnalyCanceled = true;

    return ;
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


