#pragma once
/**
 * @file I2CBoard.h
 * @brief The class be used to operate board
 * @author Guangyun Wang
 * @date 2020/07/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "..\StdAfx.h"
#include "..\Pattern.h"
#include "..\HardwareFunction.h"
#include "I2CSite.h"
/**
 * @class CControllerData
 * @brief The controller data 
*/
class CControllerData
{
public:
	/**
	 * @brief Constructor
	 * @param[in] byControllerIndex The controller index
	*/
	CControllerData(BYTE byControllerIndex);
	/**
	 * @brief Set the fail line number of the controller
	 * @param[in] mapFailData The fail line number
	 * @param[in] nOffset The offset of the fail line number
	*/
	void SetFailData(const std::vector<CHardwareFunction::DATA_RESULT>& vecFailData, int nOffset);
	/**
	 * @brief Get the result of channel
	 * @param[in] usChannel The channel number
	 * @param[in] vecFail The channel data
	 * @return Execute result
	 * - 0 Get the channel result fail
	 * - -1 The channel number is over range
	*/
	int GetChannelFailLine(USHORT usChannel, std::vector<int>& vecFail);
	/**
	 * @brief Set the data valid
	 * @param[in] bValid Whether the data is valid
	*/
	void SetDataValid(BOOL bValid);
	/**
	 * @brief Check whether the data is valid
	 * @return Whether data valid
	 * - TRUE The data is valid
	 * - FALSE The data is invalid
	*/
	BOOL IsDataValid();
private:
	BYTE m_byIndex;///<The index of controller
	std::vector<CHardwareFunction::DATA_RESULT> m_vecFailData;///<The controller fail line number
	BOOL m_bDataValid;///<Whether data valid
};

