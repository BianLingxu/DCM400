#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "XTest.h"
enum TEST_DATA_FORMAT
{
	VALUE_INT_DATA,
	VALUE_DOUBLE_ONE_DECIMAL,
	VALUE_DOUBLE_TWO_DECIMAL,
	VALUE_DOUBLE_THREE_DECIMAL,
	VALUE_DOUBLE_FOUR_DECIMAL,
};

enum ERROR_TYPE
{
	VALUE_ERROR,
	STRING_ERROR_MSG,
};


enum PIN_TYPE
{
	DCM_PIN_NAME,
	DCM_PIN_GROUP,
	DCM_NO_PIN,
};

enum VALUE_ERROR_TYPE
{
	VALUE_NOT_EQUAL,
	VALUE_SCALE
};

typedef struct detail_err
{
	double dExceptValue;
	double dActualValue;
	double dResolution;
	TEST_DATA_FORMAT dataFormat;
	detail_err()
	{
		dExceptValue = 0;
		dActualValue = 0;
		dResolution = 0;
		dataFormat = VALUE_INT_DATA;
	}
}VALUE_DETAIL;

typedef struct channel_err_msg
{
	BYTE m_bySlotNo;
	USHORT m_usChannelNo;
	char cParamName[16];
	VALUE_ERROR_TYPE valueType;
	std::vector<VALUE_DETAIL> vecDetailErr;
	channel_err_msg()
	{
		m_bySlotNo = 0;
		m_usChannelNo = -1;
		memset(cParamName, 0, sizeof(cParamName));
		if (0 != vecDetailErr.size())
		{
			vecDetailErr.clear();
		}
	}
}CHANNEL_ERR_MSG;


typedef struct other_err
{
	BYTE m_bySlotNo;
	USHORT m_usChannel;
	char cParamName[255];
	char cParamValue[255];
	BOOL bWithChannel;
	std::vector<std::string> vecErrMSG;
	other_err()
	{
		m_bySlotNo = 0;
		m_usChannel = -1;
		memset(cParamName, 0, sizeof(cParamName));
		memset(cParamValue, 0, sizeof(cParamValue));
		if (0 != vecErrMSG.size())
		{
			vecErrMSG.clear();
		}
	}
}OTHER_ERROR_MSG;


class CBaseErrorMSG
{
public:
	CBaseErrorMSG();
	~CBaseErrorMSG();
	/**
	 * @brief Print the error message
	 * @param[in] testProj The project of test program
	 * @param[in] lpszFilePath The file path of the error report
	*/
	virtual void ErrorPrint(XTest * testProj, char* lpszFilePath) = 0;
	/**
	 * @brief Save the pin name or pin group
	 * @param[in] pinString The name of pin or pin group
	 * @param[in] pinType The type of pinString
	*/
	void addPinNameOrPinGroup(const char* pinString = nullptr, PIN_TYPE pinType = DCM_NO_PIN);
	/**
	 * @brief Get the pin name or pin group
	 * @param[out] pinString The name of pin or pin group
	 * @param[in] nArrayLength The length of the pinString
	 * @return -1:The length of pinString is too small
	*/
	int getPinNameOrPinGroup(char* pinString, int nArrayLength);
	/**
	 * @brief Get the pin type
	 * @return The pin type
	*/
	PIN_TYPE getPinType();
private:
	char m_lpszPinName[32];
	char m_lpszPinGroup[32];
	PIN_TYPE m_enumPinType;
};

