#pragma once
/*!
* @file      TestDCMGetFailLineNoFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/30
* @version   v 1.0.0.0
* @brief     测试GetFailLineNo功能
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(FunctionFunctionTest, TestDCMGetFailLineNoFunction)
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

	//Load vector.
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		funcReport.AddTestItem("Load vector");
		funcReport.SaveAddtionMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(iterSlot->first, usChannel);
			}
			++iterSlot;
		}
		mapSlot.clear();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	const int nMutualTestGroupChannelCount = 4;//The count of channel in each mutual test group.

	const BYTE byLoopFail = 2;
	const int nFailStep = 104;//The line count between two fail section.
	const int nFailSectionCount = 8;//The count of fail section.
	const int nFirstFailSectionStart = 2;//The start line of first fail section.
	const int nFailCountPerSectionPerPin = 2;//The fail count of the channel per section.
	const int nFailCountPerPin = nFailCountPerSectionPerPin*nFailSectionCount + byLoopFail;//The sum number of the fail line per channel.
	const ULONG ulExpectFailLineNoPerGroup[nMutualTestGroupChannelCount][nFailCountPerSectionPerPin] = { { 0, 4 }, { 1, 5 }, { 3, 5 }, { 2, 4 } };


	ULONG **ulExpectFailLineNo = new ULONG*[nMutualTestGroupChannelCount];//The expect fail line number.
	for (int nIndex = 0; nIndex < nMutualTestGroupChannelCount; ++ nIndex)
	{
		ulExpectFailLineNo[ nIndex] = new ULONG[nFailSectionCount * nFailCountPerSectionPerPin + byLoopFail];
	}
	for (int nPinNo = 0; nPinNo < nMutualTestGroupChannelCount; ++nPinNo)
	{
		int nCurFailIndex = 0;
		for (int nSectionIndex = 0; nSectionIndex < nFailSectionCount; ++nSectionIndex)
		{
			ULONG ulBaseOffset = nSectionIndex * nFailStep;
			for (int nFailIndex = 0; nFailIndex < nFailCountPerSectionPerPin; ++nFailIndex)
			{
				ulExpectFailLineNo[nPinNo][nCurFailIndex++] = ulBaseOffset + ulExpectFailLineNoPerGroup[nPinNo][nFailIndex] + nFirstFailSectionStart;
			}

			if (4 == nSectionIndex)
			{
				///<Loop cycle
				ulExpectFailLineNo[nPinNo][nCurFailIndex++] = ulBaseOffset + ulExpectFailLineNoPerGroup[nPinNo][1] + nFirstFailSectionStart;
				ulExpectFailLineNo[nPinNo][nCurFailIndex++] = ulBaseOffset + ulExpectFailLineNoPerGroup[nPinNo][1] + nFirstFailSectionStart;
			}
		}
	}

	funcReport.AddTestItem("Pin Mutual Test");

	double dSTBR = 10;//The time of STBR
	const double dAddStep = 5;
	char lpszPinName[32] = { 0 };//The pin name.
	ULONG ulFailCount = 0;
	ULONG ulFailLineNo[100] = { 0 };
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);
	for (dSTBR = 30; dSTBR < 101; dSTBR += dAddStep)
	{
		dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, dSTBR);

		funcReport.AddClkSetting(10, 110, 10, 110, dSTBR, 110);

		dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");
		//dcm.SaveFailMap(0);
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
			{
				sprintf_s(lpszPinName, 5, "CH%d", usChannel);
				for (USHORT usSiteNo = 0; usSiteNo < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteNo)
				{
					nRetVal = dcm.GetFailCount(lpszPinName, iterSlot->second + usSiteNo, ulFailCount);
					XT_EXPECT_EQ(nRetVal, 0);
					XT_EXPECT_EQ((int)ulFailCount, nFailCountPerPin);

					if (0 != nRetVal || ulFailCount != nFailCountPerPin)
					{
						//Fail count is not equal to the expected.
						funcReport.SaveFailChannel(iterSlot->first, usSiteNo * DCM_CHANNELS_PER_CONTROL + usChannel);
						continue;
					}
					nRetVal = dcm.GetFailLineNo(lpszPinName, iterSlot->second + usSiteNo, nFailCountPerPin, ulFailLineNo);

					XT_EXPECT_EQ(nRetVal, 0);
					if (0 != nRetVal || 0 != memcmp(ulExpectFailLineNo[usChannel % 4], ulFailLineNo, nFailCountPerPin * sizeof(ULONG)))
					{
						XT_EXPECT_TRUE(FALSE);
						funcReport.SaveFailChannel(iterSlot->first, usSiteNo * DCM_CHANNELS_PER_CONTROL + usChannel);
					}
				}
			}
			++iterSlot;
		}
	}

	funcReport.AddTestItem("Test Limited of Fail Memory");
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_LIMIT_ST", "TEST_LIMIT_SP");
	//dcm.SaveFailMap(0);
	ULONG* pulFailLineNo = nullptr;
	do
	{

		try
		{
			pulFailLineNo = new ULONG[1024];
		}
		catch (const std::exception&)
		{
			funcReport.SaveAddtionMsg("Allocate memory fail");
		}
		USHORT usSiteNo = 0;
		int nFailSectionCount = DRAM_MAX_SAVE_FAIL_LINE_COUNT / 6;

		const ULONG ulOffset[4][2] = { {0, 4},{1,5 },{3, 5}, {2, 4} };

		for (auto& Slot : mapSlot)
		{
			usSiteNo = Slot.second;
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex, ++usSiteNo)
			{
				for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
				{
					sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannel);
					int nFailLineSaved = nFailSectionCount * 2 + (usChannel % 4 == 2 ? 0 : 1);

					memset(pulFailLineNo, 0, 1024 * sizeof(ULONG));
					dcm.GetFailLineNo(lpszPinName, usSiteNo, 1024, pulFailLineNo);
					int nFailCount = 9998 / 10;
					dcm.GetFailCount(lpszPinName, usSiteNo, ulFailCount);

					BOOL bFail = FALSE;
					for (int nFailIndex = 0; nFailIndex < nFailLineSaved; ++nFailIndex)
					{
						ULONG ulCurFailLine = nFailIndex / 2 * 10 + ulOffset[usChannel % 4][nFailIndex % 2] + 2;
						XT_EXPECT_EQ(pulFailLineNo[nFailIndex], ulCurFailLine);
						if (ulCurFailLine != pulFailLineNo[nFailIndex])
						{
							bFail = TRUE;
							break;
						}
					}
					if (bFail)
					{
						funcReport.SaveFailChannel(Slot.first, usChannel + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
						continue;
					}
					XT_EXPECT_EQ(pulFailLineNo[nFailLineSaved], (ULONG)0xFFFFFFFF);
					if (0xFFFFFFFF != pulFailLineNo[nFailLineSaved])
					{
						funcReport.SaveFailChannel(Slot.first, usChannel + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
					}
				}
			}
		}

		if (nullptr != pulFailLineNo)
		{
			delete[] pulFailLineNo;
			pulFailLineNo = nullptr;
		}

	} while (FALSE);


	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.GetFailLineNo("CH2", usInvalidSite, sizeof(ulFailLineNo) / sizeof(DWORD), ulFailLineNo);

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}

	for(auto FailLineNo :ulFailLineNo)
	{
		XT_EXPECT_EQ(FailLineNo, (ULONG)-1);
		if ((ULONG) -1 != FailLineNo)
		{
			funcReport.SaveAddtionMsg("The fail line number(0x%08X) of the invalid site is not equal to 0x%08X.", FailLineNo, (ULONG)-1);
			break;
		}
	}
	RestoreSite();


	for (int nIndex = 0; nIndex < nMutualTestGroupChannelCount; ++ nIndex)
	{
		delete[] ulExpectFailLineNo[ nIndex];
		ulExpectFailLineNo[nIndex] = nullptr;
	}
	delete[] ulExpectFailLineNo;
	ulExpectFailLineNo = nullptr;

	mapSlot.clear();
	funcReport.Print(this, g_lpszReportFilePath);


	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}