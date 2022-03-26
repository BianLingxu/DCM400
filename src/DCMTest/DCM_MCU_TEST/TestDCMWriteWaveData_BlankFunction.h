#pragma once
/*!
* @file      TestDCMWriteWaveData_BlankFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/01/06
* @version   v 1.0.0.0
* @brief     测试WriteWaveData_Blank功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMWriteWaveData_BlankFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");

	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		funcReport.SetNoBoardValid();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(funcReport, mapSlot);
	auto iterSlot = mapSlot.begin();

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	iterSlot = mapSlot.begin();
	double dPeriod = dcm_GetTimeSetPeriod(iterSlot->first, 0, 0);

	//Set the time in order to ensure the mutual-test is work well.
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 3 / 4, dPeriod / 8, dPeriod * 5 / 8);
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.7);

	funcReport.AddTestItem("Pin Mutual Test");
	funcReport.AddClkSetting(dPeriod / 8, dPeriod * 3 / 4, dPeriod / 8, dPeriod * 3 / 4, dPeriod / 2, dPeriod * 3 / 4);

	char lpszPinName[16] = { 0 };

	dcm.SetPinGroup("G_EVENCHIP", "CH0,CH1,CH4,CH5,CH8,CH9,CH12,CH13");
	dcm.SetPinGroup("G_ODDCHIP", "CH2,CH3,CH6,CH7,CH10,CH11,CH14,CH15");


	const int nLineCount = 32;
	BYTE bySiteWaveData[DCM_MAX_CONTROLLERS_PRE_BOARD][nLineCount / 8] = { { 0xAA, 0x55,0xFF,0x00 }, {0x55,0xAA,0x00,0xFF}, {0x5A,0xA5,0xF0,0x0F}, {0xA5,0x5A, 0x0F,0xF0} };

	dcm.SetWaveDataParam("G_EVENCHIP", "TEST_WRITE_ST", 1, nLineCount);
	iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
		{
			dcm.SetSiteWaveData(iterSlot->second + usSiteIndex, bySiteWaveData[usSiteIndex]);
		}
		++iterSlot;
	}
	dcm.WriteWaveData();


	dcm.SetWaveDataParam("G_ODDCHIP", "TEST_WRITE_ST", 33, nLineCount);
	iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
		{
			dcm.SetSiteWaveData(iterSlot->second + usSiteIndex, bySiteWaveData[usSiteIndex]);
		}
		++iterSlot;
	}
	dcm.WriteWaveData();

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_WRITE_ST", "TEST_WRITE_SP");

	//dcm.SaveFailMap(0);

	int nFailCount[DCM_MAX_CONTROLLERS_PRE_BOARD] = { 0 };
	ULONG ulEvenFailLineNo[DCM_MAX_CONTROLLERS_PRE_BOARD][nLineCount] = { 0 };
	ULONG ulOddFailLineNo[DCM_MAX_CONTROLLERS_PRE_BOARD][nLineCount] = { 0 };

 	int nCurBit = 0;
	for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
	{
		nFailCount[usSiteIndex] = GetExpectedFail(bySiteWaveData[usSiteIndex], nLineCount, 1, ulEvenFailLineNo[usSiteIndex]);
		for (int nFailIndex = 0; nFailIndex < nFailCount[usSiteIndex]; nFailIndex++)
		{
			ulOddFailLineNo[usSiteIndex][nFailIndex] = ulEvenFailLineNo[usSiteIndex][nFailIndex] + 32;
		}
	}

 	ULONG ulFailCount = 0;
 	ULONG ulFailLineNo[100] = { 0 };

	iterSlot = mapSlot.begin();
	USHORT uCurSiteID = 0;
	while (mapSlot.end() != iterSlot)
	{
		for (int nChannel = 0; nChannel < DCM_CHANNELS_PER_CONTROL; ++nChannel)
		{
			sts_sprintf(lpszPinName, 16, "CH%d", nChannel);

			for (int nSiteIndex = 0; nSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++nSiteIndex)
			{
				uCurSiteID = iterSlot->second + nSiteIndex;
				dcm.GetFailCount(lpszPinName, uCurSiteID, ulFailCount);

				XT_EXPECT_EQ((int)ulFailCount, nFailCount[nSiteIndex]);

				if (nFailCount[nSiteIndex] != ulFailCount)
				{
					funcReport.SaveFailChannel(iterSlot->first, uCurSiteID * DCM_CHANNELS_PER_CONTROL + nChannel);
					continue;
				}
				dcm.GetFailLineNo(lpszPinName, uCurSiteID, ulFailCount, ulFailLineNo);
				if (0 == (nChannel / 2) % 2)
				{
					if (0 != memcmp(ulFailLineNo, ulEvenFailLineNo[nSiteIndex], ulFailCount * sizeof(ULONG)))
					{
						for (int nIndex = 0; nIndex < ulFailCount; ++nIndex)
						{
							XT_EXPECT_EQ(ulFailLineNo[nIndex], ulEvenFailLineNo[nSiteIndex][nIndex]);
						}
						funcReport.SaveFailChannel(iterSlot->first, nSiteIndex * DCM_CHANNELS_PER_CONTROL + nChannel);
					}
				}
				else
				{
					if (0 != memcmp(ulFailLineNo, ulOddFailLineNo[nSiteIndex], ulFailCount * sizeof(ULONG)))
					{
						for (int nIndex = 0; nIndex < ulFailCount; ++nIndex)
						{
							XT_EXPECT_EQ(ulFailLineNo[nIndex], ulOddFailLineNo[nSiteIndex][nIndex]);
						}
						funcReport.SaveFailChannel(iterSlot->first, nSiteIndex * DCM_CHANNELS_PER_CONTROL + nChannel);
					}
				}
			}
		}
		++iterSlot;
	}

	funcReport.AddTestItem("Data different between channel and site");

	BYTE abyData[DCM_CHANNELS_PER_CONTROL][MAX_SITE][2] = { 0 };
	ULONG aulExpectedFailLine[DCM_CHANNELS_PER_CONTROL][MAX_SITE][16] = { 0 };
	int anExpectedFailCount[DCM_CHANNELS_PER_CONTROL][MAX_SITE] = { 0 };
	BYTE* pbyData = nullptr;
	ULONG* pulFailLine = nullptr;
	int* pnFailCount = nullptr;
	int nLineOffset = 0;
	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex, ++usSiteNo)
		{
			for (USHORT usPin = 0; usPin < DCM_CHANNELS_PER_CONTROL; ++usPin)
			{
				pbyData = abyData[usPin][usSiteNo];
				pulFailLine = aulExpectedFailLine[usPin][usSiteNo];
				ULONG ulData = 0xAABB + usSiteNo * DCM_CHANNELS_PER_CONTROL + usPin;
				pbyData[0] = ulData >> 16 & 0xFF;
				pbyData[1] = ulData & 0xFF;
				pnFailCount = &anExpectedFailCount[usPin][usSiteNo];

				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usPin);
				if (0 == usPin % 4 / 2)
				{
					nLineOffset = 1;
				}
				else
				{
					nLineOffset = 17;
				}

				dcm.SetWaveDataParam(lpszPinName, "TEST_RAM_ST", nLineOffset, 16);
				dcm.SetSiteWaveData(usSiteNo, pbyData);
				dcm.WriteWaveData();

				int nFailIndex = 0;
				for (int nLineIndex = 0; nLineIndex < 16; ++nLineIndex)
				{
					nCurBit = (pbyData[nLineIndex / 8] >> (7 - nLineIndex % 8) & 0x01);
					if (0 == nLineIndex)
					{
						if (0 != nCurBit)
						{
							///<The first line is '1'
							continue;
						}
					}
					else if (0 == nCurBit)
					{
						continue;
					}
					pulFailLine[nFailIndex++] = nLineIndex + nLineOffset;
					++* pnFailCount;
				}
			}
		}
	}

	ULONG ulCurFailCount = 0;

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_RAM_ST", "TEST_RAM_SP");
	//dcm.SaveFailMap(0);
	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex, ++usSiteNo)
		{
			for (int usPin = 0; usPin < DCM_CHANNELS_PER_CONTROL; ++usPin)
			{
				sts_sprintf(lpszPinName, 16, "CH%d", usPin);
				pulFailLine = aulExpectedFailLine[usPin][usSiteNo];
				pnFailCount = &anExpectedFailCount[usPin][usSiteNo];
				dcm.GetFailCount(lpszPinName, usSiteNo, ulCurFailCount);

				XT_EXPECT_EQ((int)ulCurFailCount, *pnFailCount);

				if (*pnFailCount != ulCurFailCount)
				{
					funcReport.SaveFailChannel(Slot.first, byControllerIndex* DCM_CHANNELS_PER_CONTROL + usPin);
					continue;
				}
				dcm.GetFailLineNo(lpszPinName, usSiteNo, ulCurFailCount, ulFailLineNo);
				if (0 != memcmp(ulFailLineNo, pulFailLine, ulCurFailCount * sizeof(ULONG)))
				{
					for (int nIndex = 0; nIndex < ulCurFailCount; ++nIndex)
					{
						XT_EXPECT_EQ(ulFailLineNo[nIndex], pulFailLine[nIndex]);
						break;
					}

					funcReport.SaveFailChannel(Slot.first, byControllerIndex * DCM_CHANNELS_PER_CONTROL + usPin);
				}
			}
		}
	}

	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);

	funcReport.AddTestItem("Check Invalid Site_SetWaveDataParam");
	nRetVal = dcm.SetWaveDataParam("G_ODDCHIP", "TEST_WRITE_ST", 33, nLineCount); 
	
	funcReport.AddTestItem("Check Invalid Site_SetSiteWaveData");
	nRetVal = dcm.SetSiteWaveData(usInvalidSite, bySiteWaveData[0]);
	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}

	funcReport.AddTestItem("Check Invalid Site_WriteWaveData");
	nRetVal = dcm.WriteWaveData();

	RestoreSite();

	mapSlot.clear();

	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}