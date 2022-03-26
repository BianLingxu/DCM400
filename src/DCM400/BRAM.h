#pragma once
/**
 * @file BRAM.h
 * @brief The class of BRAM
 * @detail The class be used to read and write the memory data in the FPGA of DCM board SE8212
 * @author Guangyun Wang
 * @date 2021/12/23
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "Register.h"

/**
 * @class CBRAM
 * @brief The operation of BRAM
*/
class CBRAM :
	public CBaseOperation
{
public:
	/**
	 * @enum MEM_TYPE
	 * @brief The memory type
	*/
	enum MEM_TYPE
	{
		PAT_LIST = 1,
		DDR_INFO = 2,
		PRIME_RAM = 4,
		GLO_T0_SET = 8,
		T0_RAM = 16,
		BRAM5 = 32,
		TG_DDR_INFO = 64,
		MEM_TIMMING = 128,
		MEM_DATA_SRC_SEL = 256,
		MEM_RSU = 512,
		GLO_TIMINGSET = 1024,
		MEM_ADC = 2048,
	};

	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CBRAM(BYTE bySlotNo);
	/**
	 * @brief Set the controller index of the register
	 * @param byControllerIndex The controller index
	 * @return Execute result
	 * - 0 Set the controller index successfully
	 * - -1 The Controller index is over range
	*/
	int SetControllerIndex(BYTE byControllerIndex);
	/**
	 * @brief Set the memory type and the address
	 * @param[in] nMemType The memory type
	 * @param[in] usLow16Data The low 16 bits data
	 * @return Execute result
	 * - 0 Set the address successfully
	 * - -1 The memory type is not supported
	*/
	int SetAddress(int nMemType, USHORT usLow16Data);
	/**
	 * @brief Write data to BRAM
	 * @param[in] uWriteDataCount The data count be written
	 * @param[in] pulData The memory address
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The data count is 0 or the memory address is nullptr
	*/
	int Write(UINT uWriteDataCount, const ULONG* pulData);
	/**
	 * @brief Read data from BRAM
	 * @param[in] uReadDataCount The data count read
	 * @param[out] pulData The memory address for saving data
	 * @param[in] bMainMem Whether read the main memory
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 The data count is 0 or the memory address is nullptr
	*/
	int Read(UINT uReadDataCount, ULONG* pulData, BOOL bMainMem = TRUE);

private:
	ULONG m_ulStartAddress;
	BYTE m_byFPGA;
	BYTE m_byControllerIndex;
	BYTE m_byAdditionFPGA;///<The FPGA of addition memory
};
