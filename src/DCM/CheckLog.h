#pragma once
/**
 * @file CheckLog.h
 * @brief The log for saving the information to selfcheck log file.
 * @author Guangyun Wang
 * @date 2021/04/28
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Co., Ltd.
*/
#include <windows.h>
#include <string>
#include <vector>
#include <set>
#include "HardwareInfo.h"
/**
 * @class CCheckLog
 * @brief The class for saving the information to selfcheck log file
*/
class CCheckLog
{
public:
	/**
	 * @brief Constructor
	*/
	CCheckLog();
	/**
	 * @brief Destructor
	*/
	~CCheckLog();
	/**
	 * @brief Set the check log file
	 * @param[in] strLogFile The full path of check log file
	*/
	void SetLogFile(const std::string& strLogFile);
	/**
	 * @brief Set the test item name
	 * @param[in] lpszItemName The item name
	 * @return Execute result
	 * - 0 Set test item successfully
	 * - -1 The item name is nullptr
	*/
	int SetTestItem(const char* lpszItemName);
	/**
	 * @brief Set the controller checked
	 * @param[in] setController The controller checked
	*/
	void SetCheckController(const std::set<BYTE>& setController);
	/**
	 * @brief Get the fail controller
	 * @param[out] setFailController The fail controller
	*/
	void GetFailController(std::set<BYTE>& setFailController);
	/**
	 * @brief Set the sub item name
	 * @param[in] lpszSubItem The sub item name
	 * @return Execute result
	 * - 0 Set sub item name successfully
	 * - -1 The sub item name is nullptr
	*/
	int SetSubItem(const char* lpszSubItem);
	/**
	 * @brief Set the check data name
	 * @param[in] lpszCheckDataName The data name checked
	*/
	int SetCheckDataName(const char* lpszCheckDataName);
	/**
	 * @brief Add fail data of controller
	 * @param[in] byControllerIndex The controller index
	 * @param[in] lpszFormat The format of the message
	 * @param[in] The fail data will be formated
	 * @retrurn Execute result
	 * - 0 Add fail data successfully
	 * - -1 The controller index is over range
	 * - -2 The message is nullptr
	*/
	int AddFailData(BYTE byControllerIndex, const char* lpszFormat, ...);
	/**
	 * @brief Save log to file
	*/
	void SaveLog();
private:
	/**
	 * @brief Add data name to log
	*/
	void AddDataName();
private:
	std::string m_strFileName;///<The log file name
	std::string m_strCheckItemName;///<The item checked
	std::string m_strSubItem;///<The sub item name
	std::string m_strDataName;///<The data name checked
	std::set<BYTE> m_setController;///<The controller be checked
	std::set<BYTE> m_setTotalFailController;///<The fail controller
	std::set<BYTE> m_setCurFailController;///<The fail controller in current sub item
	std::vector<std::string> m_avecControllerLog[DCM_MAX_CONTROLLERS_PRE_BOARD + 1];///<The log of each controller, first element is data name
};

