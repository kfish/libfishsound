# Set the value of $(MSVCDir) for your installation. 
# Please note that none of the directory names are allowed to have spaces 
# in them. You must use the short name 
#
# "C:\Program Files\Microsoft Visual Studio\VC98" will not work.
# "C:\Progra~1\Micros~1\VC98" will work.
MSVCDir="C:\PROGRA~1\MICROS~1.NET\Vc7"

# MSVCDir="C:\PROGRA~1\Micros~4\VC98"

CPP=cl.exe
LINK32=link.exe
LINK32_FLAGS=libogg.lib libvorbis.lib libspeex.lib /nologo /dll /incremental:no /libpath:"$(MSVCDir)\Lib" /implib:".\libfishsound.lib" /machine:I386 /out:".\libfishsound.dll" 
PROG_LINK_FLAGS=/nologo /incremental:no /libpath:"$(MSVCDir)\Lib" /machine:I386 /exetype:dynamic

CFLAGS=/nologo /MD /W1 /GX /O2 /I"." /I"..\src\libfishsound" /I"$(MSVCDir)\Include"  /D "WIN32" /D "_USRDLL" /YX /FD


#====================================================================
# Targets

ALL :  "libfishsound.dll"

CLEAN:
	-@erase ".\*.obj"
	-@erase ".\vc60.idb"
	-@erase ".\libfishsound.dll"
	-@erase ".\libfishsound.exp"
	-@erase ".\libfishsound.lib"

#====================================================================

LINK32_OBJS= \
	".\fishsound.obj" \
	".\private.obj" \
	".\speex.obj" \
	".\vorbis.obj"

"libfishsound.dll" : $(LINK32_OBJS) ".\libfishsound.def"
    $(LINK32) $(LINK32_FLAGS) /def:".\libfishsound.def" $(LINK32_OBJS)


#====================================================================
# C files in src

.\fishsound.obj:
	$(CPP) $(CFLAGS) /Fo".\fishsound.obj" /c "..\src\fishsound.c"

.\private.obj:
	$(CPP) $(CFLAGS) /Fo".\private.obj" /c "..\src\private.c"

.\speex.obj:
	$(CPP) $(CFLAGS) /Fo".\speex.obj" /c "..\src\speex.c"

.\voribs.obj:
	$(CPP) $(CFLAGS) /Fo".\voribs.obj" /c "..\src\vorbis.c"

# End of Makefile
#====================================================================