class CStringError:public CBaseErrorMSG
{
public:
	CStringError();
	~CStringError();
	/**
	 * @brief Set the parameter of the error message
	 * @param[in] cParamName The error name
	 * @param[in] cParamValue The value of the fail parameter
	 * @param[in] bWithChannel Whether the error parameter has channel message
	 * @param[in] usChannel The channel number of error message if needed
	*/
	void CStringError::SetParam(const char* cParamName, const char* cParamValue, BOOL bWithChannel, BYTE bySlotNo, USHORT usChannel);
	/**
	 * @brief Save the error message
	 * @param[in] cMSG The error message
	*/
	void AddMSG(char* cMSG);
	/**
	 * @brief Print the error message
	 * @param[in] testProj The project of test program
	 * @param[in] lpszFilePath The file path of the error report
	*/
	void ErrorPrint(XTest * testProj, char* lpszFilePath);
private:
	std::vector<OTHER_ERROR_MSG> m_vecParamName;
	USHORT m_usChannel;
	BYTE m_bySlotNo;
	char m_cParamName[255];
	char m_cParamValue[255];
	BOOL m_bWithChannel;
};

class CValueError :public CBaseErrorMSG
{
public:
	CValueError();
	~CValueError();
	/**
	 * @brief Set the parameter of the error message
	 * @param[in] cParamName The error name
	 * @param[in] usChannel The channel number of error message
	 * @param[in] enumValueErrorType The value error type
	*/
	void SetParam(const char* cParamName, BYTE bySlotNo, USHORT usChannel, VALUE_ERROR_TYPE enumValueErrorType);
	/**
	 * @brief Save the error message
	 * @param[in] dActualValue The actual value of the function param or input value if the error is value scale exceed
	 * @param[in] dExpcetValue The expect value of the function param or the biggest value of the scale if the error is value scale exceed
	 * @param[in] dResolution The resolution of the function param or the lowest value of the scale if the error is value scale exceed
	 * @param[in] enumDataFormat The value format of the function param or no care if the error is value scale exceed
	*/
	void AddMSG(double dActualValue, double dExpcetValue, double dResolution, TEST_DATA_FORMAT enumDataFormat = VALUE_INT_DATA);
	/**
	 * @brief Print the error message
	 * @param[in] testProj The project of test program
	 * @param[in] lpszFilePath The file path of the error report
	*/
	void ErrorPrint(XTest * testProj, char* lpszFilePath);
private:
	std::vector<CHANNEL_ERR_MSG> m_vecValueError;
	VALUE_ERROR_TYPE m_enumValueErrorType;
	USHORT m_usChannel;
	BYTE m_bySlotNo;
	char m_cParamName[30];
};

class CErrorMSG
{
public:
	/**
	 * @brief Constructor
	 * @param[in] cFunctionName The function name
	 * @param[in] cTestItem The test item
	*/
	CErrorMSG(const char* cFunctionName, const char* cTestItem);
	/**
	 * @brief Destructor
	*/
	~CErrorMSG();
	/**
	 * @brief Record error message
	 * @param[in] errType The error type
	 * @param[in] cPinString The name of pin name or pin group
	 * @param[in] nPinType Whether the cPinString is pin name, 0 is no pin name or pin type, 1 is pin name; 2 is pin group
	*/
	void AddNewError(ERROR_TYPE errType, const char* cPinString = nullptr, BYTE nPinType = 0);
	/**
	 * @brief Set the parameter of error message
	 * @param[in] cParamName The parameter which test fail
	 * @param[in] paramValue The value of the fail parameter
	 * @param[in] bWithChannel Whether the parameter has channel
	 * @param[in] usChannel The channel number if needed
	 * @return 0 is excute successfully; -1 is no error message be accorded before, call RecordError first
	*/
	int SetErrorItem(const char* cParamName = nullptr, const char* cParamValue = nullptr, BOOL bWithChannel = FALSE, BYTE bySlotNo = 0, USHORT usChannel = -1, VALUE_ERROR_TYPE enumValueErrorType = VALUE_NOT_EQUAL);
	/**
	 * @brief Save the error message
	 * @param[in] dActualValue The actual value of the function param or input value if the error is value scale exceed
	 * @param[in] dExpcetValue The expect value of the function param or the biggest value of the scale if the error is value scale exceed
	 * @param[in] dResolution The resolution of the function param or the lowest value of the scale if the error is value scale exceed
	 * @param[in] enumDataFormat The value format of the function param or no care if the error is value scale exceed
	 * @return 0 is excute successfully; -1 is no error message be accorded before, call RecordError first
	*/
	int SaveErrorMsg(double dActualValue, double dExpcetValue, double dResolution, TEST_DATA_FORMAT enumDataFormat = VALUE_INT_DATA);
	/**
	 * @brief Save the error message
	 * @param[in] format The format string
	 * @return 0 is excute successfully; -1 is no error message be accorded before, call RecordError first
	*/
	int SaveErrorMsg(const char* format, ...);
	/**
	 * @brief Print the error message
	 * @param[in] testProj The project of test program
	 * @param[in] lpszFilePath The file path of the error report
	*/
	void Print(XTest* testProj, char* lpszFilePath);

private:
	std::vector<CStringError*> m_vecOtherError;//Save the class
	std::vector<CValueError*> m_vecValueError;
	char m_lpszFunctionName[128];
	char m_lpszTestItem[128];
};

