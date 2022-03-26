#pragma once
/**
 * @file TestDCMI2CSetStopStatusFunction.h
 * @brief Check the function of I2CSetStopStatus
 * @author Guangyun Wang
 * @date 2021/06/07
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/

#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMI2CSetStopStatusFunction)
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
	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT usSiteCount = vecSCLChannel.size();

	vector<CHANNEL_INFO> vecI2CChannel = vecSCLChannel;
	for (auto Channel : vecSDAChannel)
	{
		vecI2CChannel.push_back(Channel);
	}

	char lpszPin[32] = { 0 };
	auto GetPinName = [&](USHORT usChannel)->const char*
	{
		sprintf_s(lpszPin, sizeof(lpszPin), "CH%d", usChannel % DCM_CHANNELS_PER_CONTROL);
		return lpszPin;
	};

	
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	
	UINT uStartLine = 0;
	UINT uLineCount = 0;
	BOOL bWithDRAM = FALSE;
	UINT uLineCountBeforeOut = 0;
	UINT uDRAMStartLine = 0;
	UINT uDRAMLineCount = 0;
	char lpszPattern[2][17] = { 0 };
	auto CheckPattern = [&](CHANNEL_OUTPUT_STATUS ChannelStatus, BOOL bExcludeChannel, vector<CHANNEL_INFO>* pvecChannel)
	{
		char cSignExpected = CHANNEL_OUTPUT_STATUS::HIGH == ChannelStatus ? '1' : 'X';
		dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);


		vector<CHANNEL_INFO>* pvecCheckChannel = bExcludeChannel ? &vecI2CChannel : pvecChannel;

		for (auto& Channel : *pvecCheckChannel)
		{
			if (bExcludeChannel && nullptr != pvecChannel)
			{
				BOOL bUncheck = FALSE;
				for (auto& CurChannel : *pvecCheckChannel)
				{
					if (Channel.m_bySlotNo == CurChannel.m_bySlotNo && Channel.m_usChannel == CurChannel.m_usChannel)
					{
						bUncheck = TRUE;
						break;
					}
				}
				if (bUncheck)
				{
					continue;
				}
			}
			BYTE byController = Channel.m_usChannel / DCM_CHANNELS_PER_CONTROL;
			dcm_GetPattern(Channel.m_bySlotNo, byController, TRUE, uStartLine + uLineCount - 1, 1, lpszPattern);
			char cRealSign = lpszPattern[0][Channel.m_usChannel % DCM_CHANNELS_PER_CONTROL];
			XT_EXPECT_EQ(cRealSign, cSignExpected);
			if (cSignExpected != cRealSign)
			{
				funcReport.SaveFailChannel(Channel.m_bySlotNo, Channel.m_usChannel);
			}
		}
	};
	BYTE bySlaveChecked = 0xAA;
	ULONG ulREGChecked = 0xA0;
	///<Check BRAM
	funcReport.AddTestItem("I2CReadData  in BRAM Stop with high driver");
	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(3.0, 0, 2.5, 0.8);
	dcm.I2CReadData(bySlaveChecked, ulREGChecked, 10);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, nullptr);



	BYTE** ppbyData = nullptr;
	ppbyData = new BYTE * [usSiteCount];
	ULONG* pulWriteData = nullptr;
	pulWriteData = new ULONG[usSiteCount];
	memset(pulWriteData, 0xAA, usSiteCount * sizeof(ULONG));
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		ppbyData[usSiteNo] = new BYTE[5];
		memset(ppbyData[usSiteNo], 0xAA, 5 * sizeof(BYTE));
	}

	funcReport.AddTestItem("I2CWriteMData in BRAM Stop with high impedance");
	dcm.I2CWriteData(bySlaveChecked, ulREGChecked, 3, pulWriteData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteMultiData in BRAM Stop with high driver");
	dcm.I2CWriteMultiData(bySlaveChecked, ulREGChecked, 5, ppbyData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, nullptr);

	funcReport.AddTestItem("I2CRead in BRAM Stop with high impedance");
	dcm.I2CSetStopStatus(TRUE);
	dcm.I2CReadData(bySlaveChecked, ulREGChecked, 10);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteData in BRAM Stop with high impedance");
	dcm.I2CWriteData(bySlaveChecked, ulREGChecked, 3, pulWriteData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteMultiData in BRAM Stop with high impedance");
	dcm.I2CWriteMultiData(bySlaveChecked, ulREGChecked, 5, ppbyData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	///<Check DRAM
	dcm_I2CDeleteMemory();
	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(3.0, 0, 2.5, 0.8);

	BYTE bySlaveAddr = 0;
	ULONG ulREG = 0;
	int nDataCount = 5;
	do
	{
		dcm.I2CReadData(bySlaveAddr++, ulREG++, nDataCount);
		dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);
	} while (!bWithDRAM);

	funcReport.AddTestItem("I2CReadData with DRAM Stop with high driver at first");
	dcm.I2CReadData(bySlaveChecked, ulREGChecked, 10);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteData with DRAM Stop with high driver at first");
	dcm.I2CWriteMultiData(bySlaveChecked, ulREGChecked, 5, ppbyData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteMultiData with DRAM Stop with high driver at first");

	dcm.I2CWriteMultiData(bySlaveChecked, ulREGChecked, 5, ppbyData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, nullptr);

	funcReport.AddTestItem("I2CReadData with DRAM Stop with high impedance at first, check high impedance");
	dcm.I2CSetStopStatus(TRUE);
	dcm.I2CReadData(bySlaveChecked, ulREGChecked, 10);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteData with DRAM Stop with high impedance at first, check high impedance");
	dcm.I2CWriteData(bySlaveChecked, ulREGChecked, 3, pulWriteData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteMultiData with DRAM Stop with high impedance at first, check high impedance");
	dcm.I2CWriteMultiData(bySlaveChecked, ulREGChecked, 5, ppbyData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);


	///<Check stop high impedance at first
	dcm_I2CDeleteMemory();
	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetStopStatus(TRUE);
	funcReport.AddTestItem("I2CReadData in BRAM Stop with high impedance at first");

	dcm.I2CReadData(bySlaveChecked, ulREGChecked, 10);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteData in BRAM Stop with high impedance at first");
	dcm.I2CWriteData(bySlaveChecked, ulREGChecked, 3, pulWriteData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteMultiData in BRAM Stop with high impedance at first");
	dcm.I2CWriteMultiData(bySlaveChecked, ulREGChecked, 5, ppbyData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	dcm_I2CDeleteMemory();
	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetStopStatus(TRUE);
	bySlaveAddr = 0;
	ulREG = 0;
	do
	{
		dcm.I2CReadData(bySlaveAddr++, ulREG++, nDataCount);
		dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);
	} while (!bWithDRAM);


	funcReport.AddTestItem("I2CReadData with DRAM Stop with high impedance at first");

	dcm.I2CReadData(bySlaveChecked, ulREGChecked, 10);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteData with DRAM Stop with high impedance at first");
	dcm.I2CWriteData(bySlaveChecked, ulREGChecked, 3, pulWriteData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteMultiData with DRAM Stop with high impedance at first");
	dcm.I2CWriteMultiData(bySlaveChecked, ulREGChecked, 5, ppbyData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, TRUE, nullptr);

	dcm.I2CSetStopStatus(FALSE);
	funcReport.AddTestItem("I2CReadData with DRAM Stop with high impedance at first, check high driver");
	dcm.I2CReadData(bySlaveChecked, ulREGChecked, 10);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteData with DRAM Stop with high impedance at first, check high driver");
	dcm.I2CWriteData(bySlaveChecked, ulREGChecked, 3, pulWriteData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, nullptr);

	funcReport.AddTestItem("I2CWriteMultiData with DRAM Stop with high impedance at first, check high driver");
	dcm.I2CWriteMultiData(bySlaveChecked, ulREGChecked, 5, ppbyData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, nullptr);

	dcm_I2CDeleteMemory();
	USHORT usInvalidSite = 2;
	funcReport.AddTestItem("Check site invalid");
	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetStopStatus(TRUE);
	do
	{
		dcm.I2CReadData(bySlaveAddr++, ulREG++, nDataCount);
		dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);
	} while (!bWithDRAM);


	InvalidSite(usInvalidSite);
	dcm.I2CSetStopStatus(FALSE);
	RestoreSite();

	vector<CHANNEL_INFO> vecInvalidSite;
	vecInvalidSite.push_back(vecSCLChannel[usInvalidSite]);
	vecInvalidSite.push_back(vecSDAChannel[usInvalidSite]);


	funcReport.AddTestItem("I2CReadData with DRAM Stop with high impedance at first, check site invalid");
	dcm.I2CReadData(bySlaveChecked, ulREGChecked, 10);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, &vecInvalidSite);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, FALSE, &vecInvalidSite);

	funcReport.AddTestItem("I2CWriteData with DRAM Stop with high impedance at first, check site invalid");
	dcm.I2CWriteData(bySlaveChecked, ulREGChecked, 3, pulWriteData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, &vecInvalidSite);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, FALSE, &vecInvalidSite);

	funcReport.AddTestItem("I2CWriteMultiData with DRAM Stop with high impedance at first, check site invalid");
	dcm.I2CWriteMultiData(bySlaveChecked, ulREGChecked, 5, ppbyData);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH, TRUE, &vecInvalidSite);
	CheckPattern(CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE, FALSE, &vecInvalidSite);

	if (nullptr != ppbyData)
	{
		for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++ usSiteNo)
		{
			if (nullptr != ppbyData[usSiteNo])
			{
				delete[] ppbyData[usSiteNo];
				ppbyData[usSiteNo] = nullptr;
			}
		}
		delete[] ppbyData;
		ppbyData = nullptr;
	}

	if (nullptr != pulWriteData)
	{
		delete[] pulWriteData;
		pulWriteData = nullptr;
	}
	dcm.SetDynamicLoad("G_I2C", DCM_OPEN_CLAMP_CLOSE_LOAD);
	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
	funcReport.Print(this, g_lpszReportFilePath);
}