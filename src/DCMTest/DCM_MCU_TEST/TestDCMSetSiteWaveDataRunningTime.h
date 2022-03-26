#pragma once
/*!
* @file      TestDCMSetSiteWaveDataRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/02/04
* @version   v 1.0.0.0
* @brief     测试SetSiteWaveData运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetSiteWaveDataRunningTime)
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

	BYTE bySiteWaveData[4] = { 0xAA,0xBB,0x55,0xAB };
	//*************************************Vector In BRAM**********************************************************//
	iterSlot = mapSlot.begin();
	//1 controllers , 16 channels ,modify 16 lines in BRAM
	int nDataCount = 0;
	for (int nTestIndex = 0; nTestIndex < 2; ++nTestIndex)
	{
		nDataCount = (nTestIndex + 1) * 16;

		dcm.SetWaveDataParam("G_ALLPIN", "FAIL_BRAM_ST", 0, nDataCount);
		timeReport.timeStart();
		dcm.SetSiteWaveData(iterSlot->second, bySiteWaveData);
		timeReport.timeStop();
		timeReport.addMsg("Site Count:%d. Line Count:%d.", 1, nDataCount);

		timeReport.timeStart();
		for (USHORT usSiteIndex = 0; usSiteIndex < 2; ++usSiteIndex)
		{
			dcm.SetSiteWaveData(iterSlot->second + usSiteIndex, bySiteWaveData);
		}
		timeReport.timeStop();
		timeReport.addMsg("Site Count:%d. Line Count:%d.", 2, nDataCount);


		timeReport.timeStart();
		for (USHORT usSiteIndex = 0; usSiteIndex < 4; ++usSiteIndex)
		{
			dcm.SetSiteWaveData(iterSlot->second + usSiteIndex, bySiteWaveData);
		}
		timeReport.timeStop();
		timeReport.addMsg("Site Count:%d. Line Count:%d.", 4, nDataCount);
	}

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
