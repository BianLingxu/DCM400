#pragma once
/*!
* @file      TestDCMI2CWriteMultiDataRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/16
* @version   v 1.0.0.0
* @brief     测试I2CWriteMultiData运行时间
* @comment
*/
#include "..\DCMTestMain.h"
void MultiWriteTimeTest(CTimeReport& timeReport, int nSiteCount, int nREGBitsCount, int nControlCount, BOOL bSaveInBRAM)
{
	BYTE pbySameData[10] = { 0xAA, 0xBB, 0xCC, 0xDD,0xAA, 0xBB, 0xCC, 0xDD,0xAA, 0xBB };
	
	BYTE pbyDiffData1[10] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB};
	BYTE pbyDiffData2[10] = { 0XCC, 0xBB, 0xC, 0xCBB, 0xCB, 0xCB, 0xCB, 0xCB, 0xBB, 0xCC};
	BYTE pbyDiffData3[10] = { 0xBB, 0xDD, 0xAA, 0xAA, 0xAD, 0xAD, 0xAD, 0xAD, 0xDA, 0xDA};
	BYTE pbyDiffData4[10] = { 0xEE, 0xBB, 0xBB, 0xEE, 0xEB, 0xEB, 0xEB, 0xEB, 0xBE, 0xBE};

	BYTE *ppbySameDataArray[4] = { pbySameData ,pbySameData ,pbySameData,pbySameData };//The data written of each site are different.
	BYTE *ppbyDiffDataArray[4] = { pbyDiffData1, pbyDiffData2, pbyDiffData3, pbyDiffData4 };//The data written of each site are same.
	
	const int nWriteArrayType = 2;
	BYTE **pppbyWriteData[nWriteArrayType] = {ppbySameDataArray, ppbyDiffDataArray};
	const char* cArrayType[nWriteArrayType] = { "All same","All diffrent" };
	const int nWriteByteTypeCount = 2;
	int nWriteDataBytes[nWriteByteTypeCount] = { 5,10 };
	
	const char * lpszType[2] = { "BRAM", "DRAM" };
	int nCurType = 0;
	if (!bSaveInBRAM)
	{
		nCurType = 1;
	}
	const int nWriteTimesTypeCount = 2;
	const char* lpszWriteTimes[nWriteTimesTypeCount] = { "First", "Second" };
	for (int nWriteArrayIndex = 0; nWriteArrayIndex < nWriteArrayType; ++nWriteArrayIndex)
	{
		for (int nReadCountIndex = 0; nReadCountIndex < nWriteByteTypeCount; ++nReadCountIndex)
		{
			for (int nIndex = 0; nIndex < nWriteTimesTypeCount; ++nIndex)
			{
				timeReport.timeStart();
				dcm.I2CWriteMultiData(2, 0, nWriteDataBytes[nReadCountIndex], pppbyWriteData[nWriteArrayIndex]);
				timeReport.timeStop();
				timeReport.addMsg("Site Count: %d, Register bits: %d, Controller Count: %d, %d bytes data, %s Write, Data %s , in %s", nSiteCount, 
					nREGBitsCount, nControlCount, nWriteDataBytes[nReadCountIndex], lpszWriteTimes[nIndex], cArrayType[nWriteArrayIndex], lpszType[nCurType]);
			}
		}
		if (1 == nSiteCount)
		{
			break;
		}
	}
}


XT_TEST(FunctionRunningTimeTest, TestDCMI2CWriteMultiDataRunningTime)
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
	dcm_CloseFile();

	I2C_REG_ADDR_MODE REGMode = DCM_REG8;
	int nREGBits = 8;
	for (int nREGType = DCM_REG8; nREGType <= DCM_REG32; ++nREGType)
	{
		REGMode = (I2C_REG_ADDR_MODE)nREGType;
		nREGBits = (nREGType + 1) * 8;
		//************************************One Site(BRAM)**********************************************//
		GetI2CChannel(mapSlot, 1, 1, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 1, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		MultiWriteTimeTest(timeReport, 1, nREGBits, 1, TRUE);

		//************************************QuadSites in one controller(BRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		MultiWriteTimeTest(timeReport, 4, nREGBits, 1, TRUE);

		//************************************QuadSites in four controllers(BRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		MultiWriteTimeTest(timeReport, 4, nREGBits, 4, TRUE);
	}

	UINT uStartLine = 0;
	UINT uStopLine = 0;
	UINT uLineBeforeDRAM = 0;
	UINT uDRAMStartLine = 0;
	UINT uDRAMLineCount = 0;
	BOOL bWithDRAM = TRUE;
	int nIndex = 0;
	for (int nREGType = DCM_REG8; nREGType <= DCM_REG32; ++nREGType)
	{
		REGMode = (I2C_REG_ADDR_MODE)nREGType;
		nREGBits = (nREGType + 1) * 8;
		//************************************One Site(DRAM)**********************************************//
		dcm.LoadVectorFile(g_lpszVectorFilePath);//Load the vector, in order to save i2c pattern to DRAM
		GetI2CChannel(mapSlot, 1, 1, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 1, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		do
		{
			dcm.I2CReadData(0x00, nIndex++, 1);
			dcm_GetLatestI2CMemory(uStartLine, uStopLine, bWithDRAM, uLineBeforeDRAM, uDRAMStartLine, uDRAMLineCount);
		} while (!bWithDRAM);

		MultiWriteTimeTest(timeReport, 1, nREGBits, 1, FALSE);
		//************************************Quad Sites in one controller(DRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		do
		{
			dcm.I2CReadData(0x00, nIndex++, 1);
			dcm_GetLatestI2CMemory(uStartLine, uStopLine, bWithDRAM, uLineBeforeDRAM, uDRAMStartLine, uDRAMLineCount);
		} while (!bWithDRAM);

		MultiWriteTimeTest(timeReport, 4, nREGBits, 1, FALSE);

		//************************************Quad Sites in four controllers(DRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		do
		{
			dcm.I2CReadData(0x00, nIndex++, 1);
			dcm_GetLatestI2CMemory(uStartLine, uStopLine, bWithDRAM, uLineBeforeDRAM, uDRAMStartLine, uDRAMLineCount);
		} while (!bWithDRAM);

		MultiWriteTimeTest(timeReport, 4, nREGBits, 4, FALSE);
	}

	timeReport.Print(this, g_lpszReportFilePath);

	mapSlot.clear();
	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}
