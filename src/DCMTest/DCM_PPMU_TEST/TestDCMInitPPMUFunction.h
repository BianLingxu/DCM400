#pragma once
/**
 * @file TestDCMInitPPMUFunction.h
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
 * @author Guangyun Wang
 * @date 2020/07/16
 * @version v 1.0.0.0
 * @brief Test the function of InitPPMU
 */
#include "..\DCMTestMain.h"
XT_TEST(PMUFunctionTest, TestDCMInitPPMUFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	int nRetVal = 0;
	CMeasurementFuncReport Report(strFuncName.c_str(), "PMUFunctionTest");//Error message.

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
		Report.SetFailInfo("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		Report.Print(this, g_lpszReportFilePath);
		return;
	}
	nRetVal = CheckPPMUConnection(Report, mapSlot);
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		Report.Print(this, g_lpszReportFilePath);
		dcm_CloseFile();
		return;
	}
	
	USHORT usSiteNo = 0;
	char lpszPinName[32] = { 0 };
	BYTE bySlotNo = 0;
	USHORT usChannel = 0;

	dcm.Connect("G_ALLPIN");
	dcm.SetPPMU("G_ALLPIN", DCM_PPMU_FVMV, 3);
	dcm.InitPPMU("G_ALLPIN");
	delay_ms(800);

	dcm.PPMUMeasure("G_ALLPIN", 10, 10);

	Report.AddTestItem("Measurement");
	Report.SetTestCondition("FVMI 3V");

	for (auto& Slot : mapSlot)
	{
		for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
		{
			usSiteNo = Slot.second + usSiteIndex;
			for (USHORT usChannelIndex = 0; usChannelIndex < DCM_CHANNELS_PER_CONTROL;++usChannelIndex)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannelIndex);
				double dMesureValaue = dcm.GetPPMUMeasResult(lpszPinName, usSiteNo);
				XT_EXPECT_REAL_EQ(dMesureValaue, 0, 0.01);
				if (0.01 < fabs(dMesureValaue))
				{
					dcm_GetPinGroupChannel(lpszPinName, usSiteNo, &bySlotNo, &usChannel, 1);
					Report.AddFailChannel(bySlotNo, usChannel, 0., dMesureValaue, 2, "V");
				}
			}
		}
	}
	dcm.SetPPMU("G_ALLPIN", DCM_PPMU_FVMV, 0);
	dcm.Disconnect("G_ALLPIN");

	Report.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}