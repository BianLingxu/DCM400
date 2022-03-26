#include "Report.h"
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include "xgTest.h"
#include "STSCoreGlobal.h"
#include <fstream>
#include "SM8213.h"
using namespace std;
/**
 * @brief Format message
 * @param[in] nTotalSize The message size
 * @param[in] nAlignmentType The alignment type, 0 is left alignment, 1 is center alignment, 2 is right alignment
 * @param[out] strFormated The message formated
 * @param[in] lpszFormat The format of the input message
 * @param[in]  The message for formated
*/
void FormatMsg(int nTotalSize, int nAlignmentType, std::string& strFormated, const char* lpszFormat, ...)
{
	strFormated.clear();
	char lpszMsg[128] = { 0 };

	va_list ap;
	va_start(ap, lpszFormat);
	vsprintf_s(lpszMsg, sizeof(lpszMsg), lpszFormat, ap);
	va_end(ap);

	strFormated = lpszMsg;
	int nValidDataSize = strlen(lpszMsg);
	int nAddSize = nTotalSize - nValidDataSize;
	if (0 >= nAddSize)
	{
		return;
	}

	int nLeftAdd = nAddSize / 2;
	switch (nAlignmentType)
	{
	case 0:
		///<Left alignment
		strFormated.append(nAddSize, ' ');
		break;
	case 1:
		///<Center alignment
		strFormated.insert(0, nLeftAdd, ' ');
		strFormated.append(nAddSize - nLeftAdd, ' ');
		break;
	case 2:
		///<Right alignment
		strFormated.append(nAddSize, ' ');
		break;
	default:
		break;
	}
}

CBaseErrorMSG::CBaseErrorMSG()
{
	memset(m_lpszPinName, 0, sizeof(m_lpszPinName));
	memset(m_lpszPinGroup, 0, sizeof(m_lpszPinGroup));
}

CBaseErrorMSG::~CBaseErrorMSG()
{

}

/**
*Save the pin name or pin group
 * @param[in] pinString The name of pin or pin group
 * @param[in] pinType The type of pinString
 * @return no return
*/
void CBaseErrorMSG::addPinNameOrPinGroup(const char* pinString, PIN_TYPE pinType)
{
	m_enumPinType = pinType;

	switch (pinType)
	{
	case DCM_PIN_NAME:
		strcpy_s(m_lpszPinName, 32, pinString);
		break;
	case DCM_PIN_GROUP:
		strcpy_s(m_lpszPinGroup, 32, pinString);
		break;
	case DCM_NO_PIN:
		break;
	default:
		break;
	}
}
/**
*Get the pin name or pin group
 * @param[out] pinString The name of pin or pin group
 * @param[in] nArrayLength The length of the pinString
 * @return -1:The length of pinString is too small
*/
int CBaseErrorMSG::getPinNameOrPinGroup(char* pinString, int nArrayLength)
{
	if (nArrayLength < 32)
	{
		return -1;
	}
	if (DCM_PIN_NAME == m_enumPinType)
	{
		strcpy_s(pinString, nArrayLength, m_lpszPinName);
	}
	else
	{
		strcpy_s(pinString, nArrayLength, m_lpszPinGroup);
	}
	return 0;
}
/**
*Get the pin type
 * @return The pin type
*/
PIN_TYPE CBaseErrorMSG::getPinType()
{
	return m_enumPinType;
}

CStringError::CStringError()
{
	memset(m_cParamName, 0, sizeof(m_cParamName));
	m_bWithChannel = 0;
	m_usChannel = -1;
	m_bySlotNo = 0;
}

CStringError::~CStringError()
{
	m_vecParamName.clear();
}

/**
*Set the parameter of the error message
 * @param[in] cParamName The error name
 * @param[in] cParamValue The value of the fail parameter
 * @param[in] bWithChannel Whether the error parameter has channel message
 * @param[in] usChannel The channel number of error message if needed
 * @return no return
*/
void CStringError::SetParam(const char* cParamName, const char* cParamValue, BOOL bWithChannel, BYTE bySlotNo, USHORT usChannel)
{
	if (m_bWithChannel == bWithChannel && bySlotNo == m_bySlotNo && m_usChannel == usChannel)
	{
		if (nullptr == cParamName && 0 == m_cParamName[0])
		{
			if (nullptr == cParamValue && 0 == m_cParamValue[0])
			{
				//The value is same as latest
				return;

			}
			else if (0 == strcmp(m_cParamValue, cParamValue))
			{
				//The value is same as latest
				return;
			}
		}
		else
		{
			if (0 == strcmp(m_cParamName, cParamName))
			{
				if (nullptr == cParamValue || 0 == m_cParamValue[0])
				{
					//The value is same as latest
					return;

				}
				else if (0 == strcmp(m_cParamValue, cParamValue))
				{
					//The value is same as latest
					return;
				}
			}
		}
	}
	if (nullptr != cParamName)
	{
		strcpy_s(m_cParamName, 255, cParamName);
	}
	else
	{
		strcpy_s(m_cParamName, 255, "nullptr");
	}
	if (nullptr != cParamValue)
	{
		strcpy_s(m_cParamValue, 255, cParamValue);
	}
	else
	{
		strcpy_s(m_cParamValue, 255, "nullptr");
	}
	m_bWithChannel = bWithChannel;
	m_bySlotNo = bySlotNo;
	m_usChannel = usChannel;
}
/**
*Save the error message
 * @param[in] cMSG The error message
 * @return no return
*/
void CStringError::AddMSG(char* cMSG)
{
	OTHER_ERROR_MSG otherMsg;
	string strMSG = cMSG;
	vector<OTHER_ERROR_MSG>::iterator iterMSG = m_vecParamName.begin();
	while (iterMSG != m_vecParamName.end())
	{
		//Find the same parameter.
		if (0 == strcmp(iterMSG->cParamName, m_cParamName) && 0 == strcmp(iterMSG->cParamValue, m_cParamValue) && m_bWithChannel == iterMSG->bWithChannel 
			&& m_bySlotNo ==iterMSG->m_bySlotNo && m_usChannel == iterMSG->m_usChannel)
		{
			break;
		}
		++iterMSG;
	}
	if (iterMSG != m_vecParamName.end())
	{
		iterMSG->vecErrMSG.push_back(strMSG);
		return;
	}

	strcpy_s(otherMsg.cParamName, 255, m_cParamName);
	strcpy_s(otherMsg.cParamValue, 255, m_cParamValue);
	otherMsg.bWithChannel = m_bWithChannel;
	otherMsg.m_bySlotNo = m_bySlotNo;
	otherMsg.m_usChannel = m_usChannel;
	otherMsg.vecErrMSG.push_back(strMSG);
	m_vecParamName.push_back(otherMsg);
	return;
}
/**
*Print the error message
 * @param[in] testProj The project of test program
 * @return no return
*/
void CStringError::ErrorPrint(XTest * testProj, char* cFilePath)
{
	vector<OTHER_ERROR_MSG>::iterator iterErrMSG = m_vecParamName.begin();
	vector<string>::iterator iterMSG;
	FILE *errFile = sts_fopen(cFilePath, "a+");
	//Print pin name or pin group
	char cPinString[32] = { 0 };
	getPinNameOrPinGroup(cPinString, 32);
	PIN_TYPE pinType = getPinType();
	testProj->PrintfA("\n=============>Error<=============\n");
	sts_fprintf(errFile,"\n=============>Error<=============\n");
	if (DCM_PIN_NAME == pinType)
	{
		testProj->PrintfA("===>Pin Name: %s\n", cPinString);
		sts_fprintf(errFile, "===>Pin Name: %s\n", cPinString);
	}
	else if (DCM_PIN_GROUP == pinType)
	{
		testProj->PrintfA("===>Pin Group: %s\n", cPinString);
		sts_fprintf(errFile, "===>Pin Group: %s\n", cPinString);
	}

	//Print error message
	int nErrIndex = 1;
	while (iterErrMSG != m_vecParamName.end())
	{
		if (0 == iterErrMSG->bWithChannel)
		{
			//The error message don't have channel message.
			if (0 != iterErrMSG->cParamName[0])
			{
				testProj->PrintfA("%-16s£¬%-16s\n", "Parameter Name", "Parameter Value");
				sts_fprintf(errFile, "%-16s£¬%-16s\n", "Parameter Name", "Parameter Value");
				testProj->PrintfA("%-16s£¬%-16s\n", iterErrMSG->cParamName, iterErrMSG->cParamValue);
				sts_fprintf(errFile, "%-16s£¬%-16s\n", iterErrMSG->cParamName, iterErrMSG->cParamValue);
			}
		}
		else
		{
			//The error message have channel message.
			if (0 != iterErrMSG->cParamName[0])
			{
				testProj->PrintfA("%-16s,%-16s\n", "Parameter Name", "Slot", "Channel");
				testProj->PrintfA("%-16s,%-16d,%-16d\n", iterErrMSG->cParamName, iterErrMSG->m_bySlotNo, iterErrMSG->m_usChannel);
				sts_fprintf(errFile, "%-16s,%-16s,%-16s\n", "Parameter Name","Slot", "Channel Number");
				sts_fprintf(errFile, "%-16s,%-16d,%-16d\n", iterErrMSG->cParamName, iterErrMSG->m_bySlotNo, iterErrMSG->m_usChannel);
			}
			else
			{
				testProj->PrintfA("%-16s,%-16s\n", "Slot", "Channel");
				testProj->PrintfA("%-16d,%-16d\n", iterErrMSG->m_bySlotNo, iterErrMSG->m_usChannel);
				sts_fprintf(errFile, "%-16s,%-16s\n", "Slot", "Channel");
				sts_fprintf(errFile, "%-16d,%-16d\n", iterErrMSG->m_bySlotNo, iterErrMSG->m_usChannel);
			}
		}
		testProj->PrintfA("===>Detail Error Message<===\n");
		sts_fprintf(errFile, "===>Detail Error Message<===\n");
		testProj->PrintfA("--> %-8s,%s\n", "Index", "Error message");
		sts_fprintf(errFile, "--> %-8s,%s\n", "Index", "Error message");
		iterMSG = iterErrMSG->vecErrMSG.begin();
		while (iterMSG != iterErrMSG->vecErrMSG.end())
		{
			testProj->PrintfA("--> %-8d,%s\n", nErrIndex, iterMSG->c_str());
			sts_fprintf(errFile, "--> %-8d,%s\n", nErrIndex, iterMSG->c_str());
			++nErrIndex;
			++iterMSG;
		}
		++iterErrMSG;
	}
	fclose(errFile);
	return;
}