class CCLKTest
{
public:
	CCLKTest(double *dCLKEdge);
	~CCLKTest();
	/**
	 * @brief Save the channel number tested fail
	 * @param[in] usChannel The channel number tested fail
	 * @param[in] lpszFilePath The file path of the error report
	*/
	void SaveFailChannel(BYTE bySlotNo, USHORT usChannel);
	void SaveFailChannel(BYTE bySlotNo, std::vector<USHORT>& vecChannel);
	BOOL CompareCLK(double *dCLKEdge);
	BOOL IsTestPass();
	void GetFailChannel(BYTE bySlotNo, std::vector<USHORT> &vecFailChannel);
	void GetCLKSetting(double* dCLKValue);
	BOOL IsHaveChannel(BYTE bySlot);
	void SaveAdditonMsg(const std::string& strAddition);
	int GetAddtionMsg(int nIndex, std::string& strAddition);
private:
	double m_dCLKEdge[6];
	std::map<BYTE, std::vector<USHORT>> m_mapFailChannel;
	BOOL m_bAllTestPass;
	std::vector<std::string> m_vecAdditionMsg;
};

class CFunctionTestMSG
{
public:
	/**
	 * @brief Constructor function
	 * @param[in] dClkSetting The detail time Edge setting
	*/
	CFunctionTestMSG(const std::string& strTestItem);
	~CFunctionTestMSG();
	/**
	 * @brief Get the Edge setting
	 * @param[in] dCLKSetting The detail time Edge setting
	*/
	void GetCLKSetting(double *dCLKSetting);
	void SetCLKSetting(double *dCLKSetting);
	/**
	 * @brief Save the channel number tested fail
	 * @param[in] usChannel The channel number tested fail
	 * @param[in] lpszFilePath The file path of the error report
	*/
	void SaveFailChannel(BYTE bySlotNo, USHORT usChannel);
	BOOL CompareTestItem(const std::string& strTestItem);
	void SaveAdditionMsg(const std::string& strAddition);
	/**
	 * @brief Get the test result of current test item
	 * @return 0:All test is Pass;1:All test is fail;2:Not all test is pass
	*/
	int GetTestResult();
	/**
	*Print the error message
	 * @param[in] testProj The project of test program
	 * @param[in] lpszFilePath The file path of the error report
	*/
	void Print(XTest* testProj, char* lpszFilePath, BYTE bySlotNo);
private:
	double m_dClkSetting[6];
	std::string m_strTestItem;
	std::vector<std::string> m_vecAdditionMsg;
	std::map<BYTE, std::vector<USHORT>> m_mapFailChannel;
	std::vector<CCLKTest*> m_vecCLKTest;
	BOOL m_bAllTestPass;
};

