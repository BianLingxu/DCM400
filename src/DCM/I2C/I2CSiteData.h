#pragma once
/**
 * @file I2CSiteData.h
 * @brief Include the class of I2C site data
 * @author Guangyun Wang
 * @date 2020/07/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd. Ltd.
*/
#include "..\StdAfx.h"
#include <vector>
#include <map>
#include <string>
#include "..\DriverAlarm.h"
#include "..\MD5.h"
/**
 * @class CI2CSiteData
 * @brief The class save the data of each site in I2C
*/
class CI2CSiteData
{
public:
	/**
	 * @brief Constructor
	 * @param[in] The class of driver alarm
	*/
	CI2CSiteData(CDriverAlarm* pAlarm);
	/**
	 * @brief The override operation of '='
	 * @param[in] SiteData The data need to be copied
	 * @return The copied site data
	*/
	CI2CSiteData& operator=(const CI2CSiteData& SiteData);
	/**
	 * @brief Destructor
	*/
	~CI2CSiteData();
	/**
	 * @brief Set the data byte of each site
	 * @param[in] uDataByteCount The data byte count
	 * @return Execute result
	 * - 0 Set the data byte count successfully
	 * - -1 The data byte have been set before
	*/
	int SetDataByteCount(UINT uDataByteCount);
	/**
	 * @brief Ge the data byte count of each site
	 * @return The data byte count
	*/
	UINT GetDataByteCount();
	/**
	 * @brief Set the data of specific site
	 * @param[in] usSiteNo The site number
	 * @param[in] pbyData The data of the site
	 * @param[in] uDataByteCount The data byte count
	 * @return Execute result
	 * - 0 Set the site data successfully
	 * - -1 The data count is 0 or the point of data is nullptr
	 * - -2 The data byte count is not equal to the data count set before
	 * - -3 The site is not existed
	 * - -4 Allocate memory fail
	*/
	int SetSiteData(USHORT usSiteNo, const BYTE* pbyData, UINT uDataByteCount);
	/**
	 * @brief Get the data of the site
	 * @param[in] usSiteNo The site number
	 * @param[in] uDataByteIndex The data byte index
	 * @return The data of the site
	 * - >=0 The data of the site
	 * - -1 The site is not existed
	 * - -2 The data byte index is over range
	*/
	int GetSiteData(USHORT usSiteNo, UINT uDataByteIndex) const;
	/**
	 * @brief Set the NACK of the site
	 * @param[in] usSiteNo The site number
	 * @param[in] nNACKIndex The NACK index, -1 is no NACK
	*/
	void SetNACK(USHORT usSiteNo, int nNACKIndex);
	/**
	 * @brief Get the NACK index
	 * @param[in] usSiteNo The site number
	 * @return The NACK index
	 * - >0 The NACK index
	 * - 0 No NACK
	 * - -1 The NACK of the site is not existed
	*/
	int GetNACK(USHORT usSiteNo);
	/**
	 * @brief Check whether data of each site is same
	 * @return Whether the data is same
	 * - TRUE The data of all site is same
	 * - FALSE The data of each site is not all same
	*/
	BOOL IsDataAllSame();
	/**
	 * @brief Get the site data
	 * @param[in] usSiteNo The site number
	 * @param[out] vecData The data of the site
	 * @return Execute result
	 * - 0 Get the site data successfully
	 * - -1 The data of current site is not existed
	*/
	int GetSiteData(USHORT usSiteNo, std::vector<BYTE>& vecData);
	/**
	 * @brief Get the key of the data
	 * @param[in] strKey The key of data
	 * @return Execute result
	 * - 0 Get the key successfully
	 * - -1 No valid data
	*/
	int GetDataKey(std::string& strKey);
	/**
	 * @brief Reset site data and clear memory
	*/
	void Reset();
	/**
	 * @brief Set the MD5 data for generating MD5
	 * @param[in] pucData The the data to generate MD5
	 * @param[in] nDataSize The data size
	 * @return Execute result
	 * - 0 Set the MD5 data successfully
	 * - -1 The point of data is nullptr or the data size is less than 1
	*/
	int SetMD5Data(const unsigned char* pucData, int nDataSize);
	/**
	 * @brief Get the MD5 value of data
	 * @param[out] strData The MD5 value
	*/
	void GetMD5(std::string& strData);
private:
	/**
	 * @brief Get the MD5 of the data
	*/
	void GetSiteDataMD5();
private:
	UINT m_uDataByteCount;///<The data byte count
	BOOL m_bSiteMD5;///<Whether get MD5 of all site
	CDriverAlarm* m_pAlarm;///<The driver alarm
	std::map<USHORT, BYTE*> m_mapSiteData;///<The data of site which is not equal to another, the key is site number and value is data
	std::map<USHORT, USHORT> m_mapSameData;///<The site whose data is equal to some site, the key is the site number and value is the site who has same data and saved in m_mapSiteData
	std::map<USHORT, int> m_mapNACK;///<The NACK of each site
	MD5Context m_MD5Context;///<The MD5 context
};