CValueError::CValueError()
{
	memset(m_cParamName, 0, sizeof(m_cParamName));
	m_usChannel = -1;
	m_bySlotNo = 0;
}

CValueError::~CValueError()
{
	m_vecValueError.clear();
}

/**
*Set the parameter of the error message.
 * @param[in] cParamName The error name.
 * @param[in] usChannel The channel number of error message.
 * @param[in] enumValueErrorType The value error type.
 * @return no return.
*/
void CValueError::SetParam(const char* cParamName, BYTE bySlotNo, USHORT usChannel, VALUE_ERROR_TYPE enumValueErrorType)
{
	if (m_bySlotNo == bySlotNo && m_usChannel == usChannel && m_enumValueErrorType == enumValueErrorType)
	{
		if (nullptr == cParamName && 0 == m_cParamName[0])
		{
			//The value is same as last.
			return;
		}
		else if (nullptr != cParamName)
		{
			if (0 == strcmp(m_cParamName, cParamName))
			{
				//The value is same as last.
				return;
			}
		}
	}
	if (nullptr != cParamName)
	{
		strcpy_s(m_cParamName, 30, cParamName);
	}
	else
	{
		strcpy_s(m_cParamName, 30, "nullptr");
	}
	m_bySlotNo = bySlotNo;
	m_usChannel = usChannel;
	m_enumValueErrorType = enumValueErrorType;
}
/**
*Save the error message.
 * @param[in] dActualValue The actual value of the function param or input value if the error is value scale excced.
 * @param[in] dExpcetValue The expect value of the function param or the biggest value of the scale if the error is value scale excced.
 * @param[in] dResolution The resolution of the function param or the lowest value of the scale if the error is value scale excced.
 * @param[in] enumDataFormat The value format of the function param or no care if the error is value scale excced.
 * @return no return.
*/
void CValueError::AddMSG(double dActualValue,double dExpcetValue, double dResolution, TEST_DATA_FORMAT enumDataFormat/* = INT_DATA*/)
{
	CHANNEL_ERR_MSG strusChannelMSG;
	VALUE_DETAIL struValue;
	struValue.dExceptValue = dExpcetValue;
	struValue.dActualValue = dActualValue;
	struValue.dResolution = dResolution;
	struValue.dataFormat = enumDataFormat;
	vector<CHANNEL_ERR_MSG>::iterator iterValueError = m_vecValueError.begin();

	while (iterValueError != m_vecValueError.end())
	{
		if (0 == strcmp(iterValueError->cParamName,m_cParamName) && iterValueError->m_bySlotNo == m_bySlotNo 
			&& iterValueError->m_usChannelNo == m_usChannel && iterValueError->valueType == m_enumValueErrorType)
		{
			break;
		}
		++iterValueError;
	}
	if (iterValueError != m_vecValueError.end())
	{
		iterValueError->vecDetailErr.push_back(struValue);
		return;
	}
	strcpy_s(strusChannelMSG.cParamName, sizeof(strusChannelMSG.cParamName), m_cParamName);
	strusChannelMSG.m_bySlotNo = m_bySlotNo;
	strusChannelMSG.m_usChannelNo = m_usChannel;
	strusChannelMSG.valueType = m_enumValueErrorType;
	strusChannelMSG.vecDetailErr.push_back(struValue);
	m_vecValueError.push_back(strusChannelMSG);
}
/**
*Print the error message.
 * @param[in] testProj The project of test program.
 * @param[in] cFilePath The file path of the error report.
 * @return no return.
*/
void CValueError::ErrorPrint(XTest * testProj, char* cFilePath)
{
	vector<CHANNEL_ERR_MSG>::iterator iterValueError = m_vecValueError.begin();
	vector<VALUE_DETAIL>::iterator iterValueDetail;
	VALUE_ERROR_TYPE enumCurType = VALUE_SCALE;
	char *cPrintFormat[5] = { "--> %-8d,%-16d,%-16d,%-16d\n", "--> %-8d,%-16.1f,%-16.1f,%-16.1f\n", "--> %-8d,%-16.2f,%-16.2f,%-16.2f\n",
		"--> %-8d,%-16.3f,%-16.3f,%-16.3f\n", "--> %-8d,%-16.4f,%-16.4f,%-16.4f\n" };

	FILE *errFile = sts_fopen(cFilePath, "a+");
	//Print pin name or pin group
	char cPinString[32] = { 0 };
	getPinNameOrPinGroup(cPinString, 32);
	PIN_TYPE pinType = getPinType();

	testProj->PrintfA("\n=============>Value type Error<=============\n");
	sts_fprintf(errFile,"\n=============>Value type Error<=============\n");

	if (DCM_PIN_NAME == pinType)
	{
		testProj->PrintfA("===>Pin Name: %s\n", cPinString);
		sts_fprintf(errFile, "===>Pin Name: %s\n", cPinString);
	}
	else if (DCM_PIN_GROUP == pinType)
	{
		testProj->PrintfA("===>Pin Group: %s\n", cPinString);
		sts_fprintf(errFile, "===>Pin Group: %s\n", cPinString);
	}

	//Print error message
	int nErrIndex = 1;
	while (iterValueError != m_vecValueError.end())
	{
		testProj->PrintfA("%-16s,%-16s,%-16s\n", "Parameter Name", "Slot", "Channel");
		sts_fprintf(errFile, "%-16s,%-16s,%-16s\n", "Parameter Name", "Slot", "Channel");
		testProj->PrintfA("%-16s,%-16d,%-16d\n", iterValueError->cParamName, iterValueError->m_bySlotNo, iterValueError->m_usChannelNo);
		sts_fprintf(errFile, "%-16s,%-16s,%-16s\n", "Parameter Name", "Slot", "Channel");
		enumCurType = iterValueError->valueType;
		if (VALUE_NOT_EQUAL == enumCurType)
		{
			testProj->PrintfA("--> %-8s,%-16s,%-16s,%-16s\n", "Index", "Actual Value", "Expect Value", "Resolution");
			sts_fprintf(errFile, "--> %-8s,%-16s,%-16s,%-16s\n", "Index", "Actual Value", "Expect Value", "Resolution");
		}
		else
		{
			testProj->PrintfA("--> %-8s,%-16s,%-16s,%-16s\n", "Index", "Set Value", "Maximum Value", "Minimum Value");
			sts_fprintf(errFile, "--> %-8s,%-16s,%-16s,%-16s\n", "Index", "Set Value", "Maximum Value", "Minimum Value");
		}

		iterValueDetail = iterValueError->vecDetailErr.begin();
		while (iterValueDetail != iterValueError->vecDetailErr.end())
		{
			testProj->PrintfA(cPrintFormat[iterValueDetail->dataFormat], nErrIndex, iterValueDetail->dActualValue, iterValueDetail->dExceptValue, iterValueDetail->dResolution);
			sts_fprintf(errFile, cPrintFormat[iterValueDetail->dataFormat], nErrIndex, iterValueDetail->dActualValue, iterValueDetail->dExceptValue, iterValueDetail->dResolution);
			++nErrIndex;
			++iterValueDetail;
		}
		++iterValueError;
	}
	fclose(errFile);
	return;
}
/**
*Constructor function.
 * @param[in] cFunctionName The function name.
 * @param[in] cTestItem The test item.
 * @return no return.
*/
CErrorMSG::CErrorMSG(const char* cFunctionName, const char* cTestItem)
{
	m_vecOtherError.clear();
	m_vecValueError.clear();
	strcpy_s(m_lpszFunctionName, 128, cFunctionName);
	strcpy_s(m_lpszTestItem, 128, cTestItem);
}

CErrorMSG::~CErrorMSG()
{
	int nValueErrorCount = m_vecValueError.size();
	int nOtherErrorCount = m_vecOtherError.size();
	int nIndex = 0;
	for (auto& ValueError : m_vecValueError)
	{
		if (nullptr!= ValueError)
		{
			delete ValueError;
			ValueError = nullptr;
		}
	}
	for (auto& Other : m_vecOtherError)
	{
		if (nullptr != Other)
		{
			delete Other;
			Other = nullptr;
		}
	}

	m_vecValueError.clear();
	m_vecOtherError.clear();
}

