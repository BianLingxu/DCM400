#pragma once
/**
 * @file TestDCMSetTMUMatrixFunction.h
 * @brief Test the function of function SetTMUMatrix
 * @author Guangyun Wang
 * @date 2020/09/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(TMUFunctionTest, TestDCMSetTMUMatrixFunction)
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

	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);

	Report.AddTestItem("Check register value");
	Report.SaveAddtionMsg("Check the TMU connected");
	const USHORT usTMUChannelCount = 2;
	USHORT usTMUChannel[usTMUChannelCount] = { 0,4 };
	for (auto& Slot : mapSlot)
	{
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			for (USHORT usTMUChannelIndex = 0; usTMUChannelIndex < 2; ++usTMUChannelIndex)
			{
				USHORT usChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usTMUChannel[usTMUChannelIndex];
				int nRetVal = dcm_ReadTMUConnectUnit(Slot.first, usChannel);
				if (usTMUChannelIndex != nRetVal)
				{
					Report.SaveFailChannel(Slot.first, usChannel);
				}

				///<Check the CTMU class
				nRetVal = dcm_GetTMUConnectUnit(Slot.first, usChannel);
				if (usTMUChannelIndex != nRetVal)
				{
					Report.SaveAddtionMsg("The parameter gotten from class CTMU is error.");
					Report.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}

	Report.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}