#pragma once
/**
 * @file TestDCMSetCaptureFunction.h
 * @brief Check the function function of SetCapture
 * @author Guangyun Wang
 * @date 2021/04/21
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"

void CheckSetCaptureData(CFuncReport& funcReport, XTest* xtTest, map<BYTE, USHORT>& mapSlot, const char* lpszStartLabel, const char* lpszStopLabel, int nOffset, const ULONG* ulExpectData)
{
	auto iterSlot = mapSlot.begin();
	if (mapSlot.end() == iterSlot)
	{
		return;
	}

	double dPeriod = dcm_GetTimeSetPeriod(iterSlot->first, 0, 0);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 5 / 8, dPeriod / 8, dPeriod * 3 / 8);

	dcm.RunVectorWithGroup("G_ALLPIN", lpszStartLabel, lpszStopLabel);
	//	dcm.SaveFailMap(0);
	char lpszPinName[8] = { 0 };
	ULONG ulCaptureData = 0;
	USHORT usSiteNo = 0;
	dcm.SetCapture("G_ALLPIN", lpszStartLabel, DCM_ALLSITE, nOffset, 16);
	for (auto& Slot : mapSlot)
	{
		for (int nChannel = 0; nChannel < DCM_CHANNELS_PER_CONTROL; ++nChannel)
		{
			usSiteNo = Slot.second;
			sts_sprintf(lpszPinName, 8, "CH%d", nChannel);
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex, ++usSiteNo)
			{
				dcm.GetCaptureData(lpszPinName, usSiteNo, ulCaptureData);
				if (0 == nChannel % 4 || 1 == nChannel % 4)
				{
					xtTest->Expect(ulExpectData[0], ulCaptureData, xtTest->EQ, __FILE__, __LINE__);
					if (ulExpectData[0] != ulCaptureData)
					{
						funcReport.SaveFailChannel(Slot.first, byControllerIndex * DCM_CHANNELS_PER_CONTROL + nChannel);
					}
				}
				else
				{
					xtTest->Expect(ulExpectData[1], ulCaptureData, xtTest->EQ, __FILE__, __LINE__);
					if (ulExpectData[1] != ulCaptureData)
					{
						funcReport.SaveFailChannel(Slot.first, byControllerIndex * DCM_CHANNELS_PER_CONTROL + nChannel);
					}
				}
			}
		}
	}
}

XT_TEST(FunctionFunctionTest, TestDCMSetCaptureFunction)
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

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	const ULONG ulBRAMCaptureData[2] = { 0x4408, 0x2204 };
	const ULONG ulDRAMCaptureData[2] = { 0x8880, 0x4440 };

	funcReport.AddTestItem("Get capture data in BRAM");
	CheckSetCaptureData(funcReport, this, mapSlot, "TEST_BRAM_ST", "TEST_BRAM_SP", 2, ulBRAMCaptureData);

	funcReport.AddTestItem("Get capture data in DRAM");
	CheckSetCaptureData(funcReport, this, mapSlot, "TEST_DRAM_ST", "TEST_DRAM_SP", 2, ulDRAMCaptureData);

	funcReport.AddTestItem("Check Capture include End Label");
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_BRAM_ST", "TEST_BRAM_SP");
	ULONG ulStartLineNo = dcm_GetLabelLineNo("TEST_BRAM_ST", FALSE);
	ULONG ulStopLineNo = dcm_GetLabelLineNo("TEST_BRAM_SP", FALSE);
	ULONG ulOffset = ulStopLineNo - ulStartLineNo - 16 + 1;
	ULONG ulCapture = 0;
	nRetVal = dcm.SetCapture("CH0", "TEST_BRAM_ST", mapSlot.begin()->second, ulOffset, 16);
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		funcReport.SaveFailChannel(mapSlot.begin()->first, 0);
	}


	funcReport.AddTestItem("Check Fail Memory Limit");
	funcReport.SaveAddtionMsg("The capture line is over the fail line number saved.");
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_LIMIT_ST", "TEST_LIMIT_SP");
	dcm.SetCapture("G_ALLPIN", "TEST_LIMIT_ST", DCM_ALLSITE, 1700, 30);
	char lpszPinName[32] = { 0 };
	ULONG ulCaptureData = 0;
	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; byControllerIndex++, ++usSiteNo)
		{
			for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannel);

				nRetVal = dcm.GetCaptureData(lpszPinName, usSiteNo, ulCaptureData);
				XT_EXPECT_EQ(nRetVal, FAIL_LINE_NOT_SAVE);
				if (FAIL_LINE_NOT_SAVE != nRetVal)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
					continue;
				}
				XT_EXPECT_EQ(ulCaptureData, (ULONG)0xFFFFFFFF);
				if (0xFFFFFFFF != ulCaptureData)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
				}
			}
		}
	}


	funcReport.AddTestItem("Check All Fail Line Number Occupied");
	nRetVal = dcm.RunVectorWithGroup("G_ALLPIN", "TEST_EXTREME_ST", "TEST_EXTREME_SP");
	dcm.SetCapture("CH0", "TEST_EXTREME_ST", DCM_ALLSITE, 1000, 32);
	nRetVal = dcm.GetCaptureData("CH0",  mapSlot.begin()->second, ulCaptureData);
	XT_EXPECT_EQ(nRetVal, FAIL_LINE_NOT_SAVE);
	if (FAIL_LINE_NOT_SAVE != nRetVal)
	{
		funcReport.SaveAddtionMsg("No fail line number saved for the fail line memory are accupied by other channels.");
		funcReport.SaveAddtionMsg("The return value(%d) is not equal to FAIL_LINE_NOT_SAVE(%d)", nRetVal, FAIL_LINE_NOT_SAVE);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}


	funcReport.AddTestItem("Check Fail Saving Type");
	funcReport.SaveAddtionMsg("In this mode, GetCaptureData not be supported");
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	dcm.SetFailSavingType("CH0", DCM_SAVING_SELECT_LINE_FAIL);
	RestoreSite();
	nRetVal = dcm.RunVectorWithGroup("G_ALLPIN", "TEST_BRAM_ST", "TEST_BRAM_SP");
	dcm.SetFailSavingType("CH0", DCM_SAVING_ALL_FAIL);
	dcm.SetCapture("CH0", "TEST_BRAM_ST", DCM_ALLSITE, 0, 16);
	int nTargetReturn = 0;
	for (auto Slot : mapSlot)
	{
		USHORT usCurSite = 0;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			usCurSite = Slot.second + byControllerIndex;

			nRetVal = dcm.GetCaptureData("CH0", usCurSite, ulCaptureData);
			if (usCurSite == usInvalidSite)
			{
				nTargetReturn = 0;
			}
			else
			{
				nTargetReturn = CAPTURE_NOT_SUPPORTTED;
			}
			XT_EXPECT_EQ(nRetVal, nTargetReturn);
			if (nRetVal != nTargetReturn)
			{
				funcReport.SaveFailChannel(Slot.first, usCurSite * byControllerIndex);
			}
		}
	}

	funcReport.AddTestItem("Check Invalid Site_SetCapture");
	strcpy_s(lpszPinName, sizeof(lpszPinName), "CH2");
	usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.SetCapture(lpszPinName, "TEST_DRAM_ST", usInvalidSite, 0, 2);

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	

	funcReport.AddTestItem("Check Invalid Site_GetCaptureData");
	nRetVal = dcm.SetCapture(lpszPinName, "TEST_DRAM_ST", DCM_ALLSITE, 0, 2);
	nRetVal = dcm.GetCaptureData(lpszPinName, usInvalidSite, ulCaptureData);
	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	XT_EXPECT_EQ(ulCaptureData, (ULONG)-1);
	if ((ULONG)-1 != ulCaptureData)
	{
		funcReport.SaveAddtionMsg("The capture data(0x%08X) of the invalid site is not equal to 0x%08X.", ulCaptureData, (ULONG)-1);
	}
	RestoreSite();

	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}