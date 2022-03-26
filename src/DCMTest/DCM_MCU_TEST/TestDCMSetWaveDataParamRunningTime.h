#pragma once
/*!
* @file      TestDCMSetWaveDataParamRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/02/04
* @version   v 1.0.0.0
* @brief     测试SetWaveDataParam运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetWaveDataParamRunningTime)
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

	char* lpszLabel[4] = { "FAIL_BRAM_ST" ,"FAIL_DRAM_ST" };

	//*************************************Vector In BRAM**********************************************************//
	iterSlot = mapSlot.begin();
	//1 controllers , 16 channels ,modify 16 lines in BRAM
	timeReport.timeStart();
	dcm.SetWaveDataParam("G_ALLPIN", lpszLabel[0], 0, 16);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Line Count:%d. Firstly. Type:%s.", 1, 16, "BRAM");

	iterSlot = mapSlot.begin();
	//1 controllers , 16 channels ,modify 16 lines in BRAM
	timeReport.timeStart();
	dcm.SetWaveDataParam("G_ALLPIN", lpszLabel[0], 0, 16);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Line Count:%d. Secondly. Type:%s.", 1, 16, "BRAM");

	//1 controllers ,modify 32 lines in BRAM
	timeReport.timeStart();
	dcm.SetWaveDataParam("G_ALLPIN", lpszLabel[0], 0, 32);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Line Count:%d. Firstly. Type:%s.", 1, 32, "BRAM");

	timeReport.timeStart();
	dcm.SetWaveDataParam("G_ALLPIN", lpszLabel[0], 0, 32);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Line Count:%d. Secondly. Type:%s.", 1, 32, "BRAM");

	//*************************************Vector In DRAM**********************************************************//
	//1 controllers , 16 channels ,modify 16 lines in DRAM
	timeReport.timeStart();
	dcm.SetWaveDataParam("G_ALLPIN", lpszLabel[1], 10, 16);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Line Count:%d. Firstly. Type:%s.", 1, 16, "DRAM");
	timeReport.timeStart();
	dcm.SetWaveDataParam("G_ALLPIN", lpszLabel[0], 10, 16);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Line Count:%d. Secondly. Type:%s.", 1, 16, "DRAM");

	//1 controllers , 16 channels ,modify 32 lines in DRAM
	timeReport.timeStart();
	dcm.SetWaveDataParam("G_ALLPIN", lpszLabel[1], 10, 32);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Line Count:%d. Firstly. Type:%s", 1, 32, "DRAM");

	//1 controllers , 16 channels ,modify 32 lines in DRAM
	timeReport.timeStart();
	dcm.SetWaveDataParam("G_ALLPIN", lpszLabel[1], 10, 32);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Line Count:%d. Secondly. Type:%s", 1, 32, "DRAM");

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
