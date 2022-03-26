#pragma once
/**
 * @file TestDCMGetTMUMeasureResultRunningTime.h
 * @brief Test the running time of function GetTMUMeasureResult
 * @author Guangyun Wang
 * @date 2020/09/21
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"

XT_TEST(TMURunningTimeTest, TestDCMGetTMUMeasureResultRunningTime)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, RUNNING_TIME);
	CTimeReport timeReport(strFuncName.c_str(), "FunctionRunningTimeTest");
	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		timeReport.SetNoBoardValid();
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		timeReport.addMsg("Load vector file(%s) fail, the vector file maybe not right.", g_lpszVectorFilePath);
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(timeReport, mapSlot);
	auto iterSlot = mapSlot.begin();

	UINT uControllerCount = mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD;

	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_TMU", "CH0,CH4");
	dcm.SetPinGroup("G_RUNPIN", "CH0,CH2,CH4,CH6");


	double dBackupEdge[EDGE_COUNT] = { 0 };
	WAVE_FORMAT BackupWaveFormat = WAVE_FORMAT::NRZ;
	IO_FORMAT BackupIoFormat = IO_FORMAT::NRZ;
	COMPARE_MODE BackupCompareMode = COMPARE_MODE::EDGE;

	double dBackupPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);
	dcm_GetEdge(mapSlot.begin()->first, 0, 0, dBackupEdge, BackupWaveFormat, BackupIoFormat, BackupCompareMode);

	double dPeriod = 1e3;
	dcm.Connect("G_RUNPIN");
	dcm.SetPinLevel("G_RUNPIN", 3, 0, 1.5, 0.8);
	dcm.SetPeriod("0", dPeriod);
	nRetVal = dcm.SetEdge("G_RUNPIN", "0", DCM_DTFT_RO, DCM_IO_NRZ, 0, dPeriod / 2, 0, dPeriod / 2, dPeriod * 3 / 4);
	USHORT usFirstValidSite = mapSlot.begin()->second;

	double dSampleNum = 100;
	double dTimeout = 2;

	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);
	dcm.SetTMUParam("G_TMU", DCM_ALLSITE, DCM_POS, 0, 0);

	dcm.RunVectorWithGroup("G_RUNPIN", "TMU_ST", "TMU_SP", FALSE);
	dcm.TMUMeasure("G_TMU", DCM_MEAS_FREQ_DUTY, dSampleNum, dTimeout);
	timeReport.timeStart();
	dcm.GetTMUMeasureResult("CH0", usFirstValidSite, DCM_FREQ);
	timeReport.timeStop();
	timeReport.addMsg("Get the measurement result of frequency");

	dcm.StopVector("G_RUNPIN");

	dcm.RunVectorWithGroup("G_RUNPIN", "TMU_ST", "TMU_SP", FALSE);
	dcm.TMUMeasure("CH0", DCM_MEAS_FREQ_DUTY, dSampleNum, dTimeout);
	timeReport.timeStart();
	dcm.GetTMUMeasureResult("CH0", usFirstValidSite, DCM_HIGH_DUTY);
	timeReport.timeStop();
	timeReport.addMsg("Get the measurement result of high duty");
	dcm.StopVector("G_RUNPIN");

	dcm.RunVectorWithGroup("G_RUNPIN", "TMU_ST", "TMU_SP", FALSE);
	dcm.TMUMeasure("CH0", DCM_MEAS_FREQ_DUTY, dSampleNum, dTimeout);
	timeReport.timeStart();
	dcm.GetTMUMeasureResult("CH0", usFirstValidSite, DCM_LOW_DUTY);
	timeReport.timeStop();
	timeReport.addMsg("Get the measurement result of low duty");
	dcm.StopVector("G_RUNPIN");

	dcm.RunVectorWithGroup("G_RUNPIN", "TMU_ST", "TMU_SP", FALSE);
	dcm.TMUMeasure("G_TMU", DCM_MEAS_EDGE, 10, dTimeout);
	timeReport.timeStart();
	dcm.GetTMUMeasureResult("CH0", usFirstValidSite, DCM_EDGE);
	timeReport.timeStop();
	timeReport.addMsg("Get the measurement result of edge");
	dcm.StopVector("G_RUNPIN");

	dcm.RunVectorWithGroup("G_RUNPIN", "TMU_ST", "TMU_SP", FALSE);
	dcm.TMUMeasure("G_TMU", DCM_MEAS_DELAY, 10, dTimeout);
	timeReport.timeStart();
	dcm.GetTMUMeasureResult("CH0", usFirstValidSite, DCM_DELAY);
	timeReport.timeStop();
	timeReport.addMsg("Get the measurement result of delay");
	dcm.StopVector("G_RUNPIN");

	timeReport.Print(this, g_lpszVectorFilePath);

	dcm_CloseFile();
}
