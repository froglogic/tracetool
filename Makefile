CPPFLAGS=/nologo /W3 /EHsc /D_CRT_SECURE_NO_WARNINGS

all: sampleapp.exe tracegen.exe

tracelib.lib: core.obj serializer.obj output.obj filter.obj
	lib /nologo /OUT:tracelib.lib core.obj serializer.obj output.obj filter.obj
core.obj: core.cpp tracelib.h
serializer.obj: serializer.cpp tracelib.h
output.obj: output.cpp tracelib.h
filter.obj: filter.cpp tracelib.h

sampleapp.exe: sampleapp.obj tracelib.lib
	link /nologo /OUT:sampleapp.exe sampleapp.obj tracelib.lib
sampleapp.obj: sampleapp.cpp tracelib.h
	cl $(CPPFLAGS) /c sampleapp.cpp /FC

tracegen.exe: tracegen.obj tracelib.lib
	link /nologo /OUT:tracegen.exe tracegen.obj tracelib.lib
tracegen.obj: tracegen.cpp tracelib.h
	cl $(CPPFLAGS) /c tracegen.cpp /FC

clean:
	del *.obj *.exe *.lib *.pdb
