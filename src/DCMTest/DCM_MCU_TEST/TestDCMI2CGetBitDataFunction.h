#pragma once
/*!
* @file      TestDCMI2CGetBitDataFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/02/03
* @version   v 1.0.0.0
* @brief     测试I2CGetBitData功能
* @comment
*/
#include "..\DCMTestMain.h"
#define READ_PATTERN 1
XT_TEST(FunctionFunctionTest, TestDCMI2CGetBitDataFunction)
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

	const int nTestCount = 2;
	const char* lpszFilePath[nTestCount] = { g_lpszI2CVectorFilePath, g_lpszVectorFilePath };
	ULONG ulWriteData[MAX_SITE] = { 0 };

	ULONG ulCurAddData = 0;
	for (int nSiteIndex = 0; nSiteIndex < MAX_SITE; ++nSiteIndex)
	{
		ulCurAddData = 0;
		for (int nIndex = 0; nIndex < 4; ++nIndex)
		{
			ulCurAddData |= nSiteIndex << (nIndex * 8);
		}
		ulWriteData[nSiteIndex] = 0x55AAAA55 + ulCurAddData;
	}

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel; 
	CHANNEL_INFO MutualSCL[MAX_SITE];
	CHANNEL_INFO MutualSDA[MAX_SITE];
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT usSiteCount = vecSCLChannel.size();
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		MutualSDA[usSiteNo].m_bySlotNo = vecSDAChannel[usSiteNo].m_bySlotNo;
		MutualSDA[usSiteNo].m_usChannel = vecSDAChannel[usSiteNo].m_usChannel + 2;
		MutualSCL[usSiteNo].m_bySlotNo = vecSCLChannel[usSiteNo].m_bySlotNo;
		MutualSCL[usSiteNo].m_usChannel = vecSCLChannel[usSiteNo].m_usChannel + 2;
	}
	dcm_I2CEnableUncompare(FALSE);

	char lpszStartLabel[20] = { 0 };

	int nMutualTestVectorBaseLine = 0;
	ULONG ulReadData = 0;
	BYTE bySlaveAddr = 0xA6;
	int nREGAddr = 0xA5A5AA55;
	double dPeriod = 2000;
	BOOL bBRAMOccupy = FALSE;
	const int nDataLength = 4;
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.SetPinGroup("G_SITE0", "CH3");
	dcm.SetPinGroup("G_SITE1", "CH7");
	dcm.SetPinGroup("G_SITE2", "CH11");
	dcm.SetPinGroup("G_SITE3", "CH15");
	dcm.SetPinGroup("G_MUTUAL", "CH3,CH7,CH11,CH15");

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	dcm.I2CSet(dPeriod, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(3, 0, 1.5, 0.8);

	funcReport.AddTestItem("I2C: BRAM. REG size:%d bits. Data: %d bytes all different.", 8, nDataLength);

	//Data of all site is different from each other.
	ULONG ulWriteWaveData = 0;
	char lpszPinGroup[32] = { 0 };
	for (int nSiteIndex = 0; nSiteIndex < usSiteCount; ++nSiteIndex)
	{
		sprintf_s(lpszPinGroup, sizeof(lpszPinGroup), "G_SITE%d", nSiteIndex % 4);

		for (int nDataIndex = 0; nDataIndex < nDataLength; ++nDataIndex)
		{
			sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "R_REG%d_%d_DATA%d", 8, nDataLength, nDataIndex + 1);
			ulWriteWaveData = (ulWriteData[nSiteIndex] >> (24 - nDataIndex * 8)) & 0xFF;
			iterSlot = mapSlot.find(MutualSCL[nSiteIndex].m_bySlotNo);
			dcm.WriteWaveData(lpszPinGroup, lpszStartLabel, iterSlot->second + nSiteIndex / 4, 0, 8, ulWriteWaveData);
		}
	}

	//Firstly, generate the I2C pattern vector.
	dcm.I2CReadData(bySlaveAddr, nREGAddr, 4);

	sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "RD_REG%d_%d_ST", 8, nDataLength);
	nMutualTestVectorBaseLine = dcm_GetLabelLineNo(lpszStartLabel);
	ModifyI2CPattern(lpszStartLabel, dPeriod, usSiteCount, MutualSCL, MutualSDA, 0xFFFF, FALSE);

	//Secondly, run the I2C pattern again, which has been added the mutual-test vector.
	dcm.I2CReadData(bySlaveAddr, nREGAddr, nDataLength);

	//Finally, compare the capture data of mutual-test with the data be written.
	BOOL bFail = FALSE;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		bFail = FALSE;
		int nSumBits = 32;
		ULONG ulTargetData = 0;
		for (int nStartBit = 0; nStartBit < nSumBits; ++nStartBit)
		{
			int nMaxBitsCount = nSumBits - nStartBit;
			for (BYTE byBitsCount = 1; byBitsCount < nMaxBitsCount; ++byBitsCount)
			{
				ulReadData = dcm.I2CGetBitData(usSiteNo, nStartBit, byBitsCount);
				ulTargetData = ulWriteData[usSiteNo] << nStartBit;
				ulTargetData = ulTargetData >> (nSumBits - byBitsCount);
				XT_EXPECT_EQ(ulReadData, ulTargetData);
				if (ulReadData != ulTargetData)
				{
					funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Expect value is 0x%X, Real value is 0x%X",
						vecSCLChannel[usSiteNo].m_bySlotNo, vecSCLChannel[usSiteNo].m_usChannel, vecSDAChannel[usSiteNo].m_bySlotNo, 
						vecSDAChannel[usSiteNo].m_usChannel,ulTargetData, ulReadData);
					funcReport.SaveFailChannel(vecSCLChannel[usSiteNo].m_bySlotNo, vecSCLChannel[usSiteNo].m_usChannel);
					funcReport.SaveFailChannel(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel);
					bFail = TRUE;
					break;

				}
				if (bFail)
				{
					break;
				}
			}
			if (bFail)
			{
				break;
			}
		}
	}

	dcm_I2CEnableUncompare(TRUE);

	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = usSiteCount / 2;
	InvalidSite(usInvalidSite);
	dcm.I2CReadData(0xAC, 0x00, 1);

	ULONG ulTargetBitData = 0;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		ULONG ulBitData = dcm.I2CGetBitData(usSiteNo, 2, 4);
		if (usSiteNo == usInvalidSite)
		{
			XT_EXPECT_EQ(ulBitData, (ULONG)-1);
			if ((ULONG)-1 != ulBitData)
			{
				funcReport.SaveFailChannel(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel);
				continue;
			}
		}
		else
		{
			XT_EXPECT_NE(ulBitData, (ULONG)-1);
			if ((ULONG)-1 == ulBitData)
			{
				funcReport.SaveFailChannel(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel);
				continue;
			}
		}
	}
	RestoreSite();


	dcm.Disconnect("G_ALLPIN");
	mapSlot.clear();
	vecSDAChannel.clear();
	vecSCLChannel.clear();

	funcReport.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}