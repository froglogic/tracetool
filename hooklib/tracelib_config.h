/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_CONFIG_H
#define TRACELIB_CONFIG_H

/**
 * @file tracelib_config.h
 * @brief Provides convenience macros to simplify access to the tracelib API.
 *
 * This file contains a set of preprocessor defines which simplify access to the
 * tracelib API as well as a few macros for performing build-time configuration
 * of the tracelib library.
 *
 * The following macros are available for logging trace entries:
 * <ol>
 * <li>Logging generic trace entries, possibly with a custom message and/or a key:
 *   <ul>
 *     <li>#TRACELIB_TRACE</li>
 *     <li>#TRACELIB_TRACE_MSG</li>
 *     <li>#TRACELIB_TRACE_KEY</li>
 *     <li>#TRACELIB_TRACE_KEY_MSG</li>
 *     <li>#TRACELIB_TRACE_STREAM</li>
 *   </ul>
 * </li>
 * <li>Logging debug trace entries, possibly with a custom message and/or a key:
 *   <ul>
 *     <li>#TRACELIB_DEBUG</li>
 *     <li>#TRACELIB_DEBUG_MSG</li>
 *     <li>#TRACELIB_DEBUG_KEY</li>
 *     <li>#TRACELIB_DEBUG_KEY_MSG</li>
 *     <li>#TRACELIB_DEBUG_STREAM</li>
 *   </ul>
 * </li>
 * <li>Logging error entries, possibly with a custom message and/or a key:
 *   <ul>
 *     <li>#TRACELIB_ERROR</li>
 *     <li>#TRACELIB_ERROR_MSG</li>
 *     <li>#TRACELIB_ERROR_KEY</li>
 *     <li>#TRACELIB_ERROR_KEY_MSG</li>
 *     <li>#TRACELIB_ERROR_STREAM</li>
 *   </ul>
 * </li>
 * <li>Logging watch point entries (including variable values), possibly with a custom message and/or a key:
 *   <ul>
 *     <li>#TRACELIB_WATCH</li>
 *     <li>#TRACELIB_WATCH_MSG</li>
 *     <li>#TRACELIB_WATCH_KEY</li>
 *     <li>#TRACELIB_WATCH_KEY_MSG</li>
 *     <li>#TRACELIB_WATCH_STREAM</li>
 *   </ul>
 *   These macros are used together with the #TRACELIB_VAR macro.
 * </li>
 * </ol>
 *
 * Furthermore, a few short alias macros are available in case the
 * TRACELIB_CLEAN_NAMESPACE symbol is not defined while compiling this header
 * file. These macros simplify the macro usage through shorter names at the
 * expense of polluting the global namespace:
 * <ul>
 *   <li>fTrace</li>
 *   <li>fDebug</li>
 *   <li>fError</li>
 *   <li>fWatch</li>
 *   <li>fVar</li>
 *   <li>fValue</li>
 * </ul>
 *
 * In addition, three macros are available for configuring the namespace within
 * which all the tracelib library's symbols are to be defined. This is useful
 * for avoiding symbol clashes with existing definitions when linking the
 * tracelib library into applications or libraries:
 *
 * \li #TRACELIB_NAMESPACE_BEGIN opens the tracelib library's namespace
 * \li #TRACELIB_NAMESPACE_END closes the tracelib library's namespace
 * \li #TRACELIB_NAMESPACE_IDENT fully-qualifies the given identifier using the
 * tracelib namespace
 *
 * There are also two macros available for performing some build-time
 * configuration:
 *
 * \li #TRACELIB_DEFAULT_PORT contains the default port to be used when
 * sending trace data over the network and no port information was found
 * in the configuration file.
 * \li #TRACELIB_DEFAULT_CONFIGFILE_NAME contains the name of the default
 * configuration file to use in case no other name was specified at runtime.
 */

/**
 * @brief Yield the fully qualified name for an identifier in the tracelib library.
 *
 * This macro should be used for directly accessing C++ identifiers (such as
 * class names) in the tracelib library. Since all the library's symbols might be
 * enclosed in a namespace, code which is not within that namespace must use the
 * fully qualified name:
 *
 * \code
 * #include "tracelib.h"
 *
 * int main() {
 *     Trace customTrace; // compile error: 'Trace' not in scope
 *
 *     TRACELIB_NAMESPACE_IDENT(Trace) customTrace; // ok!
 * }
 * \endcode
 *
 * \sa TRACELIB_NAMESPACE_BEGIN TRACELIB_NAMESPACE_END
 */
