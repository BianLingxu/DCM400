# Microsoft Developer Studio Project File - Name="DCM100" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DCM100 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DCM100.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DCM100.mak" CFG="DCM100 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DCM100 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DCM100 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DCM100 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\bin\VC\Release"
# PROP Intermediate_Dir "..\..\temp\VC\Release\DCM100"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DCM100_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "..\..\ext\ACVectorEditor" /I "$(ATLibs)\STSCore\inc" /I "$(ATLibs)\STSCore\inc\STSCore" /I "$(ATLibs)\ATInterface\ATGlobal" /I "$(ATLibs)\ATInterface\Hardware" /I "$(ATLibs)\STS8100\inc" /I "$(ATLibs)\STSPciE\inc\STSSP8201" /I "$(ATLibs)\VectFileIO\inc" /I "..\..\inc" /I "..\..\interior" /I "..\..\inc\DCM100" /I "$(ATLibs)\ATInterface\Global" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DCM100_EXPORTS" /D "_AT_CPP" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib STS8100.lib VectFileIO.lib STSSP8201.lib STSCore.lib /nologo /stack:0xa00000 /dll /pdb:"..\..\lib\VC\Release/DCM100.pdb" /machine:I386 /implib:"..\..\lib\VC\Release/DCM100.lib" /libpath:"$(ATLibs)\STSCore\lib\VC\Release" /libpath:"$(ATLibs)\STS8100\lib\VC\Release" /libpath:"$(ATLibs)\VectFileIO\lib\VC\Release" /libpath:"$(ATLibs)\STSPciE\lib\VC\Release"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "DCM100 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\bin\VC\Debug"
# PROP Intermediate_Dir "..\..\temp\VC\Debug\DCM100"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DCM100_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\ext\ACVectorEditor" /I "$(ATLibs)\STSCore\inc" /I "$(ATLibs)\STSCore\inc\STSCore" /I "$(ATLibs)\ATInterface\ATGlobal" /I "$(ATLibs)\ATInterface\Hardware" /I "$(ATLibs)\STS8100\inc" /I "$(ATLibs)\STSPciE\inc\STSSP8201" /I "$(ATLibs)\VectFileIO\inc" /I "..\..\inc" /I "..\..\interior" /I "..\..\inc\DCM100" /I "$(ATLibs)\ATInterface\Global" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DCM100_EXPORTS" /D "_AT_CPP" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib STS8100.lib VectFileIO.lib STSSP8201.lib STSCore.lib /nologo /dll /pdb:"..\..\lib\VC\Debug/DCM100.pdb" /debug /machine:I386 /implib:"..\..\lib\VC\Debug/DCM100.lib" /pdbtype:sept /libpath:"$(ATLibs)\STSCore\lib\VC\Debug" /libpath:"$(ATLibs)\STS8100\lib\VC\Debug" /libpath:"$(ATLibs)\VectFileIO\lib\VC\Debug" /libpath:"$(ATLibs)\STSPciE\lib\VC\Debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "DCM100 - Win32 Release"
# Name "DCM100 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Doctor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorDef.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorDef.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemBoard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemBoard.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemContinuity.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemContinuity.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemControl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemControl.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemDriverReceiver.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemDriverReceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemHighInstructions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemHighInstructions.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemHighMemory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemHighMemory.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemInterface.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemInterface.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemLow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemLow.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemPTMUPeriodCalibration.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDDoctorItemPTMUPeriodCalibration.h
# End Source File
# End Group
# Begin Group "DebugTool"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugTool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugTool.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemI2CTime.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemI2CTime.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemI2CVIH.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemI2CVIH.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemMcuMeas.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemMcuMeas.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemOut.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuForce.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuForce.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuInterval.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuInterval.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuIRange.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuIRange.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuMeas.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuMeas.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuMode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuMode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuSamples.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemPmuSamples.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemTmuDepthAddr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemTmuDepthAddr.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemTmuMeas.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemTmuMeas.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemTmuMode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemTmuMode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemTmuPTMCycleTimes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemTmuPTMCycleTimes.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemVIH.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemVIH.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemVIL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemVIL.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemVOH.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemVOH.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemVOL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DebugTool\HDDebugToolItemVOL.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\src\DCM100\ACVFailMapHeaderInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\ACVFailMapHeaderInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\CVectorInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DCM100.rc
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DCM100CalibrationInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DCM100FLASHHead.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DCM100HardInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DCMDataBuf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\DCMDataBuf.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDModule.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\HDModule.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\i2c.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\selftest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\SM8213.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\StdAfx.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\inc\DCM100\BaseDataOp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DCM100\CVectorInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DCM100\DCM100CalibrationInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DCM100\DCM100FLASHHead.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DCM100\DCM100HardInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DCM100\SM8213.def
# End Source File
# Begin Source File

SOURCE=..\..\inc\DCM100\SM8213.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\DCM100\SM8213_I2C.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=..\..\src\DCM100\html1.htm
# End Source File
# End Target
# End Project
