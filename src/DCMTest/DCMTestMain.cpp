/*!
 * @file      DCMTestMain.cpp
 *
 * Copyright (C) 北京华峰测控技术股份有限公司
 *
 * @author    xg
 * @date      2020/02/06
 * @version   v1.0.0.1
 * @brief     N/A
 */
#include "DCMTestMain.h"
#include "Shlwapi.h"
int main(int argc, char ** argv)
{
	{
		// 打印日志方式
		std::string strLogFileName = XTest::ExeDir();
		SYSTEMTIME sysTime;
		GetSystemTime(&sysTime);
		char lpszLogFile[MAX_PATH] = { 0 };
		sprintf_s(lpszLogFile, sizeof(lpszLogFile),"%s\\AutomaticTestCase\\AutomaticTestLog_%04d%02d%02d_%02d%02d%02d.html", strLogFileName.c_str(),
			sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		XT_SET_OUT_FILE(lpszLogFile);
	}

	XTestFilter accotest("AccoTest");
	accotest.AddTest("SystemInit");
	XTestFilter InitTest("InitTest");
	InitTest.AddTest("InitDCMTest");

	//*******************************DCM digital function Test***********************************//
   	XTestFilter ParamValidityTest("ParamValidityTest");

	XTestFilter FunctionFunctionTest("FunctionFunctionTest");
    
  	XTestFilter FunctionRunningTimeTest("FunctionRunningTimeTest");

	//*******************************Test parameter validity***********************************//
// 	ParamValidityTest.AddTest("TestDCMLoadVectorFileParamValidity");
//	ParamValidityTest.AddTest("TestDCMSetValidPinParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetPinGroupParamValidity");
// 	ParamValidityTest.AddTest("TestDCMConnectParamValidity");
// 	ParamValidityTest.AddTest("TestDCMDisconnectParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetPeriodParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetEdgeParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetPinLevelParamValidity");
// 	ParamValidityTest.AddTest("TestDCMRunVectorWithGroupParamValidity");
// 	ParamValidityTest.AddTest("TestDCMRunVectorEnParamValidity");
// 	ParamValidityTest.AddTest("TestDCMStopVectorParamValidity");
// 	ParamValidityTest.AddTest("TestDCMInitMCUParamValidity");
// 	ParamValidityTest.AddTest("TestDCMGetMCUResultParamValidity"); 
// 	ParamValidityTest.AddTest("TestDCMGetFailCountParamValidity");
// 	ParamValidityTest.AddTest("TestDCMGetFirstFailLineNoParamValidity");
// 	ParamValidityTest.AddTest("TestDCMGetFailLineNoParamValidity"); 
// 	ParamValidityTest.AddTest("TestDCMGetMCUPinRunStatusParamValidity");
// 	ParamValidityTest.AddTest("TestDCMGetMCUPinResultParamValidity");
// 	ParamValidityTest.AddTest("TestDCMGetCaptureDataParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetCaptureParamValidity");
//	ParamValidityTest.AddTest("TestDCMGetCaptureData_WithSiteCaptureParamValidity");
// 	ParamValidityTest.AddTest("TestDCMGetHardwareCaptureDataParamValidity");
// 	ParamValidityTest.AddTest("TestDCMWriteWaveDataParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetWaveDataParamParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetSiteWaveDataParamValidity");
// 	ParamValidityTest.AddTest("TestDCMWriteWaveData_BlankParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetVTParamValidity");
//	ParamValidityTest.AddTest("TestDCMSetDynamicLoadParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetDynamicLoad_GroupParamValidity");
// 	ParamValidityTest.AddTest("TestDCMGetStopLineNoParamValidity");
// 	ParamValidityTest.AddTest("TestDCMGetStopLabelParamValidity");
//	ParamValidityTest.AddTest("TestDCMGetRunLineCountParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetChannelStatusParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetInstructionParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetOperandParamValidity");
// 	ParamValidityTest.AddTest("TestDCMSetPrereadVectorParamValidity");
//	ParamValidityTest.AddTest("TestDCMSetFailSavingTypeParamValidity");
//	ParamValidityTest.AddTest("TestDCMI2CSetParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CConnectParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CDisconnectParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CSetPinLevelParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CSetPinLevel_PinParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CWriteDataParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CWriteData_SameParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CReadDataParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CWriteMultiDataParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CGetReadDataParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CGetBitDataParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CGetNACKIndexParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CSetSCLEdgeParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CSetSDAEdgeParamValidity");
// 	ParamValidityTest.AddTest("TestDCMI2CSetDynamicLoadParamValidity");
// 	ParamValidityTest.AddTest("TestDCMAlarm");
 
	//*******************************Test function function***********************************//
// 	FunctionFunctionTest.AddTest("TestDCMLoadVectorFileFunction");
//	FunctionFunctionTest.AddTest("TestDCMSetValidPinFunction");
// 	FunctionFunctionTest.AddTest("TestDCMSetPinGroupFunction");
// 	FunctionFunctionTest.AddTest("TestDCMConnectFunction");
// 	FunctionFunctionTest.AddTest("TestDCMDisconnectFunction");
// 	FunctionFunctionTest.AddTest("TestDCMSetPeriodFunction");
// 	FunctionFunctionTest.AddTest("TestDCMSetEdgeFunction");
//	FunctionFunctionTest.AddTest("TestDCMInitMCUFunction");
//	FunctionFunctionTest.AddTest("TestDCMSetPinLevelFunction");
// 	FunctionFunctionTest.AddTest("TestDCMRunVectorWithGroupFunction");
//	FunctionFunctionTest.AddTest("TestDCMRunVectorEnFunction");
//  FunctionFunctionTest.AddTest("TestDCMGetMCUResultFunction");
// 	FunctionFunctionTest.AddTest("TestDCMGetMCUPinRunStatusFunction");
// 	FunctionFunctionTest.AddTest("TestDCMGetMCUPinResultFunction");
// 	FunctionFunctionTest.AddTest("TestDCMGetFailCountFunction");
//  FunctionFunctionTest.AddTest("TestDCMGetFirstFailLineNoFunction");
//  FunctionFunctionTest.AddTest("TestDCMGetFailLineNoFunction");
//	FunctionFunctionTest.AddTest("TestDCMSetFailSavingTypeFunction");
//  FunctionFunctionTest.AddTest("TestDCMStopVectorFunction");
//  FunctionFunctionTest.AddTest("TestDCMGetCaptureDataFunction");
//  FunctionFunctionTest.AddTest("TestDCMSetCaptureFunction");
// 	FunctionFunctionTest.AddTest("TestDCMGetHardwareCaptureDataFunction");
//	FunctionFunctionTest.AddTest("TestDCMWriteWaveDataFunction");
//	FunctionFunctionTest.AddTest("TestDCMWriteWaveData_BlankFunction");
//	FunctionFunctionTest.AddTest("TestDCMSetPrereadVectorFunction");
// 	FunctionFunctionTest.AddTest("TestDCMSetVTFunction");
//	FunctionFunctionTest.AddTest("TestDCMSetDynamicLoadFunction");
//	FunctionFunctionTest.AddTest("TestDCMSetDynamicLoad_GroupFunction");
// 	FunctionFunctionTest.AddTest("TestDCMGetStopLineNoFunction");
// 	FunctionFunctionTest.AddTest("TestDCMGetStopLabelFunction");
//	FunctionFunctionTest.AddTest("TestDCMGetRunLineCountFunction");
// 	FunctionFunctionTest.AddTest("TestDCMSetChannelStatusFunction");
//	FunctionFunctionTest.AddTest("TestDCMSetInstructionFunction");
//	FunctionFunctionTest.AddTest("TestDCMSetOperandFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CConnectFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CDisconnectFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CSetPinLevelFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CSetPinLevel_PinFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CWriteDataFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CWriteData_SameFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CReadDataFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CGetBitDataFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CWriteMultiDataFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CGetNACKIndexFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CSetStopStatusFunction");
// 	FunctionFunctionTest.AddTest("TestDCMI2CSetDynamicLoadFunction");
// 
// //*******************************Test function running time***********************************//
// 	FunctionRunningTimeTest.AddTest("TestDCMLoadVectorFileRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetValidPinRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetPinGroupRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMConnectRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMDisconnectRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetPinLevelRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMRunVectorWithGroupRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMRunVectorEnRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMInitMCURunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMWriteWaveDataRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetWaveDataParamRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetSiteWaveDataRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMWriteWaveData_blankRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSaveFailMapRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetFailCountRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetFirstFailLineNoRunningTime");
//	FunctionRunningTimeTest.AddTest("TestDCMGetFailLineNoRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetStopLabelRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetStopLineNoRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetRunLineCountRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetEdgeRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetPeriodRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMStopVectorRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetVTRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetDynamicLoadRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetDynamicLoad_GroupRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetMCUResultRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetMCUPinResultRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetMCUPinRunStatusRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetCaptureDataRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetCaptureRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetCaptureData_WithSiteCaptureRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMGetHardwareCaptureDataRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetChannelStatusRunningTime");
//	FunctionRunningTimeTest.AddTest("TestDCMSetInstructionRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMSetOperandRunningTime");
//	FunctionRunningTimeTest.AddTest("TestDCMSetPrereadVectorRunningTime");
//	FunctionRunningTimeTest.AddTest("TestDCMSetFailSavingTypeRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CSetRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CConnectRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CDisconnectRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CSetSCLEdgeRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CSetSDAEdgeRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CSetPinLevelRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CSetPinLevel_PinRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CWriteDataRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CWriteData_SameRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CReadDataRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CGetReadDataRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CGetBitDataRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CGetNACKIndexRunningTime");
// 	FunctionRunningTimeTest.AddTest("TestDCMI2CWriteMultiDataRunningTime");
//	FunctionRunningTimeTest.AddTest("TestDCMI2CSetDynamicLoadRunningTime");
//	FunctionRunningTimeTest.AddTest("TestDCMWriteDataFunctionModuleRunningTime");
//*******************************TMU Test***********************************//
  	XTestFilter TMUParamValidity("TMUParamValidityTest");
   	XTestFilter TMUFunction("TMUFunctionTest");
   	XTestFilter TMURunningTime("TMURunningTimeTest");

//	TMUParamValidity.AddTest("TestDCMSetTMUMatrixParamValidity");
//	TMUParamValidity.AddTest("TestDCMSetTMUParamParamValidity");
//	TMUParamValidity.AddTest("TestDCMSetTMUParam_UnitParamValidity");
//	TMUParamValidity.AddTest("TestDCMTMUMeasureParamValidity");
//	TMUParamValidity.AddTest("TestDCMGetTMUMeasureResultParamValidity");

//  TMUFunction.AddTest("TestDCMSetTMUMatrixFunction");
//  TMUFunction.AddTest("TestDCMSetTMUParamFunction");
//  TMUFunction.AddTest("TestDCMTMUMeasureFunction");
//  TMUFunction.AddTest("TestDCMGetTMUMeasureResultFunction");
// 
// 	TMURunningTime.AddTest("TestDCMSetTMUMatixRunningTime");
// 	TMURunningTime.AddTest("TestDCMSetTMUParamRunningTime");
// 	TMURunningTime.AddTest("TestDCMTMUMeasureRunningTime");
// 	TMURunningTime.AddTest("TestDCMGetTMUMeasureResultRunningTime");

//*******************************PPMU Test***********************************//
//	XTestFilter PMUParamValidity("PMUParamValidityTest");
//	XTestFilter PMUFunction("PMUFunctionTest");
//	XTestFilter PMURunningTimeTest("PMURunningTimeTest");
// 
// 	PMUParamValidity.AddTest("TestDCMInitPPMUParamValidity");
// 	PMUParamValidity.AddTest("TestDCMSetPPMUParamValidity");
// 	PMUParamValidity.AddTest("TestDCMSetPPMUSingleSiteParamValidity");
// 	PMUParamValidity.AddTest("TestDCMGetPPMUMeasResultParamValidity");
// 	PMUParamValidity.AddTest("TestDCMPPMUMeasureParamValidity");
// 
// 	PMUFunction.AddTest("TestDCMSetPPMUFunction");
// 	PMUFunction.AddTest("TestDCMGetPPMUMeasResultFunction");
// 	//PMUFunction.AddTest("TestDCMInitPPMUFunction");///<Can't be test
// 	PMUFunction.AddTest("TestDCMSetPPMUSingleSiteFunction");
// 
// 	PMURunningTimeTest.AddTest("TestDCMInitPPMURunningTime");
// 	PMURunningTimeTest.AddTest("TestDCMSetPPMURunningTime");
// 	PMURunningTimeTest.AddTest("TestDCMSetPPMUSingleSiteRunningTime");
// 	PMURunningTimeTest.AddTest("TestDCMPPMUMeasureRunningTime");
// 	PMURunningTimeTest.AddTest("TestDCMGetPPMUMeasResultRunningTime");


	XT_RUN(argc, argv);

	//不要移动位置
// 	DCM_file_check(g_testTime_file_path);
// 	if (nullptr != g_ch_detail_Info)
// 	{
// 		delete[] g_ch_detail_Info;
// 		g_ch_detail_Info = nullptr;
// 	}
// 	if (nullptr != g_detail_testTime_singleSite)
// 	{
// 		delete[] g_detail_testTime_singleSite;
// 		g_detail_testTime_singleSite = nullptr;
// 	}

	system("pause");
	return 0;
}