#define TRACELIB_NAMESPACE_IDENT(x) froglogic::tracelib::x

/**
 * @brief Opens the namespace which contains all the tracelib symbols.
 *
 * This macro (together with #TRACELIB_NAMESPACE_END) is needed when declaring
 * or defining symbols which should reside in the same namespace as the rest
 * of the tracelib library. In particular, it is needed when implementing
 * specializations of the convertVariable template function, as in this
 * example:
 *
 * \code
 * #include "tracelib.h"
 *
 * // Specialize convertVariable to be able to log custom Person objects.
 * // convertVariable specializations are expected to be defined in the same
 * // namespace as the rest of the tracelib library.
 *
 * TRACELIB_NAMESPACE_BEGIN
 * template <>
 * VariableValue convertVariable( const Person &p ) {
 *     const std::string s = p.lastName() + ", " + p.firstName();
 *     return VariableValue::fromString( s );
 * }
 * TRACELIB_NAMESPACE_END
 * \endcode
 *
 * \sa TRACELIB_NAMESPACE_END TRACELIB_NAMESPACE_IDENT
 */
#define TRACELIB_NAMESPACE_BEGIN namespace froglogic { namespace tracelib {

/**
 * @brief Closes the namespace which contains all the tracelib symbols.
 *
 * This macro (together with #TRACELIB_NAMESPACE_BEGIN) is needed when declaring
 * or defining symbols which should reside in the same namespace as the rest
 * of the tracelib library. In particular, it is needed when implementing
 * specializations of the convertVariable template function. (See
 * #TRACELIB_NAMESPACE_BEGIN for an example.)
 *
 * \sa TRACELIB_NAMESPACE_BEGIN TRACELIB_NAMESPACE_IDENT
 */
#define TRACELIB_NAMESPACE_END } }

/**
 * @brief Default port to write trace data to when sending over a network.
 *
 * This macro defines the default port which should be used when sending the
 * trace data to the remote host specified in the configuration file. It can
 * be overridden using the configuration file's \c \<port\> element:
 *
 * \code
 * <tracelibConfiguration>
 * <process>
 *    <!-- Trace data for sampleapp.exe should to go the default port on
 *         logserver.acme.com. -->
 *    <name>sampleapp.exe</name>
 *    <output type="tcp">
 *      <option name="host">logserver.acme.com</option>
 *    </output>
 *    <!-- Trace data for helperapp.exe should to port 4711 on
 *         tracestorage.acme.com. -->
 *    <name>helperapp.exe</name>
 *    <output type="tcp">
 *      <option name="host">logserver.acme.com</option>
 *      <option name="port">4711</option>
 *    </output>
 * ...
 * \endcode
 */
#define TRACELIB_DEFAULT_PORT 12382

/**
 * @brief Default name of configuration file.
 *
 * This macro contains the default name of the configuration file to load. The
 * default name is used in case no other name was specified at runtime, for
 * instance by setting the TRACELIB_CONFIG_FILE environment variable.
 */
#define TRACELIB_DEFAULT_CONFIGFILE_NAME "tracelib.xml"

/**
 * @brief Add a debug entry to the current thread's trace.
 *
 * This macro adds a 'debug' entry to the current thread's trace without
 * specifying any custom message.
 *
 * \sa TRACELIB_DEBUG_MSG
 */
#define TRACELIB_DEBUG TRACELIB_DEBUG_IMPL

/**
 * @brief Variant of #TRACELIB_DEBUG which takes an trace key identifer
 *
 * This macro is a variant of #TRACELIB_DEBUG, making it possible to specify
 * a 'trace key'. This key can be used by clients to group trace entries.
 *
 * @param[in] key A UTF-8 encoded C string; this key must be the same for
 * all threads executing the same #TRACELIB_DEBUG_KEY statement.
 *
 * All other arguments are the same as with #TRACELIB_DEBUG.
 *
 * \sa TRACELIB_DEBUG
 */
