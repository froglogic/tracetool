/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_VARIABLEDUMPING_H
#define TRACELIB_VARIABLEDUMPING_H

#include "dlldefs.h"
#include "tracelib_config.h"

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

#if defined(_MSC_VER) && _MSC_VER < 1800
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int8 int8_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
#else
#include <stdint.h>
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
    TRACELIB_EXPORT static VariableValue numberValue( uint8_t v ) { return numberValue( static_cast<uint64_t>( v ) ); }
    TRACELIB_EXPORT static VariableValue numberValue( uint16_t v ) { return numberValue( static_cast<uint64_t>( v ) ); }
    TRACELIB_EXPORT static VariableValue numberValue( uint32_t v ) { return numberValue( static_cast<uint64_t>( v ) ); }
    TRACELIB_EXPORT static VariableValue numberValue( uint64_t v );
    TRACELIB_EXPORT static VariableValue numberValue( int8_t v ) { return numberValue( static_cast<int64_t>( v ) ); }
    TRACELIB_EXPORT static VariableValue numberValue( int16_t v ) { return numberValue( static_cast<int64_t>( v ) ); }
    TRACELIB_EXPORT static VariableValue numberValue( int32_t v ) { return numberValue( static_cast<int64_t>( v ) ); }
    TRACELIB_EXPORT static VariableValue numberValue( int64_t v );
    TRACELIB_EXPORT static VariableValue booleanValue( bool v );
    TRACELIB_EXPORT static VariableValue floatValue( long double v );
    TRACELIB_EXPORT static size_t convertToString( const VariableValue &v, char *buf, size_t bufsize );

    TRACELIB_EXPORT VariableValue( const VariableValue &other );
    TRACELIB_EXPORT ~VariableValue();

    VariableType::Value type() const;
    const char *asString() const;
    uint64_t asNumber() const;
    bool asBoolean() const;
    long double asFloat() const;
    bool isSignedNumber() const;

private:
    VariableValue();

    VariableType::Value m_type;
    union {
        uint64_t number;
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
TRACELIB_SPECIALIZE_CONVERSION(int8_t, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(uint8_t, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(int16_t, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(uint16_t, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(int32_t, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(uint32_t, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(int64_t, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(uint64_t, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(float, floatValue)
TRACELIB_SPECIALIZE_CONVERSION(double, floatValue)
TRACELIB_SPECIALIZE_CONVERSION(long double, floatValue)

#undef TRACELIB_SPECIALIZE_CONVERSION

#define TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(T) \
template <> \
inline VariableValue convertVariable( T val ) { \
    std::ostringstream str; \
    str << val; \
    return VariableValue::stringValue( str.str().c_str() ); \
}

TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(void *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const void *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(signed char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(unsigned char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const signed char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const unsigned char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(std::string)

#undef TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM

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

    inline size_t size() const { return m_variables.size(); }
    AbstractVariable *&operator[]( size_t idx ) { return m_variables[idx]; }

private:
    std::vector<AbstractVariable *> m_variables;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_VARIABLEDUMPING_H)

