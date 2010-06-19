all: sampleapp.exe

tracelib.dll: tracelib.obj
	link /nologo /DLL /OUT:tracelib.dll tracelib.obj
tracelib.obj: tracelib.c tracelib.h
	cl /nologo /W3 /c tracelib.c /DTRACELIB_MAKEDLL /D_CRT_SECURE_NO_WARNINGS

sampleapp.exe: sampleapp.obj tracelib.dll
	link /nologo /OUT:sampleapp.exe sampleapp.obj tracelib.lib

sampleapp.obj: sampleapp.cpp tracelib.h
	cl /nologo /W3 /EHsc /c sampleapp.cpp /FC

clean:
	del *.obj *.exe *.lib *.exp *.dll