#define TRACELIB_DEBUG_KEY(key) TRACELIB_DEBUG_KEY_IMPL(key)

/**
 * @brief Add an error entry to the current thread's trace.
 *
 * This macro adds a 'error' entry to the current thread's trace without
 * specifying any custom message.
 *
 * \sa TRACELIB_ERROR_MSG
 */
#define TRACELIB_ERROR TRACELIB_ERROR_IMPL

/**
 * @brief Variant of #TRACELIB_ERROR which takes an trace key identifer
 *
 * This macro is a variant of #TRACELIB_ERROR, making it possible to specify
 * a 'trace key'. This key can be used by clients to group trace entries.
 *
 * @param[in] key A UTF-8 encoded C string; this key must be the same for
 * all threads executing the same #TRACELIB_ERROR_KEY statement.
 *
 * All other arguments are the same as with #TRACELIB_ERROR.
 *
 * \sa TRACELIB_ERROR
 */
#define TRACELIB_ERROR_KEY(key) TRACELIB_ERROR_KEY_IMPL(key)

/**
 * @brief Add a generic trace entry to the current thread's trace.
 *
 * This macro adds a 'trace' entry to the current thread's trace without
 * specifying any custom message.
 *
 * \sa TRACELIB_TRACE_MSG
 */
#define TRACELIB_TRACE TRACELIB_TRACE_IMPL

/**
 * @brief Variant of #TRACELIB_TRACE which takes an trace key identifer
 *
 * This macro is a variant of #TRACELIB_TRACE, making it possible to specify
 * a 'trace key'. This key can be used by clients to group trace entries.
 *
 * @param[in] key A UTF-8 encoded C string; this key must be the same for
 * all threads executing the same #TRACELIB_TRACE_KEY statement.
 *
 * All other arguments are the same as with #TRACELIB_TRACE.
 *
 * \sa TRACELIB_TRACE
 */
#define TRACELIB_TRACE_KEY(key) TRACELIB_TRACE_KEY_IMPL(key)

/**
 * @brief Add a watch point entry to the current thread's trace.
 *
 * @param[in] vars A list of TRACELIB_VAR invocations which specify the
 * variables to be logged, separated by '<<' operator calls.
 *
 * \sa TRACELIB_WATCH_MSG
 */
#define TRACELIB_WATCH(vars) TRACELIB_WATCH_IMPL(vars)

/**
 * @brief Variant of #TRACELIB_WATCH which takes an trace key identifer
 *
 * This macro is a variant of #TRACELIB_WATCH, making it possible to specify
 * a 'trace key'. This key can be used by clients to group trace entries.
 *
 * @param[in] key A UTF-8 encoded C string; this key must be the same for
 * all threads executing the same #TRACELIB_WATCH_KEY statement.
 *
 * All other arguments are the same as with #TRACELIB_WATCH.
 *
 * \sa TRACELIB_WATCH
 */
#define TRACELIB_WATCH_KEY(key, vars) TRACELIB_WATCH_KEY_IMPL(key, vars)

/**
 * @brief Add a debug entry together with an optional message.
 *
 * This function adds a new entry to the current thread's trace.
 *
 * @param[in] msg A series of UTF-8 encoded C strings and other
 * values, separated by calls to the '<<' operator.
 *
 * \code
 * void read_file( const char *fn ) {
 *     FILE *f = fopen( fn, "r" );
 *     if ( !f ) {
 *         TRACELIB_ERROR_MSG("Failed to open file " << fn << " for reading");
 *     }
 *     TRACELIB_DEBUG_MSG("Opened file for reading");
 *     ...
 * }
 * \endcode
 *
 * \sa TRACELIB_DEBUG
 * \sa TRACELIB_VALUE
 */
#define TRACELIB_DEBUG_MSG(msg) TRACELIB_DEBUG_MSG_IMPL(msg)

/**
 * @brief Variant of #TRACELIB_DEBUG_MSG which takes an trace key identifer
 *
 * This macro is a variant of #TRACELIB_DEBUG_MSG, making it possible to specify
 * a 'trace key'. This key can be used by clients to group trace entries.
 *
 * @param[in] key A UTF-8 encoded C string; this key must be the same for
 * all threads executing the same #TRACELIB_DEBUG_KEY_MSG statement.
 *
 * All other arguments are the same as with #TRACELIB_DEBUG_MSG.
 *
 * \sa TRACELIB_DEBUG_MSG
 */
