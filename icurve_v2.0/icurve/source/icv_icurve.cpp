
#include <Qt>
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


#include "icv_icurve.h"

#define ICV_MAX_NUM_DIGITS_PERLINE    (11)  /*including tone index at head of the line*/
#define ICV_PLOT_DATA_START_POS       (1)


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
#if 0
    QComboBox *pComboBox = new QComboBox;
    pComboBox->setItemDelegate(new QPenStyleDelegate((QObject *)pComboBox));
    pComboBox->addItem(tr("Solid"),Qt::SolidLine);
    pComboBox->addItem(tr("Dash"),Qt::DashLine);
    pComboBox->addItem(tr("Dot"),Qt::DotLine);

    setCentralWidget(pComboBox);
#endif
    plotCanvas = new IcvPlotCanvas(this) ;
    plotCanvas->createCurvePopMenuAction();
    plotCanvas->createCurvePopMenu();


    analyProgressDialog  = NULL;
    isDataAnalyCanceled  = false;
    /*signals and slots*/
    connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui.actionTitle, SIGNAL(triggered()), this, SLOT(insertTitle()));
    connect(ui.actionX_label, SIGNAL(triggered()), this, SLOT(insertXLabel()));
    connect(ui.actionY_label, SIGNAL(triggered()), this, SLOT(insertYLabel()));
    connect(ui.actionFooter, SIGNAL(triggered()), this, SLOT(insertFooter()));
    
    connect(this, SIGNAL(analyDataProgress(qint16)), this, SLOT(updateAnalyProgressBar(qint16)));

}


IcvICurve::~IcvICurve()
{

}


void IcvICurve::initMainWinStyle(QMainWindow *self)
{
    self->setWindowTitle("iCurve");
    self->setWindowIcon(QIcon(":/images/iCurve.ico"));
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

    QwtPlotLegendItem *legend= new QwtPlotLegendItem();
    legend->attach(plot);

    grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(Qt::DashLine));
    grid->setMinorPen(QPen(Qt::DashLine));  
    grid->attach( plot );

    ( void ) new QwtPlotPanner( plot->canvas());
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
        //QMessageBox::information(this,"info","no file retrieved!");
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
    }

    return ;
}


void IcvICurve::insertTitle()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Input"),
        tr("Plot title:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
        plot->setTitle(text);
}



void IcvICurve::insertXLabel()
{
    bool ok;
    QString label = plot->axisTitle(QwtPlot::xBottom).text();
    QString text = QInputDialog::getText(this, tr("Input"),
        tr("X lable:"), QLineEdit::Normal, label, &ok);
    if (ok)
        plot->setAxisTitle(QwtPlot::xBottom,text);

#if 0
    QDialog *diaglog = new QDialog(this);
    diaglog->setFixedSize (300,100);

    QLabel *label = new QLabel(tr("X label:"));
    QLineEdit *lineEdit = new QLineEdit(diaglog);
    lineEdit->setFixedWidth(200);
    label->setBuddy(lineEdit);

    QPushButton *okButton = new QPushButton(tr("OK"));
    okButton->setDefault(true);

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setAutoDefault(false);

    QGridLayout *gridLayout = new QGridLayout(diaglog);

    /*{{{button*/
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok \
        |QDialogButtonBox::Cancel);

    gridLayout->addWidget(label,0,0);
    gridLayout->addWidget(lineEdit,0,1);
    gridLayout->addWidget(buttonBox,1,1);

   // gridLayout->addWidget(okButton,1,0);
   // gridLayout->addWidget(cancelButton,1,1,1,1);

#if 0
    diaglog->setLayout(gridLayout);
    diaglog->resize(40,80);
    connect(okButton, SIGNAL(clicked()), diaglog, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), diaglog, SLOT(reject()));
   
#endif
   
    diaglog->setLayout(gridLayout);
    connect(buttonBox,SIGNAL(accept()),diaglog,SLOT(accept()));
   connect(buttonBox,SIGNAL(rejected()),diaglog,SLOT(close()));

    /*}}}*/

    if(diaglog->exec() == QDialog::Accepted)
    {
        plot->setAxisTitle(QwtPlot::xBottom,lineEdit->text());
        return ;

    }
    else if(diaglog->exec() == QDialog::Rejected)  
    {



    }

#endif
    return;

}


void IcvICurve::insertYLabel()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Input"),
        tr("Y lable:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
        plot->setAxisTitle(QwtPlot::yLeft,text);

    return ;
}


void IcvICurve::insertFooter()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Input"),
        tr("Footer"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
        plot->setFooter(text);

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
    Command cmd;
    Command prevCmd;

    QTextStream dataTextStream(&file);
    totalLineNum = 0;
    while (!dataTextStream.atEnd()) 
    {
        QString line = dataTextStream.readLine();
        totalLineNum++; 
    }

    if(totalLineNum > 2000)
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
        plotData.clear();
    }


    return ICU_OK; 
}


ICU_RET_STATUS IcvICurve::assembleData(QString dataLine, Command *cmd)
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


QList <Command> * IcvICurve::getPlotData()
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


