/****************************************************************************
** MythMPD meta object code from reading C++ file 'mythmpd.h'
**
** Created: Sun Nov 11 17:38:50 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.7   edited Oct 19 16:22 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "mythmpd.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *MythMPD::className() const
{
    return "MythMPD";
}

QMetaObject *MythMPD::metaObj = 0;
static QMetaObjectCleanUp cleanUp_MythMPD( "MythMPD", &MythMPD::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString MythMPD::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MythMPD", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString MythMPD::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MythMPD", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* MythMPD::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = MythThemedDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::Out }
    };
    static const QUMethod slot_0 = {"updateStatus", 1, param_slot_0 };
    static const QUMethod slot_1 = {"statusTimerDone", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "updateStatus()", &slot_0, QMetaData::Public },
	{ "statusTimerDone()", &slot_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"MythMPD", parentObject,
	slot_tbl, 2,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_MythMPD.setMetaObject( metaObj );
    return metaObj;
}

void* MythMPD::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "MythMPD" ) )
	return this;
    return MythThemedDialog::qt_cast( clname );
}

bool MythMPD::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: static_QUType_int.set(_o,updateStatus()); break;
    case 1: statusTimerDone(); break;
    default:
	return MythThemedDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool MythMPD::qt_emit( int _id, QUObject* _o )
{
    return MythThemedDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool MythMPD::qt_property( int id, int f, QVariant* v)
{
    return MythThemedDialog::qt_property( id, f, v);
}

bool MythMPD::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
