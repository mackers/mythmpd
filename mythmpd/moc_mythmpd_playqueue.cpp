/****************************************************************************
** Meta object code from reading C++ file 'mythmpd_playqueue.h'
**
** Created: Sun Jun 24 12:57:15 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mythmpd_playqueue.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mythmpd_playqueue.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MythMPD_PlayQueue[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x08,
      48,   34,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MythMPD_PlayQueue[] = {
    "MythMPD_PlayQueue\0\0clicked_Back()\0"
    "playlist_item\0clicked_track(MythUIButtonListItem*)\0"
};

void MythMPD_PlayQueue::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MythMPD_PlayQueue *_t = static_cast<MythMPD_PlayQueue *>(_o);
        switch (_id) {
        case 0: _t->clicked_Back(); break;
        case 1: _t->clicked_track((*reinterpret_cast< MythUIButtonListItem*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MythMPD_PlayQueue::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MythMPD_PlayQueue::staticMetaObject = {
    { &MythScreenType::staticMetaObject, qt_meta_stringdata_MythMPD_PlayQueue,
      qt_meta_data_MythMPD_PlayQueue, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MythMPD_PlayQueue::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MythMPD_PlayQueue::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MythMPD_PlayQueue::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MythMPD_PlayQueue))
        return static_cast<void*>(const_cast< MythMPD_PlayQueue*>(this));
    return MythScreenType::qt_metacast(_clname);
}

int MythMPD_PlayQueue::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MythScreenType::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
