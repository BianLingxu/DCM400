#pragma once
/*!
* @file      TestDCMWriteWaveDataRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/16
* @version   v 1.0.0.0
* @brief     测试WriteWaveData运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMWriteWaveDataRunningTime)
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

	dcm.SetPinLevel("G_ALLPIN", 3.0, 0, 1.5, 0.8);
	double dPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 3 / 4, dPeriod / 8, dPeriod * 5 / 8);

	char *lpszLabel[4] = { "FAIL_BRAM_ST" ,"FAIL_DRAM_ST"};


	//*************************************Vector In BRAM**********************************************************//
	iterSlot = mapSlot.begin();
	//1 controllers , 16 channels ,modify 16 lines in BRAM
	timeReport.timeStart();
	nRetVal = dcm.WriteWaveData("G_ALLPIN", lpszLabel[0], iterSlot->second, 10, 16, 0xAA55FF00);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Channels:%d. Line Count:%d. Type:%s.",1, DCM_CHANNELS_PER_CONTROL, 16, "BRAM");

	//1 controllers , 16 channels ,modify 32 lines in BRAM
	timeReport.timeStart();
	nRetVal = dcm.WriteWaveData("G_ALLPIN", lpszLabel[0], iterSlot->second, 10, 32, 0xAA55FF00);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Channels:%d. Line Count:%d. Type:%s.", 1, DCM_CHANNELS_PER_CONTROL, 32, "BRAM");

	//nValidSiteCount controllers ,  nValidSiteCount * DCM_CHANNEL_COUNT_PER_CONTROLLER channels ,modify 16 lines in BRAM
	timeReport.timeStart();
	nRetVal = dcm.WriteWaveData("G_ALLPIN", lpszLabel[0], DCM_ALLSITE, 10, 16, 0xAA55FF00);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Channels:%d. Line Count:%d. Type:%s. In Parallel.", mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD, mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD, 16, "BRAM");

	//nValidSiteCount controllers ,  nValidSiteCount * DCM_CHANNEL_COUNT_PER_CONTROLLER channels ,modify 32 lines in BRAM
	timeReport.timeStart();
	nRetVal = dcm.WriteWaveData("G_ALLPIN", lpszLabel[0], DCM_ALLSITE, 10, 32, 0xAA55FF00);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Channels:%d. Line Count:%d. Type:%s. In Parallel.", mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD, mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD, 32, "BRAM");

	//*************************************Vector In DRAM**********************************************************//
	//1 controllers , 16 channels ,modify 16 lines in DRAM
	timeReport.timeStart();
	nRetVal = dcm.WriteWaveData("G_ALLPIN", lpszLabel[1], iterSlot->second, 10, 16, 0xAA55FF00);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Channels:%d. Line Count:%d. Type:%s.", 1, DCM_CHANNELS_PER_CONTROL, 16, "DRAM");

	//1 controllers , 16 channels ,modify 32 lines in DRAM
	timeReport.timeStart();
	nRetVal = dcm.WriteWaveData("G_ALLPIN", lpszLabel[1], iterSlot->second, 10, 32, 0xAA55FF00);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Channels:%d. Line Count:%d. Type:%s", 1, DCM_CHANNELS_PER_CONTROL, 32, "DRAM");

	//nValidSiteCount controllers ,  nValidSiteCount * DCM_CHANNEL_COUNT_PER_CONTROLLER channels ,modify 16 lines in DRAM
	timeReport.timeStart();
	nRetVal = dcm.WriteWaveData("G_ALLPIN", lpszLabel[1], DCM_ALLSITE, 10, 16, 0xAA55FF00);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Channels:%d. Line Count:%d. Type:%s. In Parllel.", mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD, mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD, 16, "DRAM");

	//nValidSiteCount controllers ,  nValidSiteCount * DCM_CHANNEL_COUNT_PER_CONTROLLER channels ,modify 32 lines in DRAM
	timeReport.timeStart();
	nRetVal = dcm.WriteWaveData("G_ALLPIN", lpszLabel[1], DCM_ALLSITE, 10, 32, 0xAA55FF00);
	timeReport.timeStop();
	timeReport.addMsg("Controllers:%d. Channels:%d. Line Count:%d. Type:%s. In Parllel.", mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD, mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD, 32, "DRAM");

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
