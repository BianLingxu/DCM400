#pragma once
/**
 * @file TestDCMSetValidPinFunction.h
 * @brief Check the function of SetValidPin
 * @author Guangyun Wang
 * @date 2021/08/03
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/

#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetValidPinFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport FuncReport(strFuncName.c_str(), "FunctionFunctionTest");

	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		FuncReport.SetNoBoardValid();
		FuncReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(FuncReport, mapSlot);

	FuncReport.AddTestItem("Not Limit for Vector Monopolized");
	nRetVal = dcm.SetValidPin("CH0,CH1");
	nRetVal = dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	XT_EXPECT_EQ(nRetVal, EXECUTE_SUCCESS);
	if (EXECUTE_SUCCESS != nRetVal)
	{
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				FuncReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}
	dcm_CloseFile();

	DCM dcm1;
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);

	FuncReport.AddTestItem("Limit Vector shared loaded");
	nRetVal = dcm.SetValidPin("CH0,CH1");
	nRetVal = dcm.SetPinGroup("G_TEST", "CH0,CH1,CH2");
	if (PIN_NOT_BELONGS != nRetVal)
	{
		FuncReport.SaveAddtionMsg("The pins are not belongs to instance, but the return value(%d) of SetPinGroup is not equal to PIN_NOT_BLONGS(%d).", nRetVal, PIN_NOT_BELONGS);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				FuncReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}


	FuncReport.AddTestItem("Limit Instance Share Vector");
	nRetVal = dcm1.SetValidPin("CH2,CH3");
	nRetVal = dcm1.SetPinGroup("G_TEST", "CH0,CH1,CH2");
	if (PIN_NOT_BELONGS != nRetVal)
	{
		FuncReport.SaveAddtionMsg("The pins are not belongs to instance, but the return value(%d) of SetPinGroup is not equal to PIN_NOT_BLONGS(%d).", nRetVal, PIN_NOT_BELONGS);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				FuncReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}

	dcm_CloseFile();
	FuncReport.Print(this, g_lpszReportFilePath);
}