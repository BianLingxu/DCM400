#pragma once
/*!
* @file      TestDCMI2CGetNACKIndexRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/17
* @version   v 1.0.0.0
* @brief     测试I2CGetNACKIndex运行时间
* @comment
*/
#include "..\DCMTestMain.h"

void NACKTimeTest(CTimeReport& timeReport, double dPeriod, CHANNEL_INFO* pSCLMutual, CHANNEL_INFO* pSDAMutual, int nSiteCount, int nControlCount, BOOL bSaveInBRAM)
{
	ULONG ulWriteData[4] = { 0xAABBCCDD,0xCCAABBDD,0xBBCCDDEE,0xAACCBBAA };
	ULONG *ulWriteDataArray[4] = { ulWriteData ,ulWriteData ,ulWriteData,ulWriteData };

	BYTE pbySameData[10] = { 0xAA, 0xBB, 0xCC, 0xDD,0xAA, 0xBB, 0xCC, 0xDD,0xAA, 0xBB };

	BYTE* ppbyWriteDataArray[4] = { pbySameData ,pbySameData ,pbySameData,pbySameData };//The data written of each site are different.
	 
	const int nWriteTypeCount = 2;
	int nWriteDataBytes[2] = { 1,4 };
	const char * lpszType[2] = { "BRAM", "DRAM" };
	int nCurType = 0;
	if (!bSaveInBRAM)
	{
		nCurType = 1;
	}
	char lpszStartLabel[32] = { 0 };
	int nNACKIndex = 0;

	for (int nIndex = 0; nIndex < nWriteTypeCount; ++nIndex)
	{
		dcm.I2CReadData(2, 0, nWriteDataBytes[nIndex]);
		sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "RD_REG8_%d_ST", nWriteDataBytes[nIndex]);


		ModifyI2CPattern(lpszStartLabel, dPeriod, nSiteCount, pSCLMutual, pSDAMutual);


		dcm.I2CReadData(2, 0, nWriteDataBytes[nIndex]);

		timeReport.timeStart();
		nNACKIndex = dcm.I2CGetNACKIndex(0);
		timeReport.timeStop();

		timeReport.addMsg("site Count:%d, Controller Count:%d, %d bytes data, I2C %s, in %s, NACK index: %d", nSiteCount, nControlCount, nWriteDataBytes[nIndex], "Read", lpszType[nCurType], nNACKIndex);

		dcm.I2CWriteData(2, 0, nWriteDataBytes[nIndex], ulWriteData);
		sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "WT_REG8_%d_ST", nWriteDataBytes[nIndex]);
		dcm.I2CWriteData(2, 0, nWriteDataBytes[nIndex], ulWriteData);
		ModifyI2CPattern(lpszStartLabel, 200, nSiteCount, pSCLMutual, pSDAMutual);

		timeReport.timeStart();
		nNACKIndex = dcm.I2CGetNACKIndex(0);
		timeReport.timeStop();

		timeReport.addMsg("site Count:%d, Controller Count:%d, %d bytes data, I2C %s, in %s, NACK index: %d", nSiteCount, nControlCount, nWriteDataBytes[nIndex], "Write", lpszType[nCurType], nNACKIndex);

		if (nWriteTypeCount == nIndex + 1)
		{
			continue;
		}
		dcm.I2CWriteMultiData(2, 0, nWriteDataBytes[1] + 1, ppbyWriteDataArray);
		sts_sprintf(lpszStartLabel, sizeof(lpszStartLabel), "WT_REG8_%d_ST", nWriteDataBytes[1] + 1);
		ModifyI2CPattern(lpszStartLabel, 200, nSiteCount, pSCLMutual, pSDAMutual);

		dcm.I2CWriteMultiData(2, 0, nWriteDataBytes[1] + 1, ppbyWriteDataArray);

		timeReport.timeStart();
		nNACKIndex = dcm.I2CGetNACKIndex(0);
		timeReport.timeStop();

		timeReport.addMsg("site Count:%d, Controller Count:%d, %d bytes data, I2C %s, in %s, NACK index: %d", nSiteCount, nControlCount, nWriteDataBytes[1] + 1, "MultiWrite", lpszType[nCurType], nNACKIndex);
	}
}