class CFuncReport
{
public:
	/**
	 * @brief Constructor function
	 * @param[in] cFunctionName The function name
	 * @param[in] cTestItem The test item
	*/
	CFuncReport(const char* cFunctionName, const char* cTestItem);
	~CFuncReport();

	void SaveBoardSN(BYTE bySlotNo, char *cSN);
	/**
	 * @brief Add the CLK setting
	 * @param[in] T1R The rising edge time of drive data
	 * @param[in] T1F The falling edge of drive data
	 * @param[in] IOR The rising edge of drive I/O direction
	 * @param[in] IOF The falling edge of drive I/O direction
	 * @param[in] STBR The rising edge of comparison
	 * @param[in] STBF The falling edge of comparison
	*/
	void AddClkSetting(double T1R, double T1F, double IOR, double IOF, double STBR, double STBF);
	void AddTestItem(const char* format, ...);
	/**
	 * @brief Save the channel number tested fail
	 * @param[in] usChannel The channel number tested fail
	 * @param[in] lpszFilePath The file path of the error report
	*/
	void SaveFailChannel(BYTE bySlotNo, USHORT usChannel);
	/**
	 * @brief No board is not valid
	*/
	void SetNoBoardValid();
	void SaveAddtionMsg(const char* lpszFormat, ...);
	/**
	 * @brief Print the error message
	 * @param[in] pTestProj The point of test project
	 * @param[out] lpszFilePath The file path
	*/
	void Print(XTest* pTestProj, char* lpszFilePath);
private:
	std::map<BYTE, std::string> m_mapBoardSN;//The key is slot number, the value is the SN
	double m_dClkSetting[6];
	std::string m_strFunctionName;//The function name be tested
	std::string m_strTestType;//The type of current test
	std::string m_strTestItem;
	std::string m_strAddtionMsg;
	BOOL m_bBoardValid;//Whether Board is valid
	std::vector<CFunctionTestMSG*> m_vecFunctionTest;//The test message of per edge setting
};
class CTimeMSG
{
public:
	CTimeMSG(const char* cMSG);
	~CTimeMSG();
	void SaveTime(double dTime);
	void Print(XTest* testProj, const char* lpszFilePath);
private:
	char m_lpszMSG[1024];
	std::string m_strUnits;
	double m_dTime;
};
#include <vector>
class CTimeReport
{
public:
	/**
	 * @brief Constructor function
	 * @param[in] cFunctionName The function name
	 * @param[in] cTestItem The test item
	*/
	CTimeReport(const char* cFunctionName, const char* cTestItem);
	~CTimeReport();
	void SaveBoardSN(BYTE bySlotNo, char* lpszSN);
	void timeStart();
	double timeStop();
	void SetTimes(int nTimes);
	void addMsg(const char* format, ...);
	void Print(XTest* testProj, const char* lpszFilePath);
	/**
	*No board is not valid
	*/
	void SetNoBoardValid();
	int SetAdditionItemTittle(const char* lpszFormat, ...);
	int AdditionItem(const char* lpszItem, double dData, const char* lpszUnit = "%");
private:
	struct ITEM_DATA 
	{
		std::string m_strName;///<The addition item
		double m_dData;///<The data
		std::string m_strUnit;
		ITEM_DATA(const char* lpszName, double dValue, const char* lpszUnit)
		{
			m_strName = lpszName;
			m_dData = dValue;
			m_strUnit = lpszUnit;
		}
	};
	std::map<BYTE, std::string> m_mapBoardSN;//The key is slot number, the value is the SN
	std::vector<CTimeMSG*> m_vecTimeMsg;
	BOOL m_bNewRecord;
	double m_dTime;
	char m_lpszFunctionName[128];//The function name be tested
	char m_lpszTestItem[128];//The type of current test
	BOOL m_bBoardValid;//Whether Board is valid
	LARGE_INTEGER m_TimeStart;
	LARGE_INTEGER m_TimeStop;
	LARGE_INTEGER m_TimeFreq;
	int m_nTimes;///<The test times
	std::string m_strAdditionTittle;///<The tittle of addition information
	std::vector<ITEM_DATA> m_vecAddition;///<The addition item
	
};
/**
 * @class CMeasurementItem
 * @brief The measurement item
*/
class CMeasurementItem
{
public:
	/**
	 * @brief Constructor
	 * @param[in] lpszItemName The item number
	*/
	CMeasurementItem(const char* lpszItemName);
	/**
	 * @brief Get the item name
	 * @param[out] strItemName The item name
	*/
	void GetName(std::string& strItemName);
	/**
	 * @brief Set the test condition
	 * @param[in] lpszCondition The condition
	 * @param[in] ArgList The input data list will be formated
	*/
	void SetTestCondition(const char* lpszCondition, va_list ArgList);
	/**
	 * @brief Set the fail information
	 * @param[in] lpszFailFormat The information will be formated
	 * @param[in] ArgList The input data list will be formated
	*/
	void SetFailInfo(const char* lpszFailFormat, va_list ArgList);
	/**
	 * @brief[in] Add the fail channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number of board
	 * @param[in] strExpectData Expect measurement data
	 * @param[in] strRealData Real measurement data
	 * @param[in] nDecimalCount The decimal count,-1 means not number
	 * @param[in] lpszAdditionMsg The addition message format
	 * @param[in]  The input message to be formated
	 * @return Execute result
	 * - 0 Add fail channel successfully
	 * - -1 The channel has existed
	*/
	int AddFailChannel(BYTE bySlotNo, USHORT usChannel, const std::string& strExpectData, const std::string& strRealData, std::string& strUnit, const char* lpszAdditionMsg = nullptr, va_list ArgList = nullptr);
	/**
	 * @brief Print test message
	 * @param[in] pTestProj The point of test project
	 * @param[in] lpszFilePath The file path of log
	*/
	void Print(XTest* pTestProj, char* lpszFilePath);
	/**
	 * @brief Check whether the item is pass
	 * @return The test result
	 * - TRUE Test pass
	 * - FALSE Test fail
	*/
	BOOL IsTestPass();
private:
	/**
	 * @brief[in] Get channel ID of the channel which will be used sign channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number of the board
	 * @return The channel ID
	*/
	inline int GetChannelID(BYTE bySlotNo, USHORT usChannel);
	/**
	 * @brief Get the channel from channel ID
	 * @param[in] uChannelID The channel ID
	 * @param[out] usChannel The channel number of board
	 * @return The slot number
	*/
	inline BYTE GetChannel(UINT uChannelID, USHORT& usChannel);
private:
	/**
	 * @struct FAIL_MSG
	 * @brief The fail message of each channel
	*/
	struct FAIL_MSG
	{
		std::string m_strExpectData;///<The expect value
		std::string m_strRealData;///<The real value measurement
		std::string m_strUnit;///<The unit of the data
		std::string m_strAddition;///<The addition message of current channel
	};
	BOOL m_bHasAdditionMsg;///<Whether has addition message
	BOOL m_bHasUnit;///<Whether the data has unit
	std::string m_strItemName;///<The item name
	std::string m_strTestCondition;///<The condition of this item
	std::string m_strFailInfo;///<The fail information
	int m_nMaxDataSize;///<The maximum data size
	std::map<UINT, FAIL_MSG> m_mapFailChannel;
};