#define TRACELIB_DEBUG_KEY_MSG(key, msg) TRACELIB_DEBUG_KEY_MSG_IMPL(key, msg)

/**
 * @brief Add an error entry together with an optional message.
 *
 * This function adds a new entry to the current thread's trace.
 *
 * @param[in] msg A series of UTF-8 encoded C strings and other
 * values, separated by calls to the '<<' operator (see
 * #TRACELIB_DEBUG_MSG for an example.)
 *
 * \sa TRACELIB_ERROR
 * \sa TRACELIB_VALUE
 */
#define TRACELIB_ERROR_MSG(msg) TRACELIB_ERROR_MSG_IMPL(msg)

/**
 * @brief Variant of #TRACELIB_ERROR_MSG which takes an trace key identifer
 *
 * This macro is a variant of #TRACELIB_ERROR_MSG, making it possible to specify
 * a 'trace key'. This key can be used by clients to group trace entries.
 *
 * @param[in] key A UTF-8 encoded C string; this key must be the same for
 * all threads executing the same #TRACELIB_ERROR_KEY_MSG statement.
 *
 * All other arguments are the same as with #TRACELIB_ERROR_MSG.
 *
 * \sa TRACELIB_ERROR_MSG
 */
#define TRACELIB_ERROR_KEY_MSG(key, msg) TRACELIB_ERROR_KEY_MSG_IMPL(key, msg)

/**
 * @brief Add a trace entry together with an optional message.
 *
 * This function adds a new entry to the current thread's trace.
 *
 * @param[in] msg A series of UTF-8 encoded C strings and other
 * values, separated by calls to the '<<' operator.
 *
 * \code
 * int get_largest_value( int a, int b, int c ) {
 *     TRACELIB_TRACE_MSG("get_largest_value called");
 *     if ( a > b ) {
 *         TRACELIB_TRACE;
 *         return a > c ? a : c;
 *     }
 *     TRACELIB_TRACE;
 *     return b > c ? b : c;
 * }
 * \endcode
 *
 * \sa TRACELIB_TRACE
 * \sa TRACELIB_VALUE
 */
#define TRACELIB_TRACE_MSG(msg) TRACELIB_TRACE_MSG_IMPL(msg)

/**
 * @brief Variant of #TRACELIB_TRACE_MSG which takes an trace key identifer
 *
 * This macro is a variant of #TRACELIB_TRACE_MSG, making it possible to specify
 * a 'trace key'. This key can be used by clients to group trace entries.
 *
 * @param[in] key A UTF-8 encoded C string; this key must be the same for
 * all threads executing the same #TRACELIB_TRACE_KEY_MSG statement.
 *
 * All other arguments are the same as with #TRACELIB_TRACE_MSG.
 *
 * \sa TRACELIB_TRACE_MSG
 */
#define TRACELIB_TRACE_KEY_MSG(key, msg) TRACELIB_TRACE_KEY_MSG_IMPL(key, msg)

/**
 * @brief Add a watch point entry together with an optional message.
 *
 * This function adds a new entry to the current thread's trace.
 *
 * @param[in] msg A series of UTF-8 encoded C strings and other
 * values, separated by calls to the '<<' operator.
 * @param[in] vars A list of TRACELIB_VARs which specify the variables
 * to be logged.
 *
 * \code
 * bool is_nonnegative_number( const char *s ) {
 *     TRACELIB_WATCH_MSG("is_nonnegative_number called", TRACELIB_VAR(s));
 *     while ( *s && *s >= '0' && *s <= '9' ) ++s;
 *     TRACELIB_WATCH_MSG("is_nonnegative_number exiting", TRACELIB_VAR(s) << TRACELIB_VAR(*s == '\0'));
 *     return *s == '\0';
 * }
 * \endcode
 *
 * \sa TRACELIB_WATCH
 * \sa TRACELIB_VALUE
 */
#define TRACELIB_WATCH_MSG(msg, vars) TRACELIB_WATCH_MSG_IMPL(msg, vars)

