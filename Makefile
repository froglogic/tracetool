CPPFLAGS=/nologo /EHsc /W3

all: sampleapp.exe

sampleapp.exe: sampleapp.obj
	link /nologo /OUT:sampleapp.exe sampleapp.obj

sampleapp.obj: sampleapp.cpp

clean:
	del *.obj *.exe
