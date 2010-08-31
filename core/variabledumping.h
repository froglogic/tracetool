/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_VARIABLEDUMPING_H
#define TRACELIB_VARIABLEDUMPING_H

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
    static VariableValue stringValue( const std::string &s );

    VariableType::Value type() const;
    const std::string &asString() const;

private:
    explicit VariableValue( const std::string &s );

    const VariableType::Value m_type;
    const std::string m_string;
};

template <typename T>
VariableValue convertVariable( T o );

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

VariableSnapshot &operator<<( VariableSnapshot &snapshot, AbstractVariable *v );

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_VARIABLEDUMPING_H)

