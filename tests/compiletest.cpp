struct CustomStruct {
    int someNumber;
};

#include <tracelib/tracelib.h>

#include <string.h>
#include <vector>

#if defined(_MSC_VER) && _MSC_VER <= 1600
#if _MSC_VER < 1600
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

// Test that TRACELIB_NAMESPACE_* macros work as expected
TRACELIB_NAMESPACE_BEGIN
int someRandomIdentifier;
TRACELIB_NAMESPACE_END

TRACELIB_NAMESPACE_BEGIN
template <>
inline VariableValue convertVariable( CustomStruct c ) {
    std::stringstream sstr;
    sstr << "CustomStruct(" << c.someNumber << ")";
    return VariableValue::stringValue( sstr.str().c_str() );
}
TRACELIB_NAMESPACE_END

static void testNamespaceMacros()
{
    TRACELIB_NAMESPACE_IDENT(someRandomIdentifier) = 3;
}

// Test that TRACELIB_DEFAULT_PORT is properly convertible to an unsigned short
static void testPortMacro()
{
    const unsigned short port = TRACELIB_DEFAULT_PORT;
}

// Test that TRACELIB_DEFAULT_CONFIGFILE_NAME can be treated as a C string
static void testConfigFileNameMacro()
{
    (void)strlen( TRACELIB_DEFAULT_CONFIGFILE_NAME );
}

// Test all 'trace' macros
static void testTraceMacros()
{
    TRACELIB_TRACE;

    TRACELIB_TRACE_KEY("somekey");

    TRACELIB_TRACE_KEY_MSG("somekey", "somemessage");

    char c;
    bool b;
    float f;
    double d;
    long double ld;
    void *vp;
    const void *cvp;
    char *cp;
    signed char *scp;
    unsigned char *ucp;
    std::string str;
    short ss;
    unsigned short us;
    int si;
    unsigned int ui;
    long sl;
    unsigned long ul;
    vlonglong sll;
    vulonglong ull;
    int8_t si8;
    int16_t si16;
    int32_t si32;
    int64_t si64;
    uint8_t ui8;
    uint16_t ui16;
    uint32_t ui32;
    uint64_t ui64;
    CustomStruct cs;
    std::vector<char> v;
    TRACELIB_TRACE_KEY_MSG("somekey", "somemessage, with "
                           << TRACELIB_VALUE(c)
                           << TRACELIB_VALUE(b)
                           << TRACELIB_VALUE(f)
                           << TRACELIB_VALUE(d)
                           << TRACELIB_VALUE(ld)
                           << TRACELIB_VALUE(vp)
                           << TRACELIB_VALUE(cvp)
                           << TRACELIB_VALUE(cp)
                           << TRACELIB_VALUE(scp)
                           << TRACELIB_VALUE(ucp)
                           << TRACELIB_VALUE(str)
                           << TRACELIB_VALUE(ss)
                           << TRACELIB_VALUE(us)
                           << TRACELIB_VALUE(si)
                           << TRACELIB_VALUE(ui)
                           << TRACELIB_VALUE(sl)
                           << TRACELIB_VALUE(ul)
                           << TRACELIB_VALUE(sll)
                           << TRACELIB_VALUE(ull)
                           << TRACELIB_VALUE(si8)
                           << TRACELIB_VALUE(si16)
                           << TRACELIB_VALUE(si32)
                           << TRACELIB_VALUE(si64)
                           << TRACELIB_VALUE(ui8)
                           << TRACELIB_VALUE(ui16)
                           << TRACELIB_VALUE(ui32)
                           << TRACELIB_VALUE(ui64)
                           << TRACELIB_VALUE(v.size())
                           << TRACELIB_VALUE(cs));

    fTrace("somekey") << "this is a message "
                      << c << fValue(c)
                      << b << fValue(b)
                      << f << fValue(f)
                      << d << fValue(d)
                      << ld << fValue(ld)
                      << vp << fValue(vp)
                      << cvp << fValue(cvp)
                      << cp << fValue(cp)
                      << scp << fValue(scp)
                      << ucp << fValue(ucp)
                      << str << fValue(str)
                      << ss << fValue(ss)
                      << us << fValue(us)
                      << si << fValue(si)
                      << ui << fValue(ui)
                      << sl << fValue(sl)
                      << ul << fValue(ul)
                      << sll << fValue(sll)
                      << ull << fValue(ull)
                      << si8 << fValue(si8)
                      << si16 << fValue(si16)
                      << si32 << fValue(si32)
                      << si64 << fValue(si64)
                      << ui8 << fValue(ui8)
                      << ui16 << fValue(ui16)
                      << ui32 << fValue(ui32)
                      << ui64 << fValue(ui64)
                      << v.size() << fValue(v.size())
                      << cs << fValue(cs)
                      << fEnd;
}

