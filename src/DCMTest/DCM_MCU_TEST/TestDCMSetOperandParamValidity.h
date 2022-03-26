#pragma once
/**
 * @file TestDCMSetOperandParamValidity
 * @brief Check the parameter validity of SetOperand
 * @author Guangyun Wang
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "MCUCase.h"

XT_TEST(ParamValidityTest, TestDCMSetOperandParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;
	GetBoardInfo(mapSlot, g_lpszVectorFilePath, TRUE);
	BYTE byBoardCount = dcm_GetBoardInfo(nullptr, 0);
	if (0 == mapSlot.size())
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	nRetVal = dcm.SetOperand("G_ALLPIN", nullptr, 0, nullptr);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
	}
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	int nLineOffset = 1;
	const char* lpszLabel = "TEST_INS_ST";
	char* lpszTestPinGroup[4] = { nullptr, "G_NOPIN", "G_ALLPIN", "CH0" };
	BOOL bFirstError = TRUE;
	for (auto& PinGroup : lpszTestPinGroup)
	{
		nRetVal = dcm.SetOperand(PinGroup, lpszLabel, nLineOffset, "0");
		int nStringType = dcm_GetStringType(PinGroup);
		if (0 != nStringType && 1 != nStringType)
		{
			XT_EXPECT_EQ(nRetVal, PIN_GROUP_ERROR);
			if (PIN_GROUP_ERROR != nRetVal)
			{
				if (bFirstError)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFirstError = FALSE;
				}
				if (nullptr != PinGroup)
				{
					errMsg.SaveErrorMsg("The pin group(%s) is not existed, but the return value(%d) is not equal to PIN_GROUP_ERROR(%d).", PinGroup, nRetVal, PIN_GROUP_ERROR);
				}
				else
				{
					errMsg.SaveErrorMsg("The pin group is nullptr, but the return value(%d) is not equal to PIN_GROUP_ERROR(%d).", nRetVal, PIN_GROUP_ERROR);
				}
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				if (bFirstError)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFirstError = FALSE;
				}
				errMsg.SaveErrorMsg("The pin group(%s) is existed, but the return value(%d) is not equal to 0", PinGroup, nRetVal);
			}
		}
	}

	const char* lpszTestLabel[4] = { nullptr, "NOT_EXISTED", lpszLabel };
	for (auto& Label : lpszTestLabel)
	{
		nRetVal = dcm.SetOperand("G_ALLPIN", Label, nLineOffset, "0");
		int nStringType = dcm_GetStringType(Label);
		if (2 != nStringType)
		{
			XT_EXPECT_EQ(nRetVal, START_LABEL_ERROR);
			if (START_LABEL_ERROR != nRetVal)
			{
				if (bFirstError)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFirstError = FALSE;
				}
				if (nullptr != Label)
				{
					errMsg.SaveErrorMsg("The start label(%s) is not existed, but the return value(%d) is not equal to START_LABEL_ERROR(%d).", Label, nRetVal, START_LABEL_ERROR);
				}
				else
				{
					errMsg.SaveErrorMsg("The start label is nullptr, but the return value(%d) is not equal to START_LABEL_ERROR(%d).", nRetVal, START_LABEL_ERROR);
				}
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				if (bFirstError)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFirstError = FALSE;
				}
				errMsg.SaveErrorMsg("The start label(%s) is existed, but the return value(%d) is not equal to 0", Label, nRetVal);
			}
		}
	}

	int nTestOffset[4] = { -1, 0, 10, DCM_BRAM_PATTERN_LINE_COUNT };
	for (auto Offset : nTestOffset)
	{
		nRetVal = dcm.SetOperand("G_ALLPIN", lpszLabel, Offset, "0");
		if ((int)8 <= Offset)
		{
			XT_EXPECT_EQ(nRetVal, OFFSET_ERROR);
			if (OFFSET_ERROR != nRetVal)
			{
				if (bFirstError)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFirstError = FALSE;
				}
				errMsg.SaveErrorMsg("The offset(%d) is over range[0, %d], but the return value(%d) is not equal to OFFSET_ERROR(%d)", Offset, 0, 8, nRetVal, OFFSET_ERROR);
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				if (bFirstError)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFirstError = FALSE;
				}
				errMsg.SaveErrorMsg("The offset(%d) is in range[0, %d], but the return value(%d) is not equal to 0", Offset, 0, 8, nRetVal);
			}
		}
	}

	const char* lpszTestOperand[5] = { nullptr, "", "-1", "0", "65536" };
	BOOL bOperand[5] = { FALSE, FALSE, FALSE, TRUE, FALSE };
	int nOperandIndex = 0;
	for (auto& Operand : lpszTestOperand)
	{
		nRetVal = dcm.SetInstruction("G_ALLPIN", lpszLabel, nLineOffset, "REPEAT", Operand);
		if (!bOperand[nOperandIndex])
		{
			XT_EXPECT_EQ(nRetVal, OPERAND_ERROR);
			if (OPERAND_ERROR != nRetVal)
			{
				if (bFirstError)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFirstError = FALSE;
				}
				if (nullptr == Operand)
				{
					errMsg.SaveErrorMsg("The operand is nullptr, but the return value(%d) is not equal to OPERAND_ERROR(%d)", nRetVal, OPERAND_ERROR);
				}
				else
				{
					errMsg.SaveErrorMsg("The operand(%s) is error, but the return value(%d) is not equal to OPERAND_ERROR(%d)", Operand, nRetVal, OPERAND_ERROR);
				}
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				if (bFirstError)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFirstError = FALSE;
				}
				errMsg.SaveErrorMsg("The operand(%s) is supported, but the return value(%d) is not equal to 0", Operand, nRetVal);
			}
		}
		++nOperandIndex;
	}


	const char* lpszTestLabelOperand[4] = { nullptr, "", "NO_EXISTED", "TEST_INS_SP" };
	BOOL bLabelOperand[4] = { FALSE, FALSE, FALSE, TRUE };
	nOperandIndex = 0;
	for (auto& Operand : lpszTestLabelOperand)
	{
		nRetVal = dcm.SetOperand("G_ALLPIN", lpszLabel, nLineOffset, Operand);
		if (!bLabelOperand[nOperandIndex])
		{
			XT_EXPECT_EQ(nRetVal, OPERAND_ERROR);
			if (OPERAND_ERROR != nRetVal)
			{
				if (nullptr == Operand)
				{
					errMsg.SaveErrorMsg("The operand is nullptr, but the return value(%d) is not equal to OPERAND_ERROR(%d)", nRetVal, OPERAND_ERROR);
				}
				else
				{
					errMsg.SaveErrorMsg("The label operand(%s) is not existed, but the return value(%d) is not equal to OPERAND_ERROR(%d)", Operand, nRetVal, OPERAND_ERROR);
				}
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				if (bFirstError)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFirstError = FALSE;
				}
				errMsg.SaveErrorMsg("The label operand(%s) is existed, but the return value(%d) is not equal to 0", Operand, nRetVal);
			}
		}
		++nOperandIndex;
	}

	errMsg.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}