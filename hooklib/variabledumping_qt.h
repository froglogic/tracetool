/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#  include <qmap.h>
#  include <qvariant.h>
#else
#  include <QByteArray>
#  include <QChar>
#  include <QDate>
#  include <QDateTime>
#  include <QString>
#  include <QStringList>
#  include <QTime>
#  include <QMap>
#  include <QVariant>

#endif

TRACELIB_NAMESPACE_BEGIN

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

inline QString variantMapToString( const QMap<QString, QVariant> &map );

inline QString variantToString( const QVariant &variant ) {
    if( variant.type() == QVariant::Map ) {
        return variantMapToString( variant.toMap() );
    }
    return variant.toString();
}

inline QString variantMapToString( const QMap<QString, QVariant> &map ) {
    QStringList keysandvalues;
    QMap<QString, QVariant>::const_iterator it;
    for( it = map.begin(); it != map.end(); ++it ) {
#if QT_VERSION < 0x040000
        QVariant value = it.data();
#else
        QVariant value = it.value();
#endif
        keysandvalues << ( "'" + it.key() + "': " + variantToString( value ) );
    }
    return QString("{%1}").arg( keysandvalues.join(", ") );
}

template <>
inline VariableValue convertVariable( QMap<QString, QVariant> val ) {
    return VariableValue::stringValue( qtTypeToUtf8Char( variantMapToString( val ) ) );
}

template <>
inline VariableValue convertVariable( QMap<QString, QString> val ) {
    QMap<QString, QVariant> dummymap;
    QMap<QString, QString>::const_iterator it;
    for( it = val.begin(); it != val.end(); ++it ) {
#if QT_VERSION < 0x040000
        dummymap[it.key()] = it.data();
#else
        dummymap[it.key()] = it.value();
#endif
    }
    return convertVariable( dummymap );
}

#undef qtTypeToUtf8Char
#undef TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_VARIABLEDUMPING_QT_H)

