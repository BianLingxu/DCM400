#pragma once
/**
 * @file TestDCMSetInstructionFunction.h
 * @brief Check the parameter validity of SetInstruction
 * @author Guangyun Wang
 * @date 2020/11/27
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetInstructionFunction)
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
	
	funcReport.AddTestItem("Check number instruction");

	char lpszTargetInstruction[16] = "REPEAT";
	char lpszActualInstruction[16] = { 0 };
	int nTargetOperand = 10;
	char lpszOperand[16] = { 0 };
	_itoa_s(nTargetOperand, lpszOperand, sizeof(lpszOperand), 10);
	int nLineOffset = 1;
	const char* lpszLabel = "TEST_INS_ST";
	dcm.SetInstruction("G_ALLPIN", lpszLabel, nLineOffset, lpszTargetInstruction, lpszOperand);

	int nLineNumber = dcm_GetLabelLineNo(lpszLabel, TRUE) + nLineOffset;

	BOOL bFirstFail = TRUE;

	for (auto& Board : mapSlot)
	{
		for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD;++byController)
		{
			BOOL bTestFail = FALSE;
			nRetVal = dcm_GetInstruction(Board.first, byController, nLineNumber, lpszActualInstruction, sizeof(lpszActualInstruction));
			if (0 != strcmp(lpszActualInstruction, lpszTargetInstruction))
			{
				XT_EXPECT_TRUE(FALSE);
				if (bFirstFail)
				{
					funcReport.SaveAddtionMsg("The actual instruction(%s) is not equal to set(%s)", lpszActualInstruction, lpszTargetInstruction);
					bFirstFail = FALSE;
				}
				bTestFail = TRUE;
			}
			nRetVal = dcm_GetOperand(Board.first, byController, nLineNumber);
			XT_EXPECT_EQ(nRetVal, nTargetOperand);
			if (nRetVal != nTargetOperand)
			{
				if (bFirstFail)
				{
					funcReport.SaveAddtionMsg("The actual operand(%d) is not equal to set(%d)", nTargetOperand, nRetVal);
					bFirstFail = FALSE;
				}
				bTestFail = TRUE;
			}
			if (bTestFail)
			{
				USHORT usChannelOffset = byController * DCM_CHANNELS_PER_CONTROL;
				for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
				{
					funcReport.SaveFailChannel(Board.first, usChannel + usChannelOffset);
				}
			}
		}
	}



	funcReport.AddTestItem("Check label instruction");

	strcpy_s(lpszTargetInstruction, sizeof(lpszTargetInstruction), "JUMP");
	strcpy_s(lpszOperand, sizeof(lpszOperand), "TEST_INS_SP");
	dcm.SetInstruction("G_ALLPIN", lpszLabel, nLineOffset, lpszTargetInstruction, lpszOperand);
	nTargetOperand = dcm_GetLabelLineNo(lpszOperand);
	for (auto& Board : mapSlot)
	{
		for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byController)
		{
			BOOL bTestFail = FALSE;
			nRetVal = dcm_GetInstruction(Board.first, byController, nLineNumber, lpszActualInstruction, sizeof(lpszActualInstruction));
			if (0 != strcmp(lpszActualInstruction, lpszTargetInstruction))
			{
				XT_EXPECT_TRUE(FALSE);
				if (bFirstFail)
				{
					funcReport.SaveAddtionMsg("The actual instruction(%s) is not equal to set(%s)", lpszActualInstruction, lpszTargetInstruction);
					bFirstFail = FALSE;
				}
				bTestFail = TRUE;
			}
			nRetVal = dcm_GetOperand(Board.first, byController, nLineNumber);
			XT_EXPECT_EQ(nRetVal, nTargetOperand);
			if (nRetVal != nTargetOperand)
			{
				if (bFirstFail)
				{
					funcReport.SaveAddtionMsg("The actual operand(%d) is not equal to set(%d)", nTargetOperand, nRetVal);
					bFirstFail = FALSE;
				}
				bTestFail = TRUE;
			}
			if (bTestFail)
			{
				USHORT usChannelOffset = byController * DCM_CHANNELS_PER_CONTROL;
				for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
				{
					funcReport.SaveFailChannel(Board.first, usChannel + usChannelOffset);
				}
			}
		}
	}

	dcm.SetInstruction("G_ALLPIN", lpszLabel, nLineOffset, "INC", "0");

	funcReport.AddTestItem("Check Invalid Site");
	const USHORT usInvalidSite = mapSlot.begin()->second;

	const char* lpszPinName = "CH5";
	const char* lpszInvalidSiteIns = "REPEAT";
	const char* lpszValidSiteIns = "SET_LOOPC";

	const int nInvalidSiteValue = 10;
	const int nValidSiteValue = 100;
	char lpszSetNum[32] = { 0 };
	_itoa_s(nInvalidSiteValue, lpszSetNum, sizeof(lpszSetNum), 10);
	dcm.SetInstruction("G_ALLPIN", lpszLabel, nLineOffset, lpszInvalidSiteIns, lpszSetNum);

	InvalidSite(usInvalidSite);
	_itoa_s(nValidSiteValue, lpszSetNum, sizeof(lpszSetNum), 10);
	dcm.SetInstruction("G_ALLPIN", lpszLabel, nLineOffset, lpszValidSiteIns, lpszSetNum);

	USHORT usChannel = 0;

	int nTargetNumber = 0;

	char lpszIns[32] = { 0 };
	for (auto& Slot : mapSlot)
	{
		USHORT usBaseSiteNo = Slot.second;
		usChannel = 0;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			USHORT usSiteNo = usBaseSiteNo + byControllerIndex;
			if (usInvalidSite == usSiteNo)
			{
				nTargetNumber = nInvalidSiteValue;
				strcpy_s(lpszTargetInstruction, sizeof(lpszTargetInstruction), lpszInvalidSiteIns);
			}
			else
			{
				nTargetNumber = nValidSiteValue;
				strcpy_s(lpszTargetInstruction, sizeof(lpszTargetInstruction), lpszValidSiteIns);
			}
			dcm_GetInstruction(Slot.first, byControllerIndex, nLineNumber, lpszIns, sizeof(lpszIns));

			XT_EXPECT_EQ(lpszIns, lpszTargetInstruction);
			if (0 != strcmp(lpszTargetInstruction, lpszIns))
			{
				for (USHORT usChanelIndex = 0; usChanelIndex < DCM_CHANNELS_PER_CONTROL; ++usChanelIndex, ++usChannel)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
				continue;
			}

			int nOperand = dcm_GetOperand(Slot.first, byControllerIndex, nLineNumber);
			XT_EXPECT_EQ(nOperand, nTargetNumber);
			if (nTargetNumber != nOperand)
			{
				for (USHORT usChanelIndex = 0; usChanelIndex < DCM_CHANNELS_PER_CONTROL; ++usChanelIndex, ++usChannel)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}
	RestoreSite();
	///<Restore
	dcm.SetInstruction("G_ALLPIN", lpszLabel, nLineOffset, "INC", "0");


	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}