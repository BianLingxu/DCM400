#pragma once
/*!
* @file      TestDCMI2CWriteDataRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/16
* @version   v 1.0.0.0
* @brief     测试I2CWriteData运行时间
* @comment
*/
#include <fstream>
using namespace std;
#include "..\DCMTestMain.h"
void WriteTimeTest(CTimeReport& timeReport, int nSiteCount, int nREGBitsCount, int nControlCount, BOOL bSaveInBRAM)
{
	ULONG ulWriteDataSame1[4] = { 0xAABBCCDD,0xAABBCCDD,0xAABBCCDD,0xAABBCCDD };
	ULONG ulWriteData[4] = { 0xAABBCCDD,0xBBABABAB,0xCCABABAB,0xDDBAABAA };

	const int nWriteArrayType = 2;
	ULONG* ulWriteDataType[2] = { ulWriteDataSame1,ulWriteData};
	const char* lpszArrayType[nWriteArrayType] = {"All same", "All different"};
	const int nWriteByteTypeCount = 2;
	int nWriteDataBytes[nWriteByteTypeCount] = { 1,4 };

	const char * cType[2] = { "BRAM", "DRAM" };
	int nCurType = 0;
	if (!bSaveInBRAM)
	{
		nCurType = 1;
	}
	const int nReadTimesTypeCount = 2;
	const char* lpszWriteTimes[nReadTimesTypeCount] = { "First", "Second" };
	for (int nWriteArrayIndex = 0; nWriteArrayIndex < nWriteArrayType; ++nWriteArrayIndex)
	{
		for (int nWriteCountIndex = 0; nWriteCountIndex < nWriteByteTypeCount; ++nWriteCountIndex)
		{
			for (int nIndex = 0; nIndex < nReadTimesTypeCount; ++nIndex)
			{
				timeReport.timeStart();
				dcm.I2CWriteData(2, 0, nWriteDataBytes[nWriteCountIndex], ulWriteDataType[nWriteArrayIndex]);
				timeReport.timeStop();
				timeReport.addMsg("Site Count: %d, Register Bits: %d, Controller Count: %d, %d bytes data, %s Write, Data %s , in %s", nSiteCount, nREGBitsCount, nControlCount, nWriteDataBytes[nWriteCountIndex], lpszWriteTimes[nIndex], lpszArrayType[nWriteArrayIndex], cType[nCurType]);
			}
		}
		if (1 == nSiteCount)
		{
			break;
		}
	}
}


XT_TEST(FunctionRunningTimeTest, TestDCMI2CWriteDataRunningTime)
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

	ULONG ulWriteDataSame[4] = { 0xAABBCCDD,0xAABBCCDD,0xAABBCCDD,0xAABBCCDD };
	ULONG ulWriteData[4] = { 0xAABBCCDD,0xCCAABBDD,0xBBCCDDEE,0xAACCBBAA };

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

		WriteTimeTest(timeReport, 1, nREGBits, 1, TRUE);
		
		//************************************QuadSites in one controller(BRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		WriteTimeTest(timeReport, 4, nREGBits, 1, TRUE);

		//************************************QuadSites in four controllers(BRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		WriteTimeTest(timeReport, 4, nREGBits, 4, TRUE);
	}

	BOOL bWithDRAM = TRUE;
	int nIndex = 0;
	UINT uStartLine = 0;
	UINT uStopLine = 0;
	UINT uLineBeforeDRAM = 0;
	UINT uDRAMStartLine = 0;
	UINT uDRAMLineCount = 0;

	for (int nREGType = DCM_REG8; nREGType <= DCM_REG32; ++nREGType)
	{
		REGMode = (I2C_REG_ADDR_MODE)nREGType;
		nREGBits = (nREGType + 1) * 8;

		//************************************One Site(DRAM)**********************************************//
		dcm.LoadVectorFile(g_lpszVectorFilePath);//Load the vecotor, in order to save i2c pattern to DRAM
		GetI2CChannel(mapSlot, 1, 1, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 1, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		BOOL bWithDRAM = TRUE;
		int nIndex = 0;
		do
		{
			dcm.I2CReadData(0xA0, nIndex++, 1);
			dcm_GetLatestI2CMemory(uStartLine, uStopLine, bWithDRAM, uLineBeforeDRAM, uDRAMStartLine, uDRAMLineCount);
			if (bWithDRAM)
			{
				DWORD dwData[4] = { 0 };
				for (USHORT usSiteNo = 0; usSiteNo < 4; ++usSiteNo)
				{
					dwData[usSiteNo] = nIndex;
				}
				dcm.I2CWriteData(0x00, 0x00, 1, dwData);
				dcm_GetLatestI2CMemory(uStartLine, uStopLine, bWithDRAM, uLineBeforeDRAM, uDRAMStartLine, uDRAMLineCount);
			}
			uStartLine = uStartLine;
		} while (!bWithDRAM);

		WriteTimeTest(timeReport, 1, nREGBits, 1, FALSE);

		//************************************Quad Sites in one controller(DRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
		do
		{
			dcm.I2CReadData(0xA0, nIndex++, 1);
			dcm_GetLatestI2CMemory(uStartLine, uStopLine, bWithDRAM, uLineBeforeDRAM, uDRAMStartLine, uDRAMLineCount);
			if (bWithDRAM)
			{
				DWORD dwData[4] = { 0 };
				for (USHORT usSiteNo = 0; usSiteNo < 4; ++usSiteNo)
				{
					dwData[usSiteNo] = nIndex;
				}
				dcm.I2CWriteData(0x00, 0x00, 1, dwData);
				dcm_GetLatestI2CMemory(uStartLine, uStopLine, bWithDRAM, uLineBeforeDRAM, uDRAMStartLine, uDRAMLineCount);
			}
			uStartLine = uStartLine;
		} while (!bWithDRAM);

		WriteTimeTest(timeReport, 4, nREGBits, 1, FALSE);

		//************************************Quad Sites in four controllers(DRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		do
		{
			dcm.I2CReadData(0xA0, nIndex++, 1);
			dcm_GetLatestI2CMemory(uStartLine, uStopLine, bWithDRAM, uLineBeforeDRAM, uDRAMStartLine, uDRAMLineCount);
			if (bWithDRAM)
			{
				DWORD dwData[4] = { 0 };
				for (USHORT usSiteNo = 0; usSiteNo < 4; ++usSiteNo)
				{
					dwData[usSiteNo] = nIndex;
				}
				dcm.I2CWriteData(0x00, 0x00, 1, dwData);
				dcm_GetLatestI2CMemory(uStartLine, uStopLine, bWithDRAM, uLineBeforeDRAM, uDRAMStartLine, uDRAMLineCount);
			}
			uStartLine = uStartLine;
		} while (!bWithDRAM);

		WriteTimeTest(timeReport, 4, nREGBits, 4, FALSE);
	}

	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}
