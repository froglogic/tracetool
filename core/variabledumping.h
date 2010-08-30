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

class VariableValue {
public:
    enum Type {
        String
    };

    static VariableValue stringValue( const std::string &s );

    Type type() const;
    const std::string &asString() const;

private:
    explicit VariableValue( const std::string &s );

    const Type m_type;
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

