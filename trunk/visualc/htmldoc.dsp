# Microsoft Developer Studio Project File - Name="htmldoc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=htmldoc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "htmldoc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "htmldoc.mak" CFG="htmldoc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "htmldoc - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "htmldoc - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "htmldoc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /GX /O2 /Ob2 /I "../visualc" /I "../png" /I "../jpeg" /I "../zlib" /I "../../fltk-1.0" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /out:"htmldoc.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "htmldoc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /Gm /GX /Zi /Od /I "../visualc" /I "../png" /I "../jpeg" /I "../zlib" /I "../../fltk-1.0" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /nodefaultlib:"libcmt" /out:"htmldocd.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "htmldoc - Win32 Release"
# Name "htmldoc - Win32 Debug"
# Begin Source File

SOURCE=..\htmldoc\debug.h
# End Source File
# Begin Source File

SOURCE=..\gui\CheckButton.cxx
# End Source File
# Begin Source File

SOURCE=..\gui\CheckButton.h
# End Source File
# Begin Source File

SOURCE=..\gui\FileBrowser.cxx
# End Source File
# Begin Source File

SOURCE=..\gui\FileBrowser.h
# End Source File
# Begin Source File

SOURCE=..\gui\FileChooser.cxx
# End Source File
# Begin Source File

SOURCE=..\gui\FileChooser.h
# End Source File
# Begin Source File

SOURCE=..\gui\FileChooser2.cxx
# End Source File
# Begin Source File

SOURCE=..\gui\FileIcon.cxx
# End Source File
# Begin Source File

SOURCE=..\gui\FileIcon.h
# End Source File
# Begin Source File

SOURCE=..\gui\FileInput.cxx
# End Source File
# Begin Source File

SOURCE=..\gui\FileInput.h
# End Source File
# Begin Source File

SOURCE=..\gui\HelpDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\gui\HelpDialog.h
# End Source File
# Begin Source File

SOURCE=..\gui\HelpView.cxx
# End Source File
# Begin Source File

SOURCE=..\gui\HelpView.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\gui.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\gui.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\html.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\html.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\htmldoc.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\htmldoc.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\htmllib.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\image.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\image.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\iso8859.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\iso8859.h
# End Source File
# Begin Source File

SOURCE=..\htmldoc\ps-pdf.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\toc.cxx
# End Source File
# Begin Source File

SOURCE=..\htmldoc\types.h
# End Source File
# End Target
# End Project
