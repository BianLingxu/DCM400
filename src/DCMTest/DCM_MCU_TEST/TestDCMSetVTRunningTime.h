#pragma once
/*!
* @file      TestDCMSetVTRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/12
* @version   v 1.0.0.0
* @brief     测试SetVT运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetVTRunningTime)
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

	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.SetPinGroup("G_ODDPIN", "CH1,CH3,CH5,CH7,CH9,CH11,CH13,CH15");
	dcm.SetPinGroup("G_EVENPIN", "CH0,CH2,CH4,CH6,CH8,CH10,CH12,CH14");
	
	int nChannelCount = mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD;
	const char* pinGroup[3] = { "G_ALLPIN" , "G_ODDPIN" , "G_EVENPIN" };
	VTMode vtMode[3] = { DCM_VT_FORCE, DCM_VT_REALTIME, DCM_VT_CLOSE };
	const char* cModeType[3] = {"Force VT", "RealTime VT", "Close VT"};
	for (int nIndex = 0; nIndex < 3;++nIndex)
	{
		for (int nPinGroupIndex = 0; nPinGroupIndex < 3;++nPinGroupIndex)
		{
			timeReport.timeStart();
			dcm.SetVT(pinGroup[nPinGroupIndex], 3.5, vtMode[nPinGroupIndex]);
			timeReport.timeStop();
			if (0 == nPinGroupIndex)
			{
				timeReport.addMsg("VT mode: %s, %d channels", cModeType[nIndex], nChannelCount);
			}
			else if(1 == nPinGroupIndex)
			{
				timeReport.addMsg("VT mode: %s, %d channels of odd pin", cModeType[nIndex], nChannelCount / 2);
			}
			else
			{
				timeReport.addMsg("VT mode: %s, %d channels of even pin", cModeType[nIndex], nChannelCount / 2);
			}
		}
	}
	dcm.SetVT("G_ALLPIN");

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
