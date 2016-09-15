/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

#define QT_NO_CAST_ASCII
#define QT_NO_ASCII_CAST

#include "getopt.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QStack>
#include <QString>
#include <QTextCodec>
#include <QTextStream>

#include <cassert>
#include <cstdlib>

/**
   \class GetOpt

   \brief A command line option parser.

   This class helps to overcome the repetitive, tedious and
   error-prone task of parsing the command line options passed to your
   application by the user. Specify the acceptable syntax with a
   minimum of statements in a readable way, check it against the
   actual arguments passed and find the retrieved values in variables
   of your program. The name \em GetOpt is based on similar utilities
   build into the Unix shell and other languages.

   A command line that a user might have entered is:

   \code
   app -v --config=my.cnf -Wall input.dat
   \endcode

   The typical usage has three stages:

   -# Construct a parser specifying what arguments to parse
   -# Set up the list of allowed and required options
   -# Run the parser

   For the first step there are three different constructors that
   either take arguments directly from \c main(), \c QApplication or a
   user specified list. Setting up the accepted syntax is done by a
   set of \c add functions like addSwitch(). The final step of running
   the parser is simply done by calling parse().

   A short example implementing a \c --verbose switch:

   \code
   int main(int argc, char **argv)
   {
       GetOpt opts(argc, argv);
       bool verbose;
       opts.addSwitch("verbose", &verbose);
       if (!opts.parse())
           return 1;
       if (verbose)
           cout << "VERBOSE mode on" << endl;
       ...
   \endcode

   For a better understanding of the function names we'll better
   define some terms used in the API and its documentation:

   - \em Argument An argument is a plain text token like e.g. a file
   name one typically passes to an editor when invoking it.
   - \em Switch A switch is an on/off kind of argument without the need
     of additional information. Example: \c --debug.
   - \em Option An option is a normally optional argument with a key-value
   syntax like \c --output=out.txt or \c -I/usr/include.
   - \em Short \em Option A short option is a one letter option with a
   preceding dash. Like \c -v.
   - \em Long \em Option A long option has a more verbose,
   multi-letter name like \c --debug. Currently the \c -debug form is
   accepted, too, but this might be made optional in the future.
   .

   \author froglogic GmbH <contact@froglogic.com>
*/


/**
   Constructs a command line parser from the arguments stored in a
   previously created QApplication instance.

   Example usage:
   \code
   QApplication a(argc, argv);

   GetOpt opt;
   \endcode

   This constructor is probably the most convenient one to use in a
   regular Qt application. Note that QApplication may already have
   removed Qt (or X11) specific arguments. Also see
   QApplication::argv() and QApplication::argc().
 */
GetOpt::GetOpt()
{
    if ( !QCoreApplication::instance() )
	qFatal( "GetOpt: requires a QApplication instance to be constructed first" );

    args = QCoreApplication::arguments();
    init( 0, 0 );
}

/**
   \internal
 */
GetOpt::GetOpt( int offset )
{
    if ( !QCoreApplication::instance() )
	qFatal( "GetOpt: requires a QApplication instance to be constructed first" );

    args = QCoreApplication::arguments();
    while ( offset-- && !args.isEmpty() ) {
        args.pop_front();
    }
    init( 0, 0 );
}

/**
   \internal
 */
GetOpt::GetOpt( int offset, int argc, char *argv[] )
{
    init( argc, argv, offset );
}

/**
   Construct a command line parser from the array \a argv of string
   pointers with the size \a argc. Those parameters have the form
   typically found in the \c main() function. That means that you can
   simply pass on the arguments specified by the user of your
   application.

   Example usage:

   \code
   int main(int argc, char **argv) {
       GetOpt opt(argc, argv);
       ...
   }
   \endcode
 */
GetOpt::GetOpt( int argc, char *argv[] )
{
    init( argc, argv );
}

/**
   Construct a command line parser from the arguments specified in the
   list of arguments \a a. This constructor is convenient in those
   cases where you want to parse a command line assembled on-the-fly
   instead of relying on the \c argc and \c arg parameters passed to
   the \c main() function.
 */