inline int GetMutualChannel(const vector<CHANNEL_INFO>& vecSDAChannel, const vector<CHANNEL_INFO>& vecSCLChannel, CHANNEL_INFO* pSCLMutual, CHANNEL_INFO* pSDAMutual, int nArrayLength)
{
	if (vecSDAChannel.size() != vecSDAChannel.size())
	{
		return -1;
	}
	if (0 == vecSDAChannel.size() || 0 == vecSDAChannel.size())
	{
		return 0;
	}
	int nChannelCount = vecSCLChannel.size();
	if (nullptr == pSCLMutual || nullptr == pSDAMutual || 0 == nArrayLength)
	{
		return vecSCLChannel.size();
	}
	if (nChannelCount > nArrayLength)
	{
		return -2;
	}

	USHORT usSiteCount = vecSCLChannel.size();
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		pSDAMutual[usSiteNo].m_bySlotNo = vecSDAChannel[usSiteNo].m_bySlotNo;
		pSDAMutual[usSiteNo].m_usChannel = vecSDAChannel[usSiteNo].m_usChannel + 2;
		pSCLMutual[usSiteNo].m_bySlotNo = vecSCLChannel[usSiteNo].m_bySlotNo;
		pSCLMutual[usSiteNo].m_usChannel = vecSCLChannel[usSiteNo].m_usChannel + 2;
	}

	return nChannelCount;
}

XT_TEST(FunctionRunningTimeTest, TestDCMI2CGetNACKIndexRunningTime)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, RUNNING_TIME);
	CTimeReport timeReport(strFuncName.c_str(), "FunctionRunningTimeTest");

	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		timeReport.SetNoBoardValid();
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		timeReport.addMsg("Load vector file(%s) fail, the vector file maybe not right.", g_lpszVectorFilePath);
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(timeReport, mapSlot);
	auto iterSlot = mapSlot.begin();

	string strSCLChannel;
	string strSDAChannel;

	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	CHANNEL_INFO MutualSCL[MAX_SITE];
	CHANNEL_INFO MutualSDA[MAX_SITE];
	//************************************One Site(BRAM)**********************************************//
	dcm.LoadVectorFile(g_lpszI2CVectorFilePath);
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN",3.0, 0.0, 1.5, 0.8);

	double dPeriod = 200;

	GetI2CChannel(mapSlot, 1, 1, strSCLChannel, strSDAChannel, &vecSCLChannel, &vecSDAChannel);
	dcm.I2CSet(dPeriod, 1, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	GetMutualChannel(vecSDAChannel, vecSCLChannel, MutualSCL, MutualSDA, 4);
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	NACKTimeTest(timeReport, dPeriod, MutualSCL, MutualSDA, 1, 1, TRUE);

	//************************************QuadSites in one controller(BRAM)**********************************************//
	dcm_I2CDeleteMemory();
	GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	GetMutualChannel(vecSDAChannel, vecSCLChannel, MutualSCL, MutualSDA, 4);

	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	NACKTimeTest(timeReport, dPeriod, MutualSCL, MutualSDA, 4, 1, TRUE);

	//************************************QuadSites in four controllers(BRAM)**********************************************//
	dcm_I2CDeleteMemory();
	GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	GetMutualChannel(vecSDAChannel, vecSCLChannel, MutualSCL, MutualSDA, 4);
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	NACKTimeTest(timeReport, dPeriod, MutualSCL, MutualSDA, 4, 4, TRUE);

	//************************************One Site(DRAM)**********************************************//
	dcm.LoadVectorFile(g_lpszVectorFilePath);//Load the vector, in order to save i2c pattern to DRAM

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3.0, 0.0, 1.5, 0.8);

	GetI2CChannel(mapSlot, 1, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 1, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	GetMutualChannel(vecSDAChannel, vecSCLChannel, MutualSCL, MutualSDA, 4);
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	for (int nIndex = 1; nIndex < 50; ++nIndex)
	{
		dcm.I2CReadData(0xA0, nIndex, 1);
	}
	NACKTimeTest(timeReport, dPeriod, MutualSCL, MutualSDA, 1, 1, FALSE);

	//************************************Quad Sites in one controller(DRAM)**********************************************//
	dcm_I2CDeleteMemory();
	GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	GetMutualChannel(vecSDAChannel, vecSCLChannel, MutualSCL, MutualSDA, 4);
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);	
	for (int nIndex = 1; nIndex < 50; ++nIndex)
	{
		dcm.I2CReadData(0xA0, nIndex, 1);
	}
	NACKTimeTest(timeReport, dPeriod, MutualSCL, MutualSDA, 4, 1, FALSE);

	//************************************Quad Sites in four controllers(DRAM)**********************************************//
	dcm_I2CDeleteMemory();
	GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	GetMutualChannel(vecSDAChannel, vecSCLChannel, MutualSCL, MutualSDA, 4);
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	for (int nIndex = 1; nIndex < 50; ++nIndex)
	{
		dcm.I2CReadData(0xA0, nIndex, 1);
	}
	NACKTimeTest(timeReport, dPeriod, MutualSCL, MutualSDA, 4, 4, FALSE);

	mapSlot.clear();
	vecSDAChannel.clear();
	vecSCLChannel.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}
