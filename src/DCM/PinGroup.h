#pragma once
/**
 * @file PinGroup.h
 * @brief Include the pin group information
 * @author Guangyun Wang
 * @date 2020/07/10
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Co., Ltd.
*/
#include <windows.h>
#include <set>
#include <string>
/**
 * @class CPinGroup
 * @brief The pin group information
*/
class CPinGroup
{
public:
	/**
	 * @brief Constructor
	 * @param[in] lpszPinGroupName The name of pin group
	*/
	CPinGroup(const char* lpszPinGroupName);
	/**
	 * @brief Constructor
	 * @param[in] PinGroup The source pin group
	*/
	CPinGroup(const CPinGroup& PinGroup);
	/**
	 * @brief Add pin to pin group
	 * @param[in] strPinName The pin name
	 * @return Execute result
	 * - 0 Add pin successfully
	 * - -1 The pin is existed
	*/
	int AddPinName(std::string& strPinName);
	/**
	 * @brief Set the pin in pin group
	 * @param[in] setPin The pin in the pin group
	 * @return Execute result
	 * - 0 Set pin name successfully
	 * - -1 The pin group has set pin group before
	*/
	int SetPinName(std::set<std::string> setPin);
	/**
	 * @brief Get the pin name in pin group
	 * @param[out] setPinName The pin name
	*/
	void GetPinName(std::set<std::string>& setPinName);
	/**
	 * @brief Check whether the pin name are same as current
	 * @param[in] vecPinName The pin name
	 * @return Whether the pin name are same
	 * - TRUE Same pin name
	 * - FALSE Different
	*/
	BOOL IsSamePinGroup(const std::set<std::string>& setPinName);
private:
	std::string m_strName;///<The pin group name
	std::set<std::string> m_setPin;///<The pin name in the pin group
};

