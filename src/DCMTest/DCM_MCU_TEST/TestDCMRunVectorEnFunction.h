#pragma once
/**
 * @file TestDCMRunVectorEnFunction.h
 * @brief Check the function of RunVectorEn
 * @author Guangyun Wang
 * @date 2021/06/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/

#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMRunVectorEnFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");

	map<BYTE, USHORT> mapSlot;
	BYTE byBoardCount = dcm_GetBoardInfo(nullptr, 0);
	if (0 == byBoardCount)
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		funcReport.SetNoBoardValid();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	BYTE* pbySlotNo = nullptr;
	try
	{
		pbySlotNo = new BYTE[byBoardCount];
		memset(pbySlotNo, 0, byBoardCount * sizeof(BYTE));
	}
	catch (const std::exception&)
	{
		return;
	}

	dcm_GetBoardInfo(pbySlotNo, byBoardCount * sizeof(BYTE));

	char lpszSN[64] = { 0 };
	for (BYTE byBoardIndex = 0; byBoardIndex < byBoardCount; ++byBoardIndex)
	{
		//Save the board SN
		mapSlot.insert(pair<BYTE, BYTE>(pbySlotNo[byBoardIndex], -1));
		dcm_GetModuleInfoByBoard(pbySlotNo[byBoardIndex], lpszSN, sizeof(lpszSN), DCMINFO::MODULE_SN, STS_MOTHER_BOARD);
		funcReport.SaveBoardSN(pbySlotNo[byBoardIndex], lpszSN);
	}
	if (nullptr != pbySlotNo)
	{
		delete[] pbySlotNo;
		pbySlotNo = nullptr;
	}
	//Load vector.
	int nRetVal = dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
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

	//Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	USHORT puSite[MAX_SITE] = { 0 };
	USHORT uSiteCount = 0;
	for (auto& Slot : mapSlot)
	{
		uSiteCount = dcm_GetSlotSite(Slot.first, puSite, MAX_SITE);
		if (0 < uSiteCount)
		{
			USHORT uFirstSite = puSite[0];
			for (USHORT usSiteIndex = 1; usSiteIndex < uSiteCount; ++usSiteIndex)
			{
				if (MAX_SITE <= usSiteIndex)
				{
					break;
				}
				if (uFirstSite > puSite[usSiteIndex])
				{
					uFirstSite = puSite[usSiteIndex];
				}
			}
			Slot.second = uFirstSite;
		}
	}

	double dPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);

	//Set the time in order to ensure the mutual-test is work well.
	dcm.SetEdge("G_ALLPIN", 0, DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 3 / 4, dPeriod / 8, dPeriod / 2);

	funcReport.AddTestItem("Check Vector need to be ran");
	dcm.RunVectorEn("G_ALLPIN", "TEST_RUN_ST", "TEST_RUN_SP");
	nRetVal = dcm_IsSynRun();
	XT_EXPECT_EQ(nRetVal, TRUE);
	if (!nRetVal)
	{
		funcReport.SaveAddtionMsg("Vector need to be ran, but the return value is not.");
		for (auto& Slot : mapSlot)
		{
			USHORT usChannel = 0;
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex)
			{
				for (BYTE byPin = 0; byPin < DCM_CHANNELS_PER_CONTROL;++byPin, ++usChannel)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}

	funcReport.AddTestItem("Check run vector");
	dcm_Run();
	delay_ms(10);

	char lpszPinName[32] = { 0 };
	for (auto& Slot : mapSlot)
	{
		USHORT usChannel = 0;
		USHORT usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex, ++usSiteNo)
		{
			for (BYTE byPin = 0; byPin < DCM_CHANNELS_PER_CONTROL; ++byPin, ++usChannel)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", byPin);
				nRetVal = dcm.GetMCUPinRunStatus(lpszPinName, usSiteNo);
				XT_EXPECT_EQ(nRetVal, 1);
				if (1 != nRetVal)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}
	
	funcReport.AddTestItem("Check Invalid Site");
	const USHORT usInvalidSite = mapSlot.begin()->second;
	dcm.SetPinGroup("G_PMU", "CH2,CH3,CH6,CH7,CH10,CH11,CH14,CH15");
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 2, 0.8);
	dcm.SetChannelStatus("G_ALLPIN", DCM_ALLSITE, DCM_HIGH_IMPEDANCE);

	InvalidSite(usInvalidSite);
	dcm.RunVectorEn("G_ALLPIN", "TEST_RUN_ST", "TEST_RUN_SP");
	dcm_Run();
	dcm_WaitStop();
	RestoreSite();

	dcm.SetPPMU("G_PMU", DCM_PPMU_FIMV, 1e-3, DCM_PPMUIRANGE_2MA);
	dcm.PPMUMeasure("G_PMU", 10, 10);

	double dCH0TargetVoltage = 0;
	double dCH1TargetVoltage = 3;
	double dInvalidSiteTargetVoltage = 7.5;

	double dTarget = 0;
	for (auto& Slot : mapSlot)
	{
		USHORT usBaseSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			USHORT usSiteNo = usBaseSiteNo + byControllerIndex;

			for (USHORT usChannel = 2; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannel);
				double dMeas = dcm.GetPPMUMeasResult(lpszPinName, usSiteNo);
				if (usSiteNo == usInvalidSite)
				{
					dTarget = dInvalidSiteTargetVoltage;
				}
				else
				{
					if (0 == usChannel % 2)
					{
						dTarget = dCH0TargetVoltage;
					}
					else
					{
						dTarget = dCH1TargetVoltage;
					}
				}
				XT_EXPECT_REAL_EQ(dMeas, dTarget, 0.1);
				if (0.1 < fabs(dMeas - dTarget))
				{
					funcReport.SaveFailChannel(Slot.first, byControllerIndex * DCM_CHANNELS_PER_CONTROL + usChannel - 2);
				}
				if (1 == usChannel % 2)
				{
					usChannel += 2;
				}
			}
		}
	}
	dcm.InitMCU("G_ALLPIN");

	mapSlot.clear();

	funcReport.Print(this, g_lpszReportFilePath);


	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}