/**
 * @class CI2CBoard
 * @brief The class of board operation
*/
class CI2CBoard
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CI2CBoard(BYTE bySlotNo, CDriverAlarm* pAlarm);
	/**
	 * @brief Destructor
	*/
	~CI2CBoard();
	/**
	 * @brief Set I2C site information
	 * @param[in] I2CSite The I2C site information 
	*/
	void SetSite(const CI2CSite& I2CSite);
	/**
	 * @brief Set memory type which the pattern saved
	 * @param[in] bBRAM Whether the pattern saved in BRAM
	*/
	void SetPatternRAM(BOOL bBRAM);
	/**
	 * @brief Set the channel pattern
	 * @param[in] usChannel The channel number
	 * @param[in] uLineIndex The line index
	 * @param[in] cPatternSign The pattern sign
	 * @param[in] lpszCMD The command
	 * @param[in] usCMDOperand The operand
	 * @param[in] bCapture Whether using hardware capture
	 * @param[in] bSwitch Whether switch memory
	 * @return Execute result
	 * - 0 Set the channel pattern successfully
	 * - -1 The channel number is over range
	*/
	int SetChannelPattern(USHORT usChannel, UINT uLineIndex, char cPatternSign, const char* lpszCMD, USHORT usCMDOperand = 0, USHORT bCapture = FALSE, BOOL bSwitch = FALSE);
	/**
	 * @brief Load pattern
	*/
	void Load();
	/**
	 * @brief Set the running parameter
	 * @param[in] uStartLine The start line number
	 * @param[in] uStopLine The stop line number
	 * @param[in] bWithDRAM Whether with DRAM
	 * @param[in] uDRAMStartLine The start line in DRAM
	 * @return Execute result
	 * - 0 Set running parameter successfully
	 * - -1 The start line number in BRAM is over range
	 * - -2 The stop line number in BRAM is over range
	 * - -3 The stop line number is before start line number
	 * - -4 The start line number in DRAM is over range
	*/
	int SetRunParameter(UINT uStartLine, UINT uStopLine, BOOL bWithDRAM, UINT uDRAMStartLine);
	/**
	 * @brief Enable vector running
	 * @param[in] bEnable Whether enable running
	*/
	void EnableRun(BOOL bEnable);
	/**
	 * @brief Run vector
	*/
	void Run();
	/**
	 * @brief Set I2C period
	 * @param[in] dPeriod The I2C period
	 * @param[in] bOnlyValidSite Whether only set the period of valid site
	 * @return Execute result
	 * - 0 Set period successfully
	 * - -1 Not set site information
	 * - -2 The board used is not existed
	 * - -3 The period is over range
	*/
	int SetPeriod(double dPeriod, BOOL bOnlyValidSite);
	/**
	 * @brief Set the edge information
	 * @param[in] vecChannel The channel number
	 * @param[in] pdEdge The edge value
	 * @param[in] WaveFormat The wave format
	 * @param[in] IOFormat The IO format
	 * @return Execute result
	 * - 0 Set edge successfully
	 * - -1 The board is not existed
	 * - -2 The point of the edge is NUL
	 * - -3 The format is error
	 * - -4 The edge value is over range
	*/
	int SetEdge(const std::vector<USHORT>& vecChannel, const double* pdEdge, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat);
	/**
	 * @brief Check whether board is existed
	 * @param[in] bRefresh Refresh the board status
	 * @return Whether board is existed
	 * - TRUE Board existed
	 * - FALSE The board is not existed
	*/
	BOOL IsBoardExist(BOOL bRefresh);
	/**
	 * @brief Wait vector stop
	 * @return Execute result
	 * - 0 Vector stop
	 * - -1 Not unbind
	 * - -2 Not ran before
	 * - -3 Wait timeout
	*/
	int WaitStop();
	/**
	 * @brief Check whether channel is existed
	 * @param[in] usChannl The channel number
	 * @param[in] bRefresh Whether refresh the channel existed
	 * @return Whether channel existed
	 * - 0 Channel is not existed
	 * - 1 Channel is existed
	*/
	int IsChannelExist(USHORT usChannel, BOOL bRefresh = TRUE);
	/**
	 * @brief Set the pin level of I2C channel
	 * @param[in] dVIH The input voltage of logic high
	 * @param[in] dVIL The input voltage of logic low
	 * @param[in] dVOH The output voltage of logic high
	 * @param[in] dVOL The output voltage of logic low
	 * @param[in] byChannelType The channel type, 1 is SCL, 2 is SDA, 3 is All
	 * @return Execute result
	 * - 0 Set pin level successfully
	 * - -1 No set site
	 * - -2 The pin level is over range
	*/
	int SetPinLevel(double dVIH, double dVIL, double dVOH, double dVOL, BYTE byChannelType);
	/**
	 * @brief Get the result of channel in latest ran
	 * @param[in] usChannel The channel number
	 * @param[out] vecResult The result
	 * @return Execute result
	*/
	int GetResult(USHORT usChannel, std::vector<int>& vecResult);
	/**
	 * @brief Set the dynamic load of I2C channel
	 * @param[in] byChannelType The channel type
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
	int SetDynamicLoad(BYTE byChanneltype, BOOL bEnable, double dIOH, double dIOL, double dVTVoltValue, double dClampHighVoltValue, double dClampLowVoltValue);
	/**
	 * @brief Enable compare shield
	 * @param[in] bEnable Whether enabel compare shield
	*/
	void EnableCopmareShield(BOOL bEnable);
private:
	/**
	 * @brief Delete memory of map parameter whose value in heap
	 * @param[in] mapParam The map parameter
	*/
	template <typename Key, typename Value>
	void DelteMemory(std::map<Key, Value>& mapParam);
	/**
	 * @brief Get the point of CHardwareFunction
	 * @param[in] byController The controller index
	 * @return The point of CHardwareFunction
	*/
	inline CHardwareFunction* GetHardware(BYTE byController);
private:
	BYTE m_bySlotNo;///<The slot number of the board
	BOOL m_bBRAM;///<Whether the pattern saved in BRAM
	BYTE m_byTimeset;///<The timeset index of the pattern
	UINT m_uBRAMStartLine;///<The BRAM start line number
	CDriverAlarm* m_pAlarm;///<The point of alarm
	const CI2CSite* m_pSite;///<The site information of I2C
	BOOL m_bEnableCompareShield;///<Whether enable compare shield
	std::map<BYTE, CPattern*> m_mapPattern;///<The pattern of the controller, the key is controller index and value is point of pattern
	std::map<BYTE, CHardwareFunction*> m_mapHardwareFunction;///<The hardware function class of each controller, the key is controller index and value is point of class function
	std::map<BYTE, CControllerData*> m_mapControllerData;///<The controller result data, the key is controller index and value is point of class function
	std::vector<CHardwareFunction*> m_vecRunController;///<The controller ran by I2C
};
