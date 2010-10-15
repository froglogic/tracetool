/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_VARIABLEDUMPING_QT_H
#define TRACELIB_VARIABLEDUMPING_QT_H

#include "tracelib_config.h"
#include "variabledumping.h"

#include <QByteArray>
#include <QChar>
#include <QDate>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QVariant>

TRACELIB_NAMESPACE_BEGIN

#define TRACELIB_SPECIALIZE_CONVERSION(T, factoryFn) \
template <> \
inline VariableValue convertVariable( T val ) { \
    return VariableValue::factoryFn( val ); \
}

TRACELIB_SPECIALIZE_CONVERSION(qlonglong, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(qulonglong, numberValue)

#undef TRACELIB_SPECIALIZE_CONVERSION

#define TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(T) \
template <> \
inline VariableValue convertVariable( T val ) { \
    return VariableValue::stringValue( QVariant::fromValue( val ).toString().toUtf8().data() ); \
}

TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QString)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QByteArray)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QChar)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QDate)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QDateTime)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QStringList)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QTime)

#undef TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_VARIABLEDUMPING_QT_H)