/**
*Record error message.
 * @param[in] errType The error type.
 * @param[in] cPinString The name of pin name or pin group.
 * @param[in] nPinType Whether the cPinString is pin name, 0 is no pin name or pin type, 1 is pin name; 2 is pin group.
 * @return no return.
*/
void CErrorMSG::AddNewError(ERROR_TYPE errType, const char* cPinString, BYTE nPinType)
{
	char cCurPinString[16] = { 0 };
	PIN_TYPE enumCurPinType = DCM_PIN_NAME;
	BOOL bFindExit = FALSE;
	char cNewPinString[16] = { 0 };
	if (nullptr == cPinString)
	{
		strcpy_s(cNewPinString, 16, "nullptr");
	}
	else
	{
		strcpy_s(cNewPinString, 16, cPinString);
	}
	if (VALUE_ERROR == errType)
	{
		//The Value Error.

		for (auto& Error : m_vecValueError)
		{
			enumCurPinType = Error->getPinType();
			if (0 == nPinType)
			{
				if (DCM_NO_PIN == enumCurPinType)
				{
					bFindExit = TRUE;
					break;
				}
			}
			else if (1 == nPinType)
			{
				if (DCM_PIN_NAME == enumCurPinType)
				{
					Error->getPinNameOrPinGroup(cCurPinString, 10);
					if (0 == strcmp(cCurPinString, cNewPinString))
					{
						bFindExit = TRUE;
						break;
					}
				}
			}
			else if(2 == nPinType)
			{
				if (DCM_PIN_GROUP == enumCurPinType)
				{
					Error->getPinNameOrPinGroup(cCurPinString, 10);
					if (0 == strcmp(cCurPinString, cNewPinString))
					{
						bFindExit = TRUE;
						break;
					}
				}
			}
		}
	}
	else
	{
		//The other error
		for (auto& Error : m_vecOtherError)
		{
			enumCurPinType = Error->getPinType();
			if (0 == nPinType)
			{
				if (DCM_NO_PIN == enumCurPinType)
				{
					bFindExit = TRUE;
					break;
				}
			}
			else if (1 == nPinType)
			{
				if (DCM_PIN_NAME == enumCurPinType)
				{
					Error->getPinNameOrPinGroup(cCurPinString, 10);
					if (0 == strcmp(cCurPinString, cNewPinString))
					{
						bFindExit = TRUE;
						break;
					}
				}
			}
			else if (2 == nPinType)
			{
				if (DCM_PIN_GROUP == enumCurPinType)
				{
					Error->getPinNameOrPinGroup(cCurPinString, 10);
					if (0 == strcmp(cCurPinString, cNewPinString))
					{
						bFindExit = TRUE;
					}
				}
			}
		}
	}
	if (!bFindExit)
	{
		if (VALUE_ERROR == errType)
		{
			CValueError *valueError = new CValueError();
			if (0 == nPinType)
			{
				valueError->addPinNameOrPinGroup();
			}
			else if (1 == nPinType)
			{
				valueError->addPinNameOrPinGroup(cNewPinString, DCM_PIN_NAME);
			}
			else if (2 == nPinType)
			{
				valueError->addPinNameOrPinGroup(cNewPinString, DCM_PIN_GROUP);
			}
			m_vecValueError.push_back(valueError);
		}
		else
		{
			CStringError *otherError = new CStringError();
			if (0 == nPinType)
			{
				otherError->addPinNameOrPinGroup();
			}
			else if (1 == nPinType)
			{
				otherError->addPinNameOrPinGroup(cNewPinString, DCM_PIN_NAME);
			}
			else if (2 == nPinType)
			{
				otherError->addPinNameOrPinGroup(cNewPinString, DCM_PIN_GROUP);
			}
			m_vecOtherError.push_back(otherError);
		}
	}
}

/**
*Set the parameter of error message.
 * @param[in] cParamName The parameter which test fail.
 * @param[in] cParamValue The value of the fail parameter.
 * @param[in] bWithChannel Whether the parameter has channel.
 * @param[in] usChannel The channel number if needed.
 * @return 0 is excute successfully; -1 is no error message be accorded before, call RecordError first.
*/
int CErrorMSG::SetErrorItem(const char* cParamName/* = nullptr*/, const char* cParamValue/* = nullptr*/, BOOL bWithChannel/* = FALSE*/, BYTE bySlotNo, USHORT usChannel/* = -1*/, VALUE_ERROR_TYPE enumValueErrorType/* = VALUE_NOT_EQUAL*/)
{
	int nOtherErrorCount = m_vecOtherError.size();
	int nValueErrorCount = m_vecValueError.size();
	if (0 == nValueErrorCount && 0 == nOtherErrorCount)
	{
		return -1;
	}
	if (nOtherErrorCount)
	{
		m_vecOtherError[nOtherErrorCount - 1]->SetParam(cParamName, cParamValue, bWithChannel, bySlotNo, usChannel);
	}
	if (nValueErrorCount)
	{
		m_vecValueError[nValueErrorCount - 1]->SetParam(cParamName, bySlotNo, usChannel, enumValueErrorType);
	}
	return 0;
}