GetOpt::GetOpt( const QStringList &a )
    : args( a )
{
    init( 0, 0 );
}

/**
   \internal
*/
void GetOpt::init( int argc, char *argv[], int offset )
{
    numReqArgs = numOptArgs = 0;
    currArg = 1; // appname is not part of the arguments
    if ( argc ) {
	// application name
	aname = QFileInfo( QString::fromLocal8Bit( argv[0] ) ).fileName();
	// arguments
	for ( int i = offset; i < argc; ++i )
	    args.append( QString::fromLocal8Bit( argv[i] ) );
    }
}

/**
   Enable the command file feature. A command file is a plain text file
   that contains arguments normally passed via the command line.
 */

void GetOpt::addCommandFileOption( char s,
                                   const QString &l )
{
    assert( !l.isEmpty() );
    cmdFileOptionName = l;
    addOption( s, l, &cmdFile );
}

// split lines into distinct arguments and push them on the stack
// single or double quotes can be used to group arguments with
// spaces together
static void parseLine( const QString &line, QStringList *l )
{
    QString arg;
    QChar quotes;
    for ( int i = 0; i < int(line.length()); ++i ) {
        QChar c = line[i];
        if ( !quotes.isNull() && c == quotes ) {
            l->append( arg );
            arg = "";
            continue;
        }
        if ( c == '"' || c == '\'' ) {
            quotes = c;
            continue;
        }
        if ( c.isSpace() && quotes.isNull() ) {
            if ( !arg.isEmpty() ) {
                l->append( arg );
                arg = "";
            }
        } else {
            arg += QString( c );
        }
    }
    if ( !arg.isEmpty() ) {
        l->append( arg );
    }
}

/**
   \internal
*/
bool GetOpt::parseCommandFile( const QString &n, QStringList *l )
{
    assert( l );

    QFile f( n );
    if ( !f.open( QFile::ReadOnly ) ) {
        qWarning( "Error opening command file '%s': %s",
                  qPrintable( n ),
                  qPrintable( f.errorString() ) );
        return false;
    }

    QTextStream str( &f );
    str.setCodec( QTextCodec::codecForName( "UTF-8" ) );
    while ( !str.atEnd() ) {
        QString line = str.readLine().trimmed();
        // ignore empty lines and comments
        if ( line.isEmpty() || line[0] == '#' )
            continue;
        parseLine( line, l );
    }
    return true;
}

/**
   \fn bool GetOpt::parse()

   Parse the command line arguments specified in the constructor under
   the conditions set by the various \c add*() functions. On success,
   the given variable reference will be initialized with their
   respective values and true will be returned. Returns false
   otherwise.

   In the future there'll be a way to retrieve an error message. In
   the current version the message will be printed to \c stderr.
*/

#if QT_VERSION < 0x040000
static void pushOnStack( QValueStack<QString> *st,
                         const QStringList &l )
#else
static void pushOnStack( QStack<QString> *st,
                         const QStringList &l )
#endif
{
    QStringList::const_iterator it = l.end();
    const QStringList::const_iterator begin = l.begin();
    if ( !l.isEmpty() )
        do {
            --it;
            st->push( *it );
        } while ( it != begin );
}

