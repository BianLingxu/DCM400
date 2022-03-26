#pragma once
/*!
* @file      TestDCMI2CWriteMultiDataFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/13
* @version   v 1.0.0.0
* @brief     测试I2CWriteMultiData功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMI2CWriteMultiDataFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");

	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, nullptr);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		funcReport.SetNoBoardValid();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(funcReport, mapSlot);
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


	const int nTestCount = 2;
	const char* lpszFilePath[nTestCount] = { g_lpszI2CVectorFilePath, g_lpszVectorFilePath };


	char lpszStartLabel[32] = { 0 };
	const int nREGModeTestCount = 4;
	I2C_REG_ADDR_MODE testREG[nREGModeTestCount] = { DCM_REG8, DCM_REG16, DCM_REG24, DCM_REG32 };
	int nREGByte[nREGModeTestCount] = { 0 };
	for (int nIndex = 0; nIndex < nREGModeTestCount; ++nIndex)
	{
		nREGByte[nIndex] = (testREG[nIndex] + 1) * 8;
	}
	int nMutualTestVectorBaseLine = 0;
	BYTE bySlaveAddr = 0xA6;
	BYTE bySCLRealData = 0;
	int nREGAddr = 0xA5A555AA;
	double dPeriod = 2000;
	BOOL bBRAMOccupy = FALSE;

	const BYTE byDataTypeCount = 2;

	const int nMaxDataLength = 5;
	const int nDataTestCount = 2;
	int nTestDataLength[nDataTestCount] = { 4, nMaxDataLength };

	BYTE byWriteData[SITE_NUM][byDataTypeCount][nMaxDataLength] = { 0 };
	BYTE bySameData[nMaxDataLength] = { 0xAA,0x55,0xA5,0x5A,0x0F };
	BYTE* pbySameData[SITE_NUM] = { nullptr };
	BYTE* pbyDifferentData[SITE_NUM] = { nullptr };

	dcm_I2CEnableUncompare(FALSE);
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		pbySameData[usSiteNo] = byWriteData[usSiteNo][0];
		pbyDifferentData[usSiteNo] = byWriteData[usSiteNo][0];
		for (int nDataType = 0; nDataType < byDataTypeCount; ++nDataType)
		{
			for (int nIndex = 0; nIndex < nDataTestCount; ++nIndex)
			{
				if (0 == nDataType)
				{
					byWriteData[usSiteNo][nDataType][nIndex] = bySameData[nIndex];
				}
				else
				{
					byWriteData[usSiteNo][nDataType][nIndex] = 0x55 + usSiteNo;
				}
			}
		}
	}


	UINT* puSCLBRAMFailLineNo = nullptr;
	UINT* puSCLDRAMFailLineNo = nullptr;
	int nSCLFailCount = 0;
	int nSCLBRAMFailCount = 0;
	int nSCLDRAMFailCount = 0;

	UINT* puSDABRAMFailLineNo = nullptr;
	UINT* puSDADRAMFailLineNo = nullptr;
	int nSDAFailCount = 0;
	int nSDABRAMFailCount = 0;
	int nSDADRAMFailCount = 0;

	for (int nTestIndex = 0; nTestIndex < nTestCount; ++nTestIndex)
	{
		dcm.LoadVectorFile(lpszFilePath[nTestIndex], FALSE);
		dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
		dcm.Connect("G_ALLPIN");
		dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);
		dcm.InitMCU("G_ALLPIN");
		for (int nDataTestIndex = 0; nDataTestIndex < nDataTestCount; ++nDataTestIndex)
		{
			int nCurDataLength = nTestDataLength[nDataTestIndex];
			for (int nREGModeIndex = 0; nREGModeIndex < nREGModeTestCount; ++nREGModeIndex)
			{
				bBRAMOccupy = FALSE;
				dcm.I2CSet(dPeriod, usSiteCount, testREG[nREGModeIndex], strSCLChannel.c_str(), strSDAChannel.c_str());
				dcm.I2CSetPinLevel(3, 0, 1.5, 0.8);
				for (int nWriteDataType = 0; nWriteDataType < byDataTypeCount; ++nWriteDataType)
				{
					UINT uStartLine = 0;
					UINT uLineCount = 0;
					BOOL bWithDRAM = FALSE;
					UINT uLineCountBeforeOut = 0;
					UINT uDRAMStartLine = 0;
					UINT uDRAMLineCount = 0;
					if (0 == nTestIndex)
					{
						if (0 == nWriteDataType)
						{
							funcReport.AddTestItem("I2C: BRAM. REG size:%d bits. Data: %d bytes all same.", nREGByte[nREGModeIndex], nCurDataLength);
						}
						else
						{
							funcReport.AddTestItem("I2C: BRAM. REG size:%d bits. Data: %d bytes all different.", nREGByte[nREGModeIndex], nCurDataLength);
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
							for (int nSiteIndex = 0; nSiteIndex < MAX_SITE; ++nSiteIndex)
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
						if (0 == nWriteDataType)
						{
							funcReport.AddTestItem("I2C: DRAM. REG size:%d bits. Data: %d bytes all same.", nREGByte[nREGModeIndex], nCurDataLength);
						}
						else
						{
							funcReport.AddTestItem("I2C: DRAM. REG size:%d bits. Data: %d bytes all different.", nREGByte[nREGModeIndex], nCurDataLength);
						}
					}

					BYTE** ppbyWriteData = nullptr;
					//Firstly, generate the I2C pattern vector.
					if (0 == nWriteDataType)
					{
						ppbyWriteData = pbySameData;
					}
					else
					{
						ppbyWriteData = pbyDifferentData;
					}
					dcm.I2CWriteMultiData(bySlaveAddr, nREGAddr, nCurDataLength, ppbyWriteData);

					sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "WT_REG%d_%d_ST", nREGByte[nREGModeIndex], nCurDataLength);
					nMutualTestVectorBaseLine = dcm_GetLabelLineNo(lpszStartLabel);

					dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);
					int nRAMOffset = nMutualTestVectorBaseLine - uStartLine;
					if (bWithDRAM)
					{
						nRAMOffset = nMutualTestVectorBaseLine + uLineCountBeforeOut;
					}

					ModifyI2CPattern(lpszStartLabel, dPeriod, usSiteCount, MutualSCL, MutualSDA);

					//Secondly, run the I2C pattern again, which has been added the mutual-test vector.
					dcm.I2CWriteMultiData(bySlaveAddr, nREGAddr, nCurDataLength, ppbyWriteData);

					//Finally, compare the capture data of mutual-test with the data be written.
					int nOffset = 0;
					BYTE byExpectData = 0;
					BOOL bFail = FALSE;
					BYTE byRealData = 0;
					for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
					{
						bFail = FALSE;
						///<Check SCL
						nSCLFailCount = dcm_GetChannelFailCount(MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel);
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
							nSCLDRAMFailCount = dcm_GetRAMFailLineNo(MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel, FALSE, nSCLFailCount, puSCLDRAMFailLineNo);
						}

						bFail = FALSE;
						nSDAFailCount = dcm_GetChannelFailCount(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel);
						XT_EXPECT_LESSEQ(0, nSDAFailCount);
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
						bFail = FALSE;
						///<Check the start and stop pattern
						int nSCLBRAMFailIndex = 0;
						int nSCLDRAMFailIndex = 0;
						int nSDABRAMFailIndex = 0;
						int nSDADRAMFailIndex = 0;
						UINT*& puSDADataFailLineNo = bWithDRAM ? puSDADRAMFailLineNo : puSDABRAMFailLineNo;
						int& nSDADataFailIndex = bWithDRAM ? nSDADRAMFailIndex : nSDABRAMFailIndex;
						int& nSDADataFailCount = bWithDRAM ? nSDADRAMFailCount : nSDABRAMFailCount;
						if (bWithDRAM)
						{
							if (3 == uLineCountBeforeOut)
							{
								///<SCL start bits in BRAM
								bySCLRealData = GetData(uStartLine, 3, &nSCLBRAMFailIndex, nSCLBRAMFailCount, puSCLBRAMFailLineNo);
								///<SDA start bits in BRAM
								byRealData = GetData(uStartLine, 3, &nSDABRAMFailIndex, nSDABRAMFailCount, puSDABRAMFailLineNo);
							}
							else
							{
								///<SCL start bits in BRAM
								bySCLRealData = GetData(uStartLine, 2, &nSCLBRAMFailIndex, nSCLBRAMFailCount, puSCLBRAMFailLineNo);
								///<SDA start bits in BRAM
								byRealData = GetData(uStartLine, 2, &nSDABRAMFailIndex, nSDABRAMFailCount, puSDABRAMFailLineNo);
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

						//Compare the Slave address.
						sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "W_REG%d_%d_SLA", nREGByte[nREGModeIndex], nCurDataLength);
						nOffset = dcm_GetLabelLineNo(lpszStartLabel) - nRAMOffset;
						//nFailIndex = 0;
						byRealData = GetData(nOffset, 7, &nSDADataFailIndex, nSDADataFailCount, puSDADataFailLineNo);
						byRealData <<= 1;
						XT_EXPECT_EQ(byRealData, bySlaveAddr);
						if (byRealData != bySlaveAddr)
						{
							funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Slave address fail, Expect value is 0x%X, Real value is 0x%X",
								MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
								bySlaveAddr, byRealData);
							funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
							bFail = TRUE;
							continue;
						}

						//Compare the Register address
						for (int nREGIndex = nREGModeIndex; nREGIndex >= 0; --nREGIndex)
						{
							sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "W_REG%d_%d_REG%d", nREGByte[nREGModeIndex], nCurDataLength, nREGModeIndex - nREGIndex + 1);
							nOffset = dcm_GetLabelLineNo(lpszStartLabel) - nRAMOffset;
							byRealData = GetData(nOffset, 8, &nSDADataFailIndex, nSDADataFailCount, puSDADataFailLineNo);
							byExpectData = (nREGAddr >> (nREGIndex * 8)) & 0xFF;
							XT_EXPECT_EQ(byRealData, byExpectData);
							if (byRealData != byExpectData)
							{
								funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Register address(%d) fail, Expect value is 0x%X, Real value is 0x%X",
									MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
									nREGIndex + 1, byExpectData, byRealData);
								funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
								bFail = TRUE;
								break;
							}
						}

						if (bFail)
						{
							continue;
						}
						//Compare the data written.
						ULONG ulCurCompareData = 0;
						for (int nDataIndex = 0; nDataIndex < nCurDataLength; ++nDataIndex)
						{
							sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "W_REG%d_%d_DATA%d", nREGByte[nREGModeIndex], nCurDataLength, nDataIndex + 1);
							nOffset = dcm_GetLabelLineNo(lpszStartLabel) - nRAMOffset;
							byRealData = GetData(nOffset, 8, &nSDADataFailIndex, nSDADataFailCount, puSDADataFailLineNo);
							byExpectData = ppbyWriteData[usSiteNo][nDataIndex];
							XT_EXPECT_EQ(byRealData, byExpectData);
							if (byRealData != byExpectData)
							{
								funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Data(%d) fail, Expect value is 0x%X, Real value is 0x%X",
									MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
									nDataIndex + 1, byExpectData, byRealData);
								funcReport.SaveFailChannel(MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2);
								bFail = TRUE;
								break;
							}
						}

						///<Check Stop sequence
						if (bWithDRAM)
						{
							UINT uStopLineInBRAMCount = uLineCount - uLineCountBeforeOut;
							///<SCL stop bits in DRAM
							bySCLRealData = GetData(uDRAMLineCount + uStopLineInBRAMCount - 4, 4 - uStopLineInBRAMCount, &nSCLDRAMFailIndex, nSCLDRAMFailCount, puSCLDRAMFailLineNo);
							bySCLRealData <<= uStopLineInBRAMCount;
							///<SDA stop bits in DRAM
							byRealData = GetData(uDRAMLineCount + uStopLineInBRAMCount - 4, 4 - uStopLineInBRAMCount, &nSDADRAMFailIndex, nSDADRAMFailCount, puSDADRAMFailLineNo);
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
							sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "W_REG%d_%d_STOP", nREGByte[nREGModeIndex], nCurDataLength);
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
								(int)0x07, (int)bySCLRealData);
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
						}
						if (bFail)
						{
							continue;
						}
						///<Check clock sequence of SCL
						XT_EXPECT_EQ(nSCLFailCount, 6);
						if (6 < nSCLFailCount)
						{
							funcReport.SaveAddtionMsg("SCLChannel(S%d_%d),SDAChannel(S%d_%d): Data sequence of SCL fail, Expected value is 0x%X, Real value is 0x%X",
								MutualSCL[usSiteNo].m_bySlotNo, MutualSCL[usSiteNo].m_usChannel - 2, MutualSDA[usSiteNo].m_bySlotNo, MutualSDA[usSiteNo].m_usChannel - 2,
								(int)0x06, (int)nSCLFailCount);
						}
					}
				}
			}
		}
		dcm.Disconnect("G_ALLPIN");
		dcm_CloseFile();
		dcm_I2CDeleteMemory();
	}
	dcm_I2CEnableUncompare(TRUE);

	funcReport.AddTestItem("Check Invalid Site");

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
	BYTE byData[1] = { 0 };

	BYTE* ppbyData[32] = { 0 };
	for (auto& pbyData : ppbyData)
	{
		pbyData = byData;
	}

	dcm.I2CWriteMultiData(0x20, 0x00, 1, ppbyData);
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


	dcm_I2CDeleteMemory();
	dcm.Disconnect("G_ALLPIN");


	vecSDAChannel.clear();
	vecSCLChannel.clear();
	funcReport.Print(this, g_lpszReportFilePath);
}