#pragma once
/**
 * @file I2CLine.h
 * @brief Include the class for loading I2C pattern line
 * @author Guangyun Wang
 * @date 2020/07/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controler Co., Ltd.
*/
#include "..\StdAfx.h"
#include <vector>
#include <string>
#include "I2CSite.h"
#include "I2CBoard.h"
#include "I2CSiteData.h"
#include "..\DriverAlarm.h"
#include "I2CBoardManage.h"
#include "I2CRAM.h"
/**
 * @class CI2CLine
 * @brief The class for loading I2C pattern line
*/
class CI2CLine
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bRead Whether is I2C read
	 * @param[in] I2CSite The I2CSite information
	*/
	CI2CLine(BOOL bRead, const CI2CSite& I2CSite, CDriverAlarm* pAlarm, CI2CBoardManage* pBoardMange, CI2CRAM* pRAMMange);
	/**
	 * @brief Set the stop staus of I2C
	 * @param[in] bHighImpedance Whether the status is stop with high impedance
	*/
	void SetStopStatus(BOOL bHighImpedance);
	/**
	 * @brief Destructor
	*/
	~CI2CLine();
	/**
	 * @brief Set the byte count of register address
	 * @param[in] byByteCount The byte count of register address
	*/
	void SetRegisterByte(BYTE byByteCount);
	/**
	 * @brief Get the byte count of register address
	 * @return The byte count of register address
	*/
	BYTE GetRegisterByte();
	/**
	 * @brief Set the address
	 * @param[in] bySlaveAddress The slave address
	 * @param[in] uRegisterAddress The register address
	 * @param[in] uDataByteCount The data byte count
	*/
	void SetAddress(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataByteCount);
	/**
	 * @brief Get the slave address
	 * @return The slave address
	*/
	int GetSlaveAddress();
	/**
	 * @brief Get the register address
	 * @return The register address
	*/
	UINT GetRegisterAddress();
	/**
	 * @brief Get the data byte count
	 * @return The data byte count
	*/
	UINT GetDataCount();
	/**
	 * @brief Set the data of each site
	 * @param[in] SiteData The site data
	*/
	void SetData(const CI2CSiteData& SiteData);
	/**
	 * @brief Get the site data
	 * @param[in] usSiteNo The site number
	 * @param[in] uDataByteIndex The byte index of data
	 * @return The site data
	 * - >=0 The site data
	 * - -1 The site number is over range
	 * - -2 The data byte index is over range
	 * - -3 The channel of the site is not existed
	 * - -4 The site is not read data before
	*/
	int GetSiteData(USHORT usSiteNo, UINT uDataByteIndex);
	/**
	 * @brief Get the site data 
	 * @param[out] SiteData The site data
	*/
	void GetData(CI2CSiteData& SiteData);
	/**
	 * @brief Load I2C vector
	 * @return Execute result
	 * - 0 Load vector successfully
	 * - -1 No enough line
	*/
	int LoadVector();
	/**
	 * @brief Run pattern
	 * @return Execute result
	 * - 0 Run vector successfully
	 * - -1 No enough line
	*/
	int Run();
	/**
	 * @brief Get the pattern line count
	 * @param[in] bBRAM Whether get the BRAM pattern count
	 * @param[out] puStartLine The start line index
	 * @param[out] puLineCount The line count
	 * @return Execute result
	 * - 0 Get the line count successfully
	 * - -1 Not load vector before
	 * - -2 The point of start line or line count is nullptr
	*/
	int GetLineCount(BOOL bBRAM, UINT* puStartLine, UINT* puLineCount);
	/**
	 * @brief Get the pattern line count of I2C
	 * @return The pattern line count
	*/
	UINT GetPatternCount();
	/**
	 * @brief Initialize the common pattern line which be shard by the I2C line used DRAM
	*/
	void InitialCommonLine();
	/**
	 * @brief Change the stop satus of common line
	 * @param[in] bHighImpedance Whether the channel stop with high impedance
	*/
	void ChangeCommonLineStopStatus(BOOL bHighImpedance);
	/**
	 * @brief Get the NACK index of site
	 * @param[in] usSiteNo The site number
	 * @return The NACK index
	 * - >0 The NACK index
	 * - 0 The site number is over range
	 * - -1 The site is over range
	 * - -2 The site is invalid
	 * - -3 The board on the site is not valid
	*/
	int GetNACK(CI2CSiteData& SiteData, USHORT usSiteNo);
	/**
	 * @brief Is load vector before
	 * @return Whether load vector before
	 * - TRUE Load vector before
	 * - FALSE Not load vector before
	*/
	BOOL IsLoadVector();
	/**
	 * @brief Whether the pattern use DRAM
	 * @return Whether use DRAM
	 * - TRUE Use DRAM
	 * - FALSE Not use DRAM
	*/
	BOOL IsWithDRAM();
	/**
	 * @brief Get vector line count exclusive the common pattern
	 * @param[out] puBRAMLineCount The line count in BRAM
	 * @param[out] puDRAMLineCount The line count in DRAM
	 * @return The line count
	*/
	UINT GetExclusiveLineCount(UINT* puBRAMLineCount = nullptr, UINT* puDRAMLineCount = nullptr);
	/**
	 * @brief Get the start line of the pattern
	 * @param[out] puBRAMStartLine The start line in BRAM
	 * @param[out] puDRAMStartLine The start line number in DRAM
	 * @return Whether using DRAM
	 * - TRUE Using DRAM
	 * - FALSE Not use DRAM
	*/
	BOOL GetStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine);
	/**
	 * @brief Get the line count before switch out
	 * @return The line count before switch out
	 * - >0 The line count before switch out
	 * - - 0 Without switch
	*/
	UINT GetLineCountBeforeSwitch();