/**
*Save the error message.
 * @param[in] dActualValue The actual value of the function param or input value if the error is value scale exceed.
 * @param[in] dExpcetValue The expect value of the function param or the biggest value of the scale if the error is value scale exceed.
 * @param[in] dResolution The resolution of the function param or the lowest value of the scale if the error is value scale exceed.
 * @param[in] enumDataFormat The value format of the function param or no care if the error is value scale exceed.
 * @return 0 is excute successfully; -1 is no error message be accorded before, call RecordError first.
*/
int CErrorMSG::SaveErrorMsg(double dActualValue, double dExpcetValue, double dResolution, TEST_DATA_FORMAT enumDataFormat /*= INT_DATA*/)
{
	int nValueError = m_vecValueError.size();
	if (0 == nValueError)
	{
		return -1;
	}
	m_vecValueError[nValueError - 1]->AddMSG(dActualValue, dExpcetValue, dResolution, enumDataFormat);
	return 0;
}
/**
*Save the error message.
 * @param[in] format The format string.
 * @return 0 is excute successfully; -1 is no error message be accorded before, call RecordError first.
*/
int CErrorMSG::SaveErrorMsg(const char* format, ...)
{
	int nOtherErrorCount = m_vecOtherError.size();
	if (0 == nOtherErrorCount)
	{
		return -1;
	}	
	char cErrMsg[1024] = { 0 };

	va_list args;
	va_start(args, format);
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	vsprintf_s(cErrMsg, 1024, format, args);
#else
	vsprintf(cErrMsg, format, args);
#endif

	va_end(args);

	m_vecOtherError[nOtherErrorCount - 1]->AddMSG(cErrMsg);
	return 0;
}
/**
*Print the error message.
 * @param[in] testProj The project of test program.
 * @param[in] cFilePath The file path of the error report.
 * @return no return.
*/
void CErrorMSG::Print(XTest* testProj, char* lpszFilePath)
{
	FILE *errFile = sts_fopen(lpszFilePath, "a+");
	
	time_t nowTime = time(nullptr);
	tm t;
	sts_localtime(&t, &nowTime);
	char cTestTime[256] = { 0 };
	sts_sprintf(cTestTime,256, "Test Date: %d-%02d-%02d %02d:%02d:%02d\n", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	testProj->PrintfA(cTestTime);
	sts_fprintf(errFile, "%s",cTestTime);

	testProj->PrintfA("Test Item: %s\nFunction Name: %s\n", m_lpszTestItem, m_lpszFunctionName);
	sts_fprintf(errFile, "Test Item: %s\nFunction Name: %s\n", m_lpszTestItem, m_lpszFunctionName);
	int nValueErrorCount = m_vecValueError.size();
	int nOtherErrorCount = m_vecOtherError.size();
	if (0 == nValueErrorCount && 0 == nOtherErrorCount)
	{
		testProj->PrintfA("Test Result: PASS\n");
		sts_fprintf(errFile, "Test Result: PASS\n");
		sts_fprintf(errFile, "<========================================>\n\n");
		fclose(errFile);
		return;
	}
	testProj->PrintfA("Test Result: FAIL!\n");
	sts_fprintf(errFile, "Test Result: FAIL!\n");
	fclose(errFile);

	//Print the message of value error.
	for (auto& Error : m_vecValueError)
	{
		Error->ErrorPrint(testProj, lpszFilePath);

	}
	//Print the message of other error.
	for (auto& Error : m_vecOtherError)
	{
		Error->ErrorPrint(testProj, lpszFilePath);
	}

	errFile = sts_fopen(lpszFilePath, "a+");
	sts_fprintf(errFile, "<========================================>\n\n");
	fclose(errFile);
}
/**
*Constructor function.
 * @param[in] cFunctionName The function name.
 * @param[in] cTestItem The test item.
 * @return no return.
*/
CFuncReport::CFuncReport(const char* cFunctionName, const char* lpszTestType)
{
	m_strFunctionName = cFunctionName;
	int nPos = m_strFunctionName.find("TestDCM");
	if (string::npos != nPos)
	{
		m_strFunctionName.erase(0, 10);
	}
	nPos = m_strFunctionName.find("Function");
	if (string::npos != nPos)
	{
		m_strFunctionName.erase(nPos, 8);
	}
	m_strTestType = lpszTestType;
	m_strTestItem = "nullptr";
	for (int nIndex = 0; nIndex < 6; ++nIndex)
	{
		m_dClkSetting[nIndex] = -1;
	}

	m_bBoardValid = TRUE;
	if (0 != m_vecFunctionTest.size())
	{
		for (auto& pTest : m_vecFunctionTest)
		{
			delete pTest;
			pTest = nullptr;
		}
		m_vecFunctionTest.clear();
	}
}

CFuncReport::~CFuncReport()
{
	for (auto& Func : m_vecFunctionTest)
	{
		for (auto& pTest : m_vecFunctionTest)
		{
			delete pTest;
			pTest = nullptr;
		}
	}
	m_vecFunctionTest.clear();
}
void CFuncReport::SaveBoardSN(BYTE bySlotNo, char * cSN)
{
	if (nullptr == cSN)
	{
		return;
	}
	map<BYTE, string>::iterator iterBoardMSG = m_mapBoardSN.find(bySlotNo);
	if (m_mapBoardSN.end() == iterBoardMSG)
	{
		string strSN = cSN;
		m_mapBoardSN.insert(make_pair(bySlotNo, strSN));
	}
}
/**
*Add the CLK setting.
 * @param[in] T1R The rising edge time of drive data.
 * @param[in] T1F The falling edge of drive data.
 * @param[in] IOR The rising edge of drive I/O direction.
 * @param[in] IOF The falling edge of drive I/O direction.
 * @param[in] STBR The rising edge of comparsion.
 * @param[in] STBF The falling edge of comparsion.
 * @return no return.
*/
void CFuncReport::AddClkSetting(double T1R, double T1F, double IOR, double IOF, double STBR, double STBF)
{
	m_dClkSetting[0] = T1R;
	m_dClkSetting[1] = T1F;
	m_dClkSetting[2] = IOR;
	m_dClkSetting[3] = IOF;
	m_dClkSetting[4] = STBR;
	m_dClkSetting[5] = STBF;
//	for (int nIndex = 0; nIndex < 6; ++nIndex)
	{
//		if (-1 != m_dClkSetting[0])
		{
			vector<CFunctionTestMSG*>::iterator iterFuncTest = m_vecFunctionTest.begin();
			while (m_vecFunctionTest.end() != iterFuncTest)
			{
				if (TRUE == ((CFunctionTestMSG*)*iterFuncTest)->CompareTestItem(m_strTestItem))
				{
					((CFunctionTestMSG*)*iterFuncTest)->SetCLKSetting(m_dClkSetting);
					break;
				}
				++iterFuncTest;
			}
//			break;
		}
	}
}

void CFuncReport::AddTestItem(const char* format, ...)
{
	if (0 != m_strTestItem.compare("nullptr"))
	{
		vector<CFunctionTestMSG*>::iterator iterFuncTest = m_vecFunctionTest.begin();
		while (m_vecFunctionTest.end() != iterFuncTest)
		{
			if (TRUE == ((CFunctionTestMSG*)*iterFuncTest)->CompareTestItem(m_strTestItem))
			{
				break;
			}
			++iterFuncTest;
		}
		if (m_vecFunctionTest.end() == iterFuncTest)
		{
			CFunctionTestMSG* cFuncTest = new CFunctionTestMSG(m_strTestItem);
			m_vecFunctionTest.push_back(cFuncTest);
		}
	}
	char lpszTestItem[256] = { 0 };
	va_list args;
	va_start(args, format);
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	vsprintf_s(lpszTestItem, 256, format, args);
#else
	vsprintf(lpszTestItem, format, args);
#endif
	va_end(args);

	m_strTestItem = lpszTestItem;
	vector<CFunctionTestMSG*>::iterator iterFuncTest = m_vecFunctionTest.begin();
	while (m_vecFunctionTest.end() != iterFuncTest)
	{
		if (TRUE == ((CFunctionTestMSG*)*iterFuncTest)->CompareTestItem(m_strTestItem))
		{
			break;
		}
		++iterFuncTest;
	}
	if (m_vecFunctionTest.end() == iterFuncTest)
	{
		CFunctionTestMSG* cFuncTest = new CFunctionTestMSG(m_strTestItem);
		m_vecFunctionTest.push_back(cFuncTest);
	}
}

/**
*Save the channel number tested fail.
 * @param[in] usChannel The channel number tested fail.
 * @param[in] cFilePath The file path of the error report.
 * @return no return.
*/
void CFuncReport::SaveFailChannel(BYTE bySlotNo, USHORT usChannel)
{
	CFunctionTestMSG * pTestMSG = nullptr;
	vector<CFunctionTestMSG*>::iterator iterFuncTest = m_vecFunctionTest.begin();
	while (m_vecFunctionTest.end() != iterFuncTest)
	{
		if (TRUE == ((CFunctionTestMSG*)*iterFuncTest)->CompareTestItem(m_strTestItem))
		{

			break;
		}
		++iterFuncTest;
	}
	if (m_vecFunctionTest.end() != iterFuncTest)
	{
		((CFunctionTestMSG*)*iterFuncTest)->SaveFailChannel(bySlotNo ,usChannel);
		return;
	}
	pTestMSG = new CFunctionTestMSG(m_strTestItem);
	pTestMSG->SaveFailChannel(bySlotNo, usChannel);
	m_vecFunctionTest.push_back(pTestMSG);
	return;
}

void CFuncReport::SetNoBoardValid()
{
	m_bBoardValid = FALSE;
}

void CFuncReport::SaveAddtionMsg(const char* lpszFormat,...)
{
	char lpszMsg[256] = { 0 };
	va_list args;
	va_start(args, lpszFormat);
	vsprintf_s(lpszMsg, sizeof(lpszMsg), lpszFormat,args);
	m_strAddtionMsg = lpszMsg;

	CFunctionTestMSG* pTestMSG = nullptr;
	vector<CFunctionTestMSG*>::iterator iterFuncTest = m_vecFunctionTest.begin();
	while (m_vecFunctionTest.end() != iterFuncTest)
	{
		if (TRUE == ((CFunctionTestMSG*)*iterFuncTest)->CompareTestItem(m_strTestItem))
		{
			break;
		}
		++iterFuncTest;
	}
	if (m_vecFunctionTest.end() != iterFuncTest)
	{
		((CFunctionTestMSG*)*iterFuncTest)->SaveAdditionMsg(m_strAddtionMsg);
		return;
	}
	pTestMSG = new CFunctionTestMSG(m_strTestItem);
	pTestMSG->SaveAdditionMsg(m_strAddtionMsg);
	m_vecFunctionTest.push_back(pTestMSG);

}

/**
*Print the error message.
 * @param[in] testProj The project of test program.
 * @param[in] cFilePath The file path of the error report.
 * @return no return.
*/
void CFuncReport::Print(XTest* testProj, char* lpszFilePath)
{
	FILE *errFile = sts_fopen(lpszFilePath, "a+");
	//	sts_fprintf(errFile, "<========================================>\n");

	time_t nowTime = time(nullptr);
	tm t;
	sts_localtime(&t, &nowTime);
	char cTestTime[256] = { 0 };
	sts_sprintf(cTestTime, 256, "Test Date: %d-%02d-%02d %02d:%02d:%02d\n", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	testProj->PrintfA(cTestTime);
	sts_fprintf(errFile, "%s", cTestTime);

	//	testProj->PrintfA("\n==========================================================================\n");
	testProj->PrintfA("Test Type: %s\nFunction Name: %s\n", m_strTestType.c_str(), m_strFunctionName.c_str());
	//	sts_fprintf(errFile, "\n==========================================================================\n");
	sts_fprintf(errFile, "Test Type: %s\nFunction Name: %s\n", m_strTestType.c_str(), m_strFunctionName.c_str());

	if (0 == m_vecFunctionTest.size() && 0 != m_strAddtionMsg.size())
	{
		//	testProj->PrintfA("\n==========================================================================\n");
		testProj->PrintfA("Addition: %s\n", m_strAddtionMsg.c_str());
		//	sts_fprintf(errFile, "\n==========================================================================\n");
		sts_fprintf(errFile, "Addition: %s\n", m_strAddtionMsg.c_str());
	}

	if (FALSE == m_bBoardValid)
	{
		testProj->PrintfA("Test Result: FAIL!\n");
		sts_fprintf(errFile, "Test Result: FAIL!\n");
		testProj->PrintfA("\n=============>Detail Fail MSG<=============\n");
		sts_fprintf(errFile, "\n=============>Detail Fail MSG<=============\n");
		testProj->PrintfA("No valid board is inserted!\n");
		sts_fprintf(errFile, "No valid board is inserted!\n");

		sts_fprintf(errFile, "<========================================>\n\n");
		fclose(errFile);
		return;
	}
	
	BOOL bHavePass = TRUE;
	BOOL bHaveFail = FALSE;
	int nCurTestResult = FALSE;
	for (auto& Test : m_vecFunctionTest)
	{
		nCurTestResult = Test->GetTestResult();
		if (1 != nCurTestResult)
		{
			bHavePass = TRUE;
			if (2 == nCurTestResult)
			{
				bHaveFail = TRUE;
				break;
			}
		}
		else
		{
			bHaveFail = TRUE;
		}
	}
	if (bHavePass && !bHaveFail)
	{
		testProj->PrintfA("Test Result:All Test is PASS!\n");
		sts_fprintf(errFile, "Test Result:All Test is PASS!\n");
	}
	else if (bHavePass && bHaveFail)//0 != nPassItemCount)
	{
		testProj->PrintfA("Test Result:Not All Test is PASS!\n");
		sts_fprintf(errFile, "Test Result: Not All Test is PASS!\n");
	}
	else
	{
		testProj->PrintfA("Test Result:All Test is FAIL!\n");
		sts_fprintf(errFile, "Test Result:All Test is FAIL!\n");
	}


	testProj->PrintfA("\n=============>Detail Test Item<=============\n");
	sts_fprintf(errFile, "\n=============>Detail Test Item<=============\n");
	fclose(errFile);

	for (auto& Board : m_mapBoardSN)
	{
		FILE *errFile = sts_fopen(lpszFilePath, "a+");
		testProj->PrintfA("\n--------------------------------Slot: %d-----------------------------------------------\n", Board.first);
		sts_fprintf(errFile, "\n-----------------------------Slot: %d--------------------------------------------------\n", Board.first);
		testProj->PrintfA("SN: %s\n", Board.second.c_str());
		sts_fprintf(errFile, "SN: %s\n", Board.second.c_str());
		fclose(errFile);

		for (auto& FuncTest : m_vecFunctionTest)
		{
			FuncTest->Print(testProj, lpszFilePath, Board.first);
		}

		errFile = sts_fopen(lpszFilePath, "a+");
		testProj->PrintfA("\n-------------------------------------------------------------------------------------------\n");
		sts_fprintf(errFile, "\n-------------------------------------------------------------------------------------------\n");
		fclose(errFile);
	}

	errFile = sts_fopen(lpszFilePath, "a+");
	sts_fprintf(errFile, "<========================================>\n\n");
	fclose(errFile);
}

CFunctionTestMSG::CFunctionTestMSG(const string& strTestItem)
{
	m_strTestItem = strTestItem;
	m_bAllTestPass = TRUE;
}

CFunctionTestMSG::~CFunctionTestMSG()
{
	if (0 != m_mapFailChannel.size())
	{
		for (auto& FailChannel : m_mapFailChannel)
		{
			if (0 != FailChannel.second.size())
			{
				FailChannel.second.clear();
			}
		}
		m_mapFailChannel.clear();
	}
	if (0 != m_vecCLKTest.size())
	{
		for (auto& CLKTest : m_vecCLKTest)
		{
			if (nullptr != CLKTest)
			{
				delete CLKTest;
				CLKTest = nullptr;
			}
		}
		m_vecCLKTest.clear();
	}
}

void CFunctionTestMSG::GetCLKSetting(double *dCLKSetting)
{
	memcpy(dCLKSetting, m_dClkSetting, 6 * sizeof(double));
}

void CFunctionTestMSG::SetCLKSetting(double * dCLKSetting)
{
	for (auto& CLKTest : m_vecCLKTest)
	{
		if (TRUE == CLKTest->CompareCLK(dCLKSetting))
		{
			return;
		}
	}
	memcpy_s(m_dClkSetting, 6 * sizeof(double), dCLKSetting, 6 * sizeof(double));
	CCLKTest* clkTest = new CCLKTest(dCLKSetting);
	if (0 != m_mapFailChannel.size())
	{
		for (auto& FailChannel : m_mapFailChannel)
		{
			clkTest->SaveFailChannel(FailChannel.first, FailChannel.second);
		}
		m_mapFailChannel.clear();
		m_bAllTestPass = TRUE;
	}
	m_vecCLKTest.push_back(clkTest);
}

/**
*Save the channel number tested fail.
 * @param[in] usChannel The channel number tested fail.
 * @param[in] cFilePath The file path of the error report.
 * @return no return.
*/
void CFunctionTestMSG::SaveFailChannel(BYTE bySlotNo, USHORT usChannel)
{
	if (-1 != m_dClkSetting[0])
	{
		for (auto& CLKTest : m_vecCLKTest)
		{
			if (TRUE == CLKTest->CompareCLK(m_dClkSetting))
			{
				CLKTest->SaveFailChannel(bySlotNo, usChannel);
				m_bAllTestPass = FALSE;
				return;
			}
		}
	}
	map<BYTE, vector<USHORT>>::iterator iterSlot = m_mapFailChannel.find(bySlotNo);
	if (m_mapFailChannel.end() == iterSlot)
	{
		vector<USHORT> vecChannel;
		m_mapFailChannel.insert(pair < BYTE, vector<USHORT>>(bySlotNo, vecChannel));
		iterSlot = m_mapFailChannel.find(bySlotNo);
	}
	iterSlot->second.push_back(usChannel);
	m_bAllTestPass = FALSE;
}

BOOL CFunctionTestMSG::CompareTestItem(const string& strTestItem)
{
	if (0 !=m_strTestItem.compare(strTestItem))
	{
		return FALSE;
	}
	return TRUE;
}

void CFunctionTestMSG::SaveAdditionMsg(const string& strAddition)
{
	m_vecAdditionMsg.push_back(strAddition);
	if (-1 != m_dClkSetting[0])
	{
		for (auto& CLKTest : m_vecCLKTest)
		{
			if (TRUE == CLKTest->CompareCLK(m_dClkSetting))
			{
				int nAddtionCount = m_vecAdditionMsg.size();
				for (auto& Addition : m_vecAdditionMsg)
				{
					CLKTest->SaveAdditonMsg(Addition);
				}
				m_bAllTestPass = FALSE;
				return;
			}
		}
	}
}

int CFunctionTestMSG::GetTestResult()
{
	if (m_bAllTestPass)
	{
		return 0;
	}
	if (0 != m_mapFailChannel.size())
	{
		return 1;
	}
	BOOL bAllTestPass = TRUE;
	BOOL bHavePass = FALSE;
	BOOL bCurTestResult = FALSE;
	BOOL bHaveFail = FALSE;
	for (auto& CLKTest : m_vecCLKTest)
	{
		bCurTestResult = CLKTest->IsTestPass();
		if (bCurTestResult)
		{
			bHavePass = TRUE;
		}
		else
		{
			bHaveFail = TRUE;
		}
	}
	if (bHavePass && bHaveFail)
	{
		return 2;
	}
	else if (bHavePass && !bHaveFail)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

template <typename T>
void QuickSort(T* SortNum, int nLeft, int nRight)
{
	if (nLeft < nRight)
	{
		T key = SortNum[nLeft];
		int nLow = nLeft;
		int nHigh = nRight;
		while (nLow < nHigh)
		{
			while (nLow < nHigh && SortNum[nHigh] > key)
			{
				--nHigh;
			}
			SortNum[nLow] = SortNum[nHigh];
			while (nLow < nHigh && SortNum[nLow] <= key)
			{
				++nLow;
			}
			SortNum[nHigh] = SortNum[nLow];
		}
		SortNum[nLow] = key;
		QuickSort(SortNum, nLeft, nLow - 1);
		QuickSort(SortNum, nLow + 1, nRight);
	}
}

void printChannel(BYTE bySlotNo, vector<USHORT> &vecFailChannel, XTest *testProj, const char* cFilePath)
{
	FILE *errFile = sts_fopen(cFilePath, "a+");
	testProj->PrintfA("%-16s%-16s%-16s\n", "Slot", "Control Index", "Channel No.");
	sts_fprintf(errFile, "%-16s%-16s%-16s\n", "Slot", "Control Index", "Channel No.");
	USHORT * pusChannel = new USHORT[vecFailChannel.size()];
	int nChannelCount = 0;
	for (auto usChannel : vecFailChannel)
	{
		pusChannel[nChannelCount++] = usChannel;
	}
	QuickSort(pusChannel, 0, nChannelCount - 1);
	int nCtlIndex = 0;
	int nLastSlotNo = 0;
	int nLastCtlIndex = 0;
	USHORT usChannelOffset = 0;
	for (int nIndex = 0; nIndex < nChannelCount; ++nIndex)
	{
		if (pusChannel[nIndex] == pusChannel[nIndex - 1])
		{
			//Not print the same channel and the channel not in the scale.
			continue;
		}
		usChannelOffset = pusChannel[nIndex] % 64;
		nCtlIndex = usChannelOffset / 16;

		if (0 == nIndex || nLastSlotNo != bySlotNo || nLastCtlIndex != nCtlIndex)
		{
			if (0 != nIndex)
			{
				testProj->PrintfA("\n");
				sts_fprintf(errFile, "\n");
			}
			testProj->PrintfA("%-16d%-16d", bySlotNo, nCtlIndex);
			sts_fprintf(errFile, "%-16d%-16d", bySlotNo, nCtlIndex);
		}
		testProj->PrintfA("%2d ", usChannelOffset % 16);
		sts_fprintf(errFile, "%2d ", usChannelOffset % 16);
		nLastSlotNo = bySlotNo;
		nLastCtlIndex = nCtlIndex;
	}
	testProj->PrintfA("\n");
	sts_fprintf(errFile, "\n");
	fclose(errFile);
	if (nullptr != pusChannel)
	{
		delete[] pusChannel;
		pusChannel = nullptr;
	}
}


/**
*Print the error message.
 * @param[in] testProj The project of test program.
 * @param[in] cFilePath The file path of the error report.
 * @return no return.
*/
void CFunctionTestMSG::Print(XTest* testProj, char* cFilePath, BYTE bySlotNo)
{
	FILE *errFile = sts_fopen(cFilePath, "a+");

	testProj->PrintfA("\n===============> %s <===============\n", m_strTestItem.c_str());
	sts_fprintf(errFile, "\n===============> %s <===============\n", m_strTestItem.c_str());
	double dCLKEdge[6] = { 0 };
	BOOL bPrintPass = FALSE;
	if (0 != m_vecCLKTest.size())
	{
		int nFailCount = 0;
		for (auto& CLKTest : m_vecCLKTest)
		{
			if (TRUE == CLKTest->IsTestPass())
			{
				string strAddtion;
				int nRetVal = 0;
				int nIndex = 0;
				while (TRUE)
				{
					nRetVal = CLKTest->GetAddtionMsg(nIndex, strAddtion);
					if (0 != nRetVal)
					{
						break;
					}
					if (0 != strAddtion.size())
					{
						testProj->PrintfA("\n%s\n", strAddtion.c_str());
						sts_fprintf(errFile, "\n%s\n", strAddtion.c_str());
					}
					++nIndex;
				}
				if (!bPrintPass)
				{
					testProj->PrintfA("\n------------> Detail Pass CLK Edge <------------\n");
					sts_fprintf(errFile, "\n------------> Detail Pass CLK Edge <-----------\n");
					bPrintPass = TRUE;
				}
				CLKTest->GetCLKSetting(dCLKEdge);
				++nFailCount;
				testProj->PrintfA("The CLK Setting:T1R %.1f, T1F %.1f, IOR %.1f, IOF %.1f, STBR %.1f, STBF %.1f\n", dCLKEdge[0], dCLKEdge[1], dCLKEdge[2], dCLKEdge[3], dCLKEdge[4], dCLKEdge[5]);
				sts_fprintf(errFile, "The CLK Setting:T1R %.1f, T1F %.1f, IOR %.1f, IOF %.1f, STBR %.1f, STBF %.1f\n", dCLKEdge[0], dCLKEdge[1], dCLKEdge[2], dCLKEdge[3], dCLKEdge[4], dCLKEdge[5]);
			}
		}
		fclose(errFile);
		if (nFailCount == m_vecCLKTest.size())
		{
			return;
		}
		BOOL bPrintFail = FALSE;
		for (auto& CLKTest : m_vecCLKTest)
		{
			if (FALSE == CLKTest->IsTestPass() && CLKTest->IsHaveChannel(bySlotNo))
			{
				errFile = sts_fopen(cFilePath, "a+");

				string strAddtion;
				int nRetVal = 0;
				int nIndex = 0;
				while (TRUE)
				{
					nRetVal = CLKTest->GetAddtionMsg(nIndex, strAddtion);
					if (0 != nRetVal)
					{
						break;
					}
					if (0 != strAddtion.size())
					{
						testProj->PrintfA("\n%s\n", strAddtion.c_str());
						sts_fprintf(errFile, "\n%s\n", strAddtion.c_str());
					}
					++nIndex;
				}
				if (!bPrintFail)
				{
					testProj->PrintfA("\n------------> Detail FAIL CLK Edge <------------\n");
					sts_fprintf(errFile, "\n------------> Detail FAIL CLK Edge <-----------\n");
					bPrintFail = TRUE;
				}
				CLKTest->GetCLKSetting(dCLKEdge);
				//errFile = sts_fopen(cFilePath, "a+");
				testProj->PrintfA("The CLK Setting:T1R %.1f, T1F %.1f, IOR %.1f, IOF %.1f, STBR %.1f, STBF %.1f\n", dCLKEdge[0], dCLKEdge[1], dCLKEdge[2], dCLKEdge[3], dCLKEdge[4], dCLKEdge[5]);
				sts_fprintf(errFile, "The CLK Setting:T1R %.1f, T1F %.1f, IOR %.1f, IOF %.1f, STBR %.1f, STBF %.1f\n", dCLKEdge[0], dCLKEdge[1], dCLKEdge[2], dCLKEdge[3], dCLKEdge[4], dCLKEdge[5]);
				fclose(errFile); 
				vector<USHORT>vecFailChannel;
				CLKTest->GetFailChannel(bySlotNo, vecFailChannel);
				printChannel(bySlotNo, vecFailChannel, testProj, cFilePath);
			}
		}
		return;
	}
	BOOL bAllTestPass = TRUE;
	auto iterSlot = m_mapFailChannel.find(bySlotNo);
	if (!m_bAllTestPass)
	{
		if (m_mapFailChannel.end() != iterSlot && 0 != iterSlot->second.size())
		{
			bAllTestPass = FALSE;
		}
	}

	for (auto& Addition : m_vecAdditionMsg)
	{
		testProj->PrintfA("\n%s\n", Addition.c_str());
		sts_fprintf(errFile, "\n%s\n", Addition.c_str());
	}

	if (bAllTestPass)
	{
		testProj->PrintfA("\n------------> All Test PASS <------------\n");
		sts_fprintf(errFile, "\n------------> All Test PASS <-----------\n");
		fclose(errFile);
	}
	else
	{
		testProj->PrintfA("\n------------> Detail FAIL MSG <------------\n");
		sts_fprintf(errFile, "\n------------> Detail FAIL MSG <-----------\n");
		fclose(errFile);
		printChannel(bySlotNo, iterSlot->second, testProj, cFilePath);
	}
}

CTimeReport::CTimeReport(const char* cFunctionName, const char* cTestItem)
	: m_nTimes(1)
{
	string strFuncName = cFunctionName;
	int nPos = strFuncName.find("TestDCM");
	if (string::npos != nPos)
	{
		strFuncName = strFuncName.erase(0, 10);
	}
	nPos = strFuncName.find("RunningTime");
	if (string::npos != nPos)
	{
		strFuncName = strFuncName.erase(nPos, 11);
	}

	strcpy_s(m_lpszFunctionName, 128, strFuncName.c_str());
	strcpy_s(m_lpszTestItem, 128, cTestItem);
	QueryPerformanceFrequency(&m_TimeFreq);
	m_dTime = -1;
	m_bBoardValid = TRUE;
	if (0 != m_vecTimeMsg.size())
	{
		for (auto& TimeMsg : m_vecTimeMsg)
		{
			if (nullptr != TimeMsg)
			{
				delete TimeMsg;
			}
		}
		m_vecTimeMsg.clear();
	}
	m_bNewRecord = FALSE;
	m_TimeStart.QuadPart = 0;
	m_TimeStop.QuadPart = 0;
}

CTimeReport::~CTimeReport()
{
	for (auto& TimeMsg : m_vecTimeMsg)
	{
		if (nullptr != TimeMsg)
		{
			delete TimeMsg;
			TimeMsg = nullptr;
		}
	}
	m_vecTimeMsg.clear();
}

void CTimeReport::SaveBoardSN(BYTE bySlotNo, char * cSN)
{
	if (nullptr == cSN)
	{
		return;
	}
	auto iterBoardMSG = m_mapBoardSN.find(bySlotNo);
	if (m_mapBoardSN.end() == iterBoardMSG)
	{
		string strSN = cSN;
		m_mapBoardSN.insert(make_pair(bySlotNo, strSN));
	}
}

void CTimeReport::timeStart()
{
	QueryPerformanceCounter(&m_TimeStart);
}

double CTimeReport::timeStop()
{
	QueryPerformanceCounter(&m_TimeStop);
	m_dTime = (double)(m_TimeStop.QuadPart - m_TimeStart.QuadPart) / m_TimeFreq.QuadPart / m_nTimes;
	if (m_bNewRecord)
	{
		m_vecTimeMsg.at(m_vecTimeMsg.size() - 1)->SaveTime(m_dTime);
		m_dTime = -1;
		m_bNewRecord = FALSE;
	}
	return m_dTime;
}

void CTimeReport::SetTimes(int nTimes)
{
	m_nTimes = nTimes;
}

void CTimeReport::addMsg(const char* format, ...)
{
	char lpszErrMsg[1024] = { 0 };

	//	sts_sprintf(cErrMsg, 256, format, args);
	va_list args;
	va_start(args, format);
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	vsprintf_s(lpszErrMsg, 1024, format, args);
#else
	vsprintf(lpszErrMsg, format, args);
#endif
	va_end(args);
	CTimeMSG* timeMSG = new CTimeMSG(lpszErrMsg);

	if (-1 != m_dTime)
	{
		timeMSG->SaveTime(m_dTime);
		m_dTime = -1;
	}
	else
	{
		m_bNewRecord = TRUE;
	}
	m_vecTimeMsg.push_back(timeMSG);
}

void CTimeReport::Print(XTest* testProj, const char* cFilePath)
{
	FILE *pReport = sts_fopen(cFilePath, "a+");

	time_t nowTime = time(nullptr);
	tm t;
	sts_localtime(&t, &nowTime);
	char cTestTime[256] = { 0 };
	sts_sprintf(cTestTime, 256, "Test Date: %d-%02d-%02d %02d:%02d:%02d\n", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	testProj->PrintfA(cTestTime);
	sts_fprintf(pReport, "%s", cTestTime);

	//	testProj->PrintfA("\n==========================================================================\n");
	testProj->PrintfA("Test Item: %s\nFunction Name: %s\n", m_lpszTestItem, m_lpszFunctionName);
	//	sts_fprintf(errFile, "\n==========================================================================\n");
	sts_fprintf(pReport, "Test Item: %s\nFunction Name: %s\n", m_lpszTestItem, m_lpszFunctionName);

	if (FALSE == m_bBoardValid)
	{
		testProj->PrintfA("\nNo valid board is inserted!\n");
		sts_fprintf(pReport, "\nNo valid board is inserted!\n");

		testProj->PrintfA("<========================================>\n\n");
		sts_fprintf(pReport, "<========================================>\n\n");
		fclose(pReport);
		return;
	}
	
	testProj->PrintfA("\n----------Board Tested------------\n");
	sts_fprintf(pReport, "\n----------Board Tested------------\n");

	testProj->PrintfA("%-16s %s\n", "Board Index", "Board SN");
	sts_fprintf(pReport, "%-16s %s\n", "Board Index", "Board SN");


	for (auto& BoardSN : m_mapBoardSN)
	{
		testProj->PrintfA("%-16d %s", BoardSN.first, BoardSN.second.c_str());
		sts_fprintf(pReport, "%-16d %s", BoardSN.first, BoardSN.second.c_str());
	}

	testProj->PrintfA("\n=============>Detail Time consumption MSG<=============\n");
	sts_fprintf(pReport, "\n=============>Detail Time consumption MSG<=============\n");
	if (-1 != m_dTime)
	{
		fprintf(pReport, "Time Consumption: %f us\n\n", m_dTime);
		testProj->PrintfA("Time Consumption: %f us\n\n", m_dTime);
	}

	fclose(pReport);
	for (auto& TimeMsg : m_vecTimeMsg)
	{
		TimeMsg->Print(testProj, cFilePath);
	}
	if (0 != m_strAdditionTittle.size())
	{
		pReport = sts_fopen(cFilePath, "a+");

		fprintf(pReport, "\n===================%s=====================\n", m_strAdditionTittle.c_str());
		testProj->PrintfA("\n===================%s=====================\n", m_strAdditionTittle.c_str());
		for (auto& Data : m_vecAddition)
		{
			fprintf(pReport, "%s:%.1f%s\n", Data.m_strName.c_str(),Data.m_dData, Data.m_strUnit.c_str());
			testProj->PrintfA("%s:%.1f%s\n", Data.m_strName.c_str(), Data.m_dData, Data.m_strUnit.c_str());
		}
		fclose(pReport);
	}
}

void CTimeReport::SetNoBoardValid()
{
	m_bBoardValid = FALSE;
}

int CTimeReport::SetAdditionItemTittle(const char* lpszFormat, ...)
{
	if (nullptr == lpszFormat)
	{
		return -1;
	}
	va_list ap;
	va_start(ap, lpszFormat);
	char lpszMsg[128] = { 0 };
	vsprintf_s(lpszMsg, sizeof(lpszMsg), lpszFormat, ap);
	va_end(ap);
	m_strAdditionTittle = lpszMsg;
	return 0;
}

int CTimeReport::AdditionItem(const char* lpszItem, double dData, const char* lpszUnit)
{
	if (nullptr == lpszItem)
	{
		return -1;
	}
	string strUnit = "";
	if (nullptr != lpszUnit)
	{
		strUnit = lpszUnit;
	}
	m_vecAddition.push_back(ITEM_DATA(lpszItem, dData, strUnit.c_str()));
	return 0;
}

CTimeMSG::CTimeMSG(const char * cMSG)
{
	m_dTime = 0;
	strcpy_s(m_lpszMSG, 1024, cMSG);
}

CTimeMSG::~CTimeMSG()
{

}

void CTimeMSG::SaveTime(double dTime)
{
	m_dTime = dTime;
	m_strUnits = "s";
	if (1e-3 > m_dTime)
	{
		m_dTime *= 1e6;
		m_strUnits = "us";
	}
	else if (1 > m_dTime)
	{
		m_dTime *= 1e3;
		m_strUnits = "ms";
	}
	else if (60 > m_dTime)
	{
		m_strUnits = "s";
	}
	else if (3600 > m_dTime)
	{
		m_dTime /= 60;
		m_strUnits = "min";
	}
	else
	{
		m_dTime /= 3600;
		m_strUnits = "h";
	}
}

void CTimeMSG::Print(XTest * testProj, const char * cFilePath)
{
	FILE *errFile = sts_fopen(cFilePath, "a+");

	fprintf(errFile, "%s\n", m_lpszMSG);
	testProj->PrintfA("%s\n", m_lpszMSG);
	fprintf(errFile, "Time Consumption: %f %s\n\n", m_dTime, m_strUnits.c_str());
	testProj->PrintfA("Time Consumption: %f %s\n\n", m_dTime, m_strUnits.c_str());

	fclose(errFile);
}

CCLKTest::CCLKTest(double * dCLKEdge)
{
	if (nullptr == dCLKEdge)
	{
		memset(m_dCLKEdge, -1, sizeof(m_dCLKEdge));
	}
	else
	{
		memcpy(m_dCLKEdge, dCLKEdge, sizeof(m_dCLKEdge));
	}
	
	m_bAllTestPass = TRUE;
}

CCLKTest::~CCLKTest()
{
	if (0 != m_mapFailChannel.size())
	{
		map<BYTE, vector<USHORT>>::iterator iterSlot = m_mapFailChannel.begin();
		while (m_mapFailChannel.end() != iterSlot)
		{
			if (0 != iterSlot->second.size())
			{
				iterSlot->second.clear();
			}
			++iterSlot;
		}
		m_mapFailChannel.clear();
	}
}

void CCLKTest::SaveFailChannel(BYTE bySlotNo, USHORT usChannel)
{
	map<BYTE, vector<USHORT>>::iterator iterSlot = m_mapFailChannel.find(bySlotNo);
	if (m_mapFailChannel.end() == iterSlot)
	{
		vector<USHORT> vecChannel;
		m_mapFailChannel.insert(pair<BYTE, vector<USHORT>>(bySlotNo, vecChannel));
		iterSlot = m_mapFailChannel.find(bySlotNo);
	}
	iterSlot->second.push_back(usChannel);
	m_bAllTestPass = FALSE;
}

void CCLKTest::SaveFailChannel(BYTE bySlotNo, vector<USHORT>& vecChannel)
{
	map<BYTE, vector<USHORT>>::iterator iterSlot = m_mapFailChannel.find(bySlotNo);
	if (m_mapFailChannel.end() == iterSlot)
	{
		m_mapFailChannel.insert(pair<BYTE, vector<USHORT>>(bySlotNo, vecChannel));
		iterSlot = m_mapFailChannel.find(bySlotNo);
		return;
	}
	vector<USHORT>* pvecChannel = &iterSlot->second;

	int nChannelCount = vecChannel.size();
	for (int nChannelIndex = 0; nChannelIndex < nChannelCount;++nChannelIndex)
	{
		pvecChannel->push_back(vecChannel[nChannelIndex]);
	}
}

BOOL CCLKTest::CompareCLK(double * dCLKEdge)
{
	if (nullptr == dCLKEdge)
	{
		return FALSE;
	}
	if (0 != memcmp(dCLKEdge, m_dCLKEdge, sizeof(m_dCLKEdge)))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CCLKTest::IsTestPass()
{
	return m_bAllTestPass;
}

void CCLKTest::GetFailChannel(BYTE bySlotNo, vector<USHORT>& vecFailChannel)
{
	vecFailChannel.clear();
	map<BYTE, vector<USHORT>>::iterator iterSlot = m_mapFailChannel.find(bySlotNo);
	if (m_mapFailChannel.end() == iterSlot)
	{
		return;
	}
	vector<USHORT>* pvecChannel = &iterSlot->second;
	int nChannelCount = pvecChannel->size();
	for (int nIndex = 0; nIndex < pvecChannel->size(); ++nIndex)
	{
		vecFailChannel.push_back(pvecChannel->at(nIndex));
	}
}

void CCLKTest::GetCLKSetting(double * dCLKValue)
{
	memcpy(dCLKValue, m_dCLKEdge, sizeof(m_dCLKEdge));
}

BOOL CCLKTest::IsHaveChannel(BYTE bySlotNo)
{
	map<BYTE, vector<USHORT>>::iterator iterSlot = m_mapFailChannel.find(bySlotNo);
	if (m_mapFailChannel.end() == iterSlot)
	{
		return FALSE;
	}
	return TRUE;
}

void CCLKTest::SaveAdditonMsg(const string& strAddition)
{
	m_vecAdditionMsg.push_back(strAddition);
}

int CCLKTest::GetAddtionMsg(int nIndex, string& strAddition)
{
	if (m_vecAdditionMsg.size() <= nIndex)
	{
		return -1;
	}
	strAddition = m_vecAdditionMsg[nIndex];
	return 0;
}

CMeasurementItem::CMeasurementItem(const char* lpszItemName)
{
	if (nullptr != lpszItemName)
	{
		m_strItemName = lpszItemName;
	}
	m_bHasAdditionMsg = FALSE;
	m_bHasUnit = FALSE;
	m_nMaxDataSize = 12;
}

void CMeasurementItem::GetName(string& strItemName)
{
	strItemName = m_strItemName;
}

void CMeasurementItem::SetTestCondition(const char* lpszCondition, va_list ArgList)
{
	char lpszTotalCondition[256] = { 0 };
	vsprintf_s(lpszTotalCondition, sizeof(lpszTotalCondition), lpszCondition, ArgList);
	m_strTestCondition = lpszTotalCondition;
}

void CMeasurementItem::SetFailInfo(const char* lpszFailFormat, va_list ArgList)
{
	char lpszFailInfo[256] = { 0 };
	vsprintf_s(lpszFailInfo, sizeof(lpszFailInfo), lpszFailFormat, ArgList);

	m_strFailInfo = lpszFailInfo;
}

int CMeasurementItem::AddFailChannel(BYTE bySlotNo, USHORT usChannel, const string& strExpectData, const string& strRealData, std::string& strUnit, const char* lpszAdditionMsg, va_list ArgList)
{
	UINT uChannelID = GetChannelID(bySlotNo, usChannel);
	auto iterChannel = m_mapFailChannel.find(uChannelID);
	if (m_mapFailChannel.end() != m_mapFailChannel.find(uChannelID))
	{
		///<Channel is existed
		return -1;
	}
	FAIL_MSG FailMsg;
	FailMsg.m_strRealData = strRealData;
	FailMsg.m_strExpectData = strExpectData;
	FailMsg.m_strUnit = strUnit;
	if (nullptr != lpszAdditionMsg)
	{
		char lpszMsg[256] = { 0 };
		vsprintf_s(lpszMsg, sizeof(lpszMsg), lpszAdditionMsg, ArgList);
		FailMsg.m_strAddition = lpszMsg;
	}
	if (!m_bHasUnit && 0 != FailMsg.m_strUnit.size())
	{
		m_bHasUnit = TRUE;
	}
	if (!m_bHasAdditionMsg && 0 != FailMsg.m_strAddition.size())
	{
		m_bHasAdditionMsg = TRUE;
	}
	m_mapFailChannel.insert(make_pair(uChannelID, FailMsg));
	return 0;
}

void CMeasurementItem::Print(XTest* pTestProj, char* lpszFilePath)
{
	if (nullptr == pTestProj || nullptr == lpszFilePath)
	{
		return;
	}
	FILE* pLogFile = nullptr;
	fopen_s(&pLogFile, lpszFilePath, "a+");
	if (nullptr == pLogFile)
	{
		return;
	}

	char lpszMsg[256] = { 0 };
	if (0 != m_strItemName.size())
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "=============== %s ===============\n\n", m_strItemName.c_str());
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pLogFile, lpszMsg);
	}
	else
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "==============================\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pLogFile, lpszMsg);
	}

	if (0 != m_strTestCondition.size())
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "----------> Test Condition <----------\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pLogFile, lpszMsg);

		pTestProj->PrintfA(m_strTestCondition.c_str());
		fprintf_s(pLogFile, m_strTestCondition.c_str());

		pTestProj->PrintfA("\n\n");
		fprintf_s(pLogFile, "\n\n");

	}

	if (0 == m_mapFailChannel.size() && 0 == m_strFailInfo.size())
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "All channels test pass\n\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pLogFile, lpszMsg);
		fclose(pLogFile);
		return;
	}

	if (0 != m_strFailInfo.size())
	{

		sprintf_s(lpszMsg, sizeof(lpszMsg), "----------> Fail information <----------\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pLogFile, lpszMsg);

		pTestProj->PrintfA(m_strFailInfo.c_str());
		fprintf_s(pLogFile, m_strFailInfo.c_str());
		pTestProj->PrintfA("\n\n");
		fprintf_s(pLogFile, "\n\n");
	}
	if (0 == m_mapFailChannel.size())
	{
		fclose(pLogFile);
		return;
	}

	sprintf_s(lpszMsg, sizeof(lpszMsg), "----------> Detail fail information <----------\n");
	pTestProj->PrintfA(lpszMsg);
	fprintf_s(pLogFile, lpszMsg);


	m_nMaxDataSize += 2;

	string strMsg;
	string strCurMsg;
	FormatMsg(5, 1, strCurMsg, "Slot"); 
	strMsg += strCurMsg;

	FormatMsg(9, 1, strCurMsg, "Channel");
	strMsg += strCurMsg;

	FormatMsg(m_nMaxDataSize, 1, strCurMsg, "Expect data");
	strMsg += strCurMsg;

	FormatMsg(m_nMaxDataSize, 1, strCurMsg, "Real data");
	strMsg += strCurMsg;

	if (m_bHasUnit)
	{
		FormatMsg(10, 1, strCurMsg, "Unit");
		strMsg += strCurMsg;
	}

	if (m_bHasAdditionMsg)
	{
		FormatMsg(20, 0, strCurMsg, "Addition message");
		strMsg += strCurMsg;
	}
	strMsg += "\n";
	pTestProj->PrintfA(strMsg.c_str());
	fprintf_s(pLogFile, strMsg.c_str());

	BYTE bySlotNo = 0;
	USHORT usChannel = 0;
	auto iterChannel = m_mapFailChannel.begin();
	while (m_mapFailChannel.end() != iterChannel)
	{
		strMsg.clear();

		bySlotNo = GetChannel(iterChannel->first, usChannel);
		///<Slot number
		FormatMsg(5, 1, strCurMsg, "%d", bySlotNo);
		strMsg += strCurMsg;

		///<Channel
		FormatMsg(9, 1, strCurMsg, "%d", usChannel);
		strMsg += strCurMsg;

		///<Expect data
		FormatMsg(m_nMaxDataSize, 1, strCurMsg, iterChannel->second.m_strExpectData.c_str());
		strMsg += strCurMsg;

		///<Real data
		FormatMsg(m_nMaxDataSize, 1, strCurMsg, iterChannel->second.m_strRealData.c_str());
		strMsg += strCurMsg;

		if (m_bHasUnit && 0 != iterChannel->second.m_strUnit.size())
		{
			///<Unit of channel
			FormatMsg(10, 1, strCurMsg, iterChannel->second.m_strUnit.c_str());
			strMsg += strCurMsg;
		}
		if (m_bHasAdditionMsg && 0 != iterChannel->second.m_strAddition.size())
		{
			///<Addition message of the channel
			FormatMsg(0, 0, strCurMsg, iterChannel->second.m_strAddition.c_str());
			strMsg += strCurMsg;
		}
		strMsg += "\n";

		pTestProj->PrintfA(strMsg.c_str());
		fprintf_s(pLogFile, strMsg.c_str());
		++iterChannel;
	}
	pTestProj->PrintfA("\n");
	fprintf_s(pLogFile, "\n");

	fclose(pLogFile);
}

