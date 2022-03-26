#pragma once
/*!
* @file      TestDCMI2CGetBitDataRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/17
* @version   v 1.0.0.0
* @brief     测试I2CGetBitData运行时间
* @comment
*/
#include "..\DCMTestMain.h"

void GetBitDataTimeTest(CTimeReport& timeReport, int nSiteCount, int nControllerCount, BOOL bSaveInBRAM)
{
	const int nReadTypeCount = 2;
	int nReadDataBytes[nReadTypeCount] = { 1,4 };
	const char* lpszRAMType[2] = { "BRAM", "DRAM" };
	int nCurType = 0;
	if (!bSaveInBRAM)
	{
		nCurType = 1;
	}
	for (int nReadDataIndex = 0; nReadDataIndex < nReadTypeCount; ++nReadDataIndex)
	{
		dcm.I2CReadData(2, 0, nReadDataBytes[nReadDataIndex]);
		USHORT usSiteIndex = 0;
		//for (USHORT usSiteIndex = 0; usSiteIndex < nSiteCount; ++usSiteIndex)
		{
			int nBitsCount = nReadDataBytes[nReadDataIndex] * 8;
			for (int nStartPos = 0; nStartPos < nBitsCount; nStartPos += 4)
			{
				int nReadBitsCount = nBitsCount - nStartPos;
				for (int nReadCount = 1;nReadCount <= nReadBitsCount;nReadCount += 4)
				{
					timeReport.timeStart();
					dcm.I2CGetBitData(usSiteIndex, nStartPos, nReadCount);
					timeReport.timeStop();
					timeReport.addMsg("Site Count: %d, Controller Count: %d, %d bytes data, in %s\nSite Index: %d, Start position: %d, Bits count: %d",
						nSiteCount, nControllerCount, nReadDataBytes[nReadDataIndex], lpszRAMType[nCurType], usSiteIndex, nStartPos, nReadCount);
				}
			}
		}
	}
}


XT_TEST(FunctionRunningTimeTest, TestDCMI2CGetBitDataRunningTime)
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

	string strSCLChannel;
	string strSDAChannel;

	//************************************One Site(BRAM)**********************************************//
	GetI2CChannel(mapSlot, 1, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 1, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	GetBitDataTimeTest(timeReport, 1, 1, TRUE);
	//************************************QuadSites in one controller(BRAM)**********************************************//
	dcm_I2CDeleteMemory();
	GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	GetBitDataTimeTest(timeReport, 4, 1, TRUE);

	//************************************QuadSites in four controllers(BRAM)**********************************************//
	dcm_I2CDeleteMemory();
	GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	GetBitDataTimeTest(timeReport, 4, 4, TRUE);

	//************************************One Site(DRAM)**********************************************//
	dcm.LoadVectorFile(g_lpszVectorFilePath);//Load the vector, in order to save i2c pattern to DRAM
	GetI2CChannel(mapSlot, 1, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 1, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	for (int nIndex = 1; nIndex < 50; ++nIndex)
	{
		dcm.I2CReadData(0xA0, nIndex, 1);
	}
	GetBitDataTimeTest(timeReport, 1, 1, FALSE);

	//************************************Quad Sites in one controller(DRAM)**********************************************//
	dcm_I2CDeleteMemory();
	GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	for (int nIndex = 1; nIndex < 50; ++nIndex)
	{
		dcm.I2CReadData(0xA0, nIndex, 1);
	}
	GetBitDataTimeTest(timeReport, 4, 1, FALSE);

	//************************************Quad Sites in four controllers(DRAM)**********************************************//
	dcm_I2CDeleteMemory();
	GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	for (int nIndex = 1; nIndex < 50; ++nIndex)
	{
		dcm.I2CReadData(0xA0, nIndex, 1);
	}
	GetBitDataTimeTest(timeReport, 4, 4, FALSE);


	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}
