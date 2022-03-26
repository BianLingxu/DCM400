#pragma once
/**
 * @file I2CSite.h
 * @brief Include the class of I2C site channel information
 * @author Guangyun Wang
 * @date 2020/07/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "..\StdAfx.h"
#include <vector>
#include <map>
/**
 * @class CBoardSite
 * @brief The site used in board
*/
class CBoardSite
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CBoardSite(BYTE bySlotNo);
	/**
	 * @brief Get the site in the board
	 * @param[out] mapOnBoard The site used in the board
	*/
	void GetOnBoardSite(std::map<USHORT, BYTE>& mapOnBoard);
	/**
	 * @brief Add channel
	 * @param[in] usSiteNo The site number
	 * @param[in] bSCL Whether the SCL channel in the board
	*/
	void AddChannel(USHORT usSiteNo, BOOL bSCL);
	/**
	 * @brief Get the site count excluding the site inputed
	 * @param[in] vecExcludeSite The site excluding
	 * @return Site count
	*/
	USHORT GetExcludeSiteCount(const std::vector<USHORT>& vecExcludeSite);
private:
	BYTE m_bySlotNo;///<The slot number
	std::map<USHORT, BYTE> m_mapOnBoardSite;///<The site in the board, key is site number and value is channel in board, bit0 is SCL, bit1 is SDA
};

