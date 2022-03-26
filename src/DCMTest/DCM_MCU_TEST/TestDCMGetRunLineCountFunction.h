#pragma once
/**
 * @file TestDCMGetRunLineCountFunction.h
 * @brief Check the function function of GetRunLineCount
 * @author Guangyun Wang
 * @date 2021/04/23
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMGetRunLineCountFunction)
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
	
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	funcReport.AddTestItem("BRAM Vector");
	ULONG ulLineCount = 0;
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_BRAM_ST", "TEST_BRAM_SP");
	dcm.GetRunLineCount("SDA_DP", 0, ulLineCount);

	ULONG ulTargetLineCount = dcm_GetLabelLineNo("TEST_BRAM_SP", FALSE) - dcm_GetLabelLineNo("TEST_BRAM_ST", FALSE) + 1;

	char lpszPinName[32] = { 0 };
	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		USHORT usChannel = 0;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex, ++usSiteNo)
		{
			for (BYTE byPin = 0; byPin < DCM_CHANNELS_PER_CONTROL;++byPin, ++ usChannel)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", byPin);
				nRetVal = dcm.GetRunLineCount(lpszPinName, usSiteNo, ulLineCount);
				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
					continue;
				}
				XT_EXPECT_EQ(ulLineCount, ulTargetLineCount);
				if (ulLineCount != ulTargetLineCount)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}

	funcReport.AddTestItem("DRAM Vector");
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");

	ulTargetLineCount = dcm_GetLabelLineNo("TEST_FAIL_SP", FALSE) - dcm_GetLabelLineNo("TEST_FAIL_ST", FALSE) + 1;
	ulTargetLineCount += 2 * 2;///<The loop function in vector
	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		USHORT usChannel = 0;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex, ++usSiteNo)
		{
			for (BYTE byPin = 0; byPin < DCM_CHANNELS_PER_CONTROL; ++byPin, ++usChannel)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", byPin);
				nRetVal = dcm.GetRunLineCount(lpszPinName, usSiteNo, ulLineCount);
				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
					continue;
				}
				XT_EXPECT_EQ(ulLineCount, ulTargetLineCount);
				if (ulLineCount != ulTargetLineCount)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}

	funcReport.AddTestItem("Endless Vector");
	funcReport.SaveAddtionMsg("Get line count twice, the second is larger than first.");

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_ENDLESS_ST", "TEST_ENDLESS_SP", FALSE);

	ulTargetLineCount = dcm_GetLabelLineNo("TEST_ENDLESS_SP", FALSE) - dcm_GetLabelLineNo("TEST_ENDLESS_ST", FALSE) + 1;
	ULONG ulSecondLineCount = 0;
	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		USHORT usChannel = 0;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex, ++usSiteNo)
		{
			for (BYTE byPin = 0; byPin < DCM_CHANNELS_PER_CONTROL; ++byPin, ++usChannel)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", byPin);
				nRetVal = dcm.GetRunLineCount(lpszPinName, usSiteNo, ulLineCount);
				nRetVal = dcm.GetRunLineCount(lpszPinName, usSiteNo, ulSecondLineCount);
				
				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
					continue;
				}
				XT_EXPECT_GREATER(ulSecondLineCount, ulLineCount);
				if (ulLineCount >= ulSecondLineCount)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}
	dcm.StopVector("G_ALLPIN");
	mapSlot.clear();
	funcReport.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}