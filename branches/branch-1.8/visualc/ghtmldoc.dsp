# Microsoft Developer Studio Project File - Name="ghtmldoc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ghtmldoc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ghtmldoc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ghtmldoc.mak" CFG="ghtmldoc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ghtmldoc - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ghtmldoc - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ghtmldoc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "gRelease"
# PROP Intermediate_Dir "gRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /GX /Ot /Op /Ob2 /I ".." /I "../visualc" /I "../png" /I "../jpeg" /I "../zlib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# SUBTRACT CPP /Os
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 fltkimages.lib fltk.lib libeay32.lib RSAglue.lib ssleay32.lib comctl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcmt.lib" /out:"ghtmldoc.exe" /libpath:"../../openssl-0.9.6a/out32dll"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "ghtmldoc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "gDebug"
# PROP Intermediate_Dir "gDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /Gm /GX /Zi /Od /I ".." /I "../visualc" /I "../png" /I "../jpeg" /I "../zlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 fltkimages.lib fltk.lib libeay32.lib RSAglue.lib ssleay32.lib comctl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"ghtmldocd.exe" /pdbtype:sept /libpath:"../../openssl-0.9.6a/out32dll"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ghtmldoc - Win32 Release"
# Name "ghtmldoc - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "c,cxx"
# Begin Source File

SOURCE=..\htmldoc\file.c
# End Source File
# Begin Source File

SOURCE=..\htmldoc\html.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\htmldoc.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\htmllib.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\htmlsep.cxx
# End Source File
# Begin Source File

SOURCE="..\htmldoc\http-addr.c"
# End Source File
# Begin Source File

SOURCE="..\htmldoc\http-support.c"
# End Source File
# Begin Source File

SOURCE=..\htmldoc\http.c
# End Source File
# Begin Source File

SOURCE=..\htmldoc\image.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\iso8859.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\md5.c
# End Source File
# Begin Source File

SOURCE=..\htmldoc\progress.cxx
# End Source File
# Begin Source File

SOURCE="..\htmldoc\ps-pdf.cxx"
# End Source File
# Begin Source File

SOURCE=..\htmldoc\rc4.c
# End Source File
# Begin Source File

SOURCE=..\htmldoc\string.c
# End Source File
# Begin Source File

SOURCE=..\htmldoc\toc.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\util.cxx
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\debug.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\file.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\hdstring.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\html.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\htmldoc.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\http.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\image.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\iso8859.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\types.h
# End Source File
# End Group
# Begin Group "GUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\htmldoc\gui.cxx

!IF  "$(CFG)" == "ghtmldoc - Win32 Release"

# PROP Intermediate_Dir "..\GUI\Release"

!ELSEIF  "$(CFG)" == "ghtmldoc - Win32 Debug"

# PROP Intermediate_Dir "../gui/Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\htmldoc\gui.h

!IF  "$(CFG)" == "ghtmldoc - Win32 Release"

# PROP Intermediate_Dir "..\GUI\Release"

!ELSEIF  "$(CFG)" == "ghtmldoc - Win32 Debug"

# PROP Intermediate_Dir "../gui/Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\htmldoc.ico

!IF  "$(CFG)" == "ghtmldoc - Win32 Release"

# PROP Intermediate_Dir "..\GUI\Release"

!ELSEIF  "$(CFG)" == "ghtmldoc - Win32 Debug"

# PROP Intermediate_Dir "../gui/Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\icons.rc

!IF  "$(CFG)" == "ghtmldoc - Win32 Release"

# PROP Intermediate_Dir "..\GUI\Release"
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "ghtmldoc - Win32 Debug"

# PROP Intermediate_Dir "../gui/Debug"
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