/**
 * @class CI2CSite
 * @brief The site information of all site
*/
class CI2CSite
{
public:
	/**
	 * @brief Constructor
	*/
	CI2CSite();
	/*
	 * @brief Destructor
	*/
	~CI2CSite();
	/**
	 * @brief Set the site count in I2C
	 * @param[in] usSiteCount The site count
	*/
	void SetSiteCount(USHORT usSiteCount);
	/**
	 * @brief Set the channel information of specific site
	 * @param[in] usSiteNo The site number
	 * @param[in] SCL The channel information of SCL
	 * @param[in] SDA The channel information of SDA
	 * @return Execute result
	 * - 0 Set channel information successfully
	 * - -1 The site number is over range
	 * - -2 The SCL channel information of current site set before is different from current
	 * - -2 The SDA channel information of current site set before is different from current
	*/
	int SetSiteChannel(USHORT usSiteNo, const CHANNEL_INFO& SCL, const CHANNEL_INFO& SDA);
	/**
	 * @brief Get the channel information of site
	 * @param[in] usSiteNo The site number
	 * @param[out] SCL The SCL channel information
	 * @param[out] SDA The SDA channel information
	 * @return Execute result
	 * - -1 The site number is over range
	 * - -2 The site number is not existed
	*/
	int GetSiteChannel(USHORT usSiteNo, CHANNEL_INFO& SCL, CHANNEL_INFO& SDA)  const;
	/**
	 * @brief Get the data channel
	 * @param[out] mapChannel The channel of SDA
	 * @param[in] bOnlyValidSite Whether only get the valid site
	*/
	void GetDataChannel(std::map<USHORT, CHANNEL_INFO>& mapChannel, BOOL bOnlyValidSite) const;
	/**
	 * @brief Get site data channel
	 * @para[in]m usSiteNo The site number
	 * @param[out] Channel The channel information
	 * @param[in] bOnlyValidSite Whether only get valid site
	 * @return Execute result
	 * - 0 Get the data channel successfully
	 * - -1 The site is over range
	 * - -2 The site is invalid
	*/
	int GetDataChannel(USHORT usSiteNo, CHANNEL_INFO& Channel, BOOL bOnlyValidSite) const;
	/**
	 * @brief Get the board used
	 * @param[out] vecUseBoard The board used in I2C
	 * @param[in] bOnlyValidSite Whether only getting the valid site
	*/
	void GetUseBoard(std::vector<BYTE>& vecUseBoard, BOOL bOnlyValidSite = TRUE) const;
	/**
	 * @brief Get the site information which use the board
	 * @param[in] bySlotNo The slot number
	 * @param[out] mapSiteInfo The site information, key is site number and value is channel, bit0 is SCL, bit1 is SDA
	 * @return Execute result
	 * - 0 Get board site successfully
	 * - -1 The board is not used
	*/
	int GetBoardSite(BYTE bySlotNo, std::map<USHORT, BYTE>& mapSiteInfo);
	/**
	 * @brief Get the channel used in the board
	 * @param[in] bySlotNo The slot number
	 * @param[out] vecChannel The channel used in board
	 * @param[in] bOnlySite Whether only get valid site
	 * @param[in] byChanenlType The channel type, 1 is SCL , 2 is SDA, 3 is all
	 * @return Execute result
	 * - 0 Get board channel successfully
	 * - -1 The board is not used
	*/
	int GetBoardChannel(BYTE bySlotNo, std::vector<USHORT>& vecChannel, BOOL bOnlyValidSite = TRUE, BYTE byChanenlType = 3) const;
	/**
	 * @brief Get the valid site
	 * @param[out] vecSite The site number
	*/
	void GetValidSite(std::vector<USHORT>& vecSite) const;
	/**
	 * @brief Reset channel information
	*/
	void Reset();
	/**
	 * @brief Get the site count
	 * @return The site count
	*/
	int GetSiteCount() const;
	/**
	 * @brief Check whether the site is valid
	 * @param[in] usSiteNo The site number
	 * @return Whether the site is valid
	 * - TRUE The site is valid
	 * - FALSE The site is invalid
	*/
	BOOL IsSiteValid(USHORT usSiteNo) const;
	/**
	 * @brief Get the channel information of specific pin
	 * @param[in] bSCL Whether get the SCL
	 * @param[out] mapChannel The channel information, the key is slot number and value is the channel in the board
	 * @param[in] bOnlyValidSite Whether only getting the channels in valid site
	*/
	void GetChannel(BOOL bSCL, std::map<BYTE, std::vector<USHORT>>& mapChannel, BOOL bOnlyValidSite = TRUE);
	/**
	 * @brief Get the channel in invalid site of the board
	 * @param[in] bySlotNo The slot number
	 * @param[out] vecChannel The channel in invalid site
	*/
	void GetInvalidSiteChannel(BYTE bySlotNo, std::vector<USHORT>& vecChannel) const;
	/**
	 * @brief Set the stop status of valid site
	 * @param[in] bHighImpedance Whether the stop status is high impedance
	*/
	void SetSiteStopStatus(BOOL bHighImpedance);
	/**
	 * @brief Get the stop status of the site
	 * @param[in] usSiteNo The site number
	 * @return The stop status
	 * - TRUE Stop with high impedance
	 * - FALSE Stop with high driver
	*/
	BOOL GetSiteStopStatus(USHORT usSiteNo) const;
	/**
	 * @brief Get all channel used
	 * @param[out] vecChannel The channel used
	*/
	void GetAllChannel(std::vector<CHANNEL_INFO>& vecChannel) const;
private:
	/**
	 * @brief Get the site board relation
	*/
	void SiteBoardRelation();
	/**
	 * @brief Get site status
	 * @param[out] pbySiteStatus The status of each site, 0 means site invalid and 1 is valid
	 * @param[in] usSiteCount The site count of the array
	*/
	inline void GetSiteStatus(BYTE* pbySiteStatus, USHORT usSiteCount) const;
private:
	/**
	 * @struct I2CSITE
	 * @brief I2C site pin
	*/
	struct I2CSITE
	{
		CHANNEL_INFO m_SDA;///<The SDA channel
		CHANNEL_INFO m_SCL;///<The SCL channel
	};
	USHORT m_usSiteCount;///<The site count
	std::map<USHORT, I2CSITE> m_mapSite;///<The site channel information, key is site number and value is channel information
	std::map<BYTE, CBoardSite*> m_mapSiteBoardRelation;///<The site board relation, key is slot number and value is point of board site
	BYTE* m_pbySiteStatus;///<Site status
	std::map<USHORT, bool> m_mapStopStatus;///<The stop status of each site, key is site number and value is whether stop with high impedance
};

