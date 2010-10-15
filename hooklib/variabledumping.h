/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_VARIABLEDUMPING_H
#define TRACELIB_VARIABLEDUMPING_H

#include "dlldefs.h"
#include "tracelib_config.h"

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
    TRACELIB_EXPORT static VariableValue stringValue( const std::string &s );
    TRACELIB_EXPORT static VariableValue numberValue( unsigned long v );
    TRACELIB_EXPORT static VariableValue booleanValue( bool v );
    TRACELIB_EXPORT static VariableValue floatValue( long double v );
    TRACELIB_EXPORT static std::string convertToString( const VariableValue &v );

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

#define TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(T) \
template <> \
TRACELIB_EXPORT VariableValue convertVariable( T val );

TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(bool)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(short)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(unsigned short)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(int)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(unsigned int)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(long)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(unsigned long)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(float)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(double)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(long double)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(void *)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(const void *)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(char)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(signed char)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(unsigned char)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(char *)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(signed char *)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(unsigned char *)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(const char *)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(const signed char *)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(const unsigned char *)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(std::string)

#undef TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION

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

