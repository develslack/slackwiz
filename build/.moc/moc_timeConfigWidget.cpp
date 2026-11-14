/****************************************************************************
** Meta object code from reading C++ file 'timeConfigWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../src/ui/timeConfigWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'timeConfigWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TimeConfigWidget_t {
    QByteArrayData data[11];
    char stringdata0[157];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TimeConfigWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TimeConfigWidget_t qt_meta_stringdata_TimeConfigWidget = {
    {
QT_MOC_LITERAL(0, 0, 16), // "TimeConfigWidget"
QT_MOC_LITERAL(1, 17, 21), // "onMapTimezoneSelected"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 10), // "timezoneId"
QT_MOC_LITERAL(4, 51, 22), // "onComboTimezoneChanged"
QT_MOC_LITERAL(5, 74, 5), // "index"
QT_MOC_LITERAL(6, 80, 12), // "onDstToggled"
QT_MOC_LITERAL(7, 93, 7), // "checked"
QT_MOC_LITERAL(8, 101, 17), // "onSyncModeChanged"
QT_MOC_LITERAL(9, 119, 22), // "updateLiveClockPreview"
QT_MOC_LITERAL(10, 142, 14) // "onApplyClicked"

    },
    "TimeConfigWidget\0onMapTimezoneSelected\0"
    "\0timezoneId\0onComboTimezoneChanged\0"
    "index\0onDstToggled\0checked\0onSyncModeChanged\0"
    "updateLiveClockPreview\0onApplyClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TimeConfigWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x08 /* Private */,
       4,    1,   47,    2, 0x08 /* Private */,
       6,    1,   50,    2, 0x08 /* Private */,
       8,    0,   53,    2, 0x08 /* Private */,
       9,    0,   54,    2, 0x08 /* Private */,
      10,    0,   55,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void TimeConfigWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TimeConfigWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onMapTimezoneSelected((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->onComboTimezoneChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onDstToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->onSyncModeChanged(); break;
        case 4: _t->updateLiveClockPreview(); break;
        case 5: _t->onApplyClicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TimeConfigWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_TimeConfigWidget.data,
    qt_meta_data_TimeConfigWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TimeConfigWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TimeConfigWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TimeConfigWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int TimeConfigWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
