#pragma once
/**
 * @file I2CBase.h
 * @brief Include the class of base operation of I2C
 * @author Guangyun Wang
 * @date 2020/07/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <string>
#include "..\StdAfx.h"
#include "I2CLine.h"
#include "..\DriverAlarm.h"
#include "I2CRAM.h"
#include "I2CBoardManage.h"
/**
 * @class CI2CBase
 * @brief The base operation of I2C
*/
class CI2CBase
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bRead Whether is the read operation
	 * @param[in] I2CSite The I2C site channel information
	*/
	CI2CBase(BOOL bRead, const CI2CSite& I2CSite, CDriverAlarm* pAlarm, CI2CBoardManage* pBoardMange, CI2CRAM* pRAMMange);
	/**
	* @bried Destructor
	*/
	~CI2CBase();
	/**
	 * @brief Set the channel status after stop
	 * @param[in] bHighImpedance Whether the channel stop with high impedance
	*/
	void SetStopStatus(BOOL bHighImpedance);
	/**
	 * @brief Get the NACK index of the site
	 * @param[in] usSiteNo The site number
	 * @return NACK index
	 * - >0 The NACK index
	 * - 0 Not NACK in latest ran
	 * - -1 The site number is over range
	 * - -2 Not ran before
	 * - -3 The site is invalid
	 * - -4 The board of the site is not existed
	*/
	int GetNACKIndex(USHORT usSiteNo);
	/**
	 * @brief Set the byte count of register address
	 * @param[in] byByteCount The byte count
	 * @return Execute result
	 * - 0 Set byte count successfully
	 * - -1 The byte count is over range
	*/
	int SetREGByte(BYTE byByteCount);
	/**
	 * @brief Set the address and data count
	 * @param[in] bySlaveAddress The slave address
	 * @param[in] uRegisterAddress The register address
	 * @param[in] uDataCount The data byte count
	*/
	void SetAddress(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataCount);
	/**
	 * @brief Set data of site
	 * @param[in] usSiteNo The site number
	 * @param[in] pbyData The point of the data
	 * @return Execute result
	 * - 0 Set site data successfully
	 * - -1 The site number is over range
	 * - -2 Not set data byte count before
	 * - -3 The point of the data is nullptr
	 * - -4 The site number is not existed
	 * - -5 Allocate memory fail
	*/
	int SetSiteData(USHORT usSiteNo, const BYTE* pbyData);
	/**
	 * @brief Get the data of site
	 * @param[in] usSiteNo The site number
	 * @param[in] uDataByteIndex The data byte index
	 * @return Execute result
	 * - 0 Get the site data successfully
	 * - -1 The site number is over range
	 * - -2 The site is invalid
	 * - -3 The data index is over range
	 * - -4 The data of the site is not existed
	*/
	int GetSiteData(USHORT usSiteNo, UINT uDataByteIndex);
	/**
	 * @brief Get the data byte count
	 * @return The data byte count
	*/
	UINT GetDataByteCount();
	/**
	 * @brief Run vector
	 * @return Execute result
	 * - 0 Run successfully
	 * - >0 No enough line, the pattern line count of new opeation
	*/
	int Run();
	/**
	 * @brief Clear all I2C operation
	*/
	void Reset();
	/**
	 * @brief Get the latest I2C memory information
	 * @param[out] uStartLine The start line in BRAM
	 * @param[out] uLineCount The line count in DRAM
	 * @param[out] bWithDRAM Whether use BRAM
	 * @param[out] uLineCountBeforeOut The line count before switch out
	 * @param[out] uDRAMStartLine The start line in DRAM
	 * @param[out] uDRAMLineCount The line count in DRAM
	 * @return Execute result
	 * - 0 Get the memory information successfully
	 * - -1 No I2C before
	*/
	int GetLatestMemoryInfo(UINT& uStartLine, UINT& uLineCount, BOOL& bWithDRAM, UINT& uLineCountBeforeOut, UINT& uDRAMStartLine, UINT& uDRAMLineCount);
	/**
	 * @brief Record line using in all I2C opeartion
	*/
	void RecordLine();
	/**
	 * @brief Free I2C
	 * @param[in] strKey The key of I2C operation
	 * @return Execute result
	 * - 0 Free I2C successfully
	 * - -1 The I2C is not existed
	*/
	int FreeI2C(std::string& strKey);
private:
	/**
	 * @brief Get the key of I2C operation
	 * @param[out] strKey The key
	*/
	inline void GetKey(std::string& strKey);
	/**
	 * @brief Create new operation of I2C
	 * @param[in] strKey The key of the new operation
	 * @return Execute result
	 * - 0 Create new operation successfully
	 * - >0 No enough line, the pattern line count of new opeation
	*/
	inline int NewOperation(std::string& strKey);
private:
	struct I2CINFO 
	{
		UINT m_uLineCount;
		std::string m_strKey;
	};
	CDriverAlarm* m_pAlarm;///<The point of the driver alarm
	BYTE m_byRegisterByteCount;///<The byte count of register address
	BOOL m_bRead;///<Whether is read I2C
	const CI2CSite* m_pSite;///<The I2C site information
	CI2CSiteData m_Data;///<The data of current operation
	UINT m_uDataByteCount;///<The data byte count
	BYTE m_bySlaveAddress;///<The slave address
	UINT m_uRegisterAddress;///<The register address
	std::string m_strLatestKey;///<The key of latest operation
	std::map<std::string, CI2CLine*> m_mapI2C;///<The I2C operation, key is key and value is its pattern line
	std::map<UINT, I2CINFO> m_mapI2CLineInfo;///<The I2C line information, key is the start line and the value is line information.
	///<The highest bit is 0 means the I2C is without DRAM. Only filled when vector line is not enough
	CI2CBoardManage* m_pBoardManage;///<The point of the class for board management
	CI2CRAM* m_pRAMManage;///<The point of the class for RAM line management
};
