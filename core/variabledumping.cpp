/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "variabledumping.h"
#include "config.h"

#include <sstream>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

VariableValue VariableValue::stringValue( const string &s )
{
    VariableValue var;
    var.m_type = VariableType::String;
    var.m_string = s;
    return var;
}

VariableValue VariableValue::numberValue( unsigned long v )
{
    VariableValue var;
    var.m_type = VariableType::Number;
    var.m_primitiveValue.number = v;
    return var;
}

VariableValue VariableValue::booleanValue( bool v )
{
    VariableValue var;
    var.m_type = VariableType::Boolean;
    var.m_primitiveValue.boolean = v;
    return var;
}

VariableValue VariableValue::floatValue( long double v )
{
    VariableValue var;
    var.m_type = VariableType::Float;
    var.m_primitiveValue.float_ = v;
    return var;
}

VariableType::Value VariableValue::type() const
{
    return m_type;
}

const string &VariableValue::asString() const
{
    return m_string;
}

unsigned long VariableValue::asNumber() const
{
    return m_primitiveValue.number;
}

bool VariableValue::asBoolean() const
{
    return m_primitiveValue.boolean;
}

long double VariableValue::asFloat() const
{
    return m_primitiveValue.float_;
}

VariableValue::VariableValue()
    : m_type( VariableType::Unknown )
{
}

VariableSnapshot &operator<<( VariableSnapshot &snapshot, AbstractVariable *v )
{
    snapshot.push_back( v );
    return snapshot;
}

template <typename T>
string stringFromStringStream( const T &val ) {
    ostringstream str;
    str << val;
    return str.str();
}

#define TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(T) \
template <> \
VariableValue convertVariable( T val ) { \
    return VariableValue::stringValue( stringFromStringStream( val ) ); \
}

TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(bool)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(short)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(unsigned short)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(int)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(unsigned int)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(long)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(unsigned long)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(float)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(double)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(long double)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const void *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(char)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(signed char)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(unsigned char)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const signed char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const unsigned char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(std::string)

#undef TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM

TRACELIB_NAMESPACE_END

#ifdef HAVE_QT
#  include <QtCore/QByteArray>
#  include <QChar>
#  include <QDate>
#  include <QDateTime>
#  include <QString>
#  include <QStringList>
#  include <QTime>
#  include <QVariant>

TRACELIB_NAMESPACE_BEGIN

template <typename T>
string stringFromQVariant( const T &val )
{
    return QVariant::fromValue( val ).toString().toUtf8().data();
}

#  define TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(T) \
template <> \
VariableValue convertVariable( T val ) { \
    return VariableValue::stringValue( stringFromQVariant( val ) ); \
}

TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QString)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QByteArray)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QChar)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QDate)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QDateTime)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(qlonglong)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QStringList)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(QTime)
TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT(qulonglong)

#  undef TRACELIB_SPECIALIZE_CONVERSION_USING_QVARIANT

TRACELIB_NAMESPACE_END

#endif

