/*!
* @file      TestDCMSetVTParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/10
* @version   v 1.0.0.0
* @brief     测试SetVT参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
void inline VTModeToString(VTMode mode, char* cMode, int iArrayLength)
{
	switch (mode)
	{
	case DCM_VT_FORCE:
		sprintf_s(cMode, iArrayLength, "VT_FORCE");
		break;
	case DCM_VT_REALTIME:
		sprintf_s(cMode, iArrayLength, "VT_REALTIME");
		break;
	case DCM_VT_CLOSE:
		sprintf_s(cMode, iArrayLength, "VT_CLOSE");
		break;
	default:
		break;
	}
}

XT_TEST(ParamValidityTest, TestDCMSetVTParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinGroupTestCount = 4;
	char* pinGroup[nPinGroupTestCount] = { nullptr, " ", "G_NOPIN", "G_ALLPIN"};
	const int nVTTestCount = 5;
	const double VTTestValue[nVTTestCount] = { -2, -1.5, 0, 6, 6.5 };
	
	int nValidBoardCount = dcm_GetValidBoard();
	if (0 == nValidBoardCount)
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	const int nVTStringLength = 16;
	char cVTMode[nVTStringLength] = { 0 };

	const double dVTResolution = 0.0006;//The minimum resolution of pin level.

	nRetVal = dcm.SetVT(nullptr, 0, DCM_VT_REALTIME);

	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		//Not warning when vector is not loaded.

		XT_EXPECT_EQ(nRetVal, VECTOR_FILE_NOT_LOADED);

		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
	}

	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	auto iterSlot = mapSlot.begin();

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	int nSringType = 0;
	BYTE bySlotNo[DCM_CHANNELS_PER_CONTROL] = { 0 };
	USHORT usChannel[DCM_CHANNELS_PER_CONTROL] = { 0 };
	for (BYTE modeIndex = DCM_VT_FORCE; modeIndex <= DCM_VT_CLOSE; ++modeIndex)
	{
		for (int nPinGroupTestIndex = 0; nPinGroupTestIndex < nPinGroupTestCount; ++nPinGroupTestIndex)
		{
			for (int nVTtestIndex = 0; nVTtestIndex < nVTTestCount; ++nVTtestIndex)
			{
				nRetVal = dcm.SetVT(pinGroup[nPinGroupTestIndex], VTTestValue[nVTtestIndex], (VTMode)modeIndex);
				nSringType = dcm_GetStringType(pinGroup[nPinGroupTestIndex]);
				VTModeToString((VTMode)modeIndex, cVTMode, nVTStringLength);
				if (0 != nSringType && 1 != nSringType)
				{
					if (PIN_GROUP_ERROR != nRetVal)
					{
						XT_EXPECT_EQ(nRetVal, PIN_GROUP_ERROR);
						errMsg.AddNewError(STRING_ERROR_MSG, pinGroup[nPinGroupTestIndex], 2);
						errMsg.SetErrorItem("PinGroup", pinGroup[nPinGroupTestIndex]);
						errMsg.SaveErrorMsg("No warning when pin group is not defined, the return value is %d", nRetVal);
					}
				}
				else
				{
					if (EXECUTE_SUCCESS != nRetVal)
					{
						if (PIN_LEVEL_ERROR == nRetVal && (-1.5 - EQUAL_ERROR > VTTestValue[nVTtestIndex] || 6 + EQUAL_ERROR < VTTestValue[nVTtestIndex]))
						{
							continue;
						}
						else
						{
							XT_EXPECT_EQ(nRetVal, EXECUTE_SUCCESS);

							errMsg.AddNewError(STRING_ERROR_MSG, pinGroup[nPinGroupTestIndex], 2);
							errMsg.SaveErrorMsg("Unknown error, the return value is %d", nRetVal);
						}
					}
					else
					{
						if (2 == modeIndex)
						{
							continue;
						}
						int nVTMode = 0;
						double dVTValue = 0;
						int nChannelCount = dcm_GetPinGroupChannel(pinGroup[nPinGroupTestIndex], iterSlot->second, bySlotNo, usChannel, DCM_CHANNELS_PER_CONTROL);
						if (nChannelCount != DCM_CHANNELS_PER_CONTROL)
						{
							errMsg.SaveErrorMsg("The vector file(%s) is not the right test vector.", g_lpszVectorFilePath);
							break;
						}
						for (int nChannelIndex = 0; nChannelIndex < nChannelCount; ++nChannelIndex)
						{
							dcm_GetVTMode(iterSlot->first, usChannel[nChannelIndex], nVTMode);
							dcm_GetLevelSettingValue(iterSlot->first, usChannel[nChannelIndex], DCM_VT, dVTValue);
							if (modeIndex != (VTMode)nVTMode || (fabs(dVTValue - VTTestValue[nVTtestIndex]) > dVTResolution && DCM_VT_CLOSE != modeIndex))
							{
								if (fabs(dVTValue - VTTestValue[nVTtestIndex]) > dVTResolution && DCM_VT_CLOSE != modeIndex)
								{
									errMsg.AddNewError(VALUE_ERROR, pinGroup[nPinGroupTestIndex], 2);
									errMsg.SetErrorItem("VT value", nullptr, 1, usChannel[nChannelIndex], VALUE_NOT_EQUAL);
									errMsg.SaveErrorMsg(dVTValue, VTTestValue[nVTtestIndex], dVTResolution, VALUE_DOUBLE_TWO_DECIMAL);
								}
								if (modeIndex != (VTMode)nVTMode)
								{
									char cRegMode[20] = { 0 };
									VTModeToString(VTMode(nVTMode), cRegMode, 20);
									errMsg.AddNewError(STRING_ERROR_MSG, pinGroup[nPinGroupTestIndex], 2);
									errMsg.SetErrorItem("VT Mode", cRegMode);
									errMsg.SaveErrorMsg("The VT mode in register is not equal to setting value, the return value is %d", nRetVal);
								}
							}
						}
					}
				}
			}
		}
	}
	dcm.SetVT("G_ALLPIN", 0, DCM_VT_CLOSE);


	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}