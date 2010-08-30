#ifndef TRACELIB_CONFIG_H
#define TRACELIB_CONFIG_H

/**
 * @file tracelib_config.h
 * @brief Provides convenience macros which simplify access to the tracelib API.
 *
 * You can find a lot of macros here.
 */

/**
 * @brief Yield the fully qualified name for an identifier in the tracelib library.
 *
 * This macro should be used for directly accessing C++ identifiers (such as
 * class names) of the tracelib library. Since all symbols of the library might be
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
 * @begin Opens the namespace which contains all tracelib symbols.
 *
 * This macro (together with TRACELIB_NAMESPACE_END) is needed when declaring
 * or definiong symbols which should reside in the same namespace as the rest
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
 * @begin Closes the namespace which contains all tracelib symbols.
 *
 * This macro (together with TRACELIB_NAMESPACE_BEGIN) is needed when declaring
 * or definiong symbols which should reside in the same namespace as the rest
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
 * \sa TRACELIB_NAMESPACE_BEGIN TRACELIB_NAMESPACE_IDENT
 */
#define TRACELIB_NAMESPACE_END } }

/**
 * @brief Default port to write trace data to when using network output mode.
 *
 * This macro defines the default port which should be used when writing the
 * trace data to the remote host specified in the configuration file. It can
 * be overridden using the <port> element of the configuration file:
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
 * @brief Add a debug entry to the trace of the current thread.
 *
 * This macro adds a 'debug' entry to the trace of the current thread without
 * specifying any custom message. It's equivalent to TRACELIB_DEBUG_MSG(0).
 *
 * \sa TRACELIB_DEBUG_MSG
 */
#define TRACELIB_DEBUG TRACELIB_DEBUG_MSG(0)

/**
 * @brief Add a error entry to the trace of the current thread.
 *
 * This macro adds a 'erro' entry to the trace of the current thread without
 * specifying any custom message. It's equivalent to TRACELIB_ERROR_MSG(0).
 *
 * \sa TRACELIB_ERROR_MSG
 */
#define TRACELIB_ERROR TRACELIB_ERROR_MSG(0)

/**
 * @brief Add a generic trace entry to the trace of the current thread.
 *
 * This macro adds a 'trace' entry to the trace of the current thread without
 * specifying any custom message. It's equivalent to TRACELIB_TRACE_MSG(0).
 *
 * \sa TRACELIB_TRACE_MSG
 */
#define TRACELIB_TRACE TRACELIB_TRACE_MSG(0)

/**
 * @brief Add a watch point entry togetherto the trace of the current thread.
 *
 * This macro is equivalent to TRACELIB_WATCH_MSG(0, vars).
 *
 * @param[in] vars A list of TRACELIB_VAR invocations which enclose the variables
 * to be logged.
 *
 * \sa TRACELIB_WATCH_MSG
 */
#define TRACELIB_WATCH(vars) TRACELIB_WATCH_MSG(0, vars)

/**
 * @brief Add a debug entry together with an optional message.
 *
 * This function adds a new entry to the trace for the current thread.
 *
 * @param[in] msg A C string containing the UTF-8 encoded message to add to
 * the trace. A null pointer is acceptable and will result in no message being
 * logged. Ownership of the referenced memory remains with the caller.
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
 * @brief Add a error entry together with an optional message.
 *
 * This function adds a new entry to the trace for the current thread.
 *
 * @param[in] msg A C string containing the UTF-8 encoded message to add to
 * the trace. A null pointer is acceptable and will result in no message being
 * logged. Ownership of the referenced memory remains with the caller.
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
 * \sa TRACELIB_ERROR
 */
#define TRACELIB_ERROR_MSG(msg) TRACELIB_VISIT_TRACEPOINT_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, msg)

/**
 * @brief Add a trace entry together with an optional message.
 *
 * This function adds a new entry to the trace for the current thread.
 *
 * @param[in] msg A C string containing the UTF-8 encoded message to add to
 * the trace. A null pointer is acceptable and will result in no message being
 * logged. Ownership of the referenced memory remains with the caller.
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
 * This function adds a new entry to the trace for the current thread.
 *
 * @param[in] msg A C string containing the UTF-8 encoded message to add to
 * the trace. A null pointer is acceptable and will result in no message being
 * logged. Ownership of the referenced memory remains with the caller.
 * @param[in] vars A list of TRACELIB_VAR invocations which enclose the variables
 * to be logged.
 *
 * \code
 * bool is_nonnegative_number( const char *s ) {
 *     TRACE_WATCH_MSG("is_nonnegative_number called", TRACE_VAR(s));
 *     while ( *s && *s >= '0' && *s <= '9' ) ++s;
 *     TRACE_WATCH_MSG("is_nonnegative_number exiting", TRACE_VAR(s) << TRACE_VAR(*s == '\0'));
 *     return *s == '\0';
 * }
 * \endcode
 *
 * \sa TRACELIB_WATCH
 */
#define TRACELIB_WATCH_MSG(msg, vars) TRACELIB_VARIABLE_SNAPSHOT_MSG(1, vars, msg)

#endif // !defined(TRACELIB_CONFIG_H)

