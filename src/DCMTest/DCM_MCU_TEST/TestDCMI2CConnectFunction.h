#pragma once
/*!
* @file      TestDCMI2CConnectFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/02/03
* @version   v 1.0.0.0
* @brief     测试I2CConnect功能
* @comment
*/
#include "..\DCMTestMain.h"
BOOL IsChannelExisted(const std::string& strPin, BYTE bySlotNo, USHORT usChannel)
{
	int nPos = 0;
	char lpszChannel[16] = { 0 };
	sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d", bySlotNo, usChannel);
	int nChannelSize = strlen(lpszChannel);
	while (-1 != nPos)
	{
		nPos = strPin.find(lpszChannel, nPos);
		if (-1 == nPos)
		{
			return FALSE;
		}
		if (nPos + nChannelSize == strPin.size() || ',' == strPin[nPos + nChannelSize])
		{
			return TRUE;
		}
		nPos += nChannelSize;
	}
	return FALSE;
}

XT_TEST(FunctionFunctionTest, TestDCMI2CConnectFunction)
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
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT usSiteCount = vecSCLChannel.size();
	//Load vector.
	dcm.I2CSet(500, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());


	dcm.Disconnect("G_ALLPIN");

	dcm.I2CConnect();

	funcReport.AddTestItem("Check function relay status");

	BOOL bConnect = FALSE;
	char lpszPinName[32] = { 0 };
	USHORT usChannel = 0;
	for (auto& Slot : mapSlot)
	{
		for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
		{
			for (USHORT usPinIndex = 0; usPinIndex < DCM_CHANNELS_PER_CONTROL; ++usPinIndex)
			{
				usChannel = usSiteIndex * DCM_CHANNELS_PER_CONTROL + usPinIndex;
				bConnect = FALSE;
				if (IsChannelExisted(strSCLChannel, Slot.first, usChannel) || IsChannelExisted(strSDAChannel, Slot.first, usChannel))
				{
					bConnect = TRUE;
				}

				int nRelayStatus = dcm_GetRelayStatus(Slot.first, usChannel, 0);
				XT_EXPECT_EQ(nRelayStatus, bConnect);
				if (nRelayStatus != bConnect)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}
	dcm.Disconnect("G_ALLPIN");

	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;

	dcm.I2CDisconnect();

	InvalidSite(usInvalidSite);
	nRetVal = dcm.I2CConnect();
	BYTE byTargetRelayStatus = 0;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		byTargetRelayStatus = 1;
		if (usInvalidSite == usSiteNo)
		{
			byTargetRelayStatus = 0;
		}
		int nRelayStatus = dcm_GetRelayStatus(vecSCLChannel[usSiteNo].m_bySlotNo, vecSCLChannel[usSiteNo].m_usChannel, 0);
		XT_EXPECT_EQ(nRelayStatus, byTargetRelayStatus);
		if (nRelayStatus != byTargetRelayStatus)
		{
			funcReport.SaveFailChannel(vecSCLChannel[usSiteNo].m_bySlotNo, vecSCLChannel[usSiteNo].m_usChannel);
			continue;
		}

		nRelayStatus = dcm_GetRelayStatus(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel, 0);
		XT_EXPECT_EQ(nRelayStatus, byTargetRelayStatus);
		if (nRelayStatus != byTargetRelayStatus)
		{
			funcReport.SaveFailChannel(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel);
		}
	}
	RestoreSite();

	dcm.Disconnect("G_ALLPIN");
	vecSCLChannel.clear();
	vecSDAChannel.clear();

	mapSlot.clear();
	funcReport.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}