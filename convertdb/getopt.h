/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef GETOPT_H
#define GETOPT_H

#include <QList>
#include <QMap>
#include <QStringList>

class GetOpt {
public:
    GetOpt();
    GetOpt( int offset );
    GetOpt( int offset, int argc, char *argv[] );
    GetOpt( int argc, char *argv[] );
    GetOpt( const QStringList &a );

    QString appName() const { return aname; }

    void addCommandFileOption( char s = 'c',
                               const QString &l =
                               QString::fromLatin1( "commandFile" ) );

    // switch (no arguments)
    void addSwitch( const QString &lname, bool *b );

    // options (with arguments, sometimes optional)
    void addOption( char s, const QString &l, QString *v );
    void addVarLengthOption( const QString &l, QStringList *v );
    void addRepeatableOption( char s, QStringList *v );
    void addRepeatableOption( const QString &l, QStringList *v );
    void addOptionalOption( const QString &l, QString *v,
                            const QString &def );
    void addOptionalOption( char s, const QString &l,
                            QString *v, const QString &def );
    
    // bare arguments
    void addArgument( const QString &name, QString *v );
    void addOptionalArgument( const QString &name, QString *v );

    bool parse( bool untilFirstSwitchOnly );
    bool parse() { return parse( false ); }

    bool isSet( const QString &name ) const;

    int currentArgument() const { return currArg; }

private:
    enum OptionType { OUnknown, OEnd, OSwitch, OArg1, OOpt, ORepeat, OVarLen };

    struct Option;
    friend struct Option;

    struct Option {
        Option( OptionType t = OUnknown,
                char s = 0, const QString &l = QString::null )
            : type( t ),
              sname( s ),
              lname( l ),
              boolValue( 0 ) { }
        
        OptionType type;
        char sname;             // short option name (0 if none)
        QString lname;  // long option name  (null if none)
        union {
            bool *boolValue;
            QString *stringValue;
            QStringList *listValue;
        };
        QString def;
    };

    QList<Option> options;
    typedef QList<Option>::const_iterator OptionConstIterator;
    QMap<QString, int> setOptions;

    void init( int argc, char *argv[], int offset = 1 );
    void addOption( Option o );
    void setSwitch( const Option &o );
    bool parseCommandFile( const QString &n, QStringList *l );

    QStringList args;
    QString aname;

    int numReqArgs;
    int numOptArgs;
    Option reqArg;
    Option optArg;

    int currArg;

    QString cmdFileOptionName, cmdFile;
};

#endif

