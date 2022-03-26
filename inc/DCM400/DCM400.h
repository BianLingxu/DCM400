#pragma once
/**
 * @file DCM400.h
 * @brief The API of board DCM400
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <windows.h>

#ifdef DCM400_EXPORTS
#define DCM400_API __declspec(dllexport)
#else
#define DCM400_API __declspec(dllimport)
#endif ///<DCM400_EXPORTS

class DCM400Data;///<For extension in future

/**
 * @enum MODULE_INSTRUCTION
 * @brief The instrction of module running
*/
enum DCM400_MODULE_INSTRUCTION
{
	INC_INS = 0,///<The INC instruction
	REPEAT_INS,///<The repeat instruction
};
/**
 * @enum DCM400_DATA_FORMAT
 * @brief The data format of DCM400 waveform
*/
enum DCM400_DATA_FORMAT
{
	DCM400_DATA_FORMAT_NRZ = 0,///<Not Return Zero format
	DCM400_DATA_FORMAT_RZ,///<Return Zero format
	DCM400_DATA_FORMAT_RO,///<Return One format
	DCM400_DATA_FORMAT_SBH,///<Surround By High format 
	DCM400_DATA_FORMAT_SBL,///<Surround By Low format 
	DCM400_DATA_FORMAT_SBC,///<Surround By Compliment format 
	DCM400_DATA_FORMAT_STAY,///<Stay format
	DCM400_DATA_FORMAT_FORCE_HIGH,///<Force ouput High format
	DCM400_DATA_FORMAT_FORCE_LOW,///<Force output Low format
};
/**
 * @enum DCM400_IO_FORMAT
 * @brief The IO format of DCM400
*/
enum DCM400_IO_FORMAT
{
	DCM400_IO_FORMAT_NRZ = 0,///<Not Return Zero format
	DCM400_IO_FORMAT_RO,///<Return One format
	DCM400_IO_FORMAT_OFF,///<Force output Low format
};
/**
 * @enum DCM400_COMPARE_MODE
 * @brief The compare mode
*/
enum DCM400_COMPARE_MODE
{
	DCM400_COMPARE_EDGE = 0,///<Compare edge
	DCM400_COMPARE_WINDOW,///<Compare window
	DCM400_COMPARE_OFF,///<Force output Low format
};
/**
 * @enum DCM400_PIN_STATUS
 * @brief The channel status
*/
enum DCM400_PIN_STATUS
{
	DCM400_PIN_LOW = 0,///<The pin is in driver low status
	DCM400_PIN_HIGH = 0,///<The pin is in driver high status
	DCM400_PIN_HIGH_IMPEDANCE = 0,///<The pin is in high impedance
};
/**
 * @enum DCM400_CHANNEL_MODE
 * @brief The channel mode
*/
enum DCM400_PIN_MODE
{
	DCM400_VECTOR_MODE = 0,///<The pin status will changed with vector running
	DCM400_FORCE_MODE,///<The pin status not changed with vector running
};
/**
 * @enum DCM400_VT_MODE
 * @brief The VT mode
*/
enum DCM400_VT_MODE
{
	DCM400_VT_FORCE = 0,///<Output VT directly
	DCM400_VT_REALTIME,///<The VT output when pin in compare mode in vector running
	DCM400_VT_CLOSE,///<Close VT
};
/**
 * @enum DCM400_FAIL_SAVING_TYPE
 * @brief The fail saving type
*/
enum DCM400_FAIL_SAVING_TYPE
{
	DCM400_SAVING_ALL_FAIL = 0,///<Saving all fail in future vector running
	DCM400_SAVING_SELECT_FAIL,///<Saving fail of selected line in future vector running
};
/**
 * @enum DCM400_TMU_UNIT
 * @brief The TMU unit
*/
enum DCM400_TMU_UNIT
{
	DCM400_TMU1 = 0,///<The TMU unit 1
	DCM400_TMU2,///<The TMU unit 2
	DCM400_ALL_TMU,///<The all TMU unit, only valid for setting parameter
};
/**
 * @enum DCM400_SLOPE_TYPE
 * @brief The slope type of TMU trigger
*/
enum DCM400_TMU_SLOPE_TYPE
{
	DCM400_SLOP_POS = 0,///<The positive slope
	DCM400_SLOP_NEG,///<The negative slope
};
enum DCM400_TMU_MEAS_MODE
{
	DCM400_TMU_MES_FREQ_DUTY = 0,///<TMU measure frequency and duty
	DCM400_TMU_MES_EDGE,///<TMU measure edge
	DCM400_TMU_MES_DELAY,///<TMU measure delay
};
/**
 * @enum DCM400_TMU_MEAS_TYPE
 * @brief The measurement type of TMU
*/
enum DCM400_TMU_MEAS_TYPE
{
	DCM400_TMU_FREQ = 0,///<TMU frequence
	DCM400_TMU_HIGH_DUTY,///<TMU high duty
	DCM400_TMU_LOW_DUTY,///<TMU low duty
	DCM400_TMU_EDGE,///<TMU edge
	DCM400_TMU_DELAY,///<TMU delay
};
/**
 * @enum DCM400_PPMU_MODE
 * @brief The PPMU mode
*/
enum DCM400_PPMU_MODE
{
	DCM400_PPMU_FVMI = 0,///<Force Voltage Measure Current
	DCM400_PPMU_FIMV,///<Force Current Measure Voltage
	DCM400_PPMU_FIMI,///<Force Current Measure Current
	DCM400_PPMU_FVMV,///<Force Voltage Measure Voltage
};
/**
 * @enum DCM400_PPMU_IRANGE
 * @brief The current range of PPMU
*/
enum DCM400_PPMU_IRANGE
{
	DCM400_PPMU_IRANGE_2UA = 0,///<Current range 2uA
	DCM400_PPMU_IRANGE_10UA,///<Current range 10uA
	DCM400_PPMU_IRANGE_100UA,///<Current range 100uA
	DCM400_PPMU_IRANGE_1MA,///<Current range 1mA
	DCM400_PPMU_IRANGE_40MA,///<Current range 40mA
};
/**
 * @enum DCM400_I2C_REG_ADDR_MODE
 * @brief The register address mode of I2C
*/
enum DCM400_I2C_REG_ADDR_MODE
{
	DCM400_I2C_REG_8 = 0,///<The register address in 8 bits
	DCM400_I2C_REG_16,///<The register address in 16 bits
	DCM400_I2C_REG_24,///<The register address in 24 bits
	DCM400_I2C_REG_36,///<The register address in 36 bits
};
/**
 * @enum DCM400_I2C_CHANNEL
 * @brief The I2C channel type
*/
enum DCM400_I2C_CHANNEL
{
	DCM400_I2C_SCL = 0,///<The clock channel of I2C
	DCM400_I2C_SDA,///<The data channel of I2C
	DCM400_I2C_BOTH,///<All channels of I2C
};
/**
 * @class DCM400
 * @brief The class of board DCM400
*/
class DCM400_API DCM400
{
public:
	/**
	 * @brief Constrcutor
	*/
	DCM400();
	/**
	 * @brief Destructor
	*/
	~DCM400();
	/**
	 * @brief Load vector file
	 * @param[in] lpszVectorFile The vector file name without path
	 * @param[in] bReload Whether reload vector file
	 * @return Execute result
	 * - 0 Load vector file successfully
	 * - -1 Load vector file fail
	*/
	int LoadVectorFile(const char* lpszVectorFile, BOOL bReload = TRUE);
	/**
	 * @brief Set the vector module information
	 * @param[in] lpszModuleName The module name
	 * @param[in] lpszStartLabel The start label of the module
	 * @param[in] lpszStopLabel The stop label of the module
	 * @return Execute result
	 * - 0 Set the vector module information successfully
	*/
	int SetVectorModule(const char* lpszModuleName, const char* lpszStartLabel, const char* lpszStopLabel);
	/**
	 * @brief Set the vector run order
	 * @param[in] lpszOrderName The name of running order
	 * @param[in] lpszModuleList The module list in the order
	 * @param[in] uModuleCount The module count
	 * @param[in] pModuleInstrction The instruction of the module, the aray elements count must not less than module count
	 * @param[in] lpszOperand The operand of each module, the aray elements count must not less than module count
	 * @return Execute result
	 * - 0 Set the running order successfully
	*/
	int SetVectorRunOrder(const char* lpszOrderName, const char* lpszModuleList, UINT uModuleCount = 0, DCM400_MODULE_INSTRUCTION* pModuleInstrction = nullptr, const char* lpszOperand = nullptr);
	/**
	 * @brief Set the vector which modified frequency
	 * @param[in] lpszOrderName The running order name
	 * @return Execute result
	 * - 0 Set the vector successfully
	*/
	int SetVectorModifiedFrequent(const char* lpszOrderName);
	/**
	 * @brief Set the valid pin for current instance
	 * @param[in] lpszPinNameList The pin name list
	 * @return 
	*/
	int SetValidPin(const char* lpszPinNameList);
	/**
	 * @brief Set the pin group
	 * @param[in] lpszPinGroupName The pin group name
	 * @param[in] lpszPinNameList The pin name list, split with ','
	 * @return Execute result
	 * - 0 Set pin group successfully
	 * - -1 Not load vector file
	*/
	int SetPinGroup(const char* lpszPinGroupName, const char* lpszPinNameList);
	/**
	 * @brief Set the voltage of pin
	 * @param[in] lpszPinGroup The pin group or pin name name
	 * @param[in] dVIH The input high voltage for chip
	 * @param[in] dVIL The input low voltaget for chip
	 * @param[in] dVOH The output high voltage of chip
	 * @param[in] dVOL The ouput low voltage of chip
	 * @return 
	*/
	int SetPinLevel(const char* lpszPinGroup, double dVIH, double dVIL, double dVOH, double dVOL);
	/**
	 * @brief Connect the relay of pin
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @return Execute result
	*/
	int Connect(const char* lpszPinGroup);
	/**
	 * @brief Disconnect the relay of pin
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @return Execute result
	*/
	int Disconnect(const char* lpszPinGroup);
	/**
	 * @brief Set the period of timeset
	 * @param[in] lpszTimeSetName The time set name
	 * @param[in] dPeriod The period will be set
	 * @return Execute result
	*/
	int SetPeriod(const char* lpszTimeSetName, double dPeriod);
	/**
	 * @brief Set edge of timeset
	 * @param[in] lpszPinGroup The name of pin group or pin name
	 * @param[in] lpszTimesetName The name of the timeset
	 * @param[in] DataFormat The wave format
	 * @param[in] IOFormat The IO format
	 * @param[in] dT1R The raise edge of driver wave
	 * @param[in] dT1F The fall edge of driver wave
	 * @param[in] dIOR The raise edge of IO
	 * @param[in] dIOF The fall edge of IO
	 * @param[in] dSTBR The raise edge of compare
	 * @param[in] dSTBF The fall edge of compare
	 * @return Execute result
	*/
	int SetEdge(const char* lpszPinGroup, const char* lpszTimeSetName, DCM400_DATA_FORMAT DataFormat, DCM400_IO_FORMAT IOFormat, DCM400_COMPARE_MODE CompareMode, double dT1R, double dT1F, double dIOR, double dIOF, double dSTBR, double dSTBF);
	/**
	 * @brief Set the VT
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] VTMode The VT mode
	 * @return Execute result
	*/
	int SetVT(const char* lpszPinGroup, DCM400_VT_MODE VTMode = DCM400_VT_CLOSE);
	/**
	 * @brief Set the dynamic load
	 * @param[in] lpszPinGroup The pin group or pin name name
	 * @param[in] bEnable Whether enable dynamic load
	 * @param[in] dVCOM The voltage for comparison
	 * @param[in] dIOH The output current if the voltage higher than VCOM
	 * @param[in] dIOL The output current if the voltage lower than VCOM
	 * @return Execute result
	*/
	int SetDynamicLoad(const char* lpszPinGroup, BOOL bEnable, double dVCOM, double dIOH, double dIOL);
	/**
	 * @brief Set the pin status
	 * @param[in] lpszPinGroup The pin group or pin name whose pin status will be set
	 * @param[in] PinStatus The pin status
	 * @param[in] PinMode The pin mode
	 * @return Execute result
	 * - 0 Set pin status successfully
	*/
	int SetPinStatus(const char* lpszPinGroup, DCM400_PIN_STATUS PinStatus = DCM400_PIN_HIGH_IMPEDANCE, DCM400_PIN_MODE PinMode = DCM400_VECTOR_MODE);
	/**
	 * @brief Set the wave data written parameter
	 * @param[in] lpszPinGroup The pin group
	 * @param[in] lpszStartLabel The start label
	 * @param[in] ulOffset The first vector line number offset to the start label
	 * @param[in] uWriteLineCount The vector line count will be written
	 * @return Execute result
	*/
	int SetWaveDataParam(const char* lpszPinGroup, const char* lpszStartLabel, ULONG ulOffset, UINT uWriteLineCount);
	/**
	 * @brief Set the wave data of each site
	 * @param[in] usSiteNo The site number
	 * @param[in] pbyWaveDta The point pointed to the wave data
	 * @note The elements count of pbyWaveData must not less than wave line count divide 8
	 * @return Execute result
	*/
	int SetSiteWaveData(USHORT usSiteNo, BYTE* pbyWaveDta);
	/**
	 * @brief Write the wave data to board
	 * @note The SetWaveDataParam and SetSiteWave must be used before for set the wave data
	 * @return Execute result
	*/
	int WriteWaveData();
	/**
	 * @brief Set the fail saving type
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] FailSavingType The fail saving type
	 * @return Execute result
	*/
	int SetFailSavingType(const char* lpszPinGroup, DCM400_FAIL_SAVING_TYPE FailSavingType);
	/**
	 * @brief Set the high voltage
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] bEnable Whether enable high voltage mode
	 * @return Execute result
	*/
	int SetHighVoltage(const char* lpszPinGroup, BOOL bEnable = FALSE);
	/**
	 * @brief Run vector with label
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] lpszStartLabel The start label of vector running
	 * @param[in] lpszStopLabel The stop label of vector running
	 * @param[in] bWaitFinish Whether wait the vector running finished
	 * @return Execute result
	*/
	int RunLabelVector(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel, BOOL bWaitFinish);
	/**
	 * @brief Run vector with label synchronization
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] lpszStartLabel The start label of vector running
	 * @param[in] lpszStopLabel The stop label of vector running
	 * @return Execute result
	*/
	int RunLabelVectorSyn(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel);
	/**
	 * @brief Run vector of module order
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] lpszOrder The module order name
	 * @param[in] bWaitFinish Whether wait the vector running finished
	 * @return Execute result
	*/
	int RunModuleVector(const char* lpszPinGroup, const char* lpszOrder, BOOL bWaitFinish);
	/**
	 * @brief Run vector with label synchronization
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] lpszOrder The module order name
	 * @return Execute result
	*/
	int RunModuleVectorSyn(const char* lpszPinGroup, const char* lpszOrder, BOOL bWaitFinish);
	/**
	 * @brief Stop vector running
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @return Execute result
	*/
	int StopVector(const char* lpszPinGroup);
	/**
	 * @brief Get the stop line number
	 * @param[in] usSiteNo The site number
	 * @param[out] ulStopLineNo The stop line number
	 * @return Execute result
	*/
	int GetStopLineNo(USHORT usSiteNo, ULONG& ulStopLineNo);
	/**
	 * @brief Save the fail line number
	 * @param[in] ulMaxFailCountSaved The maximum fail line count will be saved
	 * @return Execute result
	*/
	int SaveFailMap(ULONG ulMaxFailCountSaved);
	/**
	 * @brief Get the MCU pin running status
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return Pin running status
	 * - 0 The pin is running
	 * - 1 The pin is stop
	 * - 2 The pin not ran before
	*/
	int GetMCUPinRunStatus(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Get the MCU result of pin
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return Execute result
	 * - 0 PASS in latest ran
	 * - 1 FAIL in latest ran
	*/
	int GetMCUPinResult(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Get the MCU running result
	 * @param[in] usSiteNo The site number
	 * @return The MCU running result
	 * - 0 MCU running Pass
	 * - 1 MCU running Fail
	*/
	int GetMCUResult(USHORT usSiteNo);
	/**
	 * @brief Get fail count of the pin
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] ulFailCount The fail count of the pin
	 * @return Execute result
	*/
	int GetFailCount(const char* lpszPinName, USHORT usSiteNo, ULONG& ulFailCount);
	/**
	 * @brief Get fail count of the pin
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] ulFirstFailLine The first fail line number
	 * @return Execute result
	*/
	int GetFirstFailLineNo(const char* lpszPinName, USHORT usSiteNo, ULONG& ulFirstFailLine);
	/**
	 * @brief Get the fail line number of the pin
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] uLineCount The line count of the variable pulLineNo
	 * @param[out] pulLineNo The array for saving fail line number
	 * @return Execute result
	*/
	int GetFailLineNo(const char* lpszPinName, USHORT usSiteNo, unsigned int uLineCount, ULONG* pulLineNo);
	/**
	 * @brief Get the hardware capture data
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[out] pbyCaptureData The array for saving capture data
	 * @param[in] uBuffSize The buff size of the pbyCapture data
	 * @return Execute result
	*/
	int GetHardwareCaptureData(const char* lpszPinName, USHORT usSiteNo, BYTE* pbyCaptureData, unsigned int uBuffSize);
	/**
	 * @brief Get the line ran count
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[out] ulLineCount The line count
	 * @return Execute result
	*/
	int GetRunLineCount(const char* lpszPinName, USHORT usSiteNo, ULONG& ulLineCount);
	/**
	 * @brief Set the TMU matrix
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] TMUUnit The TMU unit
	 * @return Execute result
	*/
	int SetTMUMatrix(const char* lpszPinGroup, DCM400_TMU_UNIT TMUUnit);
	/**
	 * @brief Set TMU parameter
	 * @param[in] TMUUnit The TMU unit
	 * @param[in] SlopType The trigger edge type
	 * @param[in] uHoldOffTime The hold off time, the unit is ns
	 * @param[in] uHoldOffNum The hold off number
	 * @return Execute result
	*/
	int SetTMUParam(const char* lpszPinGroup, DCM400_TMU_SLOPE_TYPE SlopeType, UINT uHoldOffTime, UINT uHoldOffNum, DCM400_TMU_UNIT TMUUnit);
	/**
	 * @brief Start TMU measurement
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] MeasureMode The Measure mode
	 * @param[in] uSampeCount The sample count
	 * @param[in] dTimeout The timeout of measurement, the unit is ms
	 * @return Execute result
	*/
	int TMUMeasure(const char* lpszPinGroup, DCM400_TMU_MEAS_MODE MeasureMode, UINT uSampeCount, double dTimeout);
	/**
	 * @brief Get the measurement result of TMU
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] MeasureType The measurement type
	 * @return The measurement value
	*/
	double GetTMUMeasureResult(const char* lpszPinName, USHORT usSiteNo, DCM400_TMU_MEAS_TYPE MeasureType);
	/**
	 * @brief Set the PPMU mode
	 * @param[in] lpszPinGroup The pin group
	 * @param[in] Mode PMU mode
	 * @param[in] dSetValue The set value
	 * @param[in] IRange The current range
	 * @return Execute result
	*/
	int SetPPMU(const char* lpszPinGroup, DCM400_PPMU_MODE Mode, double dSetValue, DCM400_PPMU_IRANGE IRange);
	/**
	 * @brief Set PMU measure
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] uSampleTimes The sample times
	 * @param[in] dSamplePeriod The sample period
	 * @return Execute result
	*/
	int PPMUMeasure(const char* lpszPinGroup, UINT uSampleTimes, double dSamplePeriod);
	/**
	 * @brief Get the measurement result of PMU
	 * @param[in] lpszPinName The pin number
	 * @param[in] usSiteNo The site number
	 * @param[in] nSamppleTimes The sample point, -1 is the average value
	 * @return The measure result
	*/
	int GetPPMUMeasureResult(const char* lpszPinGroup, USHORT usSiteNo, int nSampleNo);
	/**
	 * @brief Set I2C parameter
	 * @param[in] dPeriod The period of I2C
	 * @param[in] usSiteCount The site count of I2C
	 * @param[in] REGMode The register mode
	 * @param[in] lpszSCLChannel The each site's clock channel
	 * @param[in] lpszSDAChannel The each site's data channel
	 * @return Execute result
	*/
	int I2CSet(double dPeriod, USHORT usSiteCount, DCM400_I2C_REG_ADDR_MODE REGMode, const char* lpszSCLChannel, const char* lpszSDAChannel);
	/**
	 * @brief Set the pin level of I2C channel
	 * @param[in] dVIH The input voltage of logic high
	 * @param[in] dVIL The input voltage of logic low
	 * @param[in] dVOH The output voltage of logic high
	 * @param[in] dVOL The output voltage of logic low
	 * @param[in] I2CChannel The channel type whose pin level will be set
	 * @return Execute result
	*/
	int I2CSetPinLevel(double dVIH, double dVIL, double dVOH, double dVOL, DCM400_I2C_CHANNEL I2CChannel = DCM400_I2C_BOTH);
	/**
	 * @brief Set the dynamic load of I2C channel
	 * @param[in] bEnable Whether enable dynamic load
	 * @param[in] dCOM The compare voltage
	 * @param[in] dIOH The current when pin level higher than VCOM
	 * @param[in] dIOL The current when pin level lower than VCOM
	 * @param[in] I2CChannel The I2C channel type
	 * @return Execute result
	*/
	int I2CSetDynamicLoad(BOOL bEnable, double dCOM, double dIOH, double dIOL, DCM400_I2C_CHANNEL I2CChannel = DCM400_I2C_BOTH);
	/**
	 * @brief Connect I2C channel
	 * @return Execute result
	*/
	int I2CConnect();
	/**
	 * @brief Disconnect I2C channel
	 * @return Execute result
	*/
	int I2CDisconnect();
	/**
	 * @brief Set the stop status of I2C
	 * @param[in] bHighImpedance Whether the I2C stop with high impedance
	 * @return Execute result
	*/
	int I2CSetStopStatus(BOOL bHighImpedance);
	/**
	 * @brief Set the edge of the I2C clock
	 * @param[in] dT1R The raise edge of driver
	 * @param[in] dT1F The fall edge of driver
	 * @return Execute result
	*/
	int I2CSetSCLEdge(double dT1R, double dT1F);
	/**
	 * @brief Set the edge of the I2C data
	 * @param[in] dT1R The raise edge of driver
	 * @param[in] dT1F The fall edge of driver
	 * @param[in] dIOR The raise edge of IO
	 * @param[in] dIOF The fall edge of IO
	 * @param[in] dSTBR The fall edge of comparison
	 * @return Execute result
	*/
	int I2CSetSDAEdge(double dT1R, double dT1F, double dIOR, double dIOF, double dSTBR);
	/**
	 * @brief Write data through I2C
	 * @param[in] bySlaveAddr The slave address
	 * @param[in] ulREGAddr The register address
	 * @param[in] uDataLength The data length
	 * @param[in] ppbyDataWritten The data will be written
	 * @return Execute result
	*/
	int I2CWriteData(BYTE bySlaveAddr, ULONG ulREGAddr, UINT uDataLength, BYTE** ppbyDataWritten);
	/**
	 * @brief Write all same data of each site through I2C
	 * @param[in] bySlaveAddr The slave address
	 * @param[in] ulREGAddr The register address
	 * @param[in] uDataLength The data length
	 * @param[in] pbyDataWritten The data will be written
	 * @return Execute result
	*/
	int I2CWriteSameData(BYTE bySlaveAddr, ULONG ulREGAddr, UINT uDataLength, BYTE* pbyDataWritten);
	/**
	 * @brief Read data through I2C
	 * @param[in] bySlaveAddr The slave address
	 * @param[in] ulREGAddr The register address
	 * @param[in] uDataLength The data byte count will be read
	 * @return Execute result
	*/
	int I2CRead(BYTE bySlaveAddr, ULONG ulREGAddr, UINT uDataLength);
	/**
	 * @brief Get the data read in latest I2C read operation
	 * @note This funciton can only be used after I2C read operation
	 * @param[in] usSiteNo The site number
	 * @param[in] uDataByteIndex The data byte index
	 * @return The data gotten
	*/
	int I2CGetReadData(USHORT usSiteNo, UINT uDataByteIndex);
	/**
	 * @brief Get the bit data of latest I2C read
	 * @note This function can ony be used after I2C read operation
	 * @param[in] usSiteNo The site number
	 * @param[in] uStartBitPos The start bit position
	 * @param[in] uBitCount The bit count will be gotten
	 * @return The bit data
	*/
	ULONG I2CGetBitData(USHORT usSiteNo, UINT uStartBitPos, UINT uBitCount);
	/**
	 * @brief Get the NACK index of latest I2C operation
	 * @param[in] usSiteNo The site number
	 * @return The NACK index
	*/
	int I2CGetNACKIndex(USHORT usSiteNo);
	/**
	 * @brief Get the board logic revision of pin
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return The logic revision
	*/
	int GetBoardLogicRev(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Get the slot ID of the board
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return The slot ID
	 * - >=0 The slot ID of the board
	 * - -1 Not load vector before
	*/
	int GetBoardSlotID(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Set the alarm mask
	 * @param[in] lpszPinGroup The pin group or pin name
	 * @param[in] bMask The alarm mask
	 * @return Execute result
	*/
	int SetAlarmMask(const char* lpszPinGroup, BOOL bMask);
	/**
	 * @brief Get the alarm mask
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return The alarm mask
	*/
	int GetAlarmMask(const char* lspzPinName, USHORT usSiteNo);
	/**
	 * @brief Set the clamp alarm
	 * @param[in] lpszPinGroup The pin group
	 * @param[in] bMask Whether mask alarm
	 * @param[in] wFunction The function whose alarm will be masked
	 * @return Execute result
	*/
	int SetClampAlarmMask(const char* lpszPinGroup, BOOL bMask, WORD wFunction);
	/**
	 * @brief Get mask status of clamp alarm
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return Execute result
	 * - 0 Not mask
	 * - 1 Mask
	*/
	int GetClampAlarmMask(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Get the board serial number
	 * @param[in] lpszPinName The pin name defined in vector file
	 * @param[in] usSiteNo The site number
	 * @param[out] lpszBoardSN The variable which will save the board serial number
	 * @param[in] uSNSize The size of the variable
	 * @return Execute result
	 * - 0  Get the module information successfully
	 * - -1 Not load vector file
	 * - -9 The pin name is not defined or its point is NULL
	 * - -10 The site is over range or invalid
	 * - -16 The board or the channel is not existed
	 * - -27 The point of serial number variable is NULL
	 * - -32 The site is invalid
	*/
	int APIENTRY GetBoardSN(const char* lpszPinName, USHORT usSiteNo, char* lpszBoardSN, UINT uSNSize);
	/**
	 * @brief Get the board hardware revision
	 * @param[in] lpszPinName The pin name defined in vector file
	 * @param[in] usSiteNo The site number
	 * @param[out] lpszHardRev The variable which will save the board hardware revision
	 * @param[in] uRevSize The size of the variable
	 * @return Execute result
	 * - 0  Get the module information successfully
	 * - -1 Not load vector file
	 * - -9 The pin name is not defined or its point is NULL
	 * - -10 The site is over range or invalid
	 * - -16 The board or the channel is not existed
	 * - -27 The point of serial number variable is NULL
	 * - -32 The site is invalid
	*/
	int APIENTRY GetBoardHDRev(const char* lpszPinName, USHORT usSiteNo, char* lpszHardRev, UINT uRevSize);
	/**
	 * @brief Read the calibration date of specific channel by pin name and site number
	 * @param[in] lpszPinName The pin name defined in vector file
	 * @param[in] bySiteNo The site number
	 * @return The calibration date of this channel
	 * - >0 The calibration date of this channel
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is NULL
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	*/
	SYSTEMTIME APIENTRY GetCalibrationTime(const char* lpszPinName, USHORT usSiteNo) const;
	/**
	 * @brief Read the calibration temperature of specific channel by pin name and site No
	 * @param[in] lpszPinName The pin name defined in vector file
	 * @param[in] bySiteNo The site No
	 * @return The calibration temperature of this channel
	 * - >=0 The calibration temperature of this channel
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is NULL
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	*/
	double APIENTRY GetCalibrationTemperature(const char* lpszPinName, USHORT usSiteNo) const;
	/**
	 * @brief Read the calibration humidity of specific channel by pin name and site number
	 * @param[in] lpszPinName The pin name defined in vector file
	 * @param[in] bySiteNo The site No
	 * @return The calibration humidity of this channel
	 * - >0 The calibration humudity of this channel
	*/
	double APIENTRY GetCalibrationHumidity(const char* lpszPinName, USHORT usSiteNo) const;
	/**
	 * @brief Get the calibration result of channel
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return The calibration result
	 * - CAL_PASS Calibration PASS
	 * - CAL_FAIL Calibration FAIL
	 * - STS_NO_RESULT_RECORD Not record the calibration result
	*/
	//CAL_RESULT APIENTRY GetCalibrationResult(const char* lpszPinName, USHORT usSiteNo) const;
	/**
	 * @brief The logic revision of board when calibrate the board
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return Logic revision
	 * - >0 Logic revision
	*/
	int APIENTRY GetCalibrationLogicRev(const char* lpszPinName, USHORT usSiteNo) const;
	/**
	 * @brief Read the meter type of specific channel when calibration by pin name and site number
	 * @param[in] lpszPinName The pin name defined in vector file
	 * @param[in] bySiteNo The site number
	 * @return Meter type
	 * - STS_KEITHLEY2000 Keithley2000
	 * - STS_AGILENT34401 Agilent34401
	 * - STS_AGILENT3458A Agilent3458A
	 * - STS_KEYSIGHT34461A Keysight 34461A
	 * - STS_AGILENT34410 Agilent 34410
	 * - STS_NO_METER_RECORD Not record the calibration meter in calibration
	*/
	//CAL_METER APIENTRY GetCalibrationMeter(const char* lpszPinName, USHORT usSiteNo, char* lpszMeterSN = NULL, UINT uSNSize = 0);
	/**
	 * @brief Get the calibration board information when do calibration
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] lpszBoardSN The variable save the board serial number
	 * @param[in] uSNSize The size of lpszBoardSN
	 * @param[out] lpszHardRev The variable save the hardware revision
	 * @param[in] uRevSize The size of the lpszHardRev
	 * @return The logic revision of the calibration board used in calibration
	 * - >0 The logic revision of the calibration board
	 * - 255 The board logic revision or get the serial number or hardware revision fail
	*/
	int APIENTRY GetCalibrationCalBoardInfo(const char* lpszPinName, USHORT usSiteNo, char* lpszBoardSN, UINT uSNSize, char* lpszHardRev, UINT uRevSize);
	/**
	 * @brief Read the revision of software used in calibration
	 * @param[in] lpszPinName The pin name defined in vector file
	 * @param[in] bySiteNo The site number
	 * @param[out] lpszSoftRev The variable which will save the software revision
	 * @param[in] uRevSize The size of the variable of software revision
	 * @return Execute result
	*/
	int APIENTRY GetCalibrationSoftRev(const char* lpszPinName, USHORT usSiteNo, char* lpszSoftRev, UINT uRevSize);
	/**
	 * @brief Read the slot id the board inserted when it be calibrated by pin name and site number
	 * @param[in] lpszPinName The pin name defined in vector file
	 * @param[in] bySiteNo The site number
	 * @return The slot ID
	*/
	int APIENTRY GetCalibrationSlotID(const char* lpszPinName, USHORT usSiteNo) const;
private:
	DCM400Data* m_pData;///<The point pointed to the data for DCM400 class, using for extension
};