/**
 * @brief Variant of #TRACELIB_WATCH_MSG which takes an trace key identifer
 *
 * This macro is a variant of #TRACELIB_WATCH_MSG, making it possible to specify
 * a 'trace key'. This key can be used by clients to group trace entries.
 *
 * @param[in] key A UTF-8 encoded C string; this key must be the same for
 * all threads executing the same #TRACELIB_WATCH_KEY_MSG statement.
 *
 * All other arguments are the same as with #TRACELIB_WATCH_MSG.
 *
 * \sa TRACELIB_WATCH_MSG
 */
#define TRACELIB_WATCH_KEY_MSG(key, msg, vars) TRACELIB_WATCH_KEY_MSG_IMPL(key, msg, vars)

/**
 * @brief Helper macro to be used together with _MSG macros
 *
 * This macro avoids that a variable name has to be typed twice when assembling
 * messages using the any of the _MSG macros. It can only be used in the first
 * argument to the _MSG macros.
 *
 * Here's an example which makes use of the macro to simplify the
 * message which is being passed to a #TRACELIB_TRACE_MSG invocation:
 *
 * \code
 * void f( int i, bool j ) {
 *     TRACELIB_TRACE_MSG("f() called with " << TRACELIB_VALUE(i) << " and " << TRACELIB_VALUE(j))
 * }
 * \endcode
 */
#define TRACELIB_VALUE TRACELIB_VALUE_IMPL

/**
 * @brief Add a debug entry to the trace in a streaming syntax.
 *
 * This macro can be added to record debug trace entries in a streaming
 * syntax
 *
 * \code
 * void f( int i, bool j ) {
 *     TRACELIB_DEBUG_STREAM(NULL) << "f() called with " << i << " and " << j;
 * }
 * \endcode
 *
 * @param[in] key A UTF-8 encoded C string specifying a trace key; specify NULL
 * to signal that no dedicated trace key should be used. This key must be the
 * same for all threads executing the same #TRACELIB_DEBUG_STREAM statement.
 */
#define TRACELIB_DEBUG_STREAM(key) TRACELIB_DEBUG_STREAM_IMPL(key)

/**
 * @brief Add an error entry to the trace in a streaming syntax.
 *
 * This macro can be added to record error trace entries in a streaming
 * syntax
 *
 * \code
 * void read_file( const char *fn ) {
 *     FILE *f = fopen( fn, "r" );
 *     if ( !f ) {
 *         TRACELIB_ERROR_STREAM(NULL) << "Failed to open file " << fn << " for reading";
 *     }
 *     TRACELIB_DEBUG_STREAM(NULL) << "Opened file for reading";
 *     ...
 * }
 * \endcode
 *
 * @param[in] key A UTF-8 encoded C string specifying a trace key; specify NULL
 * to signal that no dedicated trace key should be used. This key must be the
 * same for all threads executing the same #TRACELIB_ERROR_STREAM statement.
 */
#define TRACELIB_ERROR_STREAM(key) TRACELIB_ERROR_STREAM_IMPL(key)

/**
 * @brief Add an generic trace entry to the trace in a streaming syntax.
 *
 * This macro can be added to record generic 'trace' trace entries in a streaming
 * syntax
 *
 * \code
 * int get_largest_value( int a, int b, int c ) {
 *     TRACELIB_TRACE_STREAM(NULL) << "get_largest_value called";
 *     if ( a > b ) {
 *         TRACELIB_TRACE_STREAM(NULL);
 *         return a > c ? a : c;
 *     }
 *     TRACELIB_TRACE_STREAM(NULL);
 *     return b > c ? b : c;
 * }
 * \endcode
 *
 * @param[in] key A UTF-8 encoded C string specifying a trace key; specify NULL
 * to signal that no dedicated trace key should be used. This key must be the
 * same for all threads executing the same #TRACELIB_TRACE_STREAM statement.
 */
#define TRACELIB_TRACE_STREAM(key) TRACELIB_TRACE_STREAM_IMPL(key)