BOOL CMeasurementItem::IsTestPass()
{
	return 0 != m_mapFailChannel.size() || 0 != m_strFailInfo.size() ? FALSE : TRUE;
}

inline int CMeasurementItem::GetChannelID(BYTE bySlotNo, USHORT usChannel)
{
	return bySlotNo << 24 | usChannel;
}

inline BYTE CMeasurementItem::GetChannel(UINT uChannelID, USHORT& usChannel)
{
	usChannel = uChannelID & 0xFFFF;
	return uChannelID >> 24;
}

CMeasurementFuncReport::CMeasurementFuncReport(const char* lpszFunctionName, const char* lpszTestType)
{
	if (nullptr != lpszFunctionName)
	{
		m_strFunctionName = lpszFunctionName;
	}
	if (nullptr != lpszTestType)
	{
		m_strTestType = lpszTestType;
	}
	m_bAllTestPass = TRUE;
}

CMeasurementFuncReport::~CMeasurementFuncReport()
{
	for (auto& Item : m_vecTestItem)
	{
		if (nullptr != Item)
		{
			delete Item;
			Item = nullptr;
		}
	}
	m_vecTestItem.clear();
}

void CMeasurementFuncReport::SaveBoardSN(BYTE bySlotNo, char* lpszSN)
{
	if (m_mapBoardSN.end() != m_mapBoardSN.find(bySlotNo))
	{
		return;
	}
	m_mapBoardSN.insert(make_pair(bySlotNo, lpszSN));
}

