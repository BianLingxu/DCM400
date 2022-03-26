#pragma once
/*!
* @file      TestDCMWriteWaveData_blankRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/02/04
* @version   v 1.0.0.0
* @brief     测试WriteWaveData()运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMWriteWaveData_blankRunningTime)
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

	char* lpszLabel[4] = { "FAIL_BRAM_ST" ,"FAIL_DRAM_ST" };

	BYTE bySameData[4] = { 0xAA,0xBB,0xCC,0xDD };
	BYTE byDiffData[4] = { 0xAA,0xBB,0xCC,0xDD };
	BYTE* pbyData = nullptr;

	const BYTE byRAMTypeCount = 2;
	const char* lpszRAMType[byRAMTypeCount] = { "BRAM","DRAM" };
	const char* lpszTimes[2] = { "Firstly", "Secondly" };
	const char* lpszDataType[2] = { "All same", "All different" };
	//*************************************Vector In BRAM**********************************************************//
	for (int nRAMIndex = 0; nRAMIndex < byRAMTypeCount; ++nRAMIndex)
	{
		for (int nDataCountIndex = 0; nDataCountIndex < 2; ++nDataCountIndex)
		{
			BYTE byDataCount = (nDataCountIndex + 1) * 16;
			for (int nDataTypeIndex = 0; nDataTypeIndex < 2; ++nDataTypeIndex)
			{
				dcm.SetWaveDataParam("G_ALLPIN", lpszLabel[nRAMIndex], 10, byDataCount);
				if (0 == nDataTypeIndex)
				{
					pbyData = bySameData;
				}
				else
				{
					pbyData = byDiffData;
				}
				for (int nTimeIndex = 0; nTimeIndex < 2; ++nTimeIndex)
				{
					for (auto& Slot : mapSlot)
					{
						for (int nIndex = 0; nIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++nIndex)
						{
							if (bySameData != pbyData)
							{
								BYTE byUserData = byDataCount / 8;
								for (int nDataIndex = 0; nDataIndex < byUserData; ++nDataIndex)
								{
									pbyData[nDataIndex] = nRAMIndex * 20 + nDataCountIndex * 10 + Slot.second + nIndex;
								}
							}
							dcm.SetSiteWaveData(Slot.second + nIndex, bySameData);
						}
					}
					timeReport.timeStart();
					dcm.WriteWaveData();
					timeReport.timeStop();
					timeReport.addMsg("Controllers:%d. Line Count:%d. Data Type: %s. %s. RAM Type:%s.", mapSlot.size(), byDataCount, lpszDataType[nDataTypeIndex], lpszTimes[nTimeIndex], lpszRAMType[nRAMIndex]);
				}
			}
		}
	}
	
	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
