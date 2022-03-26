#pragma once
/**
 * @file Register.h
 * @brief Include the class of CRegister
 * @detail The class be used to read and write the data to register
 * @author Guangyun Wang
 * @date 2021/12/22
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "BaseOperation.h"
/**
 * @class CRegister
 * @brief The class for register operation
*/
class CRegister :
	public CBaseOperation
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CRegister(BYTE bySlotNo);
	/**
	 * @brief Set the controller index of the register
	 * @param byControllerIndex The controller index
	 * @return Execute result
	 * - 0 Set the controller index successfully
	 * - -1 The Controller index is over range
	*/
	int SetControllerIndex(BYTE byControllerIndex);
	/**
	 * @brief Set the type of the register
	 * @param[in] bShared Whether the register is shared by all controller
	 * @return Execute result
	 * - 0 Set the type successfully
	 * - -1 Not supported
	*/
	int SetRegisterType(BOOL bShared);
	/**
	 * @brief Read data from the address
	 * @param[in] usAddress The address will be read from
     * @param[in] byFPGAAddr The FPGA address
	 * @return The data read
	 * - The data read
	 * - -0xFFFFFFFF Not supported
	*/
    ULONG Read(USHORT usRegisterAddress, BYTE byFPGAAddr = 0);
	/**
	 * @brief Write data to the address
	 * @param[in] usAddress The address will be written to
	 * @param[in] ulData The data will be written
     * @param[in] byFPGAAddr The FPGA address
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 Not supported
	*/
    int Write(USHORT usRegisterAddress, ULONG ulData, BYTE byFPGAAddr = 0);

private:
	BYTE m_byModuleAddress;///<The module index
};

