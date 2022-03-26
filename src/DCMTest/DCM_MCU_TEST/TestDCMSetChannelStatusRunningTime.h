/**
 * @file TestDCMSetChannelStatusRunningTime.h
 * @brief Test the function running time of SetChannelStatus
 * @author Guangyun Wang
 * @date 20020/08/19
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#pragma once
#include "..\DCMTestMain.h"

XT_TEST(FunctionRunningTimeTest, TestDCMSetChannelStatusRunningTime)
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

	int nChannelCount = mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD;
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.SetPinGroup("G_ODDPIN", "CH1,CH3,CH5,CH7,CH9,CH11,CH13,CH15");
	dcm.SetPinGroup("G_EVENPIN", "CH0,CH2,CH4,CH6,CH8,CH10,CH12,CH14");

	timeReport.timeStart();
	dcm.SetChannelStatus("G_ALLPIN", DCM_ALLSITE, DCM_HIGH);
	timeReport.timeStop();
	timeReport.addMsg("%d channels output high", nChannelCount);

	timeReport.timeStart();
	dcm.SetChannelStatus("G_ODDPIN", DCM_ALLSITE, DCM_HIGH);
	timeReport.timeStop();
	timeReport.addMsg("%d channels output high", nChannelCount / 2);


	timeReport.timeStart();
	dcm.SetChannelStatus("G_ALLPIN", DCM_ALLSITE, DCM_LOW);
	timeReport.timeStop();
	timeReport.addMsg("%d channels output low", nChannelCount);


	timeReport.timeStart();
	dcm.SetChannelStatus("G_ODDPIN", DCM_ALLSITE, DCM_LOW);
	timeReport.timeStop();
	timeReport.addMsg("%d channels output low", nChannelCount / 2);

	timeReport.timeStart();
	dcm.SetChannelStatus("G_ALLPIN", DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
	timeReport.timeStop();
	timeReport.addMsg("%d channels output high impedance", nChannelCount);


	timeReport.timeStart();
	dcm.SetChannelStatus("G_ODDPIN", DCM_ALLSITE, DCM_LOW);
	timeReport.timeStop();
	timeReport.addMsg("%d channels output high impedance", nChannelCount / 2);

	timeReport.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}