/**
 * @class CMeasurementFuncReport
 * @brief The function report of measurement
*/
class CMeasurementFuncReport
{
public:
	/**
	 * @brief Constructor
	 * @param[in] lpszFunctionName The function name
	 * @param[in] lpszTestType The test type
	*/
	CMeasurementFuncReport(const char* lpszFunctionName, const char* lpszTestType);
	/**
	 * @brief Destructor
	*/
	~CMeasurementFuncReport();
	/**
	 * @brief Save the board SN
	 * @param[in] bySlotNo The slot number
	 * @param[in] lpszSN The serial number
	*/
	void SaveBoardSN(BYTE bySlotNo, char* lpszSN);
	/**
	 * @brief Add test item
	 * @param[in] lpszItemName 
	*/
	void AddTestItem(const char* lpszItemName);
	/**
	 * @brief Set the fail information
	 * @param[in] lpszFailFormat The fail format
	 * @param[in]  The information will be formated
	*/
	void SetFailInfo(const char* lpszFailFormat, ...);
	/**
	 * @brief Set the test condition
	 * @param[in] lpszCondition The condition
	 * @param[in]  The input data will be formated
	*/
	void SetTestCondition(const char* lpszCondition, ...);
	/**
	 * @brief[in] Add the fail channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number of board
	 * @param[in] ExpectData Expect measurement data
	 * @param[in] RealData Real measurement data
	 * @param[in] nDecimalCount The decimal count,-1 means not number
	 * @param[in] lpszAdditionMsg The addition message format
	 * @param[in]  The input message to be formated
	 * @return Execute result
	 * - 0 Add fail channel successfully
	 * - -1 The channel has existed
	*/
	template <typename DataType>
	int AddFailChannel(BYTE bySlotNo, USHORT usChannel, DataType ExpectData, DataType RealData, int nDecimalCount, const char* lpszUnit = nullptr, const char* lpszAdditionMsg = nullptr, ...);
	/**
	 * @brief Print test message
	 * @param[in] pTestProj The point of test project
	 * @param[in] lpszFilePath The file path of log
	*/
	void Print(XTest* pTestProj, char* lpszFilePath);
private:
	std::string m_strFunctionName;///<The function name
	std::string m_strTestType;///<The test type name
	BOOL m_bAllTestPass;///<Whether all items are pass
	std::map<BYTE, std::string> m_mapBoardSN;///<The board serial number
	std::vector< CMeasurementItem*> m_vecTestItem;///<The test item
};

