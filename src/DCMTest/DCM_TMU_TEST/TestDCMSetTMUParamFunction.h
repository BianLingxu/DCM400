#pragma once
/**
 * @file TestDCMSetTMUParamFunction.h
 * @brief Test the function of function SetTMUParam
 * @author Guangyun Wang
 * @date 2020/09/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(TMUFunctionTest, TestDCMSetTMUParamFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	int nRetVal = 0;
	CFuncReport Report(strFuncName.c_str(), "PMUFunctionTest");//Error message.

	map<BYTE, USHORT> mapSlot;

	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		Report.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(Report, mapSlot);

	///<Load vector
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		Report.AddTestItem("Load vector");
		Report.SaveAddtionMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		Report.Print(this, g_lpszReportFilePath);
		return;
	}
	dcm.SetPinGroup("G_TMU", "CH0,CH4");
	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);

	const USHORT usTMUChannelCount = 2;
	USHORT usTMUChannel[usTMUChannelCount] = { 0,4 };
	

	USHORT usCurHoldOffTime = 0;
	USHORT usCurHolfOffNum = 0;
	BOOL bTriggerEdge = TRUE;

	Report.AddTestItem("Check trigger edge");
	for (int nIndex = 0; nIndex < 2; ++nIndex)
	{
		DCM_SLOPE Slope = 0 == nIndex ? DCM_NEG : DCM_POS;
		dcm.SetTMUParam("G_TMU", DCM_ALLSITE, Slope, 0, 0);
		BOOL bTriggerEdge = TRUE;
		for (auto& Slot : mapSlot)
		{
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
			{
				for (USHORT usTMUChannelIndex = 0; usTMUChannelIndex < 2; ++usTMUChannelIndex)
				{
					USHORT usChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usTMUChannel[usTMUChannelIndex];
					dcm_ReadTMUParam(Slot.first, usChannel, bTriggerEdge, usCurHoldOffTime, usCurHolfOffNum);
					XT_EXPECT_TRUE((DCM_POS == Slope) == bTriggerEdge);
					if ((DCM_POS == Slope) != bTriggerEdge)
					{
						Report.SaveFailChannel(Slot.first, usChannel);
						continue;
					}


					///<Check the CTMU class
					dcm_GetTMUParameter(Slot.first, usChannel, bTriggerEdge, usCurHoldOffTime, usCurHolfOffNum);
					XT_EXPECT_TRUE((DCM_POS == Slope) == bTriggerEdge);
					if ((DCM_POS == Slope) != bTriggerEdge)
					{
						Report.SaveAddtionMsg("The parameter gotten from class CTMU is error.");
						Report.SaveFailChannel(Slot.first, usChannel);
						continue;
					}
				}
			}
		}
	}

	Report.AddTestItem("Hold off time");

	const BYTE byTestHoldOffTimeCount = 3;
	UINT uTestHoldOffTime[byTestHoldOffTimeCount] = { 0, 10,508 };

	for (auto uHoldHoldOffTime : uTestHoldOffTime)
	{
		dcm.SetTMUParam("G_TMU", DCM_ALLSITE, DCM_POS, uHoldHoldOffTime, 0);
		for (auto& Slot : mapSlot)
		{
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
			{
				for (USHORT usTMUChannelIndex = 0; usTMUChannelIndex < 2; ++usTMUChannelIndex)
				{
					USHORT usChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usTMUChannel[usTMUChannelIndex];
					dcm_ReadTMUParam(Slot.first, usChannel, bTriggerEdge, usCurHoldOffTime, usCurHolfOffNum);
					XT_EXPECT_LESS(abs((int)uHoldHoldOffTime - (int)usCurHoldOffTime), 8);
					if (8 < abs((int)uHoldHoldOffTime - (int)usCurHoldOffTime))
					{
						Report.SaveFailChannel(Slot.first, usChannel);
						continue;
					}

					///<Check the CTMU class
					dcm_GetTMUParameter(Slot.first, usChannel, bTriggerEdge, usCurHoldOffTime, usCurHolfOffNum);
					XT_EXPECT_LESS(abs((int)uHoldHoldOffTime - (int)usCurHoldOffTime), 8);
					if (8 < abs((int)uHoldHoldOffTime - (int)usCurHoldOffTime))
					{
						Report.SaveAddtionMsg("The parameter gotten from class CTMU is error.");
						Report.SaveFailChannel(Slot.first, usChannel);
						continue;
					}
				}
			}
		}
	}

	Report.AddTestItem("Hold off Number");
	const BYTE byTestHoldOffNumCount = 3;
	UINT uTestHoldOffNum[byTestHoldOffNumCount] = { 0, 10,2046 };
	for (auto uHoldOffNum : uTestHoldOffNum)
	{
		dcm.SetTMUParam("G_TMU", DCM_ALLSITE, DCM_POS, 0, uHoldOffNum);
		BOOL bTriggerEdge = TRUE;
		for (auto& Slot : mapSlot)
		{
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
			{
				for (USHORT usTMUChannelIndex = 0; usTMUChannelIndex < 2; ++usTMUChannelIndex)
				{
					USHORT usChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usTMUChannel[usTMUChannelIndex];
					dcm_ReadTMUParam(Slot.first, usChannel, bTriggerEdge, usCurHoldOffTime, usCurHolfOffNum);
					XT_EXPECT_EQ((int)usCurHolfOffNum, (int)uHoldOffNum);
					if (usCurHolfOffNum != uHoldOffNum)
					{
						Report.SaveFailChannel(Slot.first, usChannel);
						continue;
					}

					///<Check the CTMU class
					dcm_GetTMUParameter(Slot.first, usChannel, bTriggerEdge, usCurHoldOffTime, usCurHolfOffNum);
					XT_EXPECT_EQ((int)usCurHolfOffNum, (int)uHoldOffNum);
					if (usCurHolfOffNum != uHoldOffNum)
					{
						Report.SaveAddtionMsg("The parameter gotten from class CTMU is error.");
						Report.SaveFailChannel(Slot.first, usChannel);
						continue;
					}
				}
			}
		}
	}


	Report.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}