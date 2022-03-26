#pragma once
/**
 * @file BoardDelay.h
 * @brief Include the class of the CBoardDelay
 * @detail This class is using for read delay information from flash or write to flash
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/

#include "HardwareFunction.h"
class STSDataStream;
/**
 * @class CBoardDelay
 * @brief The board delay information class
*/
class CBoardDelay
{
public:
	/**
	 * @struct IO_DELAY
	 * @brief The delay of each channel
	*/
	struct IO_DELAY
	{
		double m_dData;///<The data delay
		double m_dDataEn;///<The data enable delay
		double m_dHigh;///<The compare high delay
		double m_dLow;///<The compare low delay
		IO_DELAY()
		{
			m_dData = 0;
			m_dDataEn = 0;
			m_dHigh = 0;
			m_dLow = 0;
		}
	};
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number of the board
	*/
	CBoardDelay(BYTE bySlotNo);
	/**
	 * @brief Save the delay information 
	 * @return Execute result
	 * - 0 Save delay successfully
	 * - -1 The flash is error
	 * - -2 No controller is not existed
	*/
	int SaveDelay();
	/**
	 * @brief Read the delay information of specific controller
	 * @param[in] byControllerIndex The controller index of board
	 * @return Execute result
	 * - 0 Get the delay of controller successfully
	 * - -1 The controller is not existed
	 * - -2 The head in flash is error
	 * - -3 The data in flash is checked fail
	*/
	int ReadDelay(BYTE byControllerIndex);
	/**
	 * @brief Get the delay of controller
	 * @param[in] pdTimesetDelay The timeset delay
	 * @param[in] pdTotalStartDelay The total start delay
	 * @return Execute result
	 * - 0 Get the controller delay successfully
	 * - -1 The point of delay is nullptr
	*/
	int GetControllerDelay(double* pdTimesetDelay, double* pdTotalStartDelay);
	/**
	 * @brief Get the IO delay of specific channel
	 * @param[in] usChannel The channel number
	 * @param[out] pIODelay The IO delay
	 * @return Execute result
	 * - 0 Get the IO delay successfully
	 * - -1 The channel is over range
	 * - -2 The point of the delay is nullptr
	 * - -3 Not read delay before
	*/
	int GetIODelay(USHORT usChannel, IO_DELAY* pIODelay);
private:
	/**
	 * @brief Save the delay information of specific controller
	 * @param[in] byControllerIndex The board index of board
	 * @return Execute result
	 * - 0 Save delay information successfully
	 * - -1 The controller is not existed
	 * - -2 Write flash fail
	*/
	int SaveDelay(BYTE byControllerIndex);
	/**
	 * @brief Write the delay information of specific controller to stream
	 * @param[in] DataStream The data stream
	 * @return Execute result
	 * - 0 Write delay successfully
	 * - -1 The controller is not existed
	*/
	int WriteDelay(STSDataStream& DataStream);
	/**
	 * @brief Save the flash head to data stream
	 * @param[in] DataStream The data stream
	*/
	void SaveHead(STSDataStream& DataStream);
	/**
	 * @brief Check the head information in flash
	 * @param[in] DataStream The data stream
	 * @return Check result
	 * - true Check head successfully
	 * - false Check head fail
	*/
	bool HeadCheck(STSDataStream& DataStream);
	/**
	 * @brief Get the delay information from data stream
	 * @param[in] DataStream The data stream
	*/
	void GetDelay(STSDataStream& DataStream);
private:
	CHardwareFunction m_Hardware;///<The hardware function class
	std::string m_strMark;///<The flash mark
	unsigned int m_uDataSize;///<The size of all flash data
	unsigned char m_ucCheckCode[16];///<The MD5 check value
	double m_dTimesetDelay;///<The timeset delay
	double m_dTotalStartDelay;///<The total start delay
	std::vector<IO_DELAY> m_vecIODelay;///<The IO delay of each channel
};

