#pragma once
/**
 * @file TestDCMSetFailSavingTypeFunction.h
 * @brief Check the function function of SetFailSavingType
 * @author Guangyun Wang
 * @date 2021/04/16
 * @copyrigh AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetFailSavingTypeFunction)
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
	char lpszPin[32] = { 0 };
	string strPinList;
	for (USHORT usPin = 0; usPin < DCM_CHANNELS_PER_CONTROL; ++usPin)
	{
		sprintf_s(lpszPin, sizeof(lpszPin), "CH%d", usPin);
		strPinList += lpszPin;
		strPinList += ",";
	}
	//Load vector.
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		funcReport.AddTestItem("Load vector");
		funcReport.SaveAddtionMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
		mapSlot.clear();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	///<Check whether the board support fail saving selected

	funcReport.AddTestItem("Support Checking");
	vector<BYTE> vecBoardExclude;
	for (auto& Slot : mapSlot)
	{
		ULONG ulRev = dcm_read_FPGA_Version(Slot.first, 0);
		if (0x120 > ulRev)
		{
			vecBoardExclude.push_back(Slot.first);
		}
	}
	for (auto Slot : vecBoardExclude)
	{
		auto iterSlot = mapSlot.find(Slot);
		if (mapSlot.end() != iterSlot)
		{
			mapSlot.erase(iterSlot);
		}
	}
	if (0 == mapSlot.size())
	{
		funcReport.SaveAddtionMsg("No board supported");
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}


	dcm.SetPinGroup("G_ALLPIN", strPinList.c_str());

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 2, 0.8);
	funcReport.AddTestItem("Saving All Fail");
	const int nMutualTestGroupChannelCount = 4;//The count of channel in each mutual test group.

	const BYTE byLoopFail = 2;
	const int nFailStep = 104;//The line count between two fail section.
	const int nFailSectionCount = 8;//The count of fail section.
	const int nFirstFailSectionStart = 2;//The start line of first fail section.
	const int nFailCountPerSectionPerPin = 2;//The fail count of the channel per section.
	const int nFailCountPerPin = nFailCountPerSectionPerPin * nFailSectionCount + byLoopFail;//The sum number of the fail line per channel.
	const ULONG ulExpectFailLineNoPerGroup[nMutualTestGroupChannelCount][nFailCountPerSectionPerPin] = { { 0, 4 }, { 1, 5 }, { 3, 5 }, { 2, 4 } };


	ULONG** ulExpectFailLineNo = new ULONG * [nMutualTestGroupChannelCount];//The expect fail line number.
	for (int nIndex = 0; nIndex < nMutualTestGroupChannelCount; ++nIndex)
	{
		ulExpectFailLineNo[nIndex] = new ULONG[nFailSectionCount * nFailCountPerSectionPerPin + byLoopFail];
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


	const int nMutualPinCount = 4;
	const int nMaxFailCount = 22;
	ULONG ulPinFailLineNo[nMaxFailCount] = { 0 };
	ULONG ulFailCount = 0;
	ULONG ulExpectFailCount = nMaxFailCount;
	dcm.SetFailSavingType("G_ALLPIN", DCM_SAVING_ALL_FAIL);
	double dSTBR = 10;//The time of STBR
	const double dAddStep = 5;
	char lpszPinName[32] = { 0 };//The pin name.
	ULONG ulFailLineNo[100] = { 0 };
	for (dSTBR = 30; dSTBR < 101; dSTBR += dAddStep)
	{
		dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, dSTBR);

		funcReport.AddClkSetting(10, 110, 10, 110, dSTBR, 110);

		dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");
		//dcm.SaveFailMap(0);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
			{
				sprintf_s(lpszPinName, 5, "CH%d", usChannel);
				for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
				{
					nRetVal = dcm.GetFailCount(lpszPinName, Slot.second + usSiteIndex, ulFailCount);
					XT_EXPECT_EQ(nRetVal, 0);
					XT_EXPECT_EQ((int)ulFailCount, nFailCountPerPin);

					if (0 != nRetVal || ulFailCount != nFailCountPerPin)
					{
						//Fail count is not equal to the expected.
						funcReport.SaveFailChannel(Slot.first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannel);
						continue;
					}
					nRetVal = dcm.GetFailLineNo(lpszPinName, Slot.second + usSiteIndex, nFailCountPerPin, ulFailLineNo);

					XT_EXPECT_EQ(nRetVal, 0);
					if (0 != nRetVal || 0 != memcmp(ulExpectFailLineNo[usChannel % 4], ulFailLineNo, nFailCountPerPin * sizeof(ULONG)))
					{
						XT_EXPECT_TRUE(FALSE);
						funcReport.SaveFailChannel(Slot.first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannel);
					}
				}
			}
		}
	}


	funcReport.AddTestItem("Saving Selected Fail");
	
	ULONG ulExpectSelectFailCount[nMutualPinCount] = { 9,7,7,8 };
	const int nMaxSelectFailCount = 9;
	ULONG ulExpectSelectFailLineNo[nMutualPinCount][nMaxSelectFailCount] = { {106,314,318,418,422,422,422,730,734},{315,319,419,423,423,423,731,-1,-1},
		{317,319,421,423,423,423,733,-1,-1},{316,318,420,422,422,422,732,734,-1} };

	dcm.SetFailSavingType("G_ALLPIN", DCM_SAVING_SELECT_LINE_FAIL);
	ulExpectFailCount = 20;
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");
	//dcm.SaveFailMap(0);
	USHORT usSiteNo = 0;
	for (auto& Slot : mapSlot)
	{
		usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex, ++usSiteNo)
		{
			for (USHORT usPin = 0; usPin < DCM_CHANNELS_PER_CONTROL; ++usPin)
			{
				sprintf_s(lpszPin, sizeof(lpszPin), "CH%d", usPin);
				dcm.GetFailCount(lpszPin, usSiteNo, ulFailCount);
				XT_EXPECT_EQ((int)ulFailCount, nFailCountPerPin);
				if (ulFailCount != nFailCountPerPin)
				{
					funcReport.SaveFailChannel(Slot.first, usPin + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
					continue;
				}
				dcm.GetFailLineNo(lpszPin, usSiteNo, nMaxSelectFailCount, ulPinFailLineNo);
				if (0 == memcmp(ulPinFailLineNo, ulExpectSelectFailLineNo[usPin % 4], nMaxSelectFailCount * sizeof(ULONG)))
				{
					continue;
				}
				funcReport.SaveFailChannel(Slot.first, usPin + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
				for (int nFailIndex = 0; nFailIndex < nMaxSelectFailCount; ++nFailIndex)
				{
					XT_EXPECT_EQ(ulExpectSelectFailLineNo[usPin % 4][nFailIndex], ulPinFailLineNo[nFailIndex]);
				}
			}
		}
	}

	dcm.SetFailSavingType("G_ALLPIN", DCM_SAVING_ALL_FAIL);
	dcm.Disconnect("G_ALLPIN");
	funcReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file
}