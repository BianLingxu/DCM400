#pragma once
/**
 * @file BaseOperation.h
 * @brief Include the class of CBaseOperation
 * @detail The base class of the DCM register and memory operation.
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <windows.h>
/**
 * @class CBaseOperation
 * @brief The pure virtual class of base operation class
*/
class CBaseOperation
{
public:
	/**
	 * @brief The constructor
	 * @param[in] bySlotNo The slot number of the board
	 * @return No return
	*/
	CBaseOperation(BYTE bySlotNo);
	/**
	 * @brief Wait us
	 * @param[in] dWaitUs The time wait 
	*/
	void WaitUs(double dWaitUs);

protected:
	/**
	 * @brief Set the module address or index
	 * @param[in] byModuleAddress The module address
	*/
	void SetModuleAddress(BYTE byModuleAddress);
	/**
	 * @brief Set the detail address
	 * @param[in] byGPGAAddr The FPGA address
	 * @param[in] usDetailAddress The detail address
	*/
	void SetDetailAddress(BYTE byFPGAAddr, USHORT usDetailAddress);
	/**
	 * @brief Read data form DCM
	 * @return The data read
	*/
	ULONG BaseRead();
	/**
	 * @brief Write data to the DCM
	 * @param[in] ulData The data will be written
	*/
	void BaseWrite(ULONG ulData);

protected:
	BYTE m_bySlotNo;///<The slot number the board inserted
	BYTE m_byModuleAddress;///<The module address
	ULONG m_ulAddress;///<The full address which will be operated
};