/**
 * @brief Add a watch point entry in a streaming syntax.
 *
 * This macro can be added to record watch entries in a streaming
 * syntax
 *
 * \code
 * bool is_nonnegative_number( const char *s ) {
 *     TRACELIB_WATCH_STREAM(NULL) << TRACELIB_VAR(s);
 *     while ( *s && *s >= '0' && *s <= '9' ) ++s;
 *     TRACELIB_WATCH_STREAM(NULL) << TRACELIB_VAR(s) << TRACELIB_VAR(*s == '\0');
 *     return *s == '\0';
 * }
 * \endcode
 *
 * @param[in] key A UTF-8 encoded C string specifying a trace key; specify NULL
 * to signal that no dedicated trace key should be used. This key must be the
 * same for all threads executing the same #TRACELIB_WATCH_STREAM statement.
 */
#define TRACELIB_WATCH_STREAM(key) TRACELIB_WATCH_STREAM_IMPL(key)

/**
 * @brief Log variables with watch entries.
 *
 * This macro can be used to log variables (their name, type and value) with
 * watch entries.
 *
 * @param[in] v A value to be logged. The value is only actually used (in the
 * case of pointer values: dereferenced) at the end of the #TRACELIB_WATCH_STREAM
 * macro invocation.
 *
 * \note
 * When passing an object (or a reference or a pointer to an object), make sure
 * that the lifetime of the given object lasts until the end of the
 * #TRACELIB_WATCH_STREAM invocation. The argument v is only dereferenced and
 * evaluated at the end of the #TRACELIB_WATCH_STREAM macro invocation.
 *
 * The lifetime of the given value must be at least as long as the
 * #TRACELIB_WATCH_STREAM macro invocation. Don't pass temporary objects to
 * #TRACELIB_VAR which are only created within the macro invocation, as in this
 * example:
 *
 * \code
 * std::string getName() {
 *     return std::string( "John Doe" );
 * }
 *
 * void f() {
 *   TRACELIB_WATCH_STREAM(NULL) << TRACELIB_VAR(getName().c_str());
 * }
 * \endcode
 *
 * In this example, the pointer returned by the c_str() function will only be
 * evaluated after the complete TRACELIB_WATCH_STREAM line was evaluated - at
 * which point the temporary std::string object returned by the getName()
 * function will be destroyed, so the pointer returned by c_str() is invalid.
 * \endnote
 *
 * \sa TRACELIB_WATCH
 * \sa TRACELIB_WATCH_MSG
 * \sa TRACELIB_WATCH_KEY
 * \sa TRACELIB_WATCH_KEY_MSG
 * \sa TRACELIB_WATCH_STREAM
 */
#define TRACELIB_VAR(v) TRACELIB_VAR_IMPL(v)

#ifndef TRACELIB_CLEAN_NAMESPACE
/**
 * @brief Short alias for #TRACELIB_ERROR_STREAM
 *
 * This macro is merely a (short) alias for the #TRACELIB_ERROR_STREAM macro.
 */
#  define fError(key) TRACELIB_ERROR_STREAM(key)

/**
 * @brief Short alias for #TRACELIB_DEBUG_STREAM
 *
 * This macro is merely a (short) alias for the #TRACELIB_DEBUG_STREAM macro.
 */
#  define fDebug(key) TRACELIB_DEBUG_STREAM(key)

/**
 * @brief Short alias for #TRACELIB_TRACE_STREAM
 *
 * This macro is merely a (short) alias for the #TRACELIB_TRACE_STREAM macro.
 */
#  define fTrace(key) TRACELIB_TRACE_STREAM(key)

/**
 * @brief Short alias for #TRACELIB_WATCH_STREAM
 *
 * This macro is merely a (short) alias for the #TRACELIB_WATCH_STREAM macro.
 */
#  define fWatch(key) TRACELIB_WATCH_STREAM(key)

/**
 * @brief Short alias for #TRACELIB_VAR
 *
 * This macro is merely a (short) alias for the #TRACELIB_VAR macro.
 */
#  define fVar(v) TRACELIB_VAR(v)

/**
 * @brief Short alias for #TRACELIB_VALUE
 *
 * This macro is merely a (short) alias for the #TRACELIB_VALUE macro.
 */
#  define fValue(v) TRACELIB_VALUE(v)
#endif // !defined(TRACELIB_CLEAN_NAMESPACE)

#endif // !defined(TRACELIB_CONFIG_H)