private:
	/**
	 * @brief Load the command line which be shared by the I2C line used DRAM
	 * @param[in] uStartLine The start line index in BRAM
	 * @param[in] byHeadLineCount The line count before switch out
	 * @param[in] byEndLineCount The line count after switch in
	*/
	inline void LoadCommonLine(UINT uStartLine, BYTE byHeadLineCount, BYTE byEndLineCount);
	/**
	 * @brief Load pattern
	 * @return Execute result
	 * - 0 Load pattern successfully
	 * - -1 No enough memory
	*/
	int LoadPattern();
	/**
	 * @brief Get the slave address pattern
	 * @param[out] vecSDAPattern The pattern of SDA
	 * @param[in] bSecond Whether the pattern is the second start slave
	 * @param[out] bCheckACK Whether check ACK
	*/
	void GetSlavePattern(std::vector<char>& vecSDAPattern, BOOL bSecond, BOOL* bCheckACK);
	/**
	 * @brief Get register address pattern
	 * @param[in] byRegisterIndex The byte index of register
	 * @param[out] vecSDAPattern The SDA pattern
	 * @param[out] bCheckACK Whether check ACK
	 * @return Execute result
	 * - 0 Get pattern successfully
	 * - -1 The register index is over range
	*/
	int GetRegisterPattern(BYTE byRegisterIndex, std::vector<char>& vecSDAPattern, BOOL* bCheckACK);
	/**
	 * @brief Get the data pattern
	 * @param[in] usSiteNo The site number
	 * @param[in] uDataByteIndex The data byte index 
	 * @param[out] vecSDAPattern The SDA pattern
	 * @param[out] pbCheckACK Whether check ACK
	 * @return Execute result
	 * - 0 Get data pattern successfully
	 * - -1 The site number is over range
	 * - -2 The data byte index is over range
	*/
	int GetDataPattern(USHORT usSiteNo, UINT uDataByteIndex, std::vector<char>& vecSDAPattern, BOOL* pbCheckACK);
	/**
	 * @brief Allocate pattern line for saving pattern
	 * @return Execute result
	 * - 0 Allocate pattern line successfully
	 * - -1 No enough pattern
	*/
	int AllocateLine();
	/**
	 * @brief Convert data to pattern
	 * @param[in] byData The data
	 * @param[out] vecPattern The SDA pattern
	*/
	inline void Data2Pattern(BYTE byData, std::vector<char>& vecPattern);
	/**
	 * @brief Load SCL and SDA pattern
	 * @param[in] vecSCLPattern The SCL pattern
	 * @param[in] vecSDAPattern The SDA pattern
	 * @param[in] bLastLineSwitch Whether last line switch memory
	 * @return Execute result
	 * - 0 Load pattern successfully
	 * - -1 The pattern of SDA is not equal to SCL's
	*/
	inline int LoadPattern(std::vector<char>& vecSCLPattern, std::vector<char>& vecSDAPattern, BOOL bLastLineSwitch = FALSE, BOOL bStopPattern = FALSE);
	/**
	 * @brief Get the last stop pattern
	 * @param[in] bHighImpedance Whether stop with high impedance
	 * @return The last stop pattern
	*/
	inline char GetLastStopPattern(BOOL bHighImpedance);
	/**
	 * @brief Load data pattern
	 * @param[in] uOffset The first data bit offset to the start line of the current memory
	*/
	inline void LoadDataPattern(UINT uOffset);
	/**
	 * @brief Get the fail line number of the site
	 * @param[in] usSiteNo The site number
	 * @param[out] vecFailLine The fail line number
	 * @return Execute result
	 * - 0 Get fail line number successfully
	 * - -1 The site number is over range
	 * - -2 The site is invalid
	 * - -3 The board on the site is not valid
	*/
	int GetFailLineNo(USHORT usSiteNo, std::vector<int>& vecFailLine);
	/**
	 * @brief Set the site data
	 * @param[in] usSiteNo The site number
	 * @param[in] vecFailLine The fail line number
	 * @return Execute result
	 * - 0 Set site data successfully
	 * - -1 Allocate memory fail
	*/
	int SetSiteData(CI2CSiteData& SiteData, USHORT usSiteNo, const std::vector<int>& vecFailLine);
	/**
	 * @brief Get the NACK index
	 * @param[in] usSiteNo The site number
	 * @param[in] vecFailLine The fail line number
	*/
	inline void SetNACK(CI2CSiteData& SiteData, USHORT usSiteNo, const std::vector<int>& vecFailLine);
	/**
	 * @brief Download pattern
	*/
	inline void DownloadPattern();
	/**
	 * @brief Get the start pattern
	 * @param[out] vecSCL The pattern of SCL
	 * @param[out] vecSDA The pattern of SDA
	*/
	inline void GetStartPattern(std::vector<char>& vecSCL, std::vector<char>& vecSDA);
	/**
	 * @brief Get the restart pattern
	 * @param[out] vecSCL The pattern of SCL
	 * @param[out] vecSDA The pattern of SDA
	*/
	inline void GetRestartPattern(std::vector<char>& vecSCL, std::vector<char>& vecSDA);
	/**
	 * @brief Get the stop pattern
	 * @param[out] vecSCL The pattern of SCL
	 * @param[out] vecSDA The pattern of SDA
	*/
	inline void GetStopPattern(std::vector<char>& vecSCL, std::vector<char>& vecSDA);
	/**
	 * @brief Get the normal SCL pattern
	 * @param[out] vecSCL The SCL pattern
	*/
	inline void GetNormalSCLPattern(std::vector<char>& vecSCL);
	/**
	 * @brief Set the stop status
	 * @param[in] uLineNo The line number
	*/
	void ChangeStopStatus(UINT uLineNo, BOOL bHigImpedance);