void CMeasurementFuncReport::AddTestItem(const char* lpszItemName)
{
	if (nullptr == lpszItemName)
	{
		return;
	}
	CMeasurementItem* pTestItem = new CMeasurementItem(lpszItemName);
	m_vecTestItem.push_back(pTestItem);
}

void CMeasurementFuncReport::SetFailInfo(const char* lpszFailFormat, ...)
{
	m_bAllTestPass = FALSE;
	CMeasurementItem* pTestItem = nullptr;
	if (0 == m_vecTestItem.size())
	{
		pTestItem = new CMeasurementItem(m_strTestType.c_str());
		m_vecTestItem.push_back(pTestItem);
	}
	pTestItem = m_vecTestItem[m_vecTestItem.size() - 1];
	va_list ap;
	va_start(ap, lpszFailFormat);
	pTestItem->SetFailInfo(lpszFailFormat, ap);
	va_end(ap);
}

void CMeasurementFuncReport::SetTestCondition(const char* lpszCondition, ...)
{
	CMeasurementItem* pTestItem = nullptr;
	if (0 == m_vecTestItem.size())
	{
		pTestItem = new CMeasurementItem(m_strTestType.c_str());
		m_vecTestItem.push_back(pTestItem);
	}
	pTestItem = m_vecTestItem[m_vecTestItem.size() - 1];

	va_list ap;
	va_start(ap, lpszCondition);
	pTestItem->SetTestCondition(lpszCondition, ap);
	va_end(ap);
}

