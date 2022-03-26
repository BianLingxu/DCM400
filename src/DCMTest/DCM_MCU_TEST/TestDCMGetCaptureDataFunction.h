#pragma once
/*!
* @file      TestDCMGetCaptureDataFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/12
* @version   v 1.0.0.0
* @brief     测试GetCaptureData功能
* @comment
*/
#include "..\DCMTestMain.h"

void CheckCaptureData(CFuncReport& funcReport, XTest * xtTest, map<BYTE,USHORT>& mapSlot, const char* lpszStartLabel, const char* lpszStopLabel, int nOffset, const ULONG* ulExpectData)
{
	if (0 == mapSlot.size())
	{
		return;
	}

	double dPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 5 / 8, dPeriod / 8, dPeriod * 3 / 8);

	dcm.RunVectorWithGroup("G_ALLPIN", lpszStartLabel, lpszStopLabel);
//	dcm.SaveFailMap(0);
	char lpszPinName[8] = { 0 };
	ULONG ulCaptureData = 0;
	for (auto& Slot : mapSlot)
	{
		for (int nChannel = 0; nChannel < DCM_CHANNELS_PER_CONTROL; ++nChannel)
		{
			sts_sprintf(lpszPinName, 8, "CH%d", nChannel);
			for (int nSiteIndex = 0; nSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++nSiteIndex)
			{
				dcm.GetCaptureData(lpszPinName, lpszStartLabel, Slot.second + nSiteIndex, nOffset, 16, ulCaptureData);
				if (0 == nChannel % 4 || 1 == nChannel % 4)
				{
					xtTest->Expect(ulExpectData[0], ulCaptureData, xtTest->EQ, __FILE__, __LINE__);
					if (ulExpectData[0] != ulCaptureData)
					{
						funcReport.SaveFailChannel(Slot.first, nSiteIndex * DCM_CHANNELS_PER_CONTROL + nChannel);
					}
				}
				else
				{
					xtTest->Expect(ulExpectData[1], ulCaptureData, xtTest->EQ, __FILE__, __LINE__);
					if (ulExpectData[1] != ulCaptureData)
					{
						funcReport.SaveFailChannel(Slot.first, nSiteIndex * DCM_CHANNELS_PER_CONTROL + nChannel);
					}
				}
			}
		}
	}	
}

XT_TEST(FunctionFunctionTest, TestDCMGetCaptureDataFunction)
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
	CheckCaptureData(funcReport, this, mapSlot, "TEST_BRAM_ST", "TEST_BRAM_SP", 2, ulBRAMCaptureData);

	funcReport.AddTestItem("Get capture data in DRAM");
	CheckCaptureData(funcReport, this, mapSlot, "TEST_DRAM_ST", "TEST_DRAM_SP", 2, ulDRAMCaptureData);

	funcReport.AddTestItem("Check Capture include End Label");
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_BRAM_ST", "TEST_BRAM_SP");
	ULONG ulStartLineNo = dcm_GetLabelLineNo("TEST_BRAM_ST",FALSE);
	ULONG ulStopLineNo = dcm_GetLabelLineNo("TEST_BRAM_SP", FALSE);
	ULONG ulOffset = ulStopLineNo - ulStartLineNo - 16 + 1;
	ULONG ulCapture = 0;
	nRetVal = dcm.GetCaptureData("CH0", "TEST_BRAM_ST", mapSlot.begin()->second, ulOffset, 16, ulCapture);
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		funcReport.SaveFailChannel(mapSlot.begin()->first, 0);
	}

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_LIMIT_ST", "TEST_LIMIT_SP");
	//dcm.SaveFailMap(0);
	ULONG ulJustEnoughCapture[4] = {0x04, 0x02, 0x00, 0x01};
	char lpszPinName[32] = { 0 };
	ULONG ulCaptureData = 0;
	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; byControllerIndex++, ++usSiteNo)
		{
			for (USHORT usChannel = 0; usChannel <DCM_CHANNELS_PER_CONTROL;++usChannel)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannel);

				funcReport.AddTestItem("Check Fail Memory Limit_JUST_ENOUGH");
				nRetVal = dcm.GetCaptureData(lpszPinName, "TEST_LIMIT_ST", usSiteNo, 1700, 5, ulCaptureData);
				///<The fail line number just enough to getting capture
				XT_EXPECT_EQ(nRetVal, EXECUTE_SUCCESS);
				if (0 != nRetVal)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
				}
				else
				{
					XT_EXPECT_EQ(ulCaptureData, ulJustEnoughCapture[usChannel % 4]);
					if (ulCaptureData != ulJustEnoughCapture[usChannel % 4])
					{
						funcReport.SaveFailChannel(Slot.first, usChannel + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
					}
				}

				funcReport.AddTestItem("Check Fail Memory Limit_NO_ENOUGH");
				nRetVal = dcm.GetCaptureData(lpszPinName, "TEST_LIMIT_ST", usSiteNo, 1700, 32, ulCaptureData);
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
	
	nRetVal = dcm.GetCaptureData("CH0", "TEST_EXTREME_ST", mapSlot.begin()->second, 1000, 32, ulCaptureData);
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
	int nTargetReturn = 0;
	nRetVal = dcm.RunVectorWithGroup("G_ALLPIN", "TEST_BRAM_ST", "TEST_BRAM_SP");
	dcm.SetFailSavingType("CH0", DCM_SAVING_ALL_FAIL);
	for (auto& Slot : mapSlot)
	{
		USHORT usCurSite = 0;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			usCurSite = Slot.second + byControllerIndex;

			nRetVal = dcm.GetCaptureData("CH0","TEST_BRAM_ST", usCurSite, 0, 16, ulCaptureData);
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


	funcReport.AddTestItem("Check Invalid Site");
	strcpy_s(lpszPinName, sizeof(lpszPinName), "CH2");
	usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.GetCaptureData(lpszPinName, "TEST_DRAM_ST", usInvalidSite, 0, 2, ulCaptureData);

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