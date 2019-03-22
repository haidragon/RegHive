# Microsoft Developer Studio Project File - Name="regedt33" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=regedt33 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "regedt33.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "regedt33.mak" CFG="regedt33 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "regedt33 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "regedt33 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "regedt33 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib ole32.lib /nologo /subsystem:windows /machine:I386 /out:"regedt33.exe"

!ELSEIF  "$(CFG)" == "regedt33 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib ole32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "regedt33 - Win32 Release"
# Name "regedt33 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\editdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\keytools.cpp
# End Source File
# Begin Source File

SOURCE=.\myctlrs.cpp
# End Source File
# Begin Source File

SOURCE=.\regedit.cpp
# End Source File
# Begin Source File

SOURCE=.\regsavld.cpp
# End Source File
# Begin Source File

SOURCE=.\srchrpl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\automgm.h
# End Source File
# Begin Source File

SOURCE=.\regedit.h
# End Source File
# Begin Source File

SOURCE=.\regsavld.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\2.bmp
# End Source File
# Begin Source File

SOURCE=.\arrowdn.bmp
# End Source File
# Begin Source File

SOURCE=.\arrowup.bmp
# End Source File
# Begin Source File

SOURCE=.\FOLDER_CLOSE.ico
# End Source File
# Begin Source File

SOURCE=.\Gray.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\key1.bmp
# End Source File
# Begin Source File

SOURCE=.\rededt33.rc
# End Source File
# Begin Source File

SOURCE=.\types.bmp
# End Source File
# Begin Source File

SOURCE=.\Yellow.ico
# End Source File
# End Group
# Begin Group "Hive"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CopyFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CopyFile.h
# End Source File
# Begin Source File

SOURCE=.\InitHive.cpp
# End Source File
# Begin Source File

SOURCE=.\InitHive.h
# End Source File
# Begin Source File

SOURCE=.\ntreg.c
# End Source File
# Begin Source File

SOURCE=.\ntreg.h
# End Source File
# Begin Source File

SOURCE=.\struct.h
# End Source File
# End Group
# Begin Group "List_SubKey_and_Value"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\List_SubKey_and_Value.cpp
# End Source File
# Begin Source File

SOURCE=.\List_SubKey_and_Value.h
# End Source File
# End Group
# Begin Group "Add_Delete_subkey_value"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Add_Delete_subkey_value.cpp
# End Source File
# Begin Source File

SOURCE=.\Add_Delete_subkey_value.h
# End Source File
# End Group
# Begin Group "Refresh_subtree"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Refresh_subtree.cpp
# End Source File
# Begin Source File

SOURCE=.\Refresh_subtree.h
# End Source File
# End Group
# End Target
# End Project
