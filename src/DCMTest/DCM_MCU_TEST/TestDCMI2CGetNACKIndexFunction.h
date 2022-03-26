#pragma once
/*!
* @file      TestDCMI2CGetNACKIndexFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/13
* @version   v 1.0.0.0
* @brief     测试I2CGetNACKIndex功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMI2CGetNACKIndexFunction)
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
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	CHANNEL_INFO MutualSCL[MAX_SITE];
	CHANNEL_INFO MutualSDA[MAX_SITE];
	USHORT usSiteCount = vecSCLChannel.size();
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		MutualSDA[usSiteNo].m_bySlotNo = vecSDAChannel[usSiteNo].m_bySlotNo;
		MutualSDA[usSiteNo].m_usChannel = vecSDAChannel[usSiteNo].m_usChannel + 2;
		MutualSCL[usSiteNo].m_bySlotNo = vecSCLChannel[usSiteNo].m_bySlotNo;
		MutualSCL[usSiteNo].m_usChannel = vecSCLChannel[usSiteNo].m_usChannel + 2;
	}

	//Load 2K lines vector before test I2C function
	const int nTestCount = 2;
	const char* lpszFilePath[nTestCount] = { g_lpszI2CVectorFilePath, g_lpszVectorFilePath };
	const BYTE byMaxDataLength = 5;
	const BYTE byDataTypeCount = 2;
	ULONG ulWaveData[byDataTypeCount][SITE_NUM] = { 0 };
	BYTE byWriteData[SITE_NUM][byDataTypeCount][byMaxDataLength] = { 0 };
	BYTE bySameData[byMaxDataLength] = { 0xAA,0x55,0xA5,0x5A,0x0F };
	BYTE* pbySameData[SITE_NUM] = { nullptr };
	BYTE* pbyDifferentData[SITE_NUM] = { nullptr };
	
	for (USHORT usSiteIndex = 0; usSiteIndex < usSiteCount; ++usSiteIndex)
	{
		ulWaveData[0][usSiteIndex] = 0xAA55A55A;
		ulWaveData[1][usSiteIndex] = ulWaveData[0][usSiteIndex] + usSiteIndex;
		pbySameData[usSiteIndex] = byWriteData[usSiteIndex][0];
		pbyDifferentData[usSiteIndex] = byWriteData[usSiteIndex][0];
		for (int nDataType = 0; nDataType < byDataTypeCount; ++nDataType)
		{
			for (int nIndex = 0; nIndex < byDataTypeCount; ++nIndex)
			{
				if (0 == nDataType)
				{
					byWriteData[usSiteIndex][nDataType][nIndex] = bySameData[nIndex];
				}
				else
				{
					byWriteData[usSiteIndex][nDataType][nIndex] = 0x55 + usSiteIndex;
				}
			}
		}
	}
	
	char lpszStartLabel[32] = { 0 };
	char lpszVectorStartLabel[32] = { 0 };
	char lpszPinGroup[64] = { 0 };
	const int nREGModeTestCount = 4; 
	I2C_REG_ADDR_MODE testREG[nREGModeTestCount] = { DCM_REG8, DCM_REG16, DCM_REG24, DCM_REG32 };
	int nREGByte[nREGModeTestCount] = { 0 };
	for (int nIndex = 0; nIndex < nREGModeTestCount; ++nIndex)
	{
		nREGByte[nIndex] = (testREG[nIndex] + 1) * 8;
	}
	int nMutualTestVectorBaseLine = 0;
	BYTE bySlaveAddr = 0xA6;
	int nREGAddr = 0xAA5AA55;
	BOOL bBRAMOccupy = FALSE;
	const BYTE byDataLengthType = 2;
	const int nI2CType = 3;
	int nDataLength[byDataLengthType] = { 4,byMaxDataLength };
	char* lpszI2CType[nI2CType] = { "I2CWriteData","I2CReadData","I2CWriteMultiData" };
	char* lpszACKLabel[nI2CType] = { "W_REG%d_%d_ACK%d" ,"R_REG%d_%d_ACK%d" ,"W_REG%d_%d_ACK%d" };
	char* lpszI2CStartLabel[nI2CType] = { "WT_REG%d_%d_ST" ,"RD_REG%d_%d_ST" ,"WT_REG%d_%d_ST" };
	char* lpszI2CStopLabel[nI2CType] = { "WT_REG%d_%d_SP" ,"RD_REG%d_%d_SP" ,"WT_REG%d_%d_SP" };
	double dPeriod = 2000;
	dcm_I2CEnableUncompare(FALSE);
	for (int nTestIndex = 0; nTestIndex < nTestCount; ++nTestIndex)
	{
		dcm.LoadVectorFile(lpszFilePath[nTestIndex], FALSE);
		dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
		dcm.SetPinGroup("G_SITE0", "CH3");
		dcm.SetPinGroup("G_SITE1", "CH7");
		dcm.SetPinGroup("G_SITE2", "CH11");
		dcm.SetPinGroup("G_SITE3", "CH15");
		dcm.SetPinGroup("G_MUTUAL", "CH3,CH7,CH11,CH15");
		dcm.Connect("G_ALLPIN");
		dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

		for (int nDataType = 0; nDataType < 2; ++nDataType)
		{
			for (int nREGModeIndex = 0; nREGModeIndex < nREGModeTestCount; ++nREGModeIndex)
			{
				dcm.I2CSet(dPeriod, usSiteCount, testREG[nREGModeIndex], strSCLChannel.c_str(), strSDAChannel.c_str());
				dcm.I2CSetPinLevel(3, 0, 1.5, 0.8);
				UINT uStartLine = 0;
				UINT uLineCount = 0;
				BOOL bWithDRAM = FALSE;
				UINT uLineCountBeforeOut = 0;
				UINT uDRAMStartLine = 0;
				UINT uDRAMLineCount = 0;

				if (1 == nTestIndex)
				{
					if (!bBRAMOccupy)
					{
						//Occupy the BRAM, in order to save the I2C pattern to DRAM
						ULONG ulOccupyData[MAX_SITE] = { 0 };
						BYTE byOccuSla = 0;
						int nOccuREGAddr = 0;
						for (int nSiteIndex = 0; nSiteIndex < usSiteCount; ++nSiteIndex)
						{
							ulOccupyData[nSiteIndex] = 0xAA55BBCC + nSiteIndex;
						}
						int nIndex = 0;
						do
						{
							byOccuSla = 0xA5 + nIndex * 2;
							nOccuREGAddr = 0xAA + nIndex;
							dcm.I2CWriteData(byOccuSla, nOccuREGAddr, 4, ulOccupyData);

							dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);
							++nIndex;
						} while (!bWithDRAM);
						bBRAMOccupy = TRUE;
					}
				}

				for (int nI2CTypeIndex = 0; nI2CTypeIndex < nI2CType; ++nI2CTypeIndex)
				{
					for (int nDataLengthIndex = 0; nDataLengthIndex < byDataLengthType; ++nDataLengthIndex)
					{
						BYTE byCurDataByte = nDataLength[nDataLengthIndex];
						if (4 < byCurDataByte && 2 > nI2CTypeIndex)
						{
							continue;
						}
						//I2CWriteData
						if (0 == nTestIndex)
						{
							if (0 == nDataType)
							{
								funcReport.AddTestItem("I2C: BRAM. REG size:%d bits. TYPE:%s. Data: %d bytes all same.", nREGByte[nREGModeIndex], lpszI2CType[nI2CTypeIndex], byCurDataByte);
							}
							else
							{
								funcReport.AddTestItem("I2C: BRAM. REG size:%d bits. TYPE:%s. Data: %d bytes all different.", nREGByte[nREGModeIndex], lpszI2CType[nI2CTypeIndex], byCurDataByte);
							}
						}
						else
						{
							if (0 == nDataType)
							{
								funcReport.AddTestItem("I2C: DRAM. REG size:%d bits. TYPE:%s. Data: %d bytes all same.", nREGByte[nREGModeIndex], lpszI2CType[nI2CTypeIndex], byCurDataByte);
							}
							else
							{
								funcReport.AddTestItem("I2C: DRAM. REG size:%d bits. TYPE:%s. Data: %d bytes all different.", nREGByte[nREGModeIndex], lpszI2CType[nI2CTypeIndex], byCurDataByte);
							}
						}
						int nCurNACKIndex = 0;
						//First run I2C function to generate the I2C pattern.
						int nOffset = 0;
						BYTE** ppbyWaveData = nullptr;
						if (0 == nDataType)
						{
							ppbyWaveData = pbySameData;
						}
						else
						{
							ppbyWaveData = pbyDifferentData;
						}
						if (0 == nI2CTypeIndex)
						{
							dcm.I2CWriteData(bySlaveAddr, nREGAddr, byCurDataByte, ulWaveData[nDataType]);
							sts_sprintf(lpszVectorStartLabel, sizeof(lpszVectorStartLabel), "WT_REG%d_%d_ST", nREGByte[nREGModeIndex], byCurDataByte);
						}
						else if (1 == nI2CTypeIndex)
						{
							dcm.I2CReadData(bySlaveAddr, nREGAddr, byCurDataByte);

							sts_sprintf(lpszVectorStartLabel, sizeof(lpszVectorStartLabel), "RD_REG%d_%d_ST", nREGByte[nREGModeIndex], byCurDataByte);
						}
						else
						{
							dcm.I2CWriteMultiData(bySlaveAddr, nREGAddr, byCurDataByte, ppbyWaveData);
							sts_sprintf(lpszVectorStartLabel, sizeof(lpszVectorStartLabel), "WT_REG%d_%d_ST", nREGByte[nREGModeIndex], byCurDataByte);
						}

						dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);

						ModifyI2CPattern(lpszVectorStartLabel, dPeriod, usSiteCount, MutualSCL, MutualSDA, 0xFFFF, FALSE);

						USHORT usSetSite = 0;
						USHORT usModifySite = 0;
						for (USHORT usSiteNo = 0; usSiteNo <= usSiteCount; ++usSiteNo)
						{
							usSetSite = usSiteNo;
							usModifySite = usSiteNo;
							int nACKValidCount = byCurDataByte + 2;

							nACKValidCount += nREGModeIndex;
							if (1 == nI2CTypeIndex)
							{
								nACKValidCount = 3;
							}
							for (int nNACKIndex = 0; nNACKIndex < nACKValidCount; ++nNACKIndex)
							{
								if (0 != nNACKIndex)
								{
									sprintf_s(lpszStartLabel, sizeof(lpszStartLabel), lpszACKLabel[nI2CTypeIndex], nREGByte[nREGModeIndex], byCurDataByte, nNACKIndex);
									//Set the ACK to NACK
									if (usSiteCount != usSiteNo)
									{
										sprintf_s(lpszPinGroup, sizeof(lpszPinGroup), "G_SITE%d", usSiteNo % 4);
										iterSlot = mapSlot.find(MutualSDA[usSiteNo].m_bySlotNo);
										usSetSite = iterSlot->second + usSiteNo / 4;
										dcm_WriteWaveData(lpszPinGroup, lpszStartLabel, usSetSite, 0, 1, 1);
									}
									else
									{
										usModifySite = DCM_ALLSITE;
										usSetSite = DCM_ALLSITE;
										strcpy_s(lpszPinGroup, sizeof(lpszPinGroup), "G_MUTUAL");
										dcm_WriteWaveData(lpszPinGroup, lpszStartLabel, usSetSite, 0, 1, 1);
									}
								}

								ModifyI2CPattern(lpszVectorStartLabel, dPeriod, usSiteCount, MutualSCL, MutualSDA, usModifySite, FALSE);

								//run the I2C pattern again, which has been added the mutual-test vector.
								if (0 == nI2CTypeIndex)
								{
									dcm.I2CWriteData(bySlaveAddr, nREGAddr, byCurDataByte, ulWaveData[nDataType]);
								}
								else if (1 == nI2CTypeIndex)
								{
									dcm.I2CReadData(bySlaveAddr, nREGAddr, byCurDataByte);
								}
								else
								{
									dcm.I2CWriteMultiData(bySlaveAddr, nREGAddr, byCurDataByte, ppbyWaveData);
								}

								int nExpectNACKIndex = nNACKIndex;
								for (int nSecondSiteIndex = 0; nSecondSiteIndex < usSiteCount; ++nSecondSiteIndex)
								{
									nExpectNACKIndex = nNACKIndex;
									if (usSiteCount != usSiteNo && nSecondSiteIndex != usSiteNo)
									{
										nExpectNACKIndex = 0;
									}
									nCurNACKIndex = dcm.I2CGetNACKIndex(nSecondSiteIndex);
									XT_EXPECT_EQ(nCurNACKIndex, nExpectNACKIndex);
									if (nCurNACKIndex != nExpectNACKIndex)
									{
										funcReport.SaveAddtionMsg("The NACK index(%d) is not equal to expected(%d).", nCurNACKIndex, nExpectNACKIndex);
										funcReport.SaveFailChannel(vecSCLChannel[nSecondSiteIndex].m_bySlotNo, vecSCLChannel[nSecondSiteIndex].m_usChannel);
										funcReport.SaveFailChannel(vecSDAChannel[nSecondSiteIndex].m_bySlotNo, vecSDAChannel[nSecondSiteIndex].m_usChannel);
									}
								}
								dcm_WriteWaveData(lpszPinGroup, lpszStartLabel, usSetSite, 0, 1, 0);///<Set all ACK to NACK
								ModifyI2CPattern(lpszVectorStartLabel, dPeriod, usSiteCount, MutualSCL, MutualSDA, usModifySite, FALSE);
							}
						}
					}
				}
				if (1 == nI2CType)
				{
					break;
				}
			}
		}
		dcm.Disconnect("G_ALLPIN");
		dcm_CloseFile();
		dcm_I2CDeleteMemory();
	}

	dcm_I2CEnableUncompare(TRUE);

	funcReport.AddTestItem("Check Invalid Site");
	
	dcm.I2CSet(200, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CReadData(0x20, 0x00, 1);
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);

	ULONG ulTargetBitData = 0;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		nRetVal = dcm.I2CGetNACKIndex(usSiteNo);
		if (usSiteNo == usInvalidSite)
		{
			XT_EXPECT_EQ(nRetVal, -2);
			if (-2 != nRetVal)
			{
				funcReport.SaveFailChannel(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel);
				continue;
			}
		}
		else
		{
			XT_EXPECT_NE(nRetVal, -2);
			if (-2 == nRetVal)
			{
				funcReport.SaveFailChannel(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel);
				continue;
			}
		}
	}
	RestoreSite();
	dcm_I2CDeleteMemory();



	funcReport.Print(this, g_lpszReportFilePath);
}