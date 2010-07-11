CPPFLAGS=/nologo /W3 /EHsc /D_CRT_SECURE_NO_WARNINGS /Zi
LFLAGS=/nologo
APPLFLAGS=/DEBUG

all: sampleapp.exe tracegen.exe traceview.exe

tracelib.lib: core.obj serializer.obj output.obj filter.obj winstringconv.obj configuration.obj backtrace.obj StackWalker.obj
	lib $(LFLAGS) /OUT:tracelib.lib core.obj serializer.obj output.obj filter.obj winstringconv.obj configuration.obj backtrace.obj StackWalker.obj user32.lib ws2_32.lib
core.obj: core.cpp tracelib.h
serializer.obj: serializer.cpp tracelib.h
output.obj: output.cpp tracelib.h
filter.obj: filter.cpp tracelib.h
configuration.obj: configuration.cpp configuration.h filter.h winstringconv.h 3rdparty/simpleini/SimpleIni.h
backtrace.obj: backtrace.cpp backtrace.h 3rdparty/stackwalker/StackWalker.h
StackWalker.obj: 3rdparty/stackwalker/StackWalker.cpp
	cl $(CPPFLAGS) /DNO_GETUSERNAME /c 3rdparty/stackwalker/StackWalker.cpp

sampleapp.exe: sampleapp.obj tracelib.lib
	link $(LFLAGS) $(APPLFLAGS) /OUT:sampleapp.exe sampleapp.obj tracelib.lib
sampleapp.obj: sampleapp.cpp tracelib.h
	cl $(CPPFLAGS) /c sampleapp.cpp /FC

tracegen.exe: tracegen.obj tracelib.lib
	link $(LFLAGS) $(APPLFLAGS) /OUT:tracegen.exe tracegen.obj tracelib.lib /SUBSYSTEM:WINDOWS
tracegen.obj: tracegen.cpp tracelib.h
	cl $(CPPFLAGS) /c tracegen.cpp /FC

traceview.exe: traceview.cpp
	cl $(CPPFLAGS) $(APPLFLAGS) traceview.cpp ws2_32.lib

clean:
	del *.obj *.exe *.lib *.pdb *.ilk *.suo
