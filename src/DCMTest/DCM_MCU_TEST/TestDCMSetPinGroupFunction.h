#pragma once
/*!
* @file      TestDCMSetPinGroupFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/12
* @version   v 1.0.0.0
* @brief     测试SetPinGroup功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetPinGroupFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");//Error message.
	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;

	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		funcReport.SetNoBoardValid();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(funcReport, mapSlot);

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

	BOOL bChannelRelayStatus[DCM_MAX_BOARD_NUM * DCM_MAX_CONTROLLERS_PRE_BOARD][DCM_CHANNELS_PER_CONTROL] = { 0 };

	
	funcReport.AddTestItem("Pin Group Not be Defined");
	int nRetval = dcm.Connect("G_ALLPIN");

	XT_EXPECT_EQ(PIN_GROUP_ERROR, nRetval);
	if (nRetval != PIN_GROUP_ERROR)
	{
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD;++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}


	//Defined pin group G_ALLPIN
	funcReport.AddTestItem("Connect G_ALLPIN: All channels");
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	nRetval = dcm.Connect("G_ALLPIN");

	XT_EXPECT_EQ(0, nRetval);
	if (0 != nRetval)
	{
		funcReport.SaveAddtionMsg("return value(%d) is not equal to expect(%d)", nRetval, 0);

		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}
	else
	{
		int nRelayStatus = 0;
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				nRelayStatus = dcm_GetRelayStatus(Slot.first, usChannel, 0);
				if (1 != nRelayStatus)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}

	//Define pin group G_ODDPIN
	funcReport.AddTestItem("Connect the odd pin");
	dcm.SetPinGroup("G_ODDPIN", "CH1,CH3,CH5,CH7,CH9,CH11,CH13,CH15");
	nRetval = dcm.Disconnect("G_ODDPIN");

	XT_EXPECT_EQ(0, nRetval);
	if (0 != nRetval)
	{
		funcReport.SaveAddtionMsg("Unexpect error(%d)", nRetval);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}
	else
	{
		int nRelayStatus = 0;
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				nRelayStatus = dcm_GetRelayStatus(Slot.first, usChannel, 0);
				if ((usChannel % 2) == nRelayStatus)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}


	funcReport.AddTestItem("Define pin group twice: Pin is not same");
	nRetval = dcm.SetPinGroup("G_ALLPIN", "CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	if (PIN_GROUP_NAME_CONFLICT != nRetval)
	{
		funcReport.SaveAddtionMsg("return value(%d) is not equal to PIN_GROUP_NAME_CONFLICT(%d)", nRetval, PIN_GROUP_NAME_CONFLICT);

		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}



	funcReport.AddTestItem("Define pin group twice: Pin is same");
	nRetval = dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	if (0 != nRetval)
	{
		funcReport.SaveAddtionMsg("return value(%d) is not equal to expect(%d)", nRetval, 0);

		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}
	funcReport.AddTestItem("Disconnect G_ALLPIN");
	dcm.Disconnect("G_ALLPIN");

	int nRelayStatus = 0;

	for (auto& Slot : mapSlot)
	{
		for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
		{
			nRelayStatus = dcm_GetRelayStatus(Slot.first, usChannel, 0);
			if (0 != nRelayStatus)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}

	///<Check pin group nested
	funcReport.AddTestItem("Pin Group Nested");
	dcm.SetPinGroup("G_FIRSTTWO", "CH0,CH1");
	dcm.SetPinGroup("G_FIRSTFOUR", "G_FIRSTTWO,CH2,CH3");
	dcm.Connect("G_FIRSTFOUR");
	int nTargetStatus = 0;
	for (auto& Slot : mapSlot)
	{
		for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
		{
			nTargetStatus = 0;
			if (4 > usChannel % DCM_CHANNELS_PER_CONTROL)
			{
				nTargetStatus = 1;
			}
			nRelayStatus = dcm_GetRelayStatus(Slot.first, usChannel, 0);
			XT_EXPECT_EQ(nRelayStatus, nTargetStatus);
			if (nTargetStatus != nRelayStatus)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}
	dcm.Disconnect("G_ALLPIN");
	funcReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}