private:
	const CI2CSite* m_pSite;///<The site channel information
	CDriverAlarm* m_pAlarm;///<The driver alarm
	const CI2CSiteData* m_pSiteData;///<The site data
	bool m_bRead;///<Whether read pattern
	bool m_bLoad;///<Whether load pattern before
	BYTE m_byRegisterAddressByteCount;///<The byte count of register address
	BYTE m_bySlaveAddress;///<The slave address
	UINT m_uRegisterAddress;///<The register address
	UINT m_uDataByteCount;///<The data byte count
	bool m_bWithDRAM;///<Whether the pattern with DRAM
	UINT m_uStartLine;///<The start line number
	UINT m_uStopLine;///<The stop line number
	UINT m_uDRAMStartLine;///<The start line of DRAM if use DRAM
	UINT m_uDRAMLineCount;///<The line count in DRAM if use DRAM
	UINT m_uCurLine;///<THe current line index
	UINT m_uDataBaseOffset;///<The first data's line offset to the start of pattern memory
	UINT m_uLineCountBeforeSwitch;///<The line count before switch out
	std::vector<UINT> m_vecACKLineOffset;///<The ACK line index offset to the start of pattern memory
	CI2CBoardManage* m_pBoardManage;///<The point of the class for board management
	CI2CRAM* m_pRAMManage;///<The point of the class for RAM line management
};