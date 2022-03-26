#pragma once
/**
 * @file Site.h
 * @brief Include the site inforamtion class
 * @detail The class in this file can used for get channel information of each site
 * @author Guangyun Wang
 * @date 2020/05/15
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "StdAfx.h"
#include <map>
#include <vector>
#include <string>
/**
 * @class CSiteChannel
 * @brief The channel information of site
*/
class CSiteChannel
{
public:
	/**
	 * @brief Constructor
	 * @param[in] usSiteNo The site index of current site
	*/
	CSiteChannel(USHORT usSiteNo);
	/**
	 * @brief Get the channel information of current site
	 * @param[in] strPinName The pin name
	 * @param[in] Channel The channel information of current pin
	 * @return Execute result
	 * - 0 Add channel successfully
	 * - -1 The pin has added before
	*/
	int AddChannel(const std::string& strPinName, CHANNEL_INFO& Channel);
	/**
	 * @brief Get the channel information of pin in specific site
	 * @param[in] strPinName The pin name
	 * @param[out] Channel The channel information
	 * @return Execute result
	 * - 0 Get channel information successfully
	 * - -1 The pin is not existed
	*/
	int GetChannel(const std::string& strPinName, CHANNEL_INFO& Channel) const;
	/**
	 * @brief Get the channel information of each pin
	 * @param[out] vecChannel The channel information of each channel
	 * @param[in] bAppend Whether append the channel information
	*/
	void GetChannel(std::vector<CHANNEL_INFO>& vecChannel, BOOL bAppend = FALSE);
	/**
	 * @brief Get the slot of the board used in current site
	 * @param[out] vecSlot The slot number of boars used
	*/
	void GetSlot(std::vector<BYTE>& vecSlot) const;
	/**
	 * @brief Get the pin name of the channel
	 * @param[in] Channel The channel information
	 * @param[out] strPinName The pin name of the channel
	 * @return Execute result
	 * - 0 Get the pin name successfully
	 * - -1 Can't find the pin name
	*/
	int GetPinName(const CHANNEL_INFO& Channel, std::string& strPinName) const;
	/**
	 * @brief Get the board and its minimum channel count needed
	 * @param[in] mapBoard The board and its minimum channel count needed
	*/
	void GetSiteBoard(std::map<BYTE, USHORT>& mapBoard) const;
	/**
	 * @brief Check whether use the board
	 * @param[in] bySlotNo The board number
	 * @return Check result
	 * - TRUE Use board
	 * - FALSE Not use board
	*/
	BOOL UseBoard(BYTE bySlotNo) const;
	/**
	 * @brief Delete pin name
	 * @param[in] strPinName The pin name
	*/
	void DeletePin(const std::string& strPinName);
private:
	USHORT m_usSiteNo;///<The site index of current site
	std::map<std::string, CHANNEL_INFO> m_mapChannel;//The channel information of each pin, the key is pin name and the value is its channel information
};
/**
 * @class CSite
 * @brief The site information
*/
class CSite
{
public:
	/**
	 * @brief Constructor
	*/
	CSite();
	/**
	 * @brief Copy class
	 * @param[in] Site The source class
	 * @return 
	*/
	CSite& operator=(const CSite& Site);
	/**
	 * @brief Destructor
	*/
	~CSite();
	/**
	 * @brief Add the channel of the pin in specific site
	 * @param[in] strPinName The pin name
	 * @param[in] usSiteNo The site index
	 * @param[in] Channel The channel of the pin name
	 * @return Execute result
	 * - 0 Get channel successfully
	 * - -1 The pin name is blank
	 * - -2 The channel of current pin has existed
	*/
	int AddChannel(const std::string& strPinName, USHORT usSiteNo, CHANNEL_INFO& Channel);
	/**
	 * @brief Get the channel of the pin in specific site
	 * @param[in] strPinName The pin name
	 * @param[in] usSiteNo The site index
	 * @param[out] Channel The channel of the pin name
	 * @return Execute result
	 * - 0 Get channel successfully
	 * - -1 The pin name is not existed
	 * - -2 The site is not existed
	*/
	int GetChannel( const std::string& strPinName, USHORT usSiteNo, CHANNEL_INFO& Channel) const;
	/**
	 * @brief Get the channel of specific site
	 * @param[in] usSiteNo The site index
	 * @param[out] vecChannel The channel information of specific site
	 * @return Execute result
	 * - 0 Get channel information successfully
	 * - -1 The site is over range
	*/
	int GetChannel(USHORT usSiteNo, std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Get the site count
	 * @return The site count
	*/
	USHORT GetSiteCount();
	/**
	 * @brief Get the board used
	 * @param[in] The slot number of the board used
	*/
	void GetUseBoard(std::vector<BYTE>& vecBoard) const;
	/**
	 * @brief Is site valid
	 * @param[in] usSiteNo The site index
	 * @return
	 * - TRUE Site is valid
	 * - FALSE Site is not valid
	*/
	BOOL IsSiteValid(USHORT usSiteNo);
	/**
	 * @brief Clear all site information
	*/
	void Reset();
	/**
	 * @brief Get the pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] Channel The channel information of the pin
	 * @param[out] strPinName The pin name of the channel
	 * @return Execute result
	 * - 0 Get pin name successfully
	 * - -1 The site number is over range
	 * - -2 Can't find the pin name
	*/
	int GetPinName(USHORT usSiteNo, const CHANNEL_INFO& Channel, std::string& strPinName);
	/**
	 * @brief Delete pin
	 * @param[in] strPin The pin name
	*/
	void DeletePin(const std::string& strPin);
	/**
	 * @brief Get the site board
	 * @param[in] usSiteNo The site number
	 * @param[in] mapBoard The board and its minimum channel count needed
	 * @return Execute result
	 * - 0 Get the channel count successfully
	 * - -1 The site is over range
	*/
	int GetSiteBoard(USHORT usSiteNo, std::map<BYTE, USHORT>& mapBoard);
	/**
	 * @brief Get the site number which use the board
	 * @param[in] bySlotNo The slot number
	*/
	void GetBoardSite(BYTE bySlotNo, std::vector<USHORT>& vecSite) const;
	/**
	 * @brief Get the channel information of invalid site
	 * @param[out] vecChannel The channel of invalid site
	*/
	void GetInvalidSiteChannel(std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Get all channel in valid site
	 * @param[out] vecChannel The channel
	*/
	void GetValidSiteChannel(std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Get site status
	 * @param[out] pbySiteStatus The status of each site, 0 means site invalid and 1 is valid
	 * @param[in] usSiteCount The site count of the array
	*/
	inline void GetSiteStatus(BYTE* pbySiteStatus, USHORT usSiteCount) const;

private:
	std::map<USHORT, CSiteChannel*> m_mapSite;///<The site channel, key is site index, and value is the channel information
	BYTE* m_pbySiteStatus;///<The site status
};