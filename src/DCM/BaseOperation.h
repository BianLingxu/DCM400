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
#include <vector>
#include <map>
#include "HardwareInfo.h"
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
	 * @brief Set the index of the controller in board
	 * @param[in] byControllerIndex The controller index
	 * @return Execute result
	 * - 0 Set successfully
	 * - -1 The controller index is over range
	*/
	virtual int SetControllerIndex(BYTE byControllerIndex) = 0;
	/**
	 * @brief Get the status
	 * @return The status of the board or controller
	 * - The status of the board or controller
	 * - 0xFFFFFFFF Not supported
	*/
	virtual ULONG GetStatus() = 0;
	/**
	 * @brief Set the type of the register
	 * @param[in] usType The type of register
	 * @return Execute result
	 * - 0 Set the type successfully
	 * - -1 Not supported
	*/
	virtual int SetRegisterType(USHORT usType) = 0;
	/**
	 * @brief Set the address
	 * @param[in] nType The type of target operated
	 * @param[in] ulAddress The address
	 * @return Execute result
	 * - 0 Set the address successfully
	 * - -1 Not supported
	*/
	virtual int SetAddress(int nType, USHORT usAddress) = 0;
	/**
	 * @brief Set the start line number of the operation
	 * @param[in] nType The type of the memory
	 * @param[in] ulStartLineNo The start line number
	 * @return Execute result
	 * - 0 Set the address successfully
	 * - -1 Not supported
	 * - -2 The type is not right
	 * - -3 The start line number is over range
	*/
	virtual int SetStartLineNo(int nType, ULONG ulStartLineNo) = 0;
	/**
	 * @brief Read data from the address
	 * @param[in] usAddress The address will be read from
	 * @return The data read
	 * - The data read
	 * - -0xFFFFFFFF Not supported
	*/
	virtual ULONG Read(USHORT usAddress) = 0;
	/**
	 * @brief Write data to the address
	 * @param[in] usAddress The address will be written to
	 * @param[in] ulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 Not supported
	*/
	virtual int Write(USHORT usAddress, ULONG ulData) = 0;
	/**
	 * @brief Read data from the DCM
	 * @param[in] uReadDataCount The bytes count will be read
	 * @param[out] pulDataBuff The data buff which will save the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 Not supported
	 * - -2 The data count is 0 or the data parameter is nullptr
	 * -  -3 The start line number is over range
	*/
	virtual int Read(UINT uReadDataCount, ULONG* pulDataBuff) = 0;
	/**
	 * @brief Write data to the DCM
	 * @param[in] uWriteDataCount The bytes count will be written
	 * @param[in] pulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 Not supported
	 * - -2 The data count is 0 or the data parameter is nullptr
	 * - -3 The start line number is over range
	*/
	virtual int Write(UINT uWriteDataCount, const ULONG* pulData) = 0;
	/**
	 * @brief Read data from the DCM
	 * @param[in] uAddress The address which the data will be read from
	 * @param[in] uReadDataCount The bytes count will be read
	 * @param[out] pulDataBuff The data buff which will save the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 Not supported
	 * - -2 The data count is 0 or the data parameter is nullptr
	*/
	virtual int Read(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff) = 0;
	/**
	 * @brief Write data to the DCM
	 * @param[in] uAddress The address which the data will be written to
	 * @param[in] uWriteDataCount The bytes count will be written
	 * @param[in] pulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 Not supported
	 * - -2 The data count is 0 or the data parameter is nullptr
	*/
	virtual int Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData) = 0;
	/**
	 * @brief Read data from the DCM
	 * @param[in] byAddress The address which the data will be read from
	 * @param[out] mapChannelData The channel will be read and data will be saved to
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 Not supported
	 * - -2 No channel will be read data from
	*/
	virtual int Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData) = 0;
	/**
	 * @brief Write data to the DCM
	 * @param[in] byAddress The address which the data will be written to
	 * @param[in] mapChannelData The channel will be written and its data
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 Not supported
	 * - -2 No channel need to be written data to
	*/
	virtual int Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData) = 0;
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
	 * @brief Set full address will be operated
	 * @param[in] byFunctionAddr The function module address
	 * @param[in] uREGAddr The register address
	*/
	void SetFunctionRegisterAddress(BYTE byFunctionAddr, USHORT usREGAddr);
	/**
	 * @brief Read data form DCM
	 * @return The data read
	*/
	ULONG BaseRead();
	/**
	 * @brief Write data to the DCM
	 * @param[in] byAddress The address which the data will be written to
	 * @param[in] ulData The data will be written
	*/
	void BaseWrite(ULONG ulData);
private:
	BYTE m_bySlotNo;///<The slot number the board inserted
	BYTE m_byModuleAddress;///<The module address
	ULONG m_ulAddress;///<The full address which will be operated
};