/**
   \internal
*/
bool GetOpt::parse( bool untilFirstSwitchOnly )
{
    //    qDebug( "parse(%s)", args.join( QString( "," ) ).ascii() );
    // push all arguments as we got them on a stack
    // more pushes might following when parsing condensed arguments
    // like --key=value.
#if QT_VERSION < 0x040000
    QValueStack<QString> stack;
#else
    QStack<QString> stack;
#endif
    pushOnStack( &stack, args );

    const OptionConstIterator obegin = options.begin();
    const OptionConstIterator oend = options.end();
    enum { StartState, ExpectingState, OptionalState } state = StartState;
    Option currOpt;
    enum TokenType { LongOpt, ShortOpt, Arg, End } t, currType = End;
    bool extraLoop = true; // we'll do an extra round. fake an End argument
    QString dashDash = QString::fromLatin1( "--" );
    while ( !stack.isEmpty() || extraLoop ) {
	QString a;
	QString origA;
	// identify argument type
	if ( !stack.isEmpty() ) {
	    a = stack.pop();
	    currArg++;
	    origA = a;
	    //	    qDebug( "popped %s", a.ascii() );
            if ( currOpt.type == OVarLen ) {
                // once we are in VarLen mode everything will be treated
                // as an argument without further interpretation.
                t = Arg;
            } else if ( a.startsWith( dashDash ) ) {
		// recognized long option
		a = a.mid( 2 );
#if 0
		if ( a.isEmpty() ) {
		    qWarning( "'--' feature not supported, yet" );
		    exit( 2 );
		}
#endif
		t = LongOpt;
		// split key=value style arguments
		int equal = a.indexOf( '=' );
		if ( equal >= 0 ) {
		    stack.push( a.mid( equal + 1 ) );
		    currArg--;
		    a = a.left( equal );
		}
	    } else if ( a.length() == 1 ) {
		t = Arg;
	    } else if ( a[0] == '-' ) {
#if 1 // compat mode for -long style options
		if ( a.length() == 2 ) {
		    t = ShortOpt;
		    a = a[1];
		} else {
		    a = a.mid( 1 );
		    t = LongOpt;
		    // split key=value style arguments
		    int equal = a.indexOf( '=' );
		    if ( equal >= 0 ) {
			stack.push( a.mid( equal + 1 ) );
			currArg--;
			a = a.left( equal );
		    }
		}
#else
		// short option
		t = ShortOpt;
		// followed by an argument ? push it for later processing.
		if ( a.length() > 2 ) {
		    stack.push( a.mid( 2 ) );
		    currArg--;
		}
		a = a[1];
#endif
	    } else {
		t = Arg;
	    }
	} else {
	    // faked closing argument
	    t = End;
	}
	// look up among known list of options
	Option opt;
	if ( t != End ) {
	    OptionConstIterator oit = obegin;
	    while ( oit != oend ) {
		const Option &o = *oit;
		if ( ( t == LongOpt && a == o.lname ) || // ### check state
		     ( t == ShortOpt && a[0].unicode() == o.sname ) ) {
		    opt = o;
		    break;
		}
		++oit;
	    }
	    if ( t == LongOpt && opt.type == OUnknown ) {
		if ( currOpt.type != OVarLen ) {
		    qWarning( "Unknown option --%s", qPrintable( a ) );
		    return false;
		} else {
		    // VarLength options support arguments starting with '-'
		    t = Arg;
		}
	    } else if ( t == ShortOpt && opt.type == OUnknown ) {
		if ( currOpt.type != OVarLen ) {
		    qWarning( "Unknown option -%c", a[0].unicode() );
		    return false;
		} else {
		    // VarLength options support arguments starting with '-'
		    t = Arg;
		}
	    }

	} else {
	    opt = Option( OEnd );
	}

	// interpret result
	switch ( state ) {
	case StartState:
	    if ( opt.type == OSwitch ) {
		setSwitch( opt );
		setOptions.insert( opt.lname, 1 );
		setOptions.insert( QString( QChar( opt.sname ) ), 1 );
	    } else if ( opt.type == OArg1 || opt.type == ORepeat ) {
		state = ExpectingState;
		currOpt = opt;
		currType = t;
		setOptions.insert( opt.lname, 1 );
		setOptions.insert( QString( QChar( opt.sname ) ), 1 );
	    } else if ( opt.type == OOpt || opt.type == OVarLen ) {
		state = OptionalState;
		currOpt = opt;
		currType = t;
		setOptions.insert( opt.lname, 1 );
		setOptions.insert( QString( QChar( opt.sname ) ), 1 );
	    } else if ( opt.type == OEnd ) {
		// we're done
	    } else if ( opt.type == OUnknown && t == Arg ) {
		if ( numReqArgs > 0 ) {
		    if ( reqArg.stringValue->isNull() ) { // ###
			*reqArg.stringValue = a;
		    } else {
			qWarning( "Too many arguments" );
			return false;
		    }
		} else if ( numOptArgs > 0 ) {
		    if ( optArg.stringValue->isNull() ) { // ###
			*optArg.stringValue = a;
		    } else {
			qWarning( "Too many arguments" );
			return false;
		    }
		}
	    } else {
		qFatal( "unhandled StartState case %d",  opt.type );
	    }
	    break;
	case ExpectingState:
	    if ( t == Arg ) {
		if ( currOpt.type == OArg1 ) {
		    *currOpt.stringValue = a;

                    // the special command file option?
                    if ( !currOpt.lname.isEmpty() &&
                         currOpt.lname == cmdFileOptionName ) {
                        // read in command file and push arguments
                        // on our stack
                        QStringList l;
                        if ( !parseCommandFile( a, &l ) )
                            return false;
                        pushOnStack( &stack, l );
                    }

		    state = StartState;
		} else if ( currOpt.type == ORepeat ) {
		    currOpt.listValue->append( a );
		    state = StartState;
		} else {
		    abort();
		}
	    } else {
		QString n = currType == LongOpt ?
			    currOpt.lname : QString( QChar( currOpt.sname ) );
		qWarning( "Expected an argument after '%s' option",
                          qPrintable( a ) );
		return false;
	    }
	    break;
	case OptionalState:
	    if ( t == Arg ) {
		if ( currOpt.type == OOpt ) {
		    *currOpt.stringValue = a;
		    state = StartState;
		} else if ( currOpt.type == OVarLen ) {
		    currOpt.listValue->append( origA );
		    // remain in this state
		} else {
		    abort();
		}
	    } else {
		// optional argument not specified
		if ( currOpt.type == OOpt )
		    *currOpt.stringValue = currOpt.def;
		if ( t != End ) {
		    // re-evaluate current argument
		    stack.push( origA );
		    currArg--;
		}
		state = StartState;
	    }
	    break;
	}

	if ( untilFirstSwitchOnly && opt.type == OSwitch )
	    return true;

	// are we in the extra loop ? if so, flag the final end
	if ( t == End )
	    extraLoop = false;
    }

    if ( numReqArgs > 0 && reqArg.stringValue->isNull() ) {
	qWarning( "Lacking required argument" );
	return false;
    }

    return true;
}

