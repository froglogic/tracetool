CPPFLAGS=/nologo /W3 /EHsc /D_CRT_SECURE_NO_WARNINGS

all: sampleapp.exe tracegen.exe traceview.exe

tracelib.lib: core.obj serializer.obj output.obj filter.obj winstringconv.obj
	lib /nologo /OUT:tracelib.lib core.obj serializer.obj output.obj filter.obj winstringconv.obj user32.lib ws2_32.lib
core.obj: core.cpp tracelib.h
serializer.obj: serializer.cpp tracelib.h
output.obj: output.cpp tracelib.h
filter.obj: filter.cpp tracelib.h

sampleapp.exe: sampleapp.obj tracelib.lib
	link /nologo /OUT:sampleapp.exe sampleapp.obj tracelib.lib
sampleapp.obj: sampleapp.cpp tracelib.h
	cl $(CPPFLAGS) /c sampleapp.cpp /FC

tracegen.exe: tracegen.obj tracelib.lib
	link /nologo /OUT:tracegen.exe tracegen.obj tracelib.lib /SUBSYSTEM:WINDOWS
tracegen.obj: tracegen.cpp tracelib.h
	cl $(CPPFLAGS) /c tracegen.cpp /FC

traceview.exe: traceview.cpp
	cl $(CPPFLAGS) traceview.cpp ws2_32.lib

clean:
	del *.obj *.exe *.lib *.pdb
