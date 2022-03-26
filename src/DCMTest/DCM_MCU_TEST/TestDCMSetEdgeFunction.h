#pragma once
/**
 * @file TestDCMSetEdgeFunction.h
 * @brief Test the function of function TMUMeasure
 * @author Guangyun Wang
 * @date 2020/09/03
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetEdgeFunction)
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
	dcm.SetPinGroup("G_EVEN", "CH0,CH2,CH4,CH6,CH8,CH10,CH12,CH14");
	const char lpszPinGroup[2][16] = { "G_ALLPIN", "G_EVEN" };

	double dPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);

	const int nTestEdgeCount = 3;
	double dRaiseEdge[nTestEdgeCount] = { 0, dPeriod / 4, dPeriod / 2 };
	double dFallEdge[nTestEdgeCount] = { dPeriod / 2, dPeriod / 2, dPeriod * 3 / 4 };
	double dDefaultEdge = 5;

	const BYTE byTestWaveFormatCount = 6;
	DataFormat TestDataFormat[byTestWaveFormatCount] = { DCM_DTFT_NRZ, DCM_DTFT_RZ, DCM_DTFT_RO, DCM_DTFT_SBC, DCM_DTFT_SBH, DCM_DTFT_SBL };
	WAVE_FORMAT TargetWaveFormat[byTestWaveFormatCount] = { WAVE_FORMAT::NRZ, WAVE_FORMAT::RZ, WAVE_FORMAT::RO, WAVE_FORMAT::SBC, WAVE_FORMAT::SBH, WAVE_FORMAT::SBL };

	const BYTE byTestIOFormatCount = 2;
	IOFormat TestIOFormat[byTestIOFormatCount] = { DCM_IO_NRZ, DCM_IO_RO };
	IO_FORMAT TargetIOFormat[byTestIOFormatCount] = { IO_FORMAT::NRZ, IO_FORMAT::RO };

	double dCurEdge[EDGE_COUNT] = { 0 };
	WAVE_FORMAT CurWaveFormat = WAVE_FORMAT::NRZ;
	IO_FORMAT CurIOFormat = IO_FORMAT::NRZ;
	COMPARE_MODE CompareMode = COMPARE_MODE::EDGE;
	double dSetEdge = 0;
	double dCurFallEdge = dFallEdge[nTestEdgeCount - 1];
	const char* lpszCurPinGroup = nullptr;
	for (BYTE byPinGroupIndex = 0; byPinGroupIndex < 2; ++byPinGroupIndex)
	{
		lpszCurPinGroup = lpszPinGroup[byPinGroupIndex];
		funcReport.AddTestItem("Raise edge check(%s)", lpszCurPinGroup);
		dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, DCM_IO_NRZ, dDefaultEdge, dDefaultEdge + 5, dDefaultEdge, dDefaultEdge + 5, dDefaultEdge);

		for (int nRaiseEdgeIndex = 0; nRaiseEdgeIndex < nTestEdgeCount; ++nRaiseEdgeIndex)
		{
			dcm.SetEdge(lpszCurPinGroup, "0", DCM_DTFT_NRZ, DCM_IO_NRZ, dRaiseEdge[nRaiseEdgeIndex], dCurFallEdge,
				dRaiseEdge[nRaiseEdgeIndex], dCurFallEdge, dRaiseEdge[nRaiseEdgeIndex]);

			iterSlot = mapSlot.begin();
			while (mapSlot.end() != iterSlot)
			{
				for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; usChannel++)
				{
					dcm_GetEdge(iterSlot->first, usChannel, 0, dCurEdge, CurWaveFormat, CurIOFormat, CompareMode);
					for (int nEdgeIndex = 0; nEdgeIndex < 5; ++nEdgeIndex)
					{
						if (0 != nEdgeIndex % 2)
						{
							continue;
						}
						if (0 != byPinGroupIndex && 0 != usChannel % 2)
						{
							dSetEdge = dDefaultEdge;
						}
						else
						{
							dSetEdge = dRaiseEdge[nRaiseEdgeIndex];
						}
						XT_EXPECT_REAL_EQ(dCurEdge[nEdgeIndex], dSetEdge, 0.1e-9);
						if (0.1e-9 < fabs(dCurEdge[nEdgeIndex] - dSetEdge))
						{
							funcReport.SaveFailChannel(iterSlot->first, usChannel);
							break;
						}
					}
				}
				++iterSlot;
			}
		}
	}

	double dCurDefauleEdge = dDefaultEdge + 5;
	for (BYTE byPinGroupIndex = 0; byPinGroupIndex < 2; ++byPinGroupIndex)
	{
		lpszCurPinGroup = lpszPinGroup[byPinGroupIndex];
		funcReport.AddTestItem("Fall edge check(%s)", lpszCurPinGroup);
		dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, DCM_IO_NRZ, dDefaultEdge, dCurDefauleEdge, dDefaultEdge, dCurDefauleEdge, dDefaultEdge);

		for (int nFallEdgeIndex = 0; nFallEdgeIndex < nTestEdgeCount; ++nFallEdgeIndex)
		{
			dcm.SetEdge(lpszCurPinGroup, "0", DCM_DTFT_NRZ, DCM_IO_NRZ, dRaiseEdge[0], dFallEdge[nFallEdgeIndex],
				dRaiseEdge[0], dFallEdge[nFallEdgeIndex], dPeriod * 3 / 4);

			iterSlot = mapSlot.begin();
			while (mapSlot.end() != iterSlot)
			{
				for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; usChannel++)
				{
					dcm_GetEdge(iterSlot->first, usChannel, 0, dCurEdge, CurWaveFormat, CurIOFormat, CompareMode);
					for (int nEdgeIndex = 1; nEdgeIndex < 5; ++nEdgeIndex)
					{
						if (0 == nEdgeIndex % 2)
						{
							continue;
						}
						if (0 != byPinGroupIndex && 0 != usChannel % 2)
						{
							dSetEdge = dCurDefauleEdge;
						}
						else
						{
							dSetEdge = dFallEdge[nFallEdgeIndex];
						}
						XT_EXPECT_REAL_EQ(dCurEdge[nEdgeIndex], dSetEdge, 0.1e-9);
						if (0.1e-9 < fabs(dCurEdge[nEdgeIndex] - dSetEdge))
						{
							funcReport.SaveFailChannel(iterSlot->first, usChannel);
							break;
						}
					}
				}
				++iterSlot;
			}
		}
	}

	funcReport.AddTestItem("Wave format check");
	for (int nWaveFormatIndex = 0; nWaveFormatIndex < byTestIOFormatCount; ++nWaveFormatIndex)
	{
		dcm.SetEdge("G_ALLPIN", "0", TestDataFormat[nWaveFormatIndex], DCM_IO_NRZ, dRaiseEdge[0], dFallEdge[0], dRaiseEdge[0], dFallEdge[0], dFallEdge[0]);
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; usChannel++)
			{
				dcm_GetEdge(iterSlot->first, usChannel, 0, dCurEdge, CurWaveFormat, CurIOFormat, CompareMode);
				XT_EXPECT_EQ((BYTE)CurWaveFormat, (BYTE)TargetWaveFormat[nWaveFormatIndex]);
				if (CurWaveFormat != TargetWaveFormat[nWaveFormatIndex])
				{
					funcReport.SaveFailChannel(iterSlot->first, usChannel);
				}
			}
			++iterSlot;
		}
	}

	funcReport.AddTestItem("IO format check");
	for (int nIoFormatIndex = 0; nIoFormatIndex < byTestIOFormatCount; ++nIoFormatIndex)
	{
		dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, TestIOFormat[nIoFormatIndex], dRaiseEdge[0], dFallEdge[0], dRaiseEdge[0], dFallEdge[0], dFallEdge[0]);
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; usChannel++)
			{
				dcm_GetEdge(iterSlot->first, usChannel, 0, dCurEdge, CurWaveFormat, CurIOFormat, CompareMode);
				XT_EXPECT_EQ((BYTE)CurIOFormat, (BYTE)TargetIOFormat[nIoFormatIndex]);
				if (CurIOFormat != TargetIOFormat[nIoFormatIndex])
				{
					funcReport.SaveFailChannel(iterSlot->first, usChannel);
				}
			}
			++iterSlot;
		}
	}

	funcReport.AddTestItem("Check Invalid Site");
	const USHORT usInvalidSite = mapSlot.begin()->second;
	double adValidSiteEdge[EDGE_COUNT] = { 10,15,10,15,20,25 };
	double adInvalidSiteEdge[EDGE_COUNT] = { 5,20,5,20,15,25 };
	DataFormat ValidSiteDataFormat = DCM_DTFT_RO;
	DataFormat InvalidSiteDataFormat = DCM_DTFT_RZ;
	WAVE_FORMAT InvalidSiteWaveFormat = WAVE_FORMAT::RZ;
	WAVE_FORMAT ValidSiteWaveFormat = WAVE_FORMAT::RO;
	IOFormat ValidSiteIOFormat = DCM_IO_RO;
	IO_FORMAT ValidIOFormat = IO_FORMAT::RO;
	IOFormat InvalidSiteIOFormat = DCM_IO_NRZ;
	IO_FORMAT InvalidIOFormat = IO_FORMAT::NRZ;

	double adOriEdge[EDGE_COUNT] = { 0 };
	WAVE_FORMAT OriWaveFormat = WAVE_FORMAT::NRZ;
	IO_FORMAT OriIOFormat = IO_FORMAT::NRZ;
	COMPARE_MODE OriCompareMode = COMPARE_MODE::EDGE;
	USHORT usTestChannel = 2;
	char lpszPinName[32] = { 0 };
	sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usTestChannel);
	dcm_GetEdge(mapSlot.begin()->first, 0, 0, adOriEdge, OriWaveFormat, OriIOFormat, OriCompareMode);

	dcm.SetEdge(lpszPinName, "0", InvalidSiteDataFormat, InvalidSiteIOFormat, adInvalidSiteEdge[0], adInvalidSiteEdge[1], adInvalidSiteEdge[2],
		adInvalidSiteEdge[3], adInvalidSiteEdge[4]);
	InvalidSite(usInvalidSite);

	dcm.SetEdge(lpszPinName, "0", ValidSiteDataFormat, ValidSiteIOFormat, adValidSiteEdge[0], adValidSiteEdge[1], adValidSiteEdge[2],
		adValidSiteEdge[3], adValidSiteEdge[4]);

	double* pdExpectedEdge = 0;
	WAVE_FORMAT ExpectedWaveFormat = WAVE_FORMAT::NRZ;
	IO_FORMAT ExpectedIOFormat = IO_FORMAT::NRZ;

	USHORT usCurChannel = 0;
	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; byControllerIndex++, ++usSiteNo)
		{
			usCurChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usTestChannel;
			dcm_GetEdge(Slot.first, usCurChannel, 0, dCurEdge, CurWaveFormat, CurIOFormat, CompareMode);
			if (usInvalidSite == usSiteNo)
			{
				pdExpectedEdge = adInvalidSiteEdge;
				ExpectedWaveFormat = InvalidSiteWaveFormat;
				ExpectedIOFormat = InvalidIOFormat;
			}
			else
			{
				pdExpectedEdge = adValidSiteEdge;
				ExpectedWaveFormat = ValidSiteWaveFormat;
				ExpectedIOFormat = ValidIOFormat;
			}
			BOOL bFail = FALSE;
			for (int nEdgeIndex = 0; nEdgeIndex < EDGE_COUNT - 1; ++nEdgeIndex)
			{
				XT_EXPECT_EQ(dCurEdge[nEdgeIndex], pdExpectedEdge[nEdgeIndex]);
				if (pdExpectedEdge[nEdgeIndex] != dCurEdge[nEdgeIndex])
				{
					funcReport.SaveFailChannel(Slot.first, usCurChannel);
					bFail = TRUE;
					break;
				}
			}
			if (bFail)
			{
				continue;
			}

			XT_EXPECT_EQ((BYTE)CurWaveFormat, (BYTE)ExpectedWaveFormat);
			if (CurWaveFormat != ExpectedWaveFormat)
			{
				funcReport.SaveFailChannel(Slot.first, usCurChannel);
				continue;
			}
			XT_EXPECT_EQ((BYTE)CurIOFormat, (BYTE)ExpectedIOFormat);
			if (CurIOFormat != ExpectedIOFormat)
			{
				funcReport.SaveFailChannel(Slot.first, usCurChannel);
				continue;
			}
		}
	}

	RestoreSite();
	dcm.SetEdge(lpszPinName, "0", (DataFormat)OriWaveFormat, (IOFormat)OriIOFormat, adOriEdge[0], adOriEdge[1], adOriEdge[2], adOriEdge[3], adOriEdge[4]);


	funcReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}