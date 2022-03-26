#pragma once
/**
 * @file I2C.h
 * @brief Include the class of I2C function
 * @author Guangyun Wang
 * @date 2020/07/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "..\DriverAlarm.h"
#include "I2CRAM.h"
/**
 * @class CI2C
 * @brief The function of I2C
*/
class CI2C
{
public:
	/**
	 * @enum REG_MODE
	 * @brief The register address mode
	*/
	enum class REG_MODE
	{
		REG_8 = 0,///<The byte count of register address is 8 bits
		REG_16,///<The byte count of register address is 16 bits
		REG_24,///<The byte count of register address is 24 bits
		REG_32,///<The byte count of register address is 32 bits
	};
	/**
	 * @brief Constructor
	 * @param[in] pAlarm The point of Alarm
	*/
	CI2C(CDriverAlarm* pAlarm);
	/**
	 * @brief Destructor
	*/
	~CI2C();
	/**
	 * @brief Get alarm class
	 * @return The point of the alarm
	*/
	CDriverAlarm* GetAlarm();
	/**
	 * @brief Set the I2C parameter
	 * @param[in] dPeriod The Period
	 * @param[in] usSiteCount The site count
	 * @param[in] RegisterMode The register mode
	 * @param[in] lpszSCL The string of SCL channel
	 * @param[in] lpszSDA The string of SDA channel
	 * @return Execute result
	 * - 0 Set I2C parameter successfully
	 * - -1 The site count is over range
	 * - -2 The register byte is over range
	 * - -3 The channel information is nullptr
	 * - -4 The channel information is blank
	 * - -5 The format of channel is wrong
	 * - -6 The channel of channel is over range
	 * - -7 The channel of channel is not existed
	 * - -8 The channel number is not equal to site count
	 * - -9 The channel is conflict
	*/
	int Set(double dPeriod, USHORT usSiteCount, REG_MODE RegisterMode, const char* lpszSCL, const char* lpszSDA);
	/**
	 * @brief Set the channel status after stop
	 * @param[in] bHighImpedance The channel stop with high impedance
	*/
	void SetStopStatus(BOOL bHighImpedance);
	/**
	 * @brief Set the pin level of I2C channel
	 * @param[in] dVIH The input voltage of logic high
	 * @param[in] dVIL The input voltage of logic low
	 * @param[in] dVOH The output voltage of logic high
	 * @param[in] dVOL The output voltage of logic low
	 * @param[in] byChannelType The channel Type
	 * - 1 is SCL
	 * - 2 is SDA
	 * - 3 is Both
	 * @return Execute result
	 * - 0 Set pin level successfully
	 * - -1 No set site
	 * - -2 The pin level is over range
	*/
	int SetPinLevel(double dVIH, double dVIL, double dVOH, double dVOL, BYTE byChannelType);
	/**
	 * @brief Get the read data of latest read
	 * @param[in] usSiteNo The site number
	 * @param[in] uDataByteIndex The data byte index
	 * @return The read data
	 * - >=0 The site data
	 * - -1 Not set site information
	 * - -2 Not read I2C before
	 * - -3 The site number is over range
	 * - -4 The site is invalid
	 * - -5 The data byte index is over range
	 * - -6 The data of the site is not existed
	*/
	int GetReadData(USHORT usSiteNo, UINT uDataByteIndex);
	/**
	 * @brief Set the base line of I2C
	 * @param[in] nBaseLine The base line number in BRAM
	 * @return Execute result
	 * - 0 Set the base line successfully
	 * - -1 The line available is not enough
	 * - -2 The DRAM line available is not enough
	*/
	int SetBaseLine(int nBaseLine);
	/**
	 * @brief Set the edge of I2C
	 * @param[in] bSCL Whether set the SCL pin
	 * @param[in] pdEdge The edge value
	 * @param[in] bOnlyValidSite Only set the edge of the channels in valid site
	 * @return Execute result
	 * - 0 Set edge successfully
	 * - -1 Not set channel information
	 * - -2 The point of edge is nullptr
	 * - -3 The edge is over range
	*/
	int SetEdge(BOOL bSCL, const double* pdEdge, BOOL bOnlyValidSite = TRUE);
	/**
	 * @brief Get the NACK index of latest ran
	 * @param[in] usSiteNo The site number
	 * @return The NACK index
	 * - >0 The NACK index
	 * - 0 No NACK
	 * - -1 Not set I2C information before
	 * - -2 The site index is over range
	 * - -3 Current site is invalid
	 * - -4 No I2C operation before
	 * - -5 The board of the site is not existed
	*/
	int GetNACKIndex(USHORT usSiteNo);
	/**
	 * @brief Get the I2C site count
	 * @return The site count
	*/
	USHORT GetSiteCount();
	/**
	 * @brief Reset I2C
	 * @param[in] Whether reset channel information of each site
	*/
	void Reset(BOOL bIncludeChannel = TRUE);
	/**
	 * @brief Connect function relay
	 * @param[in] bConnect Whether connect function relay
	 * @return Execute result
	 * - 0 Set function relay before
	 * - -1 Not set site channel information
	*/
	int Connect(BOOL bConnect);
	/**
	 * @brief Set I2C period
	 * @param[in] dPeriod The I2C period
	 * @return Execute result
	 * - -1 Not set site channel information before
	 * - -2 The edge is over range
	*/
	int SetPeriod(double dPeriod);
	/**
	 * @brief Write I2C 
	 * @param[in] bySlaveAddress The slave address
	 * @param[in] uRegisterAddress The register address
	 * @param[in] uDataByteCount The data byte count written
	 * @param[in] pbyWriteData The data of each site
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 Not set site information
	 * - -2 The data byte less than 1
	 * - -3 The point of data is nullptr
	 * - -4 Line not enough
	*/
	int Write(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataByteCount, const BYTE* const pbyWriteData[]);
	/**
	 * @brief Write I2C
	 * @param[in] bySlaveAddress The slave address
	 * @param[in] uRegisterAddress The register address
	 * @param[in] uDataByteCount The data byte count written
	 * @param[in] pbyWriteData The data written
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 Not set site information
	 * - -2 The data byte less than 1
	 * - -3 The point of data is nullptr
	 * - -4 Line not enough
	*/
	int Write(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataByteCount, const BYTE* pbyWriteData);
	/**
	 * @brief Read data
	 * @param[in] bySlaveAddress The slave address
	 * @param[in] uRegisterAddress The register address
	 * @param[in] uDataByteCount The data byte count
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 Not set site information
	 * - -2 No enough line memory for I2C read
	*/
	int Read(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataByteCount);
	/**
	 * @brief Get the read byte count
	 * @return The read byte count
	 * - >= 0 The read byte count
	 * - -1 Not set channel information
	 * - -2 The latest ran is not I2C read
	*/
	int GetReadByteCount();
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
	 * @brief Check whether the site is valid
	 * @param[in] usSiteNo The site number
	 * @return Whether the site is valid
	 * - TRUE The site is valid
	 * - FALSE The site is invalid
	*/
	BOOL IsSiteValid(USHORT usSiteNo) const;
	/**
	 * @brief Set the board existed
	 * @param[in] vecBoard The slot number of the board
	*/
	void SetExistedBoard(const std::vector<BYTE>& vecBoard);
	/**
	 * @brief Set the dynamic load of I2C channel
	 * @param[in] byChanneltype The channel type
	 * @param[in] bEnable Whether enable dynamic load
	 * @param[in] dIOH The current when pin level higher than VT
	 * @param[in] dIOL The current when pin level lower than VT
	 * @param[in] dVTVoltValue The VT
	 * @param[in] dClampHighVoltValue The clamp high voltage
	 * @param[in] dClampLowVoltValue The clampe low voltage
	 * @return Execute result
	 * - 0 Set dynamic load successfully
	 * - -1 Not set I2C channel
	 * - -2 The output current is over range
	 * - -3 The VT is over range
	 * - -4 The clamp is over range
	*/
	int SetDynamicLoad(BYTE byChannelType, BOOL bEnable, double dIOH, double dIOL, double dVTVoltValue, double dClampHighVoltValue = 7.5, double dClampLowVoltValue = -2.5);
	/**
	 * @brief Get the channel used in I2C
	 * @param[out] vecChannel The channel used
	*/
	void GetChannel(std::vector<CHANNEL_INFO>& vecChannel) const;
	/**
	 * @brief Get the channel information
	 * @param[in] usSiteNo The site number
	 * @param[in] bSCL Whether get the channel of SCL
	 * @param[out] Channel The channel gotten
	 * @return Execute result
	 * - 0 Get the chanenl successfully
	 * - -1 The site number is over range
	*/
	int GetChannel(USHORT usSiteNo, BOOL bSCL, CHANNEL_INFO& Channel) const;
	/**
	 * @brief Set the chanenl information
	 * @param dPeriod 
	 * @param RegMode 
	 * @param vecSCL 
	 * @param vecSDA 
	 * @return 
	*/
	int Set(double dPeriod, REG_MODE RegMode, const std::vector<CHANNEL_INFO>& vecSCL, const std::vector<CHANNEL_INFO>& vecSDA);
	/**
	 * @brief Get period
	 * @return The period
	*/
	double GetPeriod();
	/**
	 * @brief Get the register mode
	 * @return The register mode
	*/
	int GetRegisterByteCount();
	/**
	 * @brief Enable compare shield
	 * @param[in] bEnable Whether enabel compare shield
	 */
	void EnableCopmareShield(BOOL bEnable);
private:
	/**
	 * @brief Initialize operation
	 * @param[in] bRead Whether initialize read operation
	*/
	inline void Initialize(BOOL bRead);
	/**
	 * @brief Set the operation parameter
	 * @param[in] bRead Whether is read operation
	 * @param[in] bySlaveAddress The slave address
	 * @param[in] uRegisterAddress The register address
	 * @param[in] uDataBytesCount The data byte count
	*/
	void SetOperationParam(BOOL bRead, BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataBytesCount);
	/**
	 * @brief Set the site data
	 * @param[in] usSiteIndex The site number
	 * @param[in] pbyData The site data
	 * @return Execute result
	 * - 0 Set site data successfully
	 * - -1 Not set channel information before
	 * - -2 Latest operation is ran
	 * - -3 The site number is over range
	 * - -4 The point is data is nullptr
	 * - -5
	*/
	int SetSiteData(USHORT usSiteIndex, const BYTE* pbyData);
	/**
	 * @brief Run vector
	 * @return Execute result
	 * - 0 Ran vector successfully
	 * - -1 No enough line memory
	*/
	inline int Run();
	/**
	 * @brief Run vector
	 * @param[in] bRead Whether current run is read data
	 * @return Execute result
	 * - 0 Run vector successfully
	 * - -1 No valid operation
	 * - -2 No enough line memory for the vector
	*/
	int Run(BOOL bRead);
	/**
	 * @brief Load the common pattern which will be shared by the operation used DRAM
	*/
	void LoadCommonPattern();
	/**
	 * @brief Parse the string of channel information to channel
	 * @param[in] lpszChannel The channel string
	 * @param[out] vecChannel The channel information of all site
	 * @return Parse result
	 * - 0 Parse channel successfully
	 * - -1 The point of channel string is nullptr
	 * - -2 The channel information is blank
	 * - -3 No slot sign
	 * - -4 No channel sign
	 * - -5 No slot information
	 * - -6 The slot information is error
	 * - -7 The channel is over range
	 * - -8 The channel in some site not existed 
	*/
	int SlotParse(const char* lpszChannel, std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Check the channel
	 * @param[in] vecSCL The SCL channel of each site
	 * @param[in] vecSDA The SDA channel of each site
	 * @return The check result
	 * - 0 Check pass
	 * - -1 The site count of SCL is not equal to SDA's
	 * - -2 The channel is conflict
	*/
	int CheckChannel(const std::vector<CHANNEL_INFO>& vecSCL, const std::vector<CHANNEL_INFO>& vecSDA);
	/**
	 * @brief Set the period of I2C
	 * @param[in] bOnlyValidSite Whether only set the period of valid site
	 * @return Execute result
	 * - 0 Set period successfully
	 * - -1 The period is over range
	*/
	int SetPeriod(BOOL bOnlyValidSite = TRUE);
	/**
	 * @brief Delete memory of map parameter whose value in heap
	 * @param[in] mapParam The map parameter
	*/
	template <typename Key, typename Value>
	void DeleteMemory(std::map<Key, Value>& mapParam);
private:
// 	CI2CSite m_I2CSite;///<The site information of I2C
// 	CI2CBase* m_I2C[2];///<The I2C operation of read and write
	BYTE m_byRegisterByteCount;///<The register byte count
	BOOL m_bLatestRead;///<Whether the latest operation is read
	BOOL m_bLastRunSuccess;///<Whether latest ran is successfully
	double m_dPeriod;///<The period of I2C
	CDriverAlarm* m_pAlarm;///<The point of driver alarm
	BOOL m_bLoadCommonLine;///<Whether load common line which shared by all pattern with DRAM
//	std::map<BYTE, CHardwareFunction*> m_mapBoard;///<The board information, using for relay operation, key is slot number and value is point of CHardwareFuncction
	std::string m_strSCLChannel;///<The channel string of SCL
	std::string m_strSDAChannel;///<The channel string of SDA
//	CI2CBoardManage* m_pBoardManage;///<The point of the class for board management
	CI2CRAM* m_pRAMManage;///<The point of the class for RAM line management
};
