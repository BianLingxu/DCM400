/**
 * @file ShieldAlaram.h
 * @brief Include the class for saving the shielded alarm information
 * @author Guangyun Wang
 * @date 2020/09/09
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#pragma once

#include "HardwareInfo.h"
#include <vector>
#include <map>
#include <set>
#include <string>
/**
 * @clase CShieldPin
 * @brief The pin shielded
*/
class CShieldPin
{
public:
	/**
	 * @brief Constructor
	 * @param[in] lpszPinName The pin name
	*/
	CShieldPin(const char* lpszPinName);
	/**
	 * @brief Shield the pin in the site
	 * @param[in] usSiteNo The site number
	 * @param[in] Channel The channel information
	 * @param[in] bShield Whether shield the site
	*/
	void ShieldSite(USHORT usSiteNo, const CHANNEL_INFO& Channel, BOOL bShield);
	/**
	 * @brief Get the channel shielded
	 * @param[out] vecChannel The channel number
	 * @param[in] bAppend Whether append the channel
	*/
	void GetShieldChannel(std::vector<CHANNEL_INFO>& vecChannel, BOOL bAppend = FALSE);
	/**
	 * @brief Check whether the channel in the site is shielded
	 * @param[in] usSiteNo The site number
	 * @return Whether the channel is shielded
	 * - TRUE The channel is shielded
	 * - FALSE The channel is not shielded
	*/
	BOOL IsShield(USHORT usSiteNo);
private:
	std::string m_strPinName;///<The pin name
	std::map<USHORT, CHANNEL_INFO> m_mapShieldSite;///<The shield site
};
/**
 * @class CShieldFunction
 * @brief The shield function
*/
class CShieldFunction
{
public:
	/**
	 * @brief Constructor
	 * @param[in] lpszShieldFunction The function name shielded
	*/
	CShieldFunction(const char* lpszShieldFunction);
	/**
	 * @brief Add shield pin in site
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] Channel The channel information
	 * @param[in] bShielded Whether shield the site
	 * @return Execute result
	 * - 0 Shield pin successfully
	 * - -1 The pin name is nullptr
	*/
	int ShieldPin(const char* lpszPinName, USHORT usSiteNo, const CHANNEL_INFO& Channel, BOOL bShielded);
	/**
	 * @brief Shield alarm ID
	 * @param[in] uAlarmID The alarm ID shielded
	 * @param[in] bShield Whether shield
	*/
	void ShieldID(UINT uAlarmID, BOOL bShield);
	/**
	 * @brief Get the channel shielded
	 * @param[in] vecChannel The channel
	 * @param[in] uAlarmID The alarm ID shielded
	*/
	void GetShieldChannel(std::vector<CHANNEL_INFO>& vecChannel, UINT uAlaramID);
	/**
	 * @brief Check whether the pin is shield
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] uAlarmID The alarm ID shielded
	 * @return Whether the pin is shield
	 * - TRUE The pin is shielded
	 * - FALSE The pin is not shielded
	*/
	BOOL IsShield(const char* lpszPinName, USHORT usSiteNo, UINT uAlarmID);
private:
	std::string m_strFunciton;///<The function name shielded
	std::map<std::string, CShieldPin*> m_mapShieldPin;///<The shielded pin
	std::set<UINT> m_setID;///<The shielded ID;
};
