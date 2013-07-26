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
    TRACELIB_EXPORT static VariableValue numberValue( unsigned long v );
    TRACELIB_EXPORT static VariableValue booleanValue( bool v );
    TRACELIB_EXPORT static VariableValue floatValue( long double v );
    TRACELIB_EXPORT static size_t convertToString( const VariableValue &v, char *buf, size_t bufsize );

    TRACELIB_EXPORT ~VariableValue();

    VariableType::Value type() const;
    const std::string &asString() const;
    unsigned long asNumber() const;
    bool asBoolean() const;
    long double asFloat() const;

private:
    VariableValue();

    VariableType::Value m_type;
    std::string m_string;
    union {
        unsigned long number;
        bool boolean;
        long double float_;
    } m_primitiveValue;
};

template <typename T>
VariableValue convertVariable( T o );

#define TRACELIB_SPECIALIZE_CONVERSION(T, factoryFn) \
template <> \
inline VariableValue convertVariable( T val ) { \
    return VariableValue::factoryFn( val ); \
}

TRACELIB_SPECIALIZE_CONVERSION(bool, booleanValue)
TRACELIB_SPECIALIZE_CONVERSION(short, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(unsigned short, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(int, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(unsigned int, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(long, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(unsigned long, numberValue)
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

class AbstractVariable
{
public:
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

typedef std::vector<AbstractVariable *> VariableSnapshot;

TRACELIB_EXPORT VariableSnapshot &operator<<( VariableSnapshot &snapshot, AbstractVariable *v );

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_VARIABLEDUMPING_H)

