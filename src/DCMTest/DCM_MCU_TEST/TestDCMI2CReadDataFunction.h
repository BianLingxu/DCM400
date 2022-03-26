#pragma once
/*!
* @file      TestDCMI2CReadDataFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/14
* @version   v 1.0.0.0
* @brief     测试I2CReadData功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMI2CReadDataFunction)
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

	const int nTestCount = 2;
	const char* lpszFilePath[nTestCount] = { g_lpszI2CVectorFilePath, g_lpszVectorFilePath };
	ULONG ulWriteSameData[MAX_SITE] = { 0 };
	ULONG ulWriteDiffData[MAX_SITE] = { 0 };

	ULONG ulCurAddData = 0;
	for (int nSiteIndex = 0; nSiteIndex < MAX_SITE; ++nSiteIndex)
	{
		ulCurAddData = 0;
		for (int nIndex = 0; nIndex < 4; ++nIndex)
		{
			ulCurAddData |= nSiteIndex << (nIndex * 8);
		}
		ulWriteSameData[nSiteIndex] = 0x55AAAA55;
		ulWriteDiffData[nSiteIndex] = 0x55AAAA55 + ulCurAddData;
	}

	ULONG* ulWriteData[2] = { ulWriteSameData , ulWriteDiffData };

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT usSiteCount = vecSCLChannel.size();

	CHANNEL_INFO MutualSCL[MAX_SITE];
	CHANNEL_INFO MutualSDA[MAX_SITE];
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		MutualSDA[usSiteNo].m_bySlotNo = vecSDAChannel[usSiteNo].m_bySlotNo;
		MutualSDA[usSiteNo].m_usChannel = vecSDAChannel[usSiteNo].m_usChannel + 2;
		MutualSCL[usSiteNo].m_bySlotNo = vecSCLChannel[usSiteNo].m_bySlotNo;
		MutualSCL[usSiteNo].m_usChannel = vecSCLChannel[usSiteNo].m_usChannel + 2;
	}

	char lpszStartLabel[20] = { 0 };
	char lpszStopLabel[20] = { 0 };
	const int nREGModeTestCount = 4;
	I2C_REG_ADDR_MODE testREG[nREGModeTestCount] = { DCM_REG8, DCM_REG16, DCM_REG24, DCM_REG32 };
	int nREGByte[nREGModeTestCount] = { 0 };
	for (int nIndex = 0; nIndex < nREGModeTestCount; ++nIndex)
	{
		nREGByte[nIndex] = (testREG[nIndex] + 1) * 8;
	}


	int nSCLBRAMFailCount = 0;
	int nSDABRAMFailCount = 0;
	int nSCLDRAMFailCount = 0;
	int nSDADRAMFailCount = 0;
	UINT* puSCLBRAMFailLineNo = nullptr;
	UINT* puSDABRAMFailLineNo = nullptr;
	UINT* puSCLDRAMFailLineNo = nullptr;
	UINT* puSDADRAMFailLineNo = nullptr;

	int nMutualTestVectorBaseLine = 0;
	BYTE byRealData = 0;
	BYTE bySCLRealData = 0;
	BYTE bySlaveAddr = 0xA6;
	int nREGAddr = 0xA5A5AA55;
	double dPeriod = 2000;
	BOOL bBRAMOccupy = FALSE;
	const int nDataLength = 4;
	dcm_I2CEnableUncompare(FALSE);
	for (int nTestIndex = 0; nTestIndex < nTestCount; ++nTestIndex)
	{
		nRetVal = dcm.LoadVectorFile(lpszFilePath[nTestIndex]);
		if (0 != nRetVal)
		{
			funcReport.SaveAddtionMsg("Load vector file(%s) fail.", lpszFilePath[nTestIndex]);
			continue;
		}

		dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
		dcm.SetPinGroup("G_SITE0", "CH3");
		dcm.SetPinGroup("G_SITE1", "CH7");
		dcm.SetPinGroup("G_SITE2", "CH11");
		dcm.SetPinGroup("G_SITE3", "CH15");
		dcm.SetPinGroup("G_MUTUAL", "CH3,CH7,CH11,CH15");

		dcm.Connect("G_ALLPIN");
		dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

		for (int nREGModeIndex = 0; nREGModeIndex < nREGModeTestCount; ++nREGModeIndex)
		{
			bBRAMOccupy = FALSE;
			dcm.I2CSet(dPeriod, usSiteCount, testREG[nREGModeIndex], strSCLChannel.c_str(), strSDAChannel.c_str());
			dcm.I2CSetPinLevel(3, 0, 1.5, 0.8);
			for (int nReadDataType = 0; nReadDataType < 2; ++nReadDataType)
			{
				UINT uStartLine = 0;
				UINT uLineCount = 0;
				BOOL bWithDRAM = FALSE;
				UINT uLineCountBeforeOut = 0;
				UINT uDRAMStartLine = 0;
				UINT uDRAMLineCount = 0;
				if (0 == nTestIndex)
				{
					if (0 == nReadDataType)
					{
						funcReport.AddTestItem("I2C: BRAM. REG size:%d bits. Data: %d bytes all same.", nREGByte[nREGModeIndex], nDataLength);
					}
					else
					{
						funcReport.AddTestItem("I2C: BRAM. REG size:%d bits. Data: %d bytes all different.", nREGByte[nREGModeIndex], nDataLength);
					}
				}
				else
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
							dcm.I2CReadData(byOccuSla, nOccuREGAddr);

							dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);
							++nIndex;
						} while (!bWithDRAM);
						bBRAMOccupy = TRUE;
					}
					if (0 == nReadDataType)
					{
						funcReport.AddTestItem("I2C: DRAM. REG size:%d bits. Data: %d bytes all same.", nREGByte[nREGModeIndex], nDataLength);
					}
					else
					{
						funcReport.AddTestItem("I2C: DRAM. REG size:%d bits. Data: %d bytes all different.", nREGByte[nREGModeIndex], nDataLength);
					}
				}
				BOOL bAllSiteSame = TRUE;
				if (0 == nReadDataType)
				{
					//Data of all site is same.
					ULONG ulWriteWaveData = 0;
					for (int nDataIndex = 0; nDataIndex < nDataLength; ++nDataIndex)
					{
						sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "R_REG%d_%d_DATA%d", nREGByte[nREGModeIndex], nDataLength, nDataIndex + 1);
						ulWriteWaveData = (ulWriteData[nReadDataType][0] >> (24 - nDataIndex * 8)) & 0xFF;
						dcm.WriteWaveData("G_MUTUAL", lpszStartLabel, DCM_ALLSITE, 0, 8, ulWriteWaveData);
					}

				}
				else
				{
					//Data of all site is different from each other.
					bAllSiteSame = FALSE;
					ULONG ulWriteWaveData = 0;
					char lpszPinGroup[32] = { 0 };
					for (USHORT usSiteIndex = 0; usSiteIndex < usSiteCount; ++usSiteIndex)
					{
						sprintf_s(lpszPinGroup, sizeof(lpszPinGroup), "G_SITE%d", usSiteIndex % 4);

						for (int nDataIndex = 0; nDataIndex < nDataLength; ++nDataIndex)
						{
							sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "R_REG%d_%d_DATA%d", nREGByte[nREGModeIndex], nDataLength, nDataIndex + 1);
							ulWriteWaveData = (ulWriteData[nReadDataType][usSiteIndex] >> (24 - nDataIndex * 8)) & 0xFF;
							auto iterSlot = mapSlot.find(MutualSDA[usSiteIndex].m_bySlotNo);
							dcm.WriteWaveData(lpszPinGroup, lpszStartLabel, iterSlot->second + usSiteIndex / 4, 0, 8, ulWriteWaveData);
						}
					}
				}

				//Firstly, generate the I2C pattern vector.
				dcm.I2CReadData(bySlaveAddr, nREGAddr, 4);

				sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "RD_REG%d_%d_ST", nREGByte[nREGModeIndex], nDataLength);
				sts_sprintf(lpszStopLabel, sizeof(lpszStopLabel), "RD_REG%d_%d_SP", nREGByte[nREGModeIndex], nDataLength);
				nMutualTestVectorBaseLine = dcm_GetLabelLineNo(lpszStartLabel);
				dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);
				int nRAMOffset = nMutualTestVectorBaseLine - uStartLine;
				if (bWithDRAM)
				{
					nRAMOffset = nMutualTestVectorBaseLine + uLineCountBeforeOut;
				}
				else
				{
					bWithDRAM = bWithDRAM;
				}



				ModifyI2CPattern(lpszStartLabel, dPeriod, usSiteCount, MutualSCL, MutualSDA, 0xFFFF, bAllSiteSame);
				//Secondly, run the I2C pattern again, which has been added the mutual-test vector.
				dcm.I2CReadData(bySlaveAddr, nREGAddr, nDataLength);

				//Finally, compare the capture data of mutual-test with the data be written.
				int nOffset = 0;
				BYTE byCurData = 0;
				BOOL bFail = FALSE;
				for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
				{
					///<Check SCL
					int nSCLFailCount = dcm_GetChannelFailCount(MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel);
					if (0 > nSCLFailCount)
					{
						switch (nSCLFailCount)
						{
						case -1:
						case -2:
							///<Not will happened
							break;
						case -3:
							///<The channel is not existed
							funcReport.SaveAddtionMsg("The channel of S%d_%d is not existed", MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel);
							break;
						case -4:
							///<Not ran vector before
							funcReport.SaveAddtionMsg("Channel(S%d_%d) not ran vector before", MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel);
							break;
						case -5:
							///<Vector running
							funcReport.SaveAddtionMsg("Vector of S%d_%d is running", MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel);
							break;
						default:
							break;
						}
						continue;
					}
					if (nullptr != puSCLBRAMFailLineNo)
					{
						delete[] puSCLBRAMFailLineNo;
						puSCLBRAMFailLineNo = nullptr;
					}
					if (nullptr != puSCLDRAMFailLineNo)
					{
						delete[] puSCLDRAMFailLineNo;
						puSCLDRAMFailLineNo = nullptr;
					}

					try
					{
						puSCLBRAMFailLineNo = new UINT[nSCLFailCount];
						memset(puSCLBRAMFailLineNo, 0, nSCLFailCount * sizeof(UINT));
						if (bWithDRAM)
						{
							puSCLDRAMFailLineNo = new UINT[nSCLFailCount];
							memset(puSCLDRAMFailLineNo, 0, nSCLFailCount * sizeof(UINT));
						}
					}
					catch (const std::exception&)
					{
						funcReport.SaveAddtionMsg("Allocate memory for SCL fail line fail");
						continue;
					}

					nSCLBRAMFailCount = dcm_GetRAMFailLineNo(MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel, TRUE, nSCLFailCount, puSCLBRAMFailLineNo);
					if (bWithDRAM)
					{
						nSCLDRAMFailCount = dcm_GetRAMFailLineNo(MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel, !bWithDRAM, nSCLFailCount, puSCLDRAMFailLineNo);
					}


					bFail = FALSE;
					int nSDAFailCount = dcm_GetChannelFailCount(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel);
					if (0 > nSDAFailCount)
					{
						switch (nSDAFailCount)
						{
						case -1:
						case -2:
							///<Not will happened
							break;
						case -3:
							///<The channel is not existed
							funcReport.SaveAddtionMsg("The channel of S%d_%d is not existed", MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel);
							break;
						case -4:
							///<Not ran vector before
							funcReport.SaveAddtionMsg("Channel(S%d_%d) not ran vector before", MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel);
							break;
						case -5:
							///<Vector running
							funcReport.SaveAddtionMsg("Vector of S%d_%d is running", MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel);
							break;
						default:
							break;
						}
						continue;
					}
					if (nullptr != puSDABRAMFailLineNo)
					{
						delete[] puSDABRAMFailLineNo;
						puSDABRAMFailLineNo = nullptr;
					}

					if (nullptr != puSDADRAMFailLineNo)
					{
						delete[] puSDADRAMFailLineNo;
						puSDADRAMFailLineNo = nullptr;
					}
					try
					{
						puSDABRAMFailLineNo = new UINT[nSDAFailCount];
						memset(puSDABRAMFailLineNo, 0, nSDAFailCount * sizeof(UINT));
						if (bWithDRAM)
						{
							puSDADRAMFailLineNo = new UINT[nSDAFailCount];
							memset(puSDADRAMFailLineNo, 0, nSDAFailCount * sizeof(UINT));
						}
					}
					catch (const std::exception&)
					{
						continue;
					}
					nSDABRAMFailCount = dcm_GetRAMFailLineNo(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel, TRUE, nSDAFailCount, puSDABRAMFailLineNo);
					if (bWithDRAM)
					{
						nSDADRAMFailCount = dcm_GetRAMFailLineNo(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel, FALSE, nSDAFailCount, puSDADRAMFailLineNo);
					}
					///<Check Start sequence
					int nSCLBRAMFailIndex = 0;
					int nSCLDRAMFailIndex = 0;

					int nSDABRAMFailIndex = 0;
					int nSDADRAMFailIndex = 0;
					if (bWithDRAM)
					{
						int nBRAMFailIndex = 0;
						int nSCLBRAMFailIndex = 0;
						if (3 == uLineCountBeforeOut)
						{
							///<SCL start bits in BRAM
							bySCLRealData = GetData(uStartLine, 3, &nSCLBRAMFailIndex, nSCLBRAMFailCount, puSCLBRAMFailLineNo);
							///<SDA start bits in BRAM
							byRealData = GetData(uStartLine, 3, &nBRAMFailIndex, nSDABRAMFailCount, puSDABRAMFailLineNo);
						}
						else
						{
							///<SCL start bits in BRAM
							bySCLRealData = GetData(uStartLine, 2, &nSCLBRAMFailIndex, nSCLBRAMFailCount, puSCLBRAMFailLineNo);
							///<SDA start bits in BRAM
							byRealData = GetData(uStartLine, 2, &nBRAMFailIndex, nSDABRAMFailCount, puSDABRAMFailLineNo);
							byRealData <<= 1;
							bySCLRealData <<= 1;

							///<SCL start bits in DRAM
							BYTE byDRAMRealData = GetData(0, 1, &nSCLDRAMFailIndex, nSCLDRAMFailCount, puSCLDRAMFailLineNo);
							bySCLRealData |= byDRAMRealData;
							///<SDA start bits in DRAM
							byDRAMRealData = GetData(0, 1, &nSDADRAMFailIndex, nSDADRAMFailCount, puSDADRAMFailLineNo);
							byRealData |= byDRAMRealData;
						}
					}
					else
					{
						///<SCL start bits
						bySCLRealData = GetData(uStartLine, 3, &nSCLBRAMFailIndex, nSCLBRAMFailCount, puSCLBRAMFailLineNo);
						///<SDA start bits
						byRealData = GetData(uStartLine, 3, &nSDABRAMFailIndex, nSDABRAMFailCount, puSDABRAMFailLineNo);
					}
					XT_EXPECT_EQ(bySCLRealData, 0x07);
					if (0x07 != bySCLRealData)
					{
						funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d):Start sequence of SCL fail, Expected value is 0x%X, Real value is 0x%X",
							MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2,
							MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2, 0x07, bySCLRealData);
						funcReport.SaveFailChannel(MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2);
						bFail = TRUE;
					}

					XT_EXPECT_EQ(byRealData, 0x06);
					if (0x06 != byRealData)
					{
						funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d):Start sequence of SDA fail, Expected value is 0x%X, Real value is 0x%X",
							MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2, 0x06, byRealData);
						funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
						bFail = TRUE;
						continue;
					}
					UINT* puAddrFailLineNo = bWithDRAM ? puSDADRAMFailLineNo : puSDABRAMFailLineNo;
					int& nFailIndex = bWithDRAM ? nSDADRAMFailIndex : nSDABRAMFailIndex;
					int& nFailCount = bWithDRAM ? nSDADRAMFailCount : nSDABRAMFailCount;
					//Compare the Slave address
					int nAfterFirstSlaveFail = 0;
					for (int nSlaveIndex = 0; nSlaveIndex < 2; ++nSlaveIndex)
					{
						sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "R_REG%d_%d_SLA%d", nREGByte[nREGModeIndex], nDataLength, nSlaveIndex + 1);
						nOffset = dcm_GetLabelLineNo(lpszStartLabel) - nRAMOffset;

						byRealData = GetData(nOffset, 7, &nFailIndex, nFailCount, puAddrFailLineNo);
						if (0 == nSlaveIndex)
						{
							nAfterFirstSlaveFail = nFailIndex;
						}
						byCurData = (byRealData & 0xFF) << 1;
						XT_EXPECT_EQ(byCurData, bySlaveAddr);
						if (byCurData != bySlaveAddr)
						{
							funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Slave address(%d) fail, Expect value is 0x%X, Real value is 0x%X",
								MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
								nSlaveIndex + 1, bySlaveAddr, byCurData);
							funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
							bFail = TRUE;
							continue;
						}
					}
					nFailIndex = nAfterFirstSlaveFail;
					//Compare the Register address
					for (int nREGIndex = nREGModeIndex; nREGIndex >= 0; --nREGIndex)
					{
						sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "R_REG%d_%d_REG%d", nREGByte[nREGModeIndex], nDataLength, nREGModeIndex - nREGIndex + 1);
						nOffset = dcm_GetLabelLineNo(lpszStartLabel) - nRAMOffset;
						byRealData = GetData(nOffset, 8, &nFailIndex, nFailCount, puAddrFailLineNo);
						byCurData = (nREGAddr >> (nREGIndex * 8)) & 0xFF;
						XT_EXPECT_EQ(byRealData, byCurData);
						if (byRealData != byCurData)
						{
							funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Register address(%d) fail, Expect value is 0x%X, Real value is 0x%X",
								MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
								nREGIndex + 1, byCurData, byRealData);
							funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
							bFail = TRUE;
							break;
						}
					}
					if (bFail)
					{
						continue;
					}

					///<Check second start sequence
					sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "R_REG%d_%d_START", nREGByte[nREGModeIndex], nDataLength);
					nOffset = dcm_GetLabelLineNo(lpszStartLabel) - nRAMOffset;
					if (bWithDRAM)
					{
						///<SCL second start bits in DRAM
						bySCLRealData = GetData(nOffset, 3, &nSCLDRAMFailIndex, nSCLDRAMFailCount, puSCLDRAMFailLineNo);
					}
					else
					{
						///<SCL second start bits in BRAM
						bySCLRealData = GetData(nOffset, 3, &nSCLBRAMFailIndex, nSCLBRAMFailCount, puSCLBRAMFailLineNo);
					}
					///<SDA second start bits
					byRealData = GetData(nOffset, 3, &nFailIndex, nFailCount, puAddrFailLineNo);

					XT_EXPECT_EQ(bySCLRealData, 0x03);
					if (0x03 != bySCLRealData)
					{
						funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Second start sequence of SCL fail, Expected value is 0x%X, Real value is 0x%X",
							MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
							0x03, bySCLRealData);
						funcReport.SaveFailChannel(MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2);
						bFail = TRUE;
						continue;
					}

					XT_EXPECT_EQ(byRealData, 0x06);
					if (0x06 != byRealData)
					{
						funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Second start sequence of SDA fail, Expected value is 0x%X, Real value is 0x%X",
							MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
							0x06, byRealData);
						funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
						bFail = TRUE;
						continue;
					}

					//Compare the data written.
					for (int nDataIndex = 0; nDataIndex < nDataLength; ++nDataIndex)
					{
						sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "R_REG%d_%d_DATA%d", nREGByte[nREGModeIndex], nDataLength, nDataIndex + 1);
						nOffset = dcm_GetLabelLineNo(lpszStartLabel) - nMutualTestVectorBaseLine;
						byRealData = dcm.I2CGetReadData(usSiteNo, nDataIndex);
						byCurData = (ulWriteData[nReadDataType][usSiteNo] >> (24 - nDataIndex * 8)) & 0xFF;
						XT_EXPECT_EQ((BYTE)byRealData, byCurData);
						if (byRealData != byCurData)
						{
							funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Data(%d) fail, Expect value is 0x%X, Real value is 0x%X",
								MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
								nDataIndex + 1, byCurData, byRealData);
							funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
							bFail = TRUE;
							break;
						}
					}

					///<Check the last ACK after data
					sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "R_REG%d_%d_STOP", nREGByte[nREGModeIndex], nDataLength);
					nOffset = dcm_GetLabelLineNo(lpszStartLabel) - 1 - nRAMOffset;
					byRealData = GetData(nOffset, 1, &nFailIndex, nFailCount, puAddrFailLineNo);
					XT_EXPECT_EQ(byRealData, 0x01);
					if (0x01 != byRealData)
					{
						funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): The last ACK, Expected value is 0x%X, Real value is 0x%X",
							MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
							0x01, byRealData);
						funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
						bFail = TRUE;
						continue;
					}


					///<Check Stop sequence
					if (bWithDRAM)
					{
						UINT uStopLineInBRAMCount = uLineCount - uLineCountBeforeOut;
						///<SCL stop bits in DRAM
						bySCLRealData = GetData(uDRAMLineCount + uStopLineInBRAMCount - 4, 4 - uStopLineInBRAMCount, &nSCLDRAMFailIndex, nSCLDRAMFailCount, puSCLDRAMFailLineNo);
						bySCLRealData <<= uStopLineInBRAMCount;
						///<SDA stop bits in DRAM
						byRealData = GetData(uDRAMLineCount + uStopLineInBRAMCount - 4, 4 - uStopLineInBRAMCount, &nSDADRAMFailIndex, nSDABRAMFailCount, puSDADRAMFailLineNo);
						byRealData <<= uStopLineInBRAMCount;

						///<SCL stop bits in BRAM
						BYTE byCurData = GetData(uStartLine + uLineCountBeforeOut, uStopLineInBRAMCount, &nSCLBRAMFailIndex, nSCLBRAMFailCount, puSCLBRAMFailLineNo);
						bySCLRealData |= byCurData;
						///<SDA stop bits in BRAM
						byCurData = GetData(uStartLine + uLineCountBeforeOut, uStopLineInBRAMCount, &nSDABRAMFailIndex, nSDABRAMFailCount, puSDABRAMFailLineNo);
						byRealData |= byCurData;

					}
					else
					{
						sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "R_REG%d_%d_STOP", nREGByte[nREGModeIndex], nDataLength);
						nOffset = dcm_GetLabelLineNo(lpszStartLabel) - nRAMOffset;
						///<SCL stop bits
						bySCLRealData = GetData(nOffset, 4, &nSCLBRAMFailIndex, nSCLBRAMFailCount, puSCLBRAMFailLineNo);
						///<SDA stop bits
						byRealData = GetData(nOffset, 4, &nSDABRAMFailIndex, nSDABRAMFailCount, puSDABRAMFailLineNo);
					}
					XT_EXPECT_EQ(bySCLRealData, 0x07);
					if (0x07 != bySCLRealData)
					{
						funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Stop sequence of SCL fail, Expected value is 0x%X, Real value is 0x%X",
							MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
							0x07, bySCLRealData);
						funcReport.SaveFailChannel(MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2);
						bFail = TRUE;
					}
					XT_EXPECT_EQ(byRealData, 0x03);
					if (0x03 != byRealData)
					{
						funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Stop sequence of SDA fail, Expected value is 0x%X, Real value is 0x%X",
							MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
							0x03, byRealData);
						funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
						bFail = TRUE;
					}
					if (bFail)
					{
						continue;
					}
					///<Check clock sequence of SCL
					XT_EXPECT_EQ(nSCLFailCount, 8);
					if (8 < nSCLFailCount)
					{
						funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Data sequence of SCL fail, Expected value is 0x%X, Real value is 0x%X",
							MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
							(int)0x08, (int)nSCLFailCount);
					}
				}
			}
		}
		dcm.Disconnect("G_ALLPIN");
		dcm_CloseFile();
		dcm_I2CDeleteMemory();
	}
	if (nullptr != puSCLBRAMFailLineNo)
	{
		delete[] puSCLBRAMFailLineNo;
		puSCLBRAMFailLineNo = nullptr;
	}
	if (nullptr != puSCLDRAMFailLineNo)
	{
		delete[] puSCLDRAMFailLineNo;
		puSCLDRAMFailLineNo = nullptr;
	}

	if (nullptr != puSDABRAMFailLineNo)
	{
		delete[] puSDABRAMFailLineNo;
		puSDABRAMFailLineNo = nullptr;
	}
	if (nullptr != puSDADRAMFailLineNo)
	{
		delete[] puSDADRAMFailLineNo;
		puSDADRAMFailLineNo = nullptr;
	}

	dcm_I2CEnableUncompare(TRUE);

	GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	dcm.I2CSet(200, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	USHORT usInvalidSite = usSiteCount / 2;

	char lpszPinName[32] = { 0 };
	set<USHORT> setPin;
	string strPinList;
	USHORT usPin = 0;
	auto GetPinList = [&](const CHANNEL_INFO* pChannel)
	{
		for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
		{
			usPin = pChannel[usSiteNo].m_usChannel % DCM_CHANNELS_PER_CONTROL;
			if (setPin.end() != setPin.find(usPin))
			{
				continue;
			}
			sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usPin);
			strPinList += lpszPinName;
			strPinList += ",";
			setPin.insert(usPin);
		}
	};

	GetPinList(MutualSCL);
	GetPinList(MutualSDA);
	dcm.SetPinGroup("G_PMU", strPinList.c_str());
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	dcm.SetChannelStatus("G_ALLPIN", DCM_ALLSITE, DCM_HIGH_IMPEDANCE);

	dcm.Connect("G_ALLPIN");
	InvalidSite(usInvalidSite);
	DWORD dwData[32] = { 0 };
	dcm.I2CReadData(0x20, 0x00, 1);
	RestoreSite();
	dcm.SetPPMU("G_PMU", DCM_PPMU_FIMV, 1e-3, DCM_PPMUIRANGE_2MA);
	dcm.PPMUMeasure("G_PMU", 10, 10);
	double dTargetVoltage = 0;
	auto iterSlot = mapSlot.begin();
	double dMeasResult = 0;
	auto GetPMUResult = [&](const CHANNEL_INFO& Channel)
	{
		iterSlot = mapSlot.find(Channel.m_bySlotNo);
		if (mapSlot.end() == iterSlot)
		{
			return MAX_MEASURE_VALUE;
		}
		USHORT usSiteNo = iterSlot->second + Channel.m_usChannel / DCM_CHANNELS_PER_CONTROL;
		sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", Channel.m_usChannel % DCM_CHANNELS_PER_CONTROL);
		return dcm.GetPPMUMeasResult(lpszPinName, usSiteNo);
	};

	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		dTargetVoltage = 3;
		if (usSiteNo == usInvalidSite)
		{
			dTargetVoltage = 7.5;
		}
		dMeasResult = GetPMUResult(MutualSCL[usSiteNo]);
		XT_EXPECT_REAL_EQ(dMeasResult, dTargetVoltage, 0.1);
		if (0.1 < fabs(dMeasResult - dTargetVoltage))
		{
			funcReport.SaveFailChannel(vecSCLChannel[usSiteNo].m_bySlotNo, vecSCLChannel[usSiteNo].m_usChannel);
		}
		dMeasResult = GetPMUResult(MutualSDA[usSiteNo]);
		XT_EXPECT_REAL_EQ(dMeasResult, dTargetVoltage, 0.1);
		if (0.1 < fabs(dMeasResult - dTargetVoltage))
		{
			funcReport.SaveFailChannel(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel);
		}
	}




	funcReport.AddTestItem("Check Invalid Site_I2CGetReadData");

	InvalidSite(usInvalidSite);
	ULONG ulTargetBitData = 0;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		nRetVal = dcm.I2CGetReadData(usSiteNo, 0);
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

	vecSDAChannel.clear();
	vecSCLChannel.clear();

	funcReport.Print(this, g_lpszReportFilePath);
}