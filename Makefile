CL_DEBUGFLAGS=/Zi
LINK_DEBUGFLAGS=/DEBUG

all: sampleapp.exe

tracelib.dll: tracelib.obj
	link /nologo /DLL $(LINK_DEBUGFLAGS) /OUT:tracelib.dll tracelib.obj
tracelib.obj: tracelib.c tracelib.h
	cl /nologo /W3 /c tracelib.c $(CL_DEBUGFLAGS) /DTRACELIB_MAKEDLL /D_CRT_SECURE_NO_WARNINGS

sampleapp.exe: sampleapp.obj tracelib.dll
	link /nologo $(LINK_DEBUGFLAGS) /OUT:sampleapp.exe sampleapp.obj tracelib.lib

sampleapp.obj: sampleapp.cpp tracelib.h
	cl /nologo /W3 /EHsc /c sampleapp.cpp $(CL_DEBUGFLAGS) /FC

clean:
	del *.obj *.exe *.lib *.exp *.dll *.ilk *.pdb
