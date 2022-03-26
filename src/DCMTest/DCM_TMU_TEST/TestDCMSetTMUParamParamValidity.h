#pragma once
/**
 * @file TestDCMSetTMUParamParamValidity.h
 * @brief Test the parameter validity of function SetTMUParam
 * @author Guangyun Wang
 * @date 2020/09/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(TMUParamValidityTest, TestDCMSetTMUParamParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;

	nRetVal = dcm.SetTMUParam("CH0", 0, DCM_POS, 0, 0);
	XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		///<Not warning when vector is not loaded
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when vector is not loaded!");
	}

	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	if (0 == mapSlot.size())
	{
		///<No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	USHORT usValidSiteCount = dcm_GetVectorSiteCount();

	dcm.SetPinGroup("G_TMU", "CH0,CH4");
	dcm.SetPinGroup("G_NOTTMU", "CH1,CH3");

	const BYTE byTestPinGroupCount = 4;
	char* lpszTestPinGroup[byTestPinGroupCount] = { nullptr, "G_UNDEFINED", "G_NOTTMU","G_TMU" };
	BOOL bPinGroupValidity[byTestPinGroupCount] = {FALSE, FALSE, FALSE, TRUE};

	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);

	DWORD dwSiteStatusBackup = STSGetsSiteStatus();

	USHORT usInvalidSite = mapSlot.begin()->second + 1;

	DWORD dwCurSiteStatus = dwSiteStatusBackup & (~(1 << usInvalidSite));
	StsSetSiteStatus(dwCurSiteStatus);

	USHORT usTestSiteCount = usValidSiteCount + 1;
	int nPinGroupIndex = 0;
	for (auto& lpszPinGroup : lpszTestPinGroup)
	{
		for (USHORT usSiteNo = 0; usSiteNo < usTestSiteCount; ++usSiteNo)
		{
			nRetVal = dcm.SetTMUParam(lpszPinGroup, usSiteNo, DCM_POS, 0, 0);
			int nStringType = dcm_GetStringType(lpszPinGroup);
			if (0 != nStringType && 1 != nStringType)
			{
				XT_EXPECT_EQ(nRetVal, PIN_GROUP_ERROR);
				if (PIN_GROUP_ERROR != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					if (nullptr == lpszPinGroup)
					{
						errMsg.SaveErrorMsg("The pin group is nullptr, but the return value(%d) is not equal to PIN_GROUP_ERROR(%d)", nRetVal, PIN_GROUP_ERROR);
					}
					else
					{
						errMsg.SaveErrorMsg("The pin group(%s) is not defined, but the return value(%d) is not equal to PIN_GROUP_ERROR(%d)", lpszPinGroup, nRetVal, PIN_GROUP_ERROR);
					}
				}
				continue;
			}
			if (usValidSiteCount <= usSiteNo)
			{
				XT_EXPECT_EQ(nRetVal, SITE_ERROR);
				if (SITE_ERROR != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The site number(%d) is over range(%d), but the return value(%d) is not equal to SITE_ERROR(%d)",
						usSiteNo, usValidSiteCount, nRetVal, SITE_ERROR);
				}
				continue;
			}
			///<Check site
			BOOL bBoardExisted = FALSE;
			for (auto& Slot : mapSlot)
			{
				if (usSiteNo >= Slot.second && usSiteNo < Slot.second + DCM_MAX_CONTROLLERS_PRE_BOARD)
				{
					bBoardExisted = TRUE;
					break;
				}
			}
			if (!bBoardExisted)
			{
				XT_EXPECT_EQ(nRetVal, BOARD_NOT_INSERT_ERROR);
				if (BOARD_NOT_INSERT_ERROR != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The board used in SITE_%d is not existed, but the return value(%d) is not equal to BOARD_NOT_INSERT_ERROR(%d)", usSiteNo + 1, nRetVal, BOARD_NOT_INSERT_ERROR);
				}
				continue;
			}
			else if (usInvalidSite == usSiteNo)
			{
				XT_EXPECT_EQ(nRetVal, SITE_INVALID);
				if (SITE_INVALID != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The SITE_%d is invalid, but the return value(%d) is not equal to SITE_INVALID(%d)",
						usSiteNo + 1, nRetVal, SITE_INVALID);
				}
				continue;
			}

			if (!bPinGroupValidity[nPinGroupIndex])
			{
				XT_EXPECT_EQ(nRetVal, TMU_CHANNEL_NOT_CONNECT);
				if (TMU_CHANNEL_NOT_CONNECT != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The channel in pin group(%s) is not connected to TMU unit, but the return value(%d) is not equal to TMU_CHANNEL_NOT_CONNECT(%d)",
						lpszPinGroup, nRetVal, TMU_CHANNEL_NOT_CONNECT);
				}
				continue;
			}
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SaveErrorMsg("The channel in pin group(%s) and site (%d) is right, but the return value(%d) is not equal to 0",
					lpszPinGroup, usSiteNo, nRetVal);
			}
			continue;
		}
		++nPinGroupIndex;
	}

	const USHORT usTMUChannelCount = 2;
	USHORT usTMUChannel[usTMUChannelCount] = { 0,4 };

	const BYTE byTestHoldOffTimeCount = 4;
	UINT uTestHoldOffTime[byTestHoldOffTimeCount] = { 0, 10,508, 1000 };
	const BYTE byTestHoldOffNumCount = 4;
	UINT uTestHoldOffNum[byTestHoldOffNumCount] = { 0, 10,2046, 2047 };

	for (auto uHoldHoldOffTime : uTestHoldOffTime)
	{
		for (auto uHoldHoldOffNum : uTestHoldOffNum)
		{
			nRetVal = dcm.SetTMUParam("G_TMU", DCM_ALLSITE, DCM_POS, uHoldHoldOffTime, uHoldHoldOffNum);
			if (508 < uHoldHoldOffTime)
			{
				BOOL bTriggerEdge = 0;
				USHORT uCurHoldOffTime = 0;
				USHORT uCurHoldOffNum = 0;
				dcm_ReadTMUParam(mapSlot.begin()->first, 0, bTriggerEdge, uCurHoldOffTime, uCurHoldOffNum);
				XT_EXPECT_EQ((int)uCurHoldOffTime, 508);
				if (508 != uCurHoldOffTime)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The hold off time(%d) is over range[%d,%d], but the set value(%d) is not equal to %d",
						uHoldHoldOffTime, 0, 508, uCurHoldOffTime, 508);
				}
				continue;
			}
			else if (2046 < uHoldHoldOffNum)
			{
				BOOL bTriggerEdge = 0;
				USHORT uCurHoldOffTime = 0;
				USHORT uCurHoldOffNum = 0;
				dcm_ReadTMUParam(mapSlot.begin()->first, 0, bTriggerEdge, uCurHoldOffTime, uCurHoldOffNum);
				XT_EXPECT_EQ((int)uCurHoldOffNum, 2046);
				if (2046 != uCurHoldOffNum)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The hold off number(%d) is over range[%d,%d], but the set value(%d) is not equal to %d",
						uCurHoldOffNum, 0, 2046, uCurHoldOffNum, 2046);
				}
			}
			else
			{
				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The hold off time(%d) and hold off number(%d)is in range, but the return value(%d) is not equal to 0",
						uHoldHoldOffTime, uHoldHoldOffNum, nRetVal);
					continue;
				}
			}
		}
	}

	StsSetSiteStatus(dwSiteStatusBackup);


	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}