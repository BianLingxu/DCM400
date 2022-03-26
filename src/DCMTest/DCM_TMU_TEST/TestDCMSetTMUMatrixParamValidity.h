#pragma once
/**
 * @file TestDCMSetTMUMatrixParamValidity.h
 * @brief Test the parameter validity of function SetTMUMatrix
 * @author Guangyun Wang
 * @date 2020/09/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(TMUParamValidityTest, TestDCMSetTMUMatrixParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;

	nRetVal = dcm.SetTMUMatrix(nullptr, 0, DCM_TMU1);
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
	
	
	dcm.SetPinGroup("G_TWOPIN", "CH0,CH1");
	dcm.SetPinGroup("G_ONEPIN", "CH0");
	const int nPinGroupCount = 5;
	const char* lpszTestPinGroup[nPinGroupCount] = {nullptr, "G_UNDEFINED", "G_TWOPIN", "G_ONEPIN","CH0"};

	DWORD dwSiteStatusBackup = STSGetsSiteStatus();
	
	USHORT usInvalidSite = mapSlot.begin()->second + 1;
	
	DWORD dwCurSiteStatus = dwSiteStatusBackup & (~(1 << usInvalidSite));
	StsSetSiteStatus(dwCurSiteStatus);

	USHORT usSiteCount = usValidSiteCount + 1;
	for (auto& lpszPinGroup : lpszTestPinGroup)
	{
		for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
		{
			nRetVal = dcm.SetTMUMatrix(lpszPinGroup, usSiteNo, DCM_TMU1);
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
			else if (usSiteNo >= usValidSiteCount)
			{
				XT_EXPECT_EQ(nRetVal, SITE_ERROR);
				if (SITE_ERROR != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The site number(%d) is over range(%d), but the return value(%d) is not equal to SITE_ERROR(%d)",
						usSiteNo, usValidSiteCount, nRetVal, SITE_ERROR);
				}
			}
			else
			{
				BOOL bBoardNotInserted = TRUE;
				for (auto& Slot : mapSlot)
				{
					if (usSiteNo >= Slot.second && usSiteNo < Slot.second + DCM_MAX_CONTROLLERS_PRE_BOARD)
					{
						bBoardNotInserted = FALSE;
						break;
					}
				}
				if (bBoardNotInserted)
				{
					XT_EXPECT_EQ(nRetVal, BOARD_NOT_INSERT_ERROR);
					if (BOARD_NOT_INSERT_ERROR != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("The board is not existed, but the return value(%d) is not equal to BOARD_NOT_INSERT_ERROR(%d)",
							nRetVal, BOARD_NOT_INSERT_ERROR);
					}
					continue;
				}
				if (usInvalidSite == usSiteNo)
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
				int nChannelCount = dcm_GetPinGroupChannel(lpszPinGroup, usSiteNo, nullptr, nullptr, 0);
				if (1 < nChannelCount)
				{
					XT_EXPECT_EQ(nRetVal, TMU_CHANNEL_OVER_RANGE);
					if (TMU_CHANNEL_OVER_RANGE != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("The channel count(%d) in pin group(%s) is more than 1, but the return value(%d) is not equal to TMU_CHANNEL_OVER_RANGE(%d)",
							nChannelCount, lpszPinGroup, nRetVal, TMU_CHANNEL_OVER_RANGE);
					}
					continue;
				}

				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The pin group(%s) and site number(%d) is right, but the return value(%d) is not equal to 0",
						lpszPinGroup, nRetVal, TMU_CHANNEL_OVER_RANGE);
				}
			}
		}
	}

	StsSetSiteStatus(dwSiteStatusBackup);

	for (int nTMUIndex = 0; nTMUIndex < 3;++nTMUIndex)
	{
		int nRetVal = dcm.SetTMUMatrix("CH0", mapSlot.begin()->second, (DCM_TMU)nTMUIndex);
		if (1 < nTMUIndex)
		{
			XT_EXPECT_EQ(nRetVal, TMU_UNIT_OVER_RANGE);
			if (nRetVal != TMU_UNIT_OVER_RANGE)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SaveErrorMsg("The pin group(%s) and site number(%d) is right, but the return value(%d) is not equal to TMU_UNIT_OVER_RANGE(%d)",
					"CH0", mapSlot.begin()->second, TMU_UNIT_OVER_RANGE);
			}
		}
		else if(0 != nRetVal)
		{
			errMsg.AddNewError(STRING_ERROR_MSG);
			errMsg.SaveErrorMsg("The pin group(%s) and site number(%d) is right, but the return value(%d) is not equal to 0",
				"CH0", mapSlot.begin()->second, nRetVal, TMU_CHANNEL_OVER_RANGE);
		}
	}


	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}