// Test all 'debug' macros
static void testDebugMacros()
{
    char c;
    bool b;
    float f;
    double d;
    long double ld;
    void *vp;
    const void *cvp;
    char *cp;
    signed char *scp;
    unsigned char *ucp;
    std::string str;
    short ss;
    unsigned short us;
    int si;
    unsigned int ui;
    long sl;
    unsigned long ul;
    vlonglong sll;
    vulonglong ull;
    int8_t si8;
    int16_t si16;
    int32_t si32;
    int64_t si64;
    uint8_t ui8;
    uint16_t ui16;
    uint32_t ui32;
    uint64_t ui64;
    std::vector<char> v;
    CustomStruct cs;
    TRACELIB_DEBUG;

    TRACELIB_DEBUG_KEY("somekey");

    TRACELIB_DEBUG_KEY_MSG("somekey", "somemessage");

    TRACELIB_DEBUG_KEY_MSG("somekey", "somemessage, with "
                           << TRACELIB_VALUE(c)
                           << TRACELIB_VALUE(b)
                           << TRACELIB_VALUE(f)
                           << TRACELIB_VALUE(d)
                           << TRACELIB_VALUE(ld)
                           << TRACELIB_VALUE(vp)
                           << TRACELIB_VALUE(cvp)
                           << TRACELIB_VALUE(cp)
                           << TRACELIB_VALUE(scp)
                           << TRACELIB_VALUE(ucp)
                           << TRACELIB_VALUE(str)
                           << TRACELIB_VALUE(ss)
                           << TRACELIB_VALUE(us)
                           << TRACELIB_VALUE(si)
                           << TRACELIB_VALUE(ui)
                           << TRACELIB_VALUE(sl)
                           << TRACELIB_VALUE(ul)
                           << TRACELIB_VALUE(sll)
                           << TRACELIB_VALUE(ull)
                           << TRACELIB_VALUE(si8)
                           << TRACELIB_VALUE(si16)
                           << TRACELIB_VALUE(si32)
                           << TRACELIB_VALUE(si64)
                           << TRACELIB_VALUE(ui8)
                           << TRACELIB_VALUE(ui16)
                           << TRACELIB_VALUE(ui32)
                           << TRACELIB_VALUE(ui64)
                           << TRACELIB_VALUE(v.size())
                           << TRACELIB_VALUE(cs));

    fDebug("somekey") << "this is a message "
                      << c << fValue(c)
                      << b << fValue(b)
                      << f << fValue(f)
                      << d << fValue(d)
                      << ld << fValue(ld)
                      << vp << fValue(vp)
                      << cvp << fValue(cvp)
                      << cp << fValue(cp)
                      << scp << fValue(scp)
                      << ucp << fValue(ucp)
                      << str << fValue(str)
                      << ss << fValue(ss)
                      << us << fValue(us)
                      << si << fValue(si)
                      << ui << fValue(ui)
                      << sl << fValue(sl)
                      << ul << fValue(ul)
                      << sll << fValue(sll)
                      << ull << fValue(ull)
                      << si8 << fValue(si8)
                      << si16 << fValue(si16)
                      << si32 << fValue(si32)
                      << si64 << fValue(si64)
                      << ui8 << fValue(ui8)
                      << ui16 << fValue(ui16)
                      << ui32 << fValue(ui32)
                      << ui64 << fValue(ui64)
                      << v.size() << fValue(v.size())
                      << cs << fValue(cs)
                      << fEnd;
}

// Test all 'error' macros
static void testErrorMacros()
{
    char c;
    bool b;
    float f;
    double d;
    long double ld;
    void *vp;
    const void *cvp;
    char *cp;
    signed char *scp;
    unsigned char *ucp;
    std::string str;
    short ss;
    unsigned short us;
    int si;
    unsigned int ui;
    long sl;
    unsigned long ul;
    vlonglong sll;
    vulonglong ull;
    int8_t si8;
    int16_t si16;
    int32_t si32;
    int64_t si64;
    uint8_t ui8;
    uint16_t ui16;
    uint32_t ui32;
    uint64_t ui64;
    std::vector<char> v;
    CustomStruct cs;
    TRACELIB_ERROR;

    TRACELIB_ERROR_KEY("somekey");

    TRACELIB_ERROR_KEY_MSG("somekey", "somemessage");

    TRACELIB_ERROR_KEY_MSG("somekey", "somemessage, with "
                           << TRACELIB_VALUE(c)
                           << TRACELIB_VALUE(b)
                           << TRACELIB_VALUE(f)
                           << TRACELIB_VALUE(d)
                           << TRACELIB_VALUE(ld)
                           << TRACELIB_VALUE(vp)
                           << TRACELIB_VALUE(cvp)
                           << TRACELIB_VALUE(cp)
                           << TRACELIB_VALUE(scp)
                           << TRACELIB_VALUE(ucp)
                           << TRACELIB_VALUE(str)
                           << TRACELIB_VALUE(ss)
                           << TRACELIB_VALUE(us)
                           << TRACELIB_VALUE(si)
                           << TRACELIB_VALUE(ui)
                           << TRACELIB_VALUE(sl)
                           << TRACELIB_VALUE(ul)
                           << TRACELIB_VALUE(sll)
                           << TRACELIB_VALUE(ull)
                           << TRACELIB_VALUE(si8)
                           << TRACELIB_VALUE(si16)
                           << TRACELIB_VALUE(si32)
                           << TRACELIB_VALUE(si64)
                           << TRACELIB_VALUE(ui8)
                           << TRACELIB_VALUE(ui16)
                           << TRACELIB_VALUE(ui32)
                           << TRACELIB_VALUE(ui64)
                           << TRACELIB_VALUE(v.size())
                           << TRACELIB_VALUE(cs));

    fError("somekey") << "this is a message "
                      << c << fValue(c)
                      << b << fValue(b)
                      << f << fValue(f)
                      << d << fValue(d)
                      << ld << fValue(ld)
                      << vp << fValue(vp)
                      << cvp << fValue(cvp)
                      << cp << fValue(cp)
                      << scp << fValue(scp)
                      << ucp << fValue(ucp)
                      << str << fValue(str)
                      << ss << fValue(ss)
                      << us << fValue(us)
                      << si << fValue(si)
                      << ui << fValue(ui)
                      << sl << fValue(sl)
                      << ul << fValue(ul)
                      << sll << fValue(sll)
                      << ull << fValue(ull)
                      << si8 << fValue(si8)
                      << si16 << fValue(si16)
                      << si32 << fValue(si32)
                      << si64 << fValue(si64)
                      << ui8 << fValue(ui8)
                      << ui16 << fValue(ui16)
                      << ui32 << fValue(ui32)
                      << ui64 << fValue(ui64)
                      << v.size() << fValue(v.size())
                      << cs << fValue(cs)
                      << fEnd;
}

