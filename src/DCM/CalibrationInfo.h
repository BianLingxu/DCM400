#ifndef __QVMECALIBRATIONINFO_H__
#define __QVMECALIBRATIONINFO_H__

#include <string>
#include "BaseDataOp.h"
#include "FlashInformation.h"
#include "HardwareFunction.h"
class STSDataStream;

class CCalibrationInfo : public BaseDataOp
{
public:
	/**
	 * @brief Constructor
	 * @param[in] HardwareFunction The hardware function class of current controller
	*/
	CCalibrationInfo(CHardwareFunction& HardwareFunction);
	/**
	 * @brief Destructor
	*/
	~CCalibrationInfo();
	/**
	 * @brief Write all channels' calibration information to flash
	 * @param[in] pCalInfo The calibration information of all channels
	 * @param[in] FlashRev The flash revision
	 * @return The channel whose data is read fail, one bit is one channel LSB
	*/
	int SetCalibrationInfo(STS_CALINFO* pCalInfo, STS_FLASHINFO_REV FlashRev = STS_FLASH_REV_1);
	/**
	 * @brief Get the calibration information of the channel
	 * @param[in] dwGetChannel The channel whose calibration information will be gotten, 0xFFFF is all channel
	 * @param[in] pCalInfo The point of the calibration information
	 * @return The channel whose data is read fail, one bit is one channel LSB
	*/
	int GetCalibrationInfo(DWORD dwGetChannel, STS_CALINFO* pCalInfo);
private:
	bool Write(STSDataStream & ds, int nChannelIndex);
	bool Read(STSDataStream & ds, int nChannelIndex);
	void Clear();
	/**
	 * @brief Write all channels' calibration information to flash
	 * @return The channel whose data is read fail, one bit is one channel LSB
	*/
	int WriteToFlash();
	/**
	 * @brief Read data from flash
	 * @param[in] dwGetChannel The channel number whose channel will be gotten, 0xFFFF is all channel
	 * @return The channel whose data is read fail, one bit is one channel LSB
	*/
	int ReadFromFlash(DWORD dwGetChannel);
	/**
	 * @brief Initialize the calibration information
	 * @param[out] CalibInfo The calibration information
	*/
	void InitCalibrationInfo(STS_CALINFO& CalibInfo);
private:
 	std::string			m_cMark;					// 标志
	BYTE				m_byCtrlNo;					//The Controller index.
	STS_CALINFO*		m_pstruCalInfo;			// 该通道校准相关信息
	STS_FLASHINFO_REV	m_flashRev;					// 数据在FLASH中的存储版本
	CHardwareFunction* m_pHardwareFunction;
};

#endif /*__QVMECALIBRATIONINFO_H__*/