void CMeasurementFuncReport::Print(XTest* pTestProj, char* lpszFilePath)
{
	if (nullptr == pTestProj || nullptr == lpszFilePath)
	{
		return;
	}

	FILE* pFileLog = sts_fopen(lpszFilePath, "a+");

	SYSTEMTIME SysTime;
	GetLocalTime(&SysTime);
	char lpszTestTime[256] = { 0 };
	sprintf_s(lpszTestTime, 256, "Test Date: %d-%02d-%02d %02d:%02d:%02d\n", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wMilliseconds);
	pTestProj->PrintfA(lpszTestTime);
	fprintf_s(pFileLog, "%s", lpszTestTime);

	///<Test information
	char lpszMsg[256] = { 0 };
	sprintf_s(lpszMsg, sizeof(lpszMsg), "Test Item: %s\nFunction Name: %s\n", m_strTestType.c_str(), m_strFunctionName.c_str());
	pTestProj->PrintfA(lpszMsg);
	fprintf_s(pFileLog, lpszMsg);

	if (0 == m_mapBoardSN.size())
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "Test Result: FAIL!\n\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pFileLog, lpszMsg);

		strcpy_s(lpszMsg, sizeof(lpszMsg), "\n=============>Detail Fail MSG<=============\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pFileLog, lpszMsg);

		strcpy_s(lpszMsg, sizeof(lpszMsg), "No valid board be inserted!\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pFileLog, lpszMsg);

		fprintf_s(pFileLog, "<========================================>\n");
		fclose(pFileLog);
		return;
	}

	int nItemCount = m_vecTestItem.size();
	BOOL bHavePass = FALSE;
	if (!m_bAllTestPass)
	{
		for (int nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex)
		{
			if (!bHavePass && m_vecTestItem[nItemIndex]->IsTestPass())
			{
				bHavePass = TRUE;
				break;
			}
		}
	}
	else
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "Test Result:All test items are PASS!\n\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pFileLog, lpszMsg);
	}

	if (!m_bAllTestPass && bHavePass)
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "Test Result:Not all test items are FAIL!\n\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pFileLog, lpszMsg);
	}
	else if(!m_bAllTestPass)
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "Test Result:All test items are FAIL!\n\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pFileLog, lpszMsg);
	}

	if (0 != m_mapBoardSN.size())
	{
		///<Board
		strcpy_s(lpszMsg, sizeof(lpszMsg), "===================> Test board <===================\n");
		pTestProj->PrintfA(lpszMsg);
		fprintf_s(pFileLog, lpszMsg);

		string strMsg;
		string strCurMsg;
		FormatMsg(9, 1, strCurMsg, "Slot");
		strMsg += strCurMsg;

		FormatMsg(30, 1, strCurMsg, "SN");
		strMsg += strCurMsg;
		strMsg += "\n";

		pTestProj->PrintfA(strMsg.c_str());
		fprintf_s(pFileLog, strMsg.c_str());


		auto iterBoard = m_mapBoardSN.begin();
		while (m_mapBoardSN.end() != iterBoard)
		{
			strMsg.clear();
			FormatMsg(9, 1, strCurMsg, "%d", iterBoard->first);
			strMsg += strCurMsg;

			FormatMsg(30, 1, strCurMsg, iterBoard->second.c_str());
			strMsg += strCurMsg;
			strMsg += "\n\n";

			pTestProj->PrintfA(strMsg.c_str());
			fprintf_s(pFileLog, strMsg.c_str());

			++iterBoard;
		}
	}

	strcpy_s(lpszMsg, sizeof(lpszMsg), "\n===================> Detail Test Item <===================\n\n");
	pTestProj->PrintfA(lpszMsg);
	fprintf_s(pFileLog, lpszMsg);
	fclose(pFileLog);

	for (int nItemIndex = 0; nItemIndex < nItemCount;++nItemIndex)
	{
		m_vecTestItem[nItemIndex]->Print(pTestProj, lpszFilePath);
	}
}
