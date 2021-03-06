#ifndef ICV_CURVE_FILTER_H
#define ICV_CURVE_FILTER_H
#include <QDialog>
#include <QString>
#include "ui_icv_curve_filter.h"


#define ICV_BY_COMPLETECOMAND (1)
#define ICV_BY_COMANDNAME     (2)
#define ICV_BY_LINEID         (3)
#define ICV_BY_DIRECTION      (4)
#define ICV_BY_PROMT          (5)
#define ICV_BY_POS            (6)


class IcvCurveFilterDialog : public QDialog, public Ui::IcvCurveFilterDialog
{
    Q_OBJECT
public:
    IcvCurveFilterDialog(QWidget* parent=0);
    ~IcvCurveFilterDialog();

    qint16 getLookupType();
    QString getKeyword();
    void setKeyword();
    bool checkValidKeyword();
    Qt::CheckState getInAllCheckState();
    void showMoreSelect();
    void hideMoreSelect();

private slots:
    void accept();
    void reject();
    void prepareCommitAction();
    void toggleMoreSelect(bool needMore);
    void togglePreview(bool isPreview);
    
private:
    qint16 lookupType;
    QCompleter *completer;
    QStringList keywords;
    QString keyword;

signals:
    void previewSignal(qint16 filterType, QString keyword, qint16);
    void recoverPreviewSignal();
};

#endif
