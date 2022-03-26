#pragma once
/**
 * @file Register.h
 * @brief Include the class of CRegister
 * @detail The class be used to read and write the register data in the FPGA of DCM board SE8212
 * @author Guangyun Wang
 * @date 2020/02/20
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "FPGAA.h"
/**
 * @class CRegister
 * @brief The class for register operation
*/
class CRegister :
	public CFPGAA
{
public:
	/**
	 * @enum REG_TYPE
	 * @brief The register type
	*/
	enum REG_TYPE
	{
		SYS_REG = 0,///<The system register
		COM_REG,///<The COM register
		FUNC_REG,///<The function register
		TMU_REG,///<The TMU register
		CAL_REG,///<The calibration register
	};
	CRegister(BYTE bySlotNo);
	int SetRegisterType(USHORT usType);
	ULONG Read(USHORT usRegisterAddress);
	int Write(USHORT usRegisterAddress, ULONG ulData);
	   	 

	ULONG GetStatus();
	int SetAddress(int nType, USHORT usAddress);
	int SetStartLineNo(int nType, ULONG ulStartLineNo);
	int Read(UINT uReadDataCount, ULONG* pulDataBuff);
	int Write(UINT uWriteDataCount, const ULONG* pulData);
	int Read(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff);
	int Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData);
	int Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	int Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
};