template<typename DataType>
int CMeasurementFuncReport::AddFailChannel(BYTE bySlotNo, USHORT usChannel, DataType ExpectData, DataType RealData, int nDecimalCount, const char* lpszUnit, const char* lpszAdditionMsg, ...)
{
	CMeasurementItem* pTestItem = nullptr;
	if (0 == m_vecTestItem.size())
	{
		pTestItem = new CMeasurementItem(m_strTestType.c_str());
		m_vecTestItem.push_back(pTestItem);
	}
	pTestItem = m_vecTestItem[m_vecTestItem.size() - 1];

	std::string strExpectData;
	std::string strRealData;
	std::string strUnit;
	if (nullptr != lpszUnit)
	{
		strUnit = lpszUnit;
	}
	if (-1 == nDecimalCount)
	{
		strExpectData = ExpectData;
		strRealData = RealData;

		if (10 <=strExpectData.size() || 10 <= strRealData.size())
		{
			int i = 0;
		}
	}
	else
	{
		char lpszMsg[256] = { 0 };
		char lpszFormat[32] = { 0 };
		sprintf_s(lpszFormat, sizeof(lpszFormat), "%%.%df", nDecimalCount);
		sprintf_s(lpszMsg, sizeof(lpszMsg), lpszFormat, ExpectData);
		strExpectData = lpszMsg;

		sprintf_s(lpszMsg, sizeof(lpszMsg), lpszFormat, RealData);
		strRealData = lpszMsg;

		if (10 <= strExpectData.size() || 10 <= strRealData.size())
		{
			int i = 0;
		}
	}
	m_bAllTestPass = FALSE;
	va_list ap;
	va_start(ap, lpszAdditionMsg);

	int nRetVal = 0;

	nRetVal = pTestItem->AddFailChannel(bySlotNo, usChannel, strExpectData, strRealData, strUnit, lpszAdditionMsg, ap);
	va_end(ap);

	return nRetVal;
}
