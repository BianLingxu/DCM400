#pragma once
/**
 * @file TestDCMSetTMUParam_UnitRunningTime.h
 * @brief Test the running time of function SetTMUParam with unit
 * @author Guangyun Wang
 * @date 2020/09/03
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"

XT_TEST(TMURunningTimeTest, TestDCMSetTMUParam_UnitRunningTime)
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

	UINT uControllerCount = mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD;

	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_TMU", "CH0,CH4");

	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);
	timeReport.SetTimes(100);
	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.SetTMUParam("G_TMU", DCM_POS, 0, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Set %d units' in %d controllers parameter", uControllerCount * TMU_UNIT_COUNT_PER_CONTROLLER, uControllerCount);

	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.SetTMUParam("G_TMU", mapSlot.begin()->first, DCM_POS, 0, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Set %d units' parameter", TMU_UNIT_COUNT_PER_CONTROLLER);


	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.SetTMUParam("CH0", DCM_POS, 0, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Set %d units' in %d controllers parameter", uControllerCount, uControllerCount);


	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.SetTMUParam("CH0", DCM_POS, 0, 0);
	}
	timeReport.timeStop();
	timeReport.addMsg("Set %d units' parameter", 1);


	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);

	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.SetTMUParam("CH0", DCM_POS, 0, 0, DCM_TMU1);
	}
	timeReport.timeStop();
	timeReport.addMsg("Set %d units' parameter in same channel", 1);

	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
