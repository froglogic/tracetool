/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "variabledumping_qt.h"

TRACELIB_NAMESPACE_BEGIN

template <typename T>
std::string stringFromQVariant( const T &val )
{
    return QVariant::fromValue( val ).toString().toUtf8().data();
}

#define TRACELIB_SPECIALIZE_CONVERSION(T, factoryFn) \
template <> \
VariableValue convertVariable( T val ) { \
    return VariableValue::factoryFn( val ); \
}

#define TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(T) \
template <> \
VariableValue convertVariable( T val ) { \
    return VariableValue::stringValue( stringFromQVariant( val ) ); \
}

TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QString)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QByteArray)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QChar)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QDate)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QDateTime)
TRACELIB_SPECIALIZE_CONVERSION(qlonglong, numberValue)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QStringList)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QTime)
TRACELIB_SPECIALIZE_CONVERSION(qulonglong, numberValue)

#undef TRACELIB_SPECIALIZE_CONVERSION
#undef TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT

TRACELIB_NAMESPACE_END

