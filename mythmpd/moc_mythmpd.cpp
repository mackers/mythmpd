/****************************************************************************
** Meta object code from reading C++ file 'mythmpd.h'
**
** Created: Sun Jun 24 12:57:15 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mythmpd.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mythmpd.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MythMPD[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x0a,
      27,    8,    8,    8, 0x0a,
      42,    8,    8,    8, 0x08,
      62,    8,    8,    8, 0x08,
      81,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MythMPD[] = {
    "MythMPD\0\0statusTimerDone()\0updateStatus()\0"
    "clicked_PlayQueue()\0clicked_Database()\0"
    "clicked_PlaylistEditor()\0"
};

void MythMPD::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MythMPD *_t = static_cast<MythMPD *>(_o);
        switch (_id) {
        case 0: _t->statusTimerDone(); break;
        case 1: _t->updateStatus(); break;
        case 2: _t->clicked_PlayQueue(); break;
        case 3: _t->clicked_Database(); break;
        case 4: _t->clicked_PlaylistEditor(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MythMPD::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MythMPD::staticMetaObject = {
    { &MythScreenType::staticMetaObject, qt_meta_stringdata_MythMPD,
      qt_meta_data_MythMPD, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MythMPD::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MythMPD::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MythMPD::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MythMPD))
        return static_cast<void*>(const_cast< MythMPD*>(this));
    return MythScreenType::qt_metacast(_clname);
}

int MythMPD::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MythScreenType::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
