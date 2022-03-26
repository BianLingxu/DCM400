#pragma once
/**
* @file TestDCMGetHardwareCaptureDataFunction.h
* @brief Test the hardware capture function
* @author Guangyun Wang
* @date 2020/07/12
* @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"

XT_TEST(FunctionFunctionTest, TestDCMGetHardwareCaptureDataFunction)
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
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");


	const BYTE byCaptureCountPerSector = 6;
	const BYTE bySectorCount = 8;
	const BYTE byChannelTypeCount = 4;
	BYTE byDataPerSector[byChannelTypeCount] = { 0x22, 0x11, 0x05, 0x0A };
	const BYTE byLoop = 4;
	const int nCaptureByteCount = (byCaptureCountPerSector * bySectorCount + 7 + byLoop) / 8;
	BYTE byCapture[byChannelTypeCount][nCaptureByteCount] = { 0 };
	int nBitCount = byCaptureCountPerSector * bySectorCount + byLoop;
	int nCurBit = 0;
	for (int nChannelTypeIndex = 0; nChannelTypeIndex < byChannelTypeCount; ++nChannelTypeIndex)
	{
		nCurBit = 0;
		for (BYTE bySectorIndex = 0; bySectorIndex < bySectorCount; ++bySectorIndex)
		{
			for (int nBitIndex = byCaptureCountPerSector - 1; nBitIndex >= 0; --nBitIndex)
			{
				byCapture[nChannelTypeIndex][nCurBit / 8] |= (byDataPerSector[nChannelTypeIndex] >> nBitIndex & 0x01) << (7 - nCurBit % 8);
				++nCurBit;
			}
			if (4 == bySectorIndex)
			{
				///<Loop command
				int nLoopBit = byDataPerSector[nChannelTypeIndex] >> 1 & 0x01;
				int nLoopBit1 = byDataPerSector[nChannelTypeIndex] & 0x01;

				byCapture[nChannelTypeIndex][nCurBit / 8] |= nLoopBit << (7 - nCurBit % 8);
				++nCurBit;
				byCapture[nChannelTypeIndex][nCurBit / 8] |= nLoopBit1 << (7 - nCurBit % 8);
				++nCurBit;
				byCapture[nChannelTypeIndex][nCurBit / 8] |= nLoopBit << (7 - nCurBit % 8);
				++nCurBit;
				byCapture[nChannelTypeIndex][nCurBit / 8] |= nLoopBit1 << (7 - nCurBit % 8);
				++nCurBit;
			}
		}
		if (0 != nBitCount % 8)
		{
			byCapture[nChannelTypeIndex][nCaptureByteCount - 1] >>= 8 - nBitCount % 8;
		}
	}

	funcReport.AddTestItem("Pin Mutual Test");

	double dSTBR = 10;//The time of STBR
	const double dAddStep = 5;
	char lpszPinName[32] = { 0 };//The pin name.
	ULONG ulFailCount = 0;
	ULONG ulFailLineNo[100] = { 0 };
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	BYTE abyData[50] = { 0 };

	funcReport.AddTestItem("BRAM");

	BYTE byBRAMCapture[2][2] = { {0x44,0x44},{0x22,0x22} };

	for (dSTBR = 30; dSTBR < 101; dSTBR += dAddStep)
	{
		dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, dSTBR);

		funcReport.AddClkSetting(10, 110, 10, 110, dSTBR, 110);

		dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_BRAM_ST", "FAIL_BRAM_SP");
		//dcm.SaveFailMap(0);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
			{
				sprintf_s(lpszPinName, 5, "CH%d", usChannel);
				for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
				{
					nRetVal = dcm.GetHardwareCaptureData(lpszPinName, Slot.second + usSiteIndex, abyData, sizeof(abyData));
					XT_EXPECT_EQ(nRetVal, 16);

					if (nRetVal != 16 || 0 != memcmp(byBRAMCapture[usChannel % 4 / 2], abyData, 2))
					{
						XT_EXPECT_TRUE(FALSE);
						funcReport.SaveFailChannel(Slot.first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannel);

					}
				}
			}
		}
	}

	funcReport.AddTestItem("DRAM");
	for (dSTBR = 30; dSTBR < 101; dSTBR += dAddStep)
	{
		dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, dSTBR);

		funcReport.AddClkSetting(10, 110, 10, 110, dSTBR, 110);

		dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");
		//dcm.SaveFailMap(0);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
			{
				sprintf_s(lpszPinName, 5, "CH%d", usChannel);
				for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
				{
					nRetVal = dcm.GetHardwareCaptureData(lpszPinName, Slot.second + usSiteIndex, abyData, sizeof(abyData));
					XT_EXPECT_EQ(nRetVal, nBitCount);

					if (nRetVal != nBitCount || 0 != memcmp(byCapture[usChannel % byChannelTypeCount], abyData, nCaptureByteCount))
					{
						XT_EXPECT_TRUE(FALSE);
						funcReport.SaveFailChannel(Slot.first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannel);

					}
				}
			}
		}
	}

	funcReport.AddTestItem("Check Fail Memory Limit");
	funcReport.SaveAddtionMsg("The capture line is over the fail line number saved.");
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_LIMIT_ST", "TEST_LIMIT_SP");
	//dcm.SaveFailMap(0);
	ULONG ulCaptureData = 0;

	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; byControllerIndex++, ++usSiteNo)
		{
			for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannel);

				nRetVal = dcm.GetHardwareCaptureData(lpszPinName, usSiteNo, abyData, sizeof(abyData));
				XT_EXPECT_EQ(nRetVal, CAPTURE_NOT_ALL_SAVE);
				if (CAPTURE_NOT_ALL_SAVE != nRetVal)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
					continue;
				}

				for (auto Data : abyData)
				{
					XT_EXPECT_EQ(Data, 0xFF);
					if (0xFF != Data)
					{
						funcReport.SaveFailChannel(Slot.first, usChannel + byControllerIndex * DCM_CHANNELS_PER_CONTROL);
						break;
					}
				}
			}
		}
	}



	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);

	nRetVal = dcm.GetHardwareCaptureData("CH2", usInvalidSite, abyData, sizeof(abyData));

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	for (auto Capture : abyData)
	{
		XT_EXPECT_EQ(Capture, (BYTE)-1);
		if (BYTE(-1) != Capture)
		{
			funcReport.SaveAddtionMsg("The hardware capture(0x%02X) of the invalid site is not equal to 0x%02X.", Capture, (ULONG)-1);
			break;
		}
	}
	RestoreSite();


	mapSlot.clear();
	funcReport.Print(this, g_lpszReportFilePath);


	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}