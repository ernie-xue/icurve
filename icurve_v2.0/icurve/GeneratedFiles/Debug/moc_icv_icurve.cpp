/****************************************************************************
** Meta object code from reading C++ file 'icv_icurve.h'
**
** Created: Sun Jun 19 02:35:43 2016
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/icv_icurve.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'icv_icurve.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IcvICurve[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      32,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   11,   10,   10, 0x05,
      69,   46,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     117,   10,   10,   10, 0x08,
     128,   10,   10,   10, 0x08,
     137,   10,   10,   10, 0x08,
     149,   10,   10,   10, 0x08,
     160,   10,   10,   10, 0x08,
     172,   10,   10,   10, 0x08,
     185,   10,   10,   10, 0x08,
     199,   10,   10,   10, 0x08,
     213,   10,   10,   10, 0x08,
     225,   10,   10,   10, 0x08,
     240,   10,   10,   10, 0x08,
     257,   10,   10,   10, 0x08,
     273,   10,   10,   10, 0x08,
     289,   10,   10,   10, 0x08,
     305,   10,   10,   10, 0x08,
     322,   10,   10,   10, 0x08,
     336,   10,   10,   10, 0x08,
     363,  350,   10,   10, 0x08,
     398,   10,   10,   10, 0x08,
     420,   10,   10,   10, 0x08,
     436,   10,   10,   10, 0x08,
     450,   10,   10,   10, 0x08,
     465,   10,   10,   10, 0x08,
     480,   10,   10,   10, 0x08,
     495,   10,   10,   10, 0x08,
     513,   10,   10,   10, 0x08,
     528,   10,   10,   10, 0x08,
     558,  546,   10,   10, 0x08,
     587,   11,   10,   10, 0x08,
     618,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_IcvICurve[] = {
    "IcvICurve\0\0progress\0analyDataProgress(qint16)\0"
    "name,position,lineInfo\0"
    "displayCurveInfoSignal(QString,QString,QString)\0"
    "openFile()\0saveAs()\0closePlot()\0"
    "cutCurve()\0copyCurve()\0pasteCurve()\0"
    "removeCurve()\0deleteCurve()\0findCurve()\0"
    "showAllCurve()\0selectAllCurve()\0"
    "setCurveColor()\0setCurveWidth()\0"
    "setCurveStyle()\0setCurveMarker()\0"
    "expandCurve()\0filterCurve()\0type,keyword\0"
    "filterCurvePreview(qint16,QString)\0"
    "recoverCurveVisible()\0showCurveInfo()\0"
    "insertTitle()\0insertXLabel()\0"
    "insertYLabel()\0insertLegend()\0"
    "insertCurveName()\0insertFooter()\0"
    "insertIndicator()\0itemInfo,on\0"
    "legendChecked(QVariant,bool)\0"
    "updateAnalyProgressBar(qint16)\0"
    "cancelAnalyProgressBar()\0"
};

const QMetaObject IcvICurve::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_IcvICurve,
      qt_meta_data_IcvICurve, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IcvICurve::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IcvICurve::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IcvICurve::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IcvICurve))
        return static_cast<void*>(const_cast< IcvICurve*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int IcvICurve::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: analyDataProgress((*reinterpret_cast< qint16(*)>(_a[1]))); break;
        case 1: displayCurveInfoSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 2: openFile(); break;
        case 3: saveAs(); break;
        case 4: closePlot(); break;
        case 5: cutCurve(); break;
        case 6: copyCurve(); break;
        case 7: pasteCurve(); break;
        case 8: removeCurve(); break;
        case 9: deleteCurve(); break;
        case 10: findCurve(); break;
        case 11: showAllCurve(); break;
        case 12: selectAllCurve(); break;
        case 13: setCurveColor(); break;
        case 14: setCurveWidth(); break;
        case 15: setCurveStyle(); break;
        case 16: setCurveMarker(); break;
        case 17: expandCurve(); break;
        case 18: filterCurve(); break;
        case 19: filterCurvePreview((*reinterpret_cast< qint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 20: recoverCurveVisible(); break;
        case 21: showCurveInfo(); break;
        case 22: insertTitle(); break;
        case 23: insertXLabel(); break;
        case 24: insertYLabel(); break;
        case 25: insertLegend(); break;
        case 26: insertCurveName(); break;
        case 27: insertFooter(); break;
        case 28: insertIndicator(); break;
        case 29: legendChecked((*reinterpret_cast< const QVariant(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 30: updateAnalyProgressBar((*reinterpret_cast< qint16(*)>(_a[1]))); break;
        case 31: cancelAnalyProgressBar(); break;
        default: ;
        }
        _id -= 32;
    }
    return _id;
}

// SIGNAL 0
void IcvICurve::analyDataProgress(qint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void IcvICurve::displayCurveInfoSignal(QString _t1, QString _t2, QString _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
