#ifndef ICV_COMMAND_H
#define ICV_COMMAND_H
#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QList>
#include <QPointF>
#include <QPair>

#define CMD_NOT_FOUND                (0)
#define CMD_TITLE_MATCHED            (1)
#define CMD_GROUPSIZE_MATCHED        (2)
#define CMD_PLOTDATA_MATCHED         (3)
#define CMD_CLOSED                   (4)
#define ICV_PROMT_RFC                (1)
#define ICV_PROMT_BCM                (2)
#define ICV_PROMT_API                (3)
#define ICV_PROMT_FAST               (4)

class IcvCommand
{
public:
    IcvCommand();
    IcvCommand(QString cmd);
    IcvCommand(QString cmd, qint16 line, qint16 dir);
    ~IcvCommand();

    void reset();
    void initFamily();
    void initPromtFamily();
    void initTitlePattern();
    void initDataPattern();
    void setFamily(QStringList cmdFamily);
    QStringList getFamily();
    void setPrompt(QString promt);
    QString getPromt();
    QStringList getPromtFamily();
    bool matchGroupSize(QString dataLine);
    QList< QString> getTitlePattern();
    QString getDataPattern();
    QPair<QString, QString> getSpecReplace();
    qint16  getGroupSize();

    void setDataScopeMode(qint16 scope);
    qint16 getDataScopeMode();

    void setName(QString cmd);	
    QString getName();

    void setLineId(qint16 id);	
    qint16 getLineId();

    void setDirection(qint16 dir);	
    qint16 getDirection();

    void setData(QList<QPointF> points, bool isappend);
    QList<QPointF> getData();
    void clearData();

    void setState(qint16 cmdState);
    qint16 getState();
    
    void setDataPosInFile(qint32 pos);
    qint32  getDataPosInFile();

    void setBriefInfo(QString info);
    QString getBriefInfo();

    void setFileName(QString name);
    QString getFileName();
    
    QString getTitle();
    inline bool operator == (IcvCommand &cmd)
    {
        if((getFileName() == cmd.getFileName())&&
            (getDataPosInFile() == cmd.getDataPosInFile()))
            return true;
        return false;
    }

private:
    QString        name;
    QString        promt;
    QList<QPointF> data;
    qint16         groupSize;
    qint16         dataScopeMode;
    QStringList    family;
    QStringList    promtFamily;
    QList< QString> titlePattern;
    QList<QPair<QString, QString>> dataPattern;

    qint16 lineId;
    qint16 direction;
    qint16 state;
    QString fileName;
    qint32 dataPosInFile;
    QString briefInfo;
};

#endif 

