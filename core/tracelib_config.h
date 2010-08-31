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
 * \li #TRACELIB_TRACE, #TRACELIB_TRACE_MSG: Log generic trace entries
 * \li #TRACELIB_DEBUG, #TRACELIB_DEBUG_MSG: Log debug trace entries
 * \li #TRACELIB_ERROR, #TRACELIB_ERROR_MSG: Log error trace entries
 * \li #TRACELIB_WATCH, #TRACELIB_WATCH_MSG: Log trace entries with variables's values
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
 * There is also a macro is available for performing some build-time configuration:
 *
 * \li #TRACELIB_DEFAULT_PORT contains the default port to be used when
 * sending trace data over the network and no port information was found
 * in the configuration file.
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
#define TRACELIB_NAMESPACE_IDENT(x) ea::trace::x

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
#define TRACELIB_NAMESPACE_BEGIN namespace ea { namespace trace {

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
 * be overridden using the configuration file's <port> element:
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
 * @brief Add a debug entry to the current thread's trace.
 *
 * This macro adds a 'debug' entry to the current thread's trace without
 * specifying any custom message. It's equivalent to TRACELIB_DEBUG_MSG(0).
 *
 * \sa TRACELIB_DEBUG_MSG
 */
#define TRACELIB_DEBUG TRACELIB_DEBUG_MSG(0)

/**
 * @brief Add an error entry to the current thread's trace.
 *
 * This macro adds a 'error' entry to the current thread's trace without
 * specifying any custom message. It's equivalent to TRACELIB_ERROR_MSG(0).
 *
 * \sa TRACELIB_ERROR_MSG
 */
#define TRACELIB_ERROR TRACELIB_ERROR_MSG(0)

/**
 * @brief Add a generic trace entry to the current thread's trace.
 *
 * This macro adds a 'trace' entry to the current thread's trace without
 * specifying any custom message. It's equivalent to TRACELIB_TRACE_MSG(0).
 *
 * \sa TRACELIB_TRACE_MSG
 */
#define TRACELIB_TRACE TRACELIB_TRACE_MSG(0)

/**
 * @brief Add a watch point entry to the current thread's trace.
 *
 * This macro is equivalent to TRACELIB_WATCH_MSG(0, vars).
 *
 * @param[in] vars A list of TRACELIB_VAR invocations which specify the
 * variables to be logged.
 *
 * \sa TRACELIB_WATCH_MSG
 */
#define TRACELIB_WATCH(vars) TRACELIB_WATCH_MSG(0, vars)

/**
 * @brief Add a debug entry together with an optional message.
 *
 * This function adds a new entry to the current thread's trace.
 *
 * @param[in] msg A C string containing the UTF-8 encoded message to add to
 * the trace. A null pointer is acceptable and will result in no message being
 * logged. The caller retains ownership of the string.
 *
 * \code
 * void read_file( const char *fn ) {
 *     FILE *f = fopen( fn, "r" );
 *     if ( !f ) {
 *         TRACELIB_ERROR_MSG("Failed to open file for reading");
 *     }
 *     TRACELIB_DEBUG_MSG("Opened file for reading");
 *     ...
 * }
 * \endcode
 *
 * \sa TRACELIB_DEBUG
 */
#define TRACELIB_DEBUG_MSG(msg) TRACELIB_VISIT_TRACEPOINT_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 1, msg)

/**
 * @brief Add an error entry together with an optional message.
 *
 * This function adds a new entry to the current thread's trace.
 *
 * @param[in] msg A C string containing the UTF-8 encoded message to add to
 * the trace. A null pointer is acceptable and will result in no message being
 * logged. The caller retains ownership of the string. (See
 * #TRACELIB_DEBUG_MSG for an example.)
 *
 * \sa TRACELIB_ERROR
 */
#define TRACELIB_ERROR_MSG(msg) TRACELIB_VISIT_TRACEPOINT_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, msg)

/**
 * @brief Add a trace entry together with an optional message.
 *
 * This function adds a new entry to the current thread's trace.
 *
 * @param[in] msg A C string containing the UTF-8 encoded message to add to
 * the trace. A null pointer is acceptable and will result in no message being
 * logged. The caller retains ownership of the string.
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
 */
#define TRACELIB_TRACE_MSG(msg) TRACELIB_VISIT_TRACEPOINT_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 1, msg)

/**
 * @brief Add a watch point entry together with an optional message.
 *
 * This function adds a new entry to the current thread's trace.
 *
 * @param[in] msg A C string containing the UTF-8 encoded message to add to
 * the trace. A null pointer is acceptable and will result in no message being
 * logged. The caller retains ownership of the string.
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
 */
#define TRACELIB_WATCH_MSG(msg, vars) TRACELIB_VARIABLE_SNAPSHOT_MSG(1, vars, msg)

#endif // !defined(TRACELIB_CONFIG_H)