/**
   \internal
*/
void GetOpt::addOption( Option o )
{
    // ### check for conflicts
    options.append( o );
}

/**
   Adds a switch with the long name \a lname. If the switch is found
   during parsing the bool \a *b will bet set to true. Otherwise the
   bool will be initialized to false.

   Example:

   \code
   GetOpt opt;
   bool verbose;
   opt.addSwitch("verbose", &verbose);
   \endcode

   The boolean flag \c verbose will be set to true if \c --verbose has
   been specified in the command line; false otherwise.
*/
void GetOpt::addSwitch( const QString &lname, bool *b )
{
    Option opt( OSwitch, 0, lname );
    opt.boolValue = b;
    addOption( opt );
    // ### could do all inits at the beginning of parse()
    *b = false;
}

/**
   \internal
*/
void GetOpt::setSwitch( const Option &o )
{
    assert( o.type == OSwitch );
    *o.boolValue = true;
}

/**
   Registers an option with the short name \a s and long name \a l to
   the parser. If this option is found during parsing the value will
   be stored in the string pointed to by \a v. By default \a *v will
   be initialized to \c QString::null.
*/
void GetOpt::addOption( char s, const QString &l, QString *v )
{
    Option opt( OArg1, s, l );
    opt.stringValue = v;
    addOption( opt );
    *v = QString::null;
}

