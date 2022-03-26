#pragma once
/**
 * @file TestDCMTMUMeasureRunningTime.h
 * @brief Test the running time of function TMUMeasure
 * @author Guangyun Wang
 * @date 2020/09/03
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"

XT_TEST(TMURunningTimeTest, TestDCMTMUMeasureRunningTime)
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

	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);
	dcm.SetTMUParam("G_TMU", DCM_ALLSITE, DCM_POS, 0, 0);
	timeReport.SetTimes(100);
	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.TMUMeasure("G_TMU", DCM_MEAS_FREQ_DUTY, 10, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Measure frequency at %d units' in %d controllers sample", uControllerCount * TMU_UNIT_COUNT_PER_CONTROLLER, uControllerCount);

	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.TMUMeasure("CH0", DCM_MEAS_FREQ_DUTY, 10, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Measure frequency at %d units' in %d controllers sample 10", uControllerCount, uControllerCount);
		
	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.TMUMeasure("G_TMU", DCM_MEAS_FREQ_DUTY, 10, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Measure frequency at %d units' in %d controllers sample 100", uControllerCount * TMU_UNIT_COUNT_PER_CONTROLLER, uControllerCount);

	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.TMUMeasure("CH0", DCM_MEAS_FREQ_DUTY, 10, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Measure frequency at %d units' in %d controllers sample 100", uControllerCount, uControllerCount);


	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.TMUMeasure("G_TMU", DCM_MEAS_EDGE, 10, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Measure edge at %d units' in %d controllers", uControllerCount * TMU_UNIT_COUNT_PER_CONTROLLER, uControllerCount);

	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.TMUMeasure("CH0", DCM_MEAS_EDGE, 10, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Measure edge at %d units' in %d controllers", uControllerCount, uControllerCount);

	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.TMUMeasure("G_TMU", DCM_MEAS_DELAY, 10, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Measure delay at %d units' in %d controllers", uControllerCount * TMU_UNIT_COUNT_PER_CONTROLLER, uControllerCount);

	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.TMUMeasure("CH0", DCM_MEAS_EDGE, 10, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Measure delay at %d units' in %d controllers", uControllerCount, uControllerCount);

	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
