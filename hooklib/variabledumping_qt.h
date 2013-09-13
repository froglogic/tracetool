/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_VARIABLEDUMPING_QT_H
#define TRACELIB_VARIABLEDUMPING_QT_H

#include "tracelib_config.h"
#include "variabledumping.h"

#include <qglobal.h>

#if QT_VERSION < 0x040000
#  include <qcstring.h>
#  include <qdatetime.h>
#  include <qstring.h>
#  include <qstringlist.h>
#  include <qvariant.h>
#else
#  include <QByteArray>
#  include <QChar>
#  include <QDate>
#  include <QDateTime>
#  include <QString>
#  include <QStringList>
#  include <QTime>
#  include <QVariant>

#endif

TRACELIB_NAMESPACE_BEGIN

#define TRACELIB_SPECIALIZE_CONVERSION(T, factoryFn) \
template <> \
inline VariableValue convertVariable( T val ) { \
    return VariableValue::factoryFn( val ); \
}

#if QT_VERSION >= 0x040000
TRACELIB_SPECIALIZE_CONVERSION(qlonglong, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(qulonglong, numberValue)
#endif

#undef TRACELIB_SPECIALIZE_CONVERSION

#if QT_VERSION < 0x040000
#  define qtTypeToUtf8Char( value ) QVariant( value ).toString().utf8()
#else
#  define qtTypeToUtf8Char( value ) QVariant::fromValue( value ).toString().toUtf8().data()
#endif

#define TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(T) \
template <> \
inline VariableValue convertVariable( T val ) { \
    return VariableValue::stringValue( qtTypeToUtf8Char( val ) ); \
}

#if QT_VERSION < 0x040000
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QCString) // only in Qt3
// QVariant does not serialize QStringList properly to a string
template <> inline VariableValue convertVariable( QStringList val ) {
    return VariableValue::stringValue( val.join(",").utf8() );
}
#else
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QChar) // no QVariant overload in Qt3
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QStringList)
#endif
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QString)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QByteArray)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QDate)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QDateTime)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QTime)

#undef qtTypeToUtf8Char
#undef TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_VARIABLEDUMPING_QT_H)