// Test all 'watch' macros
static void testWatchMacros()
{
    char c;
    bool b;
    float f;
    double d;
    long double ld;
    void *vp;
    const void *cvp;
    char *cp;
    signed char *scp;
    unsigned char *ucp;
    std::string str;
    short ss;
    unsigned short us;
    int si;
    unsigned int ui;
    long sl;
    unsigned long ul;
    vlonglong sll;
    vulonglong ull;
    int8_t si8;
    int16_t si16;
    int32_t si32;
    int64_t si64;
    uint8_t ui8;
    uint16_t ui16;
    uint32_t ui32;
    uint64_t ui64;
    std::vector<char> v;
    CustomStruct cs;

    TRACELIB_WATCH_KEY("somekey", TRACELIB_VAR(c));

    TRACELIB_WATCH_KEY_MSG("somekey", "somemessage", TRACELIB_VAR(c));

    TRACELIB_WATCH_KEY_MSG("somekey", "somemessage, with ",
                           TRACELIB_VAR(c)
                           << TRACELIB_VAR(b)
                           << TRACELIB_VAR(f)
                           << TRACELIB_VAR(d)
                           << TRACELIB_VAR(ld)
                           << TRACELIB_VAR(vp)
                           << TRACELIB_VAR(cvp)
                           << TRACELIB_VAR(cp)
                           << TRACELIB_VAR(scp)
                           << TRACELIB_VAR(ucp)
                           << TRACELIB_VAR(str)
                           << TRACELIB_VAR(ss)
                           << TRACELIB_VAR(us)
                           << TRACELIB_VAR(si)
                           << TRACELIB_VAR(ui)
                           << TRACELIB_VAR(sl)
                           << TRACELIB_VAR(ul)
                           << TRACELIB_VAR(sll)
                           << TRACELIB_VAR(ull)
                           << TRACELIB_VAR(si8)
                           << TRACELIB_VAR(si16)
                           << TRACELIB_VAR(si32)
                           << TRACELIB_VAR(si64)
                           << TRACELIB_VAR(ui8)
                           << TRACELIB_VAR(ui16)
                           << TRACELIB_VAR(ui32)
                           << TRACELIB_VAR(ui64)
                           << TRACELIB_VAR(v.size())
                           << TRACELIB_VAR(cs));

    fWatch("somekey") << "this is a message "
                      << c << fVar(c)
                      << b << fVar(b)
                      << f << fVar(f)
                      << d << fVar(d)
                      << ld << fVar(ld)
                      << vp << fVar(vp)
                      << cvp << fVar(cvp)
                      << cp << fVar(cp)
                      << scp << fVar(scp)
                      << ucp << fVar(ucp)
                      << str << fVar(str)
                      << ss << fVar(ss)
                      << us << fVar(us)
                      << si << fVar(si)
                      << ui << fVar(ui)
                      << sl << fVar(sl)
                      << ul << fVar(ul)
                      << sll << fVar(sll)
                      << ull << fVar(ull)
                      << si8 << fVar(si8)
                      << si16 << fVar(si16)
                      << si32 << fVar(si32)
                      << si64 << fVar(si64)
                      << ui8 << fVar(ui8)
                      << ui16 << fVar(ui16)
                      << ui32 << fVar(ui32)
                      << ui64 << fVar(ui64)
                      << v.size() << fVar(v.size())
                      << cs << fVar(cs)
                      << fEnd;
}

int main()
{
    testNamespaceMacros();
    testPortMacro();
    testConfigFileNameMacro();
    testTraceMacros();
    testDebugMacros();
    testErrorMacros();
    testWatchMacros();
    return 0;
}