/**
   Registers a long option \a l that can have a variable number of
   corresponding value parameters. As there currently is no way to
   tell the end of the value list the only sensible use of this option
   is at the end of the command line.

   Example:

   \code
   QStringList args;
   opt.addVarLengthOption("exec", &args);
   \endcode

   Above code will lead to "-f" and "test.txt" being stored in \a args
   upon

   \code
   myapp --exec otherapp -f test.txt
   \endcode
 */
void GetOpt::addVarLengthOption( const QString &l, QStringList *v )
{
    Option opt( OVarLen, 0, l );
    opt.listValue = v;
    addOption( opt );
    *v = QStringList();
}

/**
   Registers an option with the short name \a s that can be specified
   repeatedly in the command line. The option values will be stored in
   the list pointed to by \a v. If no \a s option is found \a *v will
   remain at its default value of an empty QStringList instance.

   Example:

   To parse the \c -I options in a command line like
   \code
   myapp -I/usr/include -I/usr/local/include
   \endcode

   you can use code like this:

   \code
   GetOpt opt;
   QStringList includes;
   opt.addRepeatableOption('I', &includes);
   opt.parse();
   \endcode
 */
void GetOpt::addRepeatableOption( char s, QStringList *v )
{
    Option opt( ORepeat, s, QString::null );
    opt.listValue = v;
    addOption( opt );
    *v = QStringList();
}

/**
   Registers an option with the long name \a l that can be specified
   repeatedly in the command line.

   \sa addRepeatableOption( char, QStringList* )
 */
void GetOpt::addRepeatableOption( const QString &l, QStringList *v )
{
    Option opt( ORepeat, 0, l );
    opt.listValue = v;
    addOption( opt );
    *v = QStringList();
}

/**
   Adds a long option \a l that has an optional value parameter. If
   the value is not specified by the user it will be set to \a def.

   Example:

   \code
   GetOpt opt;
   QString file;
   opt.addOptionalOption("dump", &file, "<stdout>");
   \endcode

   \sa addOption
 */
void GetOpt::addOptionalOption( const QString &l, QString *v,
                                const QString &def )
{
    addOptionalOption( 0, l, v, def );
}

/**
   Adds a short option \a s that has an optional value parameter. If
   the value is not specified by the user it will be set to \a def.
 */
void GetOpt::addOptionalOption( char s, const QString &l,
				QString *v, const QString &def )
{
    Option opt( OOpt, s, l );
    opt.stringValue = v;
    opt.def = def;
    addOption( opt );
    *v = QString::null;
}

/**
   Registers a required command line argument \a name. If the argument
   is missing parse() will return false to indicate an error and \a *v
   will remain with its default QString::null value. Otherwise \a *v
   will be set to the value of the argument.

   Example:

   To accept simple arguments like

   \code
   myeditor letter.txt
   \endcode

   use a call like:

   \code
   QString &file;
   opt.addArgument("file", &file);
   \endcode

   Note: the \a name parameter has a rather descriptive meaning for
   now. It might be used for generating a usage or error message in
   the future. Right now, the only current use is in relation with the
   isSet() function.
 */
void GetOpt::addArgument( const QString &name, QString *v )
{
    Option opt( OUnknown, 0, name );
    opt.stringValue = v;
    reqArg = opt;
    ++numReqArgs;
    *v = QString::null;
}

/**
   Registers an optional command line argument \a name. For a more
   detailed description see the addArgument() documentation.

 */
void GetOpt::addOptionalArgument( const QString &name, QString *v )
{
    Option opt( OUnknown, 0, name );
    opt.stringValue = v;
    optArg = opt;
    ++numOptArgs;
    *v = QString::null;
}

/**
   Returns true if the (long) option or switch \a name has been found
   in the command line; returns false otherwise. Leading hyphens are
   not part of the name.

   As the set/not set decision can also be made depending on the value
   of the variable reference used in the respective \c add*() call
   there's generally little use for this function.
*/

bool GetOpt::isSet( const QString &name ) const
{
    return setOptions.find( name ) != setOptions.end();
}

/**
   \fn int GetOpt::currentArgument() const
   \internal
*/
