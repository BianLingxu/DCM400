#pragma once
/**
 * @file TestDCMSetPinLevelRunningTime.h
 * @brief Test the function running time of SetChannelStatus
 * @author Guangyun Wang
 * @date 20020/08/23
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetPinLevelRunningTime)
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

	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.SetPinGroup("G_EVENPIN", "CH0,CH2,CH4,CH6,CH8,CH10,CH12,CH14");

	timeReport.timeStart();
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);
	timeReport.timeStop();
	timeReport.addMsg("Set the pin level of %d channels in %d controllers", mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD, mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD);


	timeReport.timeStart();
	dcm.SetPinLevel("G_EVENPIN", 3, 0, 1.5, 0.8);
	timeReport.timeStop();
	timeReport.addMsg("Set the pin level of %d channels in %d controllers", mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD / 2, mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD / 2);

	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
