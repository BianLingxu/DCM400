#pragma once
/*!
* @file      TestDCMWriteWaveDataFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/13
* @version   v 1.0.0.0
* @brief     测试WriteWaveData功能
* @comment
*/
#include "..\DCMTestMain.h"

template <typename T>
int GetExpectedFail(const T WriteData, int nWaveLineCount, int  nLineOffset, ULONG* pulFailLineNo)
{
	if (nullptr == pulFailLineNo)
	{
		return -1;
	}
	BOOL bPoint = FALSE;
	BYTE* pbyData = nullptr;
	DWORD dwWaveData = 0;
	if (typeid(BYTE*) == typeid(WriteData))
	{
		pbyData = (BYTE*)WriteData;
		bPoint = TRUE;
	}
	else
	{
		dwWaveData = (DWORD)WriteData;
	}
	int nFailIndex = 0;
	int nCurBit = 0;
	for (int nIndex = 0; nIndex < nWaveLineCount; ++nIndex)
	{
		if (bPoint)
		{
			nCurBit = (pbyData[nIndex / 8] >> (7 - nIndex % 8)) & 0x01;
		}
		else
		{
			nCurBit = dwWaveData >> (nWaveLineCount - nIndex - 1) & 0x01;
		}
		if (0 == nIndex)
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
		pulFailLineNo[nFailIndex] = nIndex + nLineOffset;
		++nFailIndex;
	}
	return nFailIndex;
}

XT_TEST(FunctionFunctionTest, TestDCMWriteWaveDataFunction)
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
	//auto iterSlot = mapSlot.begin();

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	double dPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);

	//Set the time in order to ensure the mutual-test is work well.
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 3 / 4, dPeriod / 8, dPeriod * 5 / 8);
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.7);

	funcReport.AddTestItem("Pin Mutual Test");
	funcReport.AddClkSetting(dPeriod / 8, dPeriod * 3 / 4, dPeriod / 8, dPeriod * 3 / 4, dPeriod / 2, dPeriod * 3 / 4);

	char lpszPinName[16] = { 0 };

	dcm.SetPinGroup("G_EVENCHIP", "CH0,CH1,CH4,CH5,CH8,CH9,CH12,CH13");
	dcm.SetPinGroup("G_ODDCHIP", "CH2,CH3,CH6,CH7,CH10,CH11,CH14,CH15");

	DWORD dwWaveData = 0xAA55FF00;
	const int nWaveLineCount = 32;
	dcm.WriteWaveData("G_EVENCHIP", "TEST_WRITE_ST", DCM_ALLSITE, 1, nWaveLineCount, dwWaveData);
	dcm.WriteWaveData("G_ODDCHIP", "TEST_WRITE_ST", DCM_ALLSITE, 33, nWaveLineCount, dwWaveData);
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_WRITE_ST", "TEST_WRITE_SP");
	//dcm.SaveFailMap(0);

	int nFailCount = 0 ;
	ULONG ulEvenFailLineNo[nWaveLineCount] = { 0 };
	ULONG ulOddFailLineNo[nWaveLineCount] = { 0 };

	nFailCount = GetExpectedFail(dwWaveData, nWaveLineCount, 1, ulEvenFailLineNo);
	for (int nFailIndex = 0; nFailIndex < nFailCount; nFailIndex++)
	{
		ulOddFailLineNo[nFailIndex] = ulEvenFailLineNo[nFailIndex] + 32;
	}
	   
	ULONG ulFailCount = 0;
	ULONG ulFailLineNo[100] = { 0 };
	USHORT uCurSiteID = 0;
	BOOL bAdditionMsg = FALSE;
	for (auto& Slot : mapSlot)
	{
		for (int nChannel = 0; nChannel < DCM_CHANNELS_PER_CONTROL; ++nChannel)
		{
			sts_sprintf(lpszPinName, 16, "CH%d", nChannel);

			for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
			{
				uCurSiteID = Slot.second + usSiteIndex;
				dcm.GetFailCount(lpszPinName, uCurSiteID, ulFailCount);

				XT_EXPECT_EQ((int)ulFailCount, nFailCount);

				if (nFailCount != ulFailCount)
				{
					if (!bAdditionMsg)
					{
						funcReport.SaveAddtionMsg("The fail count is not right.");
						bAdditionMsg = TRUE;
					}
					funcReport.SaveFailChannel(Slot.first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + nChannel);
					continue;
				}
				dcm.GetFailLineNo(lpszPinName, uCurSiteID, ulFailCount, ulFailLineNo);
				if (0 == (nChannel / 2) % 2)
				{
					if (0 != memcmp(ulFailLineNo, ulEvenFailLineNo, ulFailCount * sizeof(ULONG)))
					{
						for (int nIndex = 0; nIndex < ulFailCount; ++nIndex)
						{
							XT_EXPECT_EQ(ulFailLineNo[nIndex], ulEvenFailLineNo[nIndex]);
						}

						funcReport.SaveFailChannel(Slot.first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + nChannel);
					}
				}
				else
				{
					if (0 != memcmp(ulFailLineNo, ulOddFailLineNo, ulFailCount * sizeof(ULONG)))
					{
						for (int nIndex = 0; nIndex < ulFailCount; ++nIndex)
						{
							XT_EXPECT_EQ(ulFailLineNo[nIndex], ulOddFailLineNo[nIndex]);
						}

						funcReport.SaveFailChannel(Slot.first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + nChannel);
					}
				}
			}
		}
	}
	dcm.SetPinGroup("G_FIRST", "CH0,CH1,CH4,CH5,CH8,CH9,CH12,CH13");
	dcm.SetPinGroup("G_SECOND", "CH2,CH3,CH6,CH7,CH10,CH11,CH14,CH15");
	funcReport.AddTestItem("Data different between channel and site");

	ULONG aulData[DCM_CHANNELS_PER_CONTROL][MAX_SITE] = { 0 };
	ULONG aulExpectedFailLine[DCM_CHANNELS_PER_CONTROL][MAX_SITE][16] = { 0 };
	int anExpectedFailCount[DCM_CHANNELS_PER_CONTROL][MAX_SITE] = { 0 };
	ULONG* pulData = nullptr;
	ULONG* pulFailLine = nullptr;
	int* pnFailCount = nullptr;
	
	int nLineOffset = 0;
	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex, ++usSiteNo)
		{
			for (USHORT usPin = 0; usPin < DCM_CHANNELS_PER_CONTROL; ++usPin)
			{
				pulData = &aulData[usPin][usSiteNo];
				pulFailLine = aulExpectedFailLine[usPin][usSiteNo];
				*pulData = 0xAABB +usSiteNo * DCM_CHANNELS_PER_CONTROL + usPin;
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

				dcm.WriteWaveData(lpszPinName, "TEST_RAM_ST", usSiteNo, nLineOffset, 16, *pulData);
				*pnFailCount = GetExpectedFail(*pulData, 16, nLineOffset, pulFailLine);
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
					}

					funcReport.SaveFailChannel(Slot.first, byControllerIndex* DCM_CHANNELS_PER_CONTROL + usPin);
				}
			}
		}
	}


	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.WriteWaveData("CH2", "TEST_SRAM_ST", usInvalidSite, 1, 16, 0xFFFF);

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	RestoreSite();

	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}