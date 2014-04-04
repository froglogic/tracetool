/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_VARIABLEDUMPING_H
#define TRACELIB_VARIABLEDUMPING_H

#include "dlldefs.h"
#include "tracelib_config.h"

#include <stdio.h> // for snprintf

#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#if defined(_MSC_VER) && _MSC_VER < 1300
// MSVC6 has no long long
typedef __int64 vlonglong;
typedef unsigned __int64 vulonglong;
#else
typedef long long vlonglong;
typedef unsigned long long vulonglong;
#endif

TRACELIB_NAMESPACE_BEGIN

struct VariableType {
    enum Value {
        Unknown = 0
#define TRACELIB_VARIABLETYPE(name) ,name
#include "variabletypes.def"
#undef TRACELIB_VARIABLETYPE
    };

    static const int *values() {
        static const int a[] = {
            Unknown,
#define TRACELIB_VARIABLETYPE(name) name,
#include "variabletypes.def"
#undef TRACELIB_VARIABLETYPE
            -1
        };
        return a;
    }

    static const char *valueAsString( Value v ) {
#define TRACELIB_VARIABLETYPE(name) static const char str_##name[] = #name;
#include "variabletypes.def"
#undef TRACELIB_VARIABLETYPE
        switch ( v ) {
            case Unknown: return "Unknown";
#define TRACELIB_VARIABLETYPE(name) case name: return str_##name;
#include "variabletypes.def"
#undef TRACELIB_VARIABLETYPE
        }
        return 0;
    }
};

class VariableValue {
public:
    TRACELIB_EXPORT static VariableValue stringValue( const char *s );
    TRACELIB_EXPORT static VariableValue numberValue( vlonglong v );
    TRACELIB_EXPORT static VariableValue numberValue( vulonglong v );
    TRACELIB_EXPORT static VariableValue booleanValue( bool v );
    TRACELIB_EXPORT static VariableValue floatValue( long double v );
    TRACELIB_EXPORT static size_t convertToString( const VariableValue &v, char *buf, size_t bufsize );

    TRACELIB_EXPORT VariableValue( const VariableValue &other );
    TRACELIB_EXPORT ~VariableValue();

    VariableType::Value type() const;
    const char *asString() const;
    vulonglong asNumber() const;
    bool asBoolean() const;
    long double asFloat() const;
    bool isSignedNumber() const;

private:
    VariableValue();

    VariableType::Value m_type;
    union {
        vulonglong number;
        bool boolean;
        long double float_;
        char *string;
    } m_primitiveValue;
    bool m_isSignedNumber;
};

template <typename T>
VariableValue convertVariable( T o );

#define TRACELIB_SPECIALIZE_CONVERSION(T, factoryFn) \
template <> \
inline VariableValue convertVariable( T val ) { \
    return VariableValue::factoryFn( val ); \
}

TRACELIB_SPECIALIZE_CONVERSION(bool, booleanValue)
TRACELIB_SPECIALIZE_CONVERSION(float, floatValue)
TRACELIB_SPECIALIZE_CONVERSION(double, floatValue)
TRACELIB_SPECIALIZE_CONVERSION(long double, floatValue)
TRACELIB_SPECIALIZE_CONVERSION(vlonglong, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(vulonglong, numberValue)

#undef TRACELIB_SPECIALIZE_CONVERSION

#define TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(T, castType) \
template <> \
inline VariableValue convertVariable( T val ) { \
    return VariableValue::numberValue( static_cast<castType>( val ) ); \
}

TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(short, vlonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(unsigned short, vulonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(int, vlonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(unsigned int, vulonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(long, vlonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(unsigned long, vulonglong)

/* A couple of specializations for MSVC6 since the compiler
 * doesn't seem to take the existing ulonglong/longlong
 * specializations.
 */
#if defined(_MSC_VER) && _MSC_VER <= 1200
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(__int8, vlonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(__int16, vlonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(__int32, vlonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(unsigned __int8, vulonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(unsigned __int16, vulonglong)
TRACELIB_SPECIALIZE_CONVERSION_INTEGRAL(unsigned __int32, vulonglong)
#endif

#define TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(T) \
template <> \
inline VariableValue convertVariable( T val ) { \
    std::ostringstream str; \
    str << val; \
    return VariableValue::stringValue( str.str().c_str() ); \
}

TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(char)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(signed char)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(unsigned char)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(signed char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(unsigned char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const signed char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const unsigned char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(std::string)

#undef TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif
// msvc's stringstream does not prepend a 0x to pointers, so do that manually
template <>
inline VariableValue convertVariable( const void *val ) {
    std::vector<char> buf( 11 );
    snprintf( &buf[0], 11, "0x%08X", (ptrdiff_t)val );
    return convertVariable( &buf[0] );
}
#if defined(_MSC_VER)
#undef snprintf
#endif
template<>
inline VariableValue convertVariable( void *val ) {
    return convertVariable<const void *>( val );
}

class AbstractVariable
{
public:
    virtual ~AbstractVariable() {}
    virtual const char *name() const = 0;
    virtual VariableValue value() const = 0;
};

template <typename T>
class Variable : public AbstractVariable
{
public:
    Variable( const char *name, const T &o ) : m_name( name ), m_o( o ) { }

    const char *name() const { return m_name; }

    virtual VariableValue value() const {
        return convertVariable( m_o );
    }

private:
    const char *m_name;
    const T &m_o;
};

template <typename T>
AbstractVariable *makeConverter( const char *name, const T &o ) {
    return new Variable<T>( name, o );
}

class VariableSnapshot
{
public:
    TRACELIB_EXPORT VariableSnapshot();
    TRACELIB_EXPORT ~VariableSnapshot();

    TRACELIB_EXPORT VariableSnapshot &operator<<( AbstractVariable *v );

    TRACELIB_EXPORT size_t size() const;
    TRACELIB_EXPORT AbstractVariable *&operator[]( size_t idx );

private:
    std::auto_ptr<std::vector<AbstractVariable *> > m_variables;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_VARIABLEDUMPING_H)

