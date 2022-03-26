#pragma once
/**
 * @file TestDCMI2CWriteData_SameRunningTime.h
 * @brief Check the running time of I2CWriteData
 * @author Guangyun Wang
 * @date 2021/08/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
void WriteSameDataTimeTest(CTimeReport& timeReport, USHORT usSiteCount, int nREGBitsCount, int nControlCount, BOOL bSaveInBRAM)
{
	BYTE abySameData[10] = { 0xAA, 0xBB, 0xCC, 0xDD,0xAA, 0xBB, 0xCC, 0xDD,0xAA, 0xBB };

	const int nWriteByteTypeCount = 2;
	int nWriteDataBytes[nWriteByteTypeCount] = { 5,10 };

	const char* lpszType[2] = { "BRAM", "DRAM" };
	int nCurType = 0;
	if (!bSaveInBRAM)
	{
		nCurType = 1;
	}
	const int nWriteTimesTypeCount = 2;
	const char* lpszWriteTimes[nWriteTimesTypeCount] = { "First", "Second" };
	for (int nReadCountIndex = 0; nReadCountIndex < nWriteByteTypeCount; ++nReadCountIndex)
	{
		for (int nIndex = 0; nIndex < nWriteTimesTypeCount; ++nIndex)
		{
			timeReport.timeStart();
			dcm.I2CWriteData(2, 0, nWriteDataBytes[nReadCountIndex], abySameData);
			timeReport.timeStop();
			timeReport.addMsg("Site Count: %d, Register bits: %d, Controller Count: %d, %d bytes data, %s Write, in %s", usSiteCount,
				nREGBitsCount, nControlCount, nWriteDataBytes[nReadCountIndex], lpszWriteTimes[nIndex], lpszType[nCurType]);
		}
	}
}


XT_TEST(FunctionRunningTimeTest, TestDCMI2CWriteData_SameRunningTime)
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

		WriteSameDataTimeTest(timeReport, 1, nREGBits, 1, TRUE);

		//************************************QuadSites in one controller(BRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		WriteSameDataTimeTest(timeReport, 4, nREGBits, 1, TRUE);

		//************************************QuadSites in four controllers(BRAM)**********************************************//
		dcm_I2CDeleteMemory();
		GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
		dcm.I2CSet(200, 4, REGMode, strSCLChannel.c_str(), strSDAChannel.c_str());
		dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);

		WriteSameDataTimeTest(timeReport, 4, nREGBits, 4, TRUE);
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

		WriteSameDataTimeTest(timeReport, 1, nREGBits, 1, FALSE);
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

		WriteSameDataTimeTest(timeReport, 4, nREGBits, 1, FALSE);

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

		WriteSameDataTimeTest(timeReport, 4, nREGBits, 4, FALSE);
	}

	timeReport.Print(this, g_lpszReportFilePath);

	mapSlot.clear();
	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}