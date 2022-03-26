#pragma once
/**
 * @file Operation.h
 * @brief The class be used to operate memory and register of board
 * @author Guangyun Wang
 * @date 2021/12/23
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "Register.h"
#include "BRAM.h"
#include "DRAM.h"
#include <map>
/**
 * @class COperation
 * @brief The operation of DCM400
*/
class COperation
{
public:
	/**
	 * @enum BRAM_TYPE
	 * @brief BRAM type
	 */
	enum class BRAM_TYPE
	{
		PAT_LIST = 0x01,///<The pattern list 
		DDR_INFO = 0x02,///<The DDR information
		PRIME_RAM = 0x04,///<
		GLO_T0_SET = 0x08,///<
		T0_RAM = 0x10,
		BRAM5 = 0x20,
		TG_DDR_INFO = 0x40,
		MEM_TIMMING = 0x80,
		MEM_DATA_SRC_SEL = 0x0100,
		MEM_RSU = 0x0200,
		GLO_TIMINGSET = 0x0400,
		MEM_ADC = 0x0800,
	};
	/**
	 * @enum DRAM_TYPE
	 * @brief DRAM data type
	 */
	enum DRAM_TYPE
	{
		CMD_DRAM0 = 0,
		CMD_DRAM1,
		PAT_DRAM0,
		PAT_DRAM1,
	};

	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number the board inserted
	 */
	COperation(BYTE bySlotNo);
	/**
	 * @brief Destructor
	 */
	~COperation();
	/**
	 * @brief Get the slot number of board
	 * @return The slot number
	 */
	BYTE GetSlotNo() const;
	/**
	 * @brief Set the index of the controller in board
	 * @param[in] byControllerIndex The controller index
	 * @return Execute result
	 * - 0 Set successfully
	 * - -1 The controller index is over range
	 */
	int SetControllerIndex(BYTE byControllerIndex);
	/**
	 * @brief Get the controller index of the operation
	 * @return The controller index
	 */
	BYTE GetControllerIndex() const;
	/*
	 *Read data from register
	 * @param[in] usREGAddress The register address
	 * @param[in] bShared Where the register is shared by all controller
     * @param[in] byFPGAAddr The FPGA address
	 * @return The data read
	 */
    ULONG ReadRegister(USHORT usREGAddress, BOOL bShared = FALSE, BYTE byFPGAAddr = 0);
	/**
	 * @brief Write data to register
	 * @param[in] bShared Where the register is shared by all controller
	 * @param[in] usREGAddress The register address
	 * @param[in] ulData The data will be written
     * @param[in] byFPGAAddr The FPGA address
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The register type is error
	 */
    int WriteRegister(USHORT usREGAddress, ULONG ulData, BOOL bShared = FALSE, BYTE byFPGAAddr = 0);
	/**
	 * @brief Read data from the memory in FPGA
	 * @param[in] BRAMType The memory type
	 * @param[in] uBRAMAddress The base memory address will be read
	 * @param[in] nReadDataCount The data count will be read
	 * @param[out] pulDataBuff The data buff which will save the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 The memory type is error
	 * - -2 The data count is 0 or the data parameter is nullptr
	 */
	int ReadBRAM(BRAM_TYPE BRAMType, USHORT usBRAMAddress, UINT uReadDataCount, ULONG* pulDataBuff, BOOL bMainMem = TRUE);
	/**
	 * @brief Write data to the memory in FPGA
	 * @param[in] BRAMType The memory type
	 * @param[in] uAddress The address which the data will be written to
	 * @param[in] nWriteDataCount The bytes count will be written
	 * @param[in] pulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The memory type is error
	 * - -2 The data count is 0 or the data parameter is nullptr
	 */
	int WriteBRAM(BRAM_TYPE BRAMType, USHORT usRAMAddress, UINT uWriteDataCount, const ULONG* pulData);
	/**
	 * @brief Read data from the DRAM
	 * @param[in] DRAMType The memory type
	 * @param[in] uStartByte The start bits of the DRAM, must be integral multiple of 64
	 * @param[in] uReadDataCount The data line count will be read
	 * @param[out] pulDataBuff The data buff which will save the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 The memory type is error
	 * - -2 The data count is 0 or the data parameter is nullptr
	 * - -3 The start bits is not right
	 * - -4 Read DRAM timeout
	 */
	int ReadDRAM(DRAM_TYPE DRAMType, UINT uStartByte, UINT uReadDataCount, ULONG* pulDataBuff);
	/**
	 * @brief Write data to the DRAM
	 * @param[in] DRAMType The memory type
	 * @param[in] uStartByte The start bits of the DRAM, must be integral multiple of 64
	 * @param[in] nWriteDataCount The data line count will be written
	 * @param[in] pulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The memory type is error
	 * - -2 The data count is 0 or the data parameter is nullptr
	 * - -3 The start bits number is not right
	 */
	int WriteDRAM(DRAM_TYPE DRAMType, UINT uStartByte, UINT uWriteDataCount, const ULONG* pulData);
	/**
	 * @brief Get the status of ADATE318
	 * @return The data in status register of ADATE318
	 * - The data in status register of ADATE318
	 */
	int Get318Status();	
	/**
	 * @brief Read data from ADATE318
	 * @param[in] byAddress The SPI address
	 * @param[I/O] mapChannelData The channel will be read and the data read
	 * @return The data in the specific address
	 */
	int Read318(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	/**
	 * @brief Write data to ADATE318
	 * @param[in] byAddress The SPI address
	 * @param[in] mapChannelData The channel will be write and the data written
	 * @return Execute result
	 * - 0 Write data successfully
	 */
	int Write318(BYTE byAddress, const std::map<USHORT, ULONG>& mapChannelData);
	/**
	 * @brief Start PMU measure
	 * @param[in] nSampleDepth The sample depth
	 * @return Execute result
	 * - 0 Start PMU successfully
	*/
	int PMUStart(int nSampleDepth);
	/**
	 * @brief Write data to 7606
	 * @param[in] ulData The data written
	 * @return Execute result
	 * - 0 Write data successfully
	*/
	int Write7606(ULONG ulData);
	/**
	 * @brief Read AD7606 parameter
	 * @param[in] usChannel AD7606 channel
	 * @param[in] nSampelDepth Sample depth
	 * @param[out] pulBuff Data buff
	 * @return Execute result
	 * - 0 Read AD7606 success
	 * - -1: Ret AD7606 fail
	 */
	int Read7606(USHORT usChannel, int nSampleDepth, ULONG* pulBuff);
	/**
	 * @brief Read data from SM8213
	 * @param[in] usREGAddress The address will be read
	 * @param[in] uReadDataCount The data line count will be read
	 * @param[out] pulDataBuff The data buff which will save the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 The data count is 0 or the data parameter is nullptr
	 */
	int ReadBoard(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff);
	/**
	 * @brief Write data to SM8213
	 * @param[in] usREGAddress The address will be written
	 * @param[in] nWriteDataCount The data line count will be written
	 * @param[in] pulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The data count is 0 or the data parameter is nullptr
	 */
	int WriteBoard(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData);
	/**
	 * @brief Wait time 
	 * @param[in] dUs The time to wait in us
	*/
	void WaitUs(double dUs);

private:
	BYTE m_bySlotNo;///<The slot number the board inserted
	BYTE m_byControllerIndex;///<The controller index
	CRegister m_Register;///<The register operation
	CDRAM m_DRAM;///<The DRAM operation
	CBRAM m_BRAM;///<The BRAM operation
};

