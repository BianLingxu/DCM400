#pragma once
/**
 * @file Pin.h
 * @brief The head of class CPin
 * @author Guangyun Wang
 * @date 2022/02/21
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control technology Co., Ltd.
*/
#include "DCM400HardwareInfo.h"
#include <vector>
#include <string>
/**
 * @class CPin
 * @brief The pin in vector
*/
class CPin
{
public:
	/**
	 * @brief Constructor
	 * @param[in] lpszName The pin name
	 * @param[in] uID The ID of pin
	 */
	CPin(const char* lpszName, UINT uID);
	/**
	 * @brief Copy constructor
	 * @param[in] Pin The pin 
	*/
	CPin(const CPin& Pin);
	/**
	 * @brief Add channel to the pin
	 * @param[in] ChannelInfo The channel information
	 */
	void AddChannel(CHANNEL_INFO ChannelInfo);
	/**
	 * @brief Get the channel information of specific site
	 * @param[in] usSiteNo The site number
	 * @param[out] Channel The channel information
	 * @return Execute result
	 * - 0 Get the channel successfully
	 * - -1 The site number is not existed
	 */
	int GetChannel(USHORT usSiteNo, CHANNEL_INFO& Channel);
	/**
	 * @brief Get the site count
	 * @return The site count
	 */
	int GetSiteCount();
	/**
	 * @brief Get the pin ID
	 * @return The pin ID
	 */
	UINT GetID();
	/**
	 * @brief Get the pin name
	 * @return The pin name
	 */
	const char* GetName();
	/**
	 * @brief Get the site number of the channel
	 * @param[in] Channel The channel info
	 * @return Site number
	 * - >=0 The site number
	 * - -1 Can't find the channel
	*/
	int GetSiteNo(const CHANNEL_INFO& Channel);
	/**
	 * @brief Get all channels of the pin
	 * @param[out] vecChannel The channel of the pin
	 * @param[in] bAppend Whether append the channels to the parameter vecChannel
	*/
	void GetAllChannel(std::vector<CHANNEL_INFO>& vecChannel, BOOL bAppend = FALSE);
private:
	UINT m_uID;///<The pin ID
	std::string m_strName;///<The pin name
	std::vector<CHANNEL_INFO> m_vecChannel;///<The channel of each site
};
