#pragma once
/**
 * @file TestDCMI2CSetPinLevelFunction.h
 * @brief Check the function function of I2CSetPinLevel
 * @author Guangyun Wang
 * @date 2021/04/16
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMI2CSetPinLevel_PinFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");

	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, g_lpszI2CVectorFilePath);
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

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	vector<CHANNEL_INFO>* pavecChannel[2] = { &vecSCLChannel, &vecSDAChannel };
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT usSiteCount = vecSCLChannel.size();
	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	funcReport.AddTestItem("Check Function");
	double adSCLExpectedPinLevel[4] = { 3.0, 0.0, 1.5, 0.8 };
	double adSDAExpectedPinLevel[4] = { 4.0, 1.0, 2.0, 1.0 };
	dcm.I2CSetPinLevel(adSCLExpectedPinLevel[0], adSCLExpectedPinLevel[1], adSCLExpectedPinLevel[2], adSCLExpectedPinLevel[3], DCM_I2C_SCL);
	dcm.I2CSetPinLevel(adSDAExpectedPinLevel[0], adSDAExpectedPinLevel[1], adSDAExpectedPinLevel[2], adSDAExpectedPinLevel[3], DCM_I2C_SDA);
	double* pdExectedLevel = 0;
	double dCurPinLevel = 0;
	CHANNEL_INFO* pChannel = nullptr;

	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		for (BYTE byChannelType = 0; byChannelType < 2; ++byChannelType)
		{
			if (0 == byChannelType)
			{
				pdExectedLevel = adSCLExpectedPinLevel;
				pChannel = &vecSCLChannel[usSiteNo];
			}
			else
			{
				pdExectedLevel = adSDAExpectedPinLevel;
				pChannel = &vecSDAChannel[usSiteNo];
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIH, dCurPinLevel);
			XT_EXPECT_REAL_EQ(dCurPinLevel, pdExectedLevel[0], 0.01);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[0]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[1]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOH, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[2]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[3]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
		}
	}

	funcReport.AddTestItem("Check Function_BOTH");
	dcm.I2CSetPinLevel(adSCLExpectedPinLevel[0], adSCLExpectedPinLevel[1], adSCLExpectedPinLevel[2], adSCLExpectedPinLevel[3], DCM_I2C_BOTH);
	pdExectedLevel = adSCLExpectedPinLevel;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		for (auto& Channel : pavecChannel)
		{
			pChannel = &Channel->at(usSiteNo);
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIH, dCurPinLevel);
			XT_EXPECT_REAL_EQ(dCurPinLevel, pdExectedLevel[0], 0.01);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[0]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[1]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOH, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[2]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[3]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
		}
	}

	funcReport.AddTestItem("Check Invalid Site_SCL");
	const USHORT usInvalidSite = mapSlot.begin()->second;

	double dValidSitePinLevel[4] = { 3.0, 0, 1.5, 0.8 };
	double dInvalidSitePinLevel[4] = { 5, 0, 3, 1 };
	dcm.I2CSetPinLevel(dInvalidSitePinLevel[0], dInvalidSitePinLevel[1], dInvalidSitePinLevel[2], dInvalidSitePinLevel[3], DCM_I2C_SCL);
	InvalidSite(usInvalidSite);
	dcm.I2CSetPinLevel(dValidSitePinLevel[0], dValidSitePinLevel[1], dValidSitePinLevel[2], dValidSitePinLevel[3], DCM_I2C_SCL);
	
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		if (usInvalidSite == usSiteNo)
		{
			pdExectedLevel = dInvalidSitePinLevel;
		}
		else
		{
			pdExectedLevel = dValidSitePinLevel;
		}
		pChannel = &vecSCLChannel.at(usSiteNo);
		dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIH, dCurPinLevel);
		XT_EXPECT_REAL_EQ(dCurPinLevel, pdExectedLevel[0], 0.01);
		if (0.01 < fabs(dCurPinLevel - pdExectedLevel[0]))
		{
			funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
			continue;
		}
		dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIL, dCurPinLevel);
		if (0.01 < fabs(dCurPinLevel - pdExectedLevel[1]))
		{
			funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
			continue;
		}
		dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOH, dCurPinLevel);
		if (0.01 < fabs(dCurPinLevel - pdExectedLevel[2]))
		{
			funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
			continue;
		}
		dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOL, dCurPinLevel);
		if (0.01 < fabs(dCurPinLevel - pdExectedLevel[3]))
		{
			funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
			continue;
		}
	}

	RestoreSite();
	funcReport.AddTestItem("Check Invalid Site_SDA");

	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(dInvalidSitePinLevel[0], dInvalidSitePinLevel[1], dInvalidSitePinLevel[2], dInvalidSitePinLevel[3], DCM_I2C_SDA);
	InvalidSite(usInvalidSite);
	dcm.I2CSetPinLevel(dValidSitePinLevel[0], dValidSitePinLevel[1], dValidSitePinLevel[2], dValidSitePinLevel[3], DCM_I2C_SDA);

	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		if (usInvalidSite == usSiteNo)
		{
			pdExectedLevel = dInvalidSitePinLevel;
		}
		else
		{
			pdExectedLevel = dValidSitePinLevel;
		}
		pChannel = &vecSDAChannel.at(usSiteNo);
		dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIH, dCurPinLevel);
		XT_EXPECT_REAL_EQ(dCurPinLevel, pdExectedLevel[0], 0.01);
		if (0.01 < fabs(dCurPinLevel - pdExectedLevel[0]))
		{
			funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
			continue;
		}
		dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIL, dCurPinLevel);
		if (0.01 < fabs(dCurPinLevel - pdExectedLevel[1]))
		{
			funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
			continue;
		}
		dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOH, dCurPinLevel);
		if (0.01 < fabs(dCurPinLevel - pdExectedLevel[2]))
		{
			funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
			continue;
		}
		dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOL, dCurPinLevel);
		if (0.01 < fabs(dCurPinLevel - pdExectedLevel[3]))
		{
			funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
			continue;
		}
	}

	RestoreSite();

	funcReport.AddTestItem("Check Invalid Site_BOTH");

	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(dInvalidSitePinLevel[0], dInvalidSitePinLevel[1], dInvalidSitePinLevel[2], dInvalidSitePinLevel[3], DCM_I2C_BOTH);
	InvalidSite(usInvalidSite);
	dcm.I2CSetPinLevel(dValidSitePinLevel[0], dValidSitePinLevel[1], dValidSitePinLevel[2], dValidSitePinLevel[3], DCM_I2C_BOTH);

	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		if (usInvalidSite == usSiteNo)
		{
			pdExectedLevel = dInvalidSitePinLevel;
		}
		else
		{
			pdExectedLevel = dValidSitePinLevel;
		}
		for (auto& Channel : pavecChannel)
		{
			pChannel = &Channel->at(usSiteNo);
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIH, dCurPinLevel);
			XT_EXPECT_REAL_EQ(dCurPinLevel, pdExectedLevel[0], 0.01);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[0]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[1]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOH, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[2]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdExectedLevel[3]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
		}
	}

	RestoreSite();


	funcReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}