#pragma warning (disable:4786)

#include "CalibrationInfo.h"
#include "STSCoreFx.h"
#include "FlashHead.h"
#include "FlashInfo.h"

CCalibrationInfo::CCalibrationInfo(CHardwareFunction& HardwareFunction)
	:m_cMark("DCM100_Calibration_Information")
{
	m_pHardwareFunction = &HardwareFunction;
	if (nullptr != m_pHardwareFunction)
	{
		m_byCtrlNo = HardwareFunction.GetControllerIndex();
	}
}


CCalibrationInfo::~CCalibrationInfo()
{
}

bool CCalibrationInfo::Write(STSDataStream & ds, int nChannelIndex)
{
	bool bRet = false;
	do
	{
		ds << (int)m_pstruCalInfo[nChannelIndex].calSystem;					// 校准系统

		ds << (unsigned long)m_pstruCalInfo[nChannelIndex].calDate;			// 校准日期

		ds << m_pstruCalInfo[nChannelIndex].temperature;						// 校准温度

		ds << m_pstruCalInfo[nChannelIndex].humidity;							// 校准湿度

		ds << m_pstruCalInfo[nChannelIndex].calResult;						// 校准结果

		ds << m_pstruCalInfo[nChannelIndex].logicRev;							// 逻辑版本

		ds << m_pstruCalInfo[nChannelIndex].meterType;						// 校准表类型

		int nLength = m_pstruCalInfo[nChannelIndex].meterSnSize;// 校准表序列号的长度
		if (0 > nLength)
		{
			nLength = 0;
		}
		if (STS_MAX_CHAR_SIZE > nLength)
		{
			m_pstruCalInfo[nChannelIndex].meterSn[nLength] = '\0';
			++nLength;
		}
		else
		{
			nLength = STS_MAX_CHAR_SIZE;
			m_pstruCalInfo[nChannelIndex].meterSn[nLength -1] = '\0';
		}
		ds << nLength;

		int i = 0;

		for (i = 0; i < nLength; i++)
		{
			ds << m_pstruCalInfo[nChannelIndex].meterSn[i];					// 校准表序列号
		}

		nLength = m_pstruCalInfo[nChannelIndex].calBdSnSize;// 校准板序列号的长度
		if (0 > nLength)
		{
			nLength = 0;
		}
		if (STS_MAX_CHAR_SIZE > nLength)
		{
			m_pstruCalInfo[nChannelIndex].calBdSn[nLength] = '\0';
			++nLength;
		}
		else
		{
			nLength = STS_MAX_CHAR_SIZE;
			m_pstruCalInfo[nChannelIndex].calBdSn[nLength - 1] = '\0';
		}
		ds << nLength;						
		
		for (i = 0; i < nLength; i++)
		{
			ds << m_pstruCalInfo[nChannelIndex].calBdSn[i];					// 校准板序列号
		}
		

		nLength = m_pstruCalInfo[nChannelIndex].calBdHardRevSize;// 校准板硬件版本号的长度
		if (0 > nLength)
		{
			nLength = 0;
		}
		if (STS_MAX_CHAR_SIZE > nLength)
		{
			m_pstruCalInfo[nChannelIndex].calBdHardRev[nLength] = '\0';
			++nLength;
		}
		else
		{
			nLength = STS_MAX_CHAR_SIZE;
			m_pstruCalInfo[nChannelIndex].calBdHardRev[nLength - 1] = '\0';
		}
		ds << nLength;					

		
		for (i = 0; i < nLength; i++)
		{
			ds << m_pstruCalInfo[nChannelIndex].calBdHardRev[i];				// 校准板硬件版本号
		}
		ds << m_pstruCalInfo[nChannelIndex].calBdLogicRev;					// 校准板逻辑版本号

		nLength = m_pstruCalInfo[nChannelIndex].softwareRevSize;// 校准使用的软件版本信息的长度
		if (0 > nLength)
		{
			nLength = 0;
		}
		if (STS_MAX_CHAR_SIZE > nLength)
		{
			m_pstruCalInfo[nChannelIndex].softwareRev[nLength] = '\0';
			++nLength;
		}
		else
		{
			nLength = STS_MAX_CHAR_SIZE;
			m_pstruCalInfo[nChannelIndex].softwareRev[nLength - 1] = '\0';
		}

		ds << nLength;					

		for (i = 0; i < nLength; i++)
		{
			ds << m_pstruCalInfo[nChannelIndex].softwareRev[i];				// 校准使用的软件版本信息
		}

		ds << m_pstruCalInfo[nChannelIndex].calSlotID;						// 校准时板卡所处的槽位号
		bRet = true;
	} while (false);
	return bRet;
}
bool CCalibrationInfo::Read(STSDataStream & ds, int nChannelIndex)
{
	bool bRet = false;
	do
	{
		int tempInt = 0;
		unsigned long tempUnLong = 0;
		double tempDouble = 0.0;
		char tempChar = 0;
		
		ds >> tempInt;									// 校准系统
		m_pstruCalInfo[nChannelIndex].calSystem = STS_SYSTEM_TYPE(tempInt);

		ds >> tempUnLong;								// 校准日期
		m_pstruCalInfo[nChannelIndex].calDate = (time_t)tempUnLong;

		ds >> tempDouble;								// 校准温度
		m_pstruCalInfo[nChannelIndex].temperature = tempDouble;

		ds >> tempDouble;								// 校准湿度
		m_pstruCalInfo[nChannelIndex].humidity = tempDouble;

		ds >> m_pstruCalInfo[nChannelIndex].calResult;				// 校准结果

		ds >> m_pstruCalInfo[nChannelIndex].logicRev;					// 逻辑版本

		ds >> m_pstruCalInfo[nChannelIndex].meterType;				// 校准表类型

		ds >> m_pstruCalInfo[nChannelIndex].meterSnSize;				// 校准表序列号的长度

		int i = 0;
		for (i = 0; i < m_pstruCalInfo[nChannelIndex].meterSnSize; i++)
		{
			ds >> tempChar;								// 校准表序列号
			m_pstruCalInfo[nChannelIndex].meterSn[i] = tempChar;
		}

		ds >> m_pstruCalInfo[nChannelIndex].calBdSnSize;				// 校准板序列号的长度

		for (i = 0; i < m_pstruCalInfo[nChannelIndex].calBdSnSize; i++)
		{
			ds >> tempChar;								// 校准板序列号
			m_pstruCalInfo[nChannelIndex].calBdSn[i] = tempChar;
		}

		ds >> m_pstruCalInfo[nChannelIndex].calBdHardRevSize;			// 校准板硬件版本号的长度

		for (i = 0; i < m_pstruCalInfo[nChannelIndex].calBdHardRevSize; i++)
		{
			ds >> tempChar;								// 校准板硬件版本号
			m_pstruCalInfo[nChannelIndex].calBdHardRev[i] = tempChar;
		}

		ds >> m_pstruCalInfo[nChannelIndex].calBdLogicRev;			// 校准板逻辑版本号

		ds >> m_pstruCalInfo[nChannelIndex].softwareRevSize;			// 校准使用的软件版本信息的长度

		for (i = 0; i < m_pstruCalInfo[nChannelIndex].softwareRevSize; i++)
		{
			ds >> tempChar;								// 校准表序列号
			m_pstruCalInfo[nChannelIndex].softwareRev[i] = tempChar;
		}

		ds >> m_pstruCalInfo[nChannelIndex].calSlotID;				// 校准时该板卡所处的槽位号
		bRet = true;
	} while (false);
	return bRet;
}

void CCalibrationInfo::Clear()
{
}

int CCalibrationInfo::WriteToFlash()
{
	int nFailChannel = 0;
	BYTE* byDataWritten = STS_NULL;
	DWORD dwDataLength = 0;
	DWORD dwDataLeft = 0;
	BYTE byCurPageNo = 0;
	int nStartIndex = 0;
	short sCurWriteByte = 0;
	BYTE bySectorNo = m_byCtrlNo + CAL_INFO_SECTOR_START;
	//Erase the sector which save current Controller's calibration information.
	m_pHardwareFunction->EraseFlash(bySectorNo);
	
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		STSBuffer buf;
		// 写入数据的头，主要目的是占位
		FlashHead dataHead;
		if (!buf.Open(STSIODevice::WriteOnly))
		{
			nFailChannel |= 1 << usChannel;
			continue;
		}

		STSDataStream ds(&buf);

		dataHead.SetMark(m_cMark.c_str(), m_cMark.size() + 1);					// 设置标志
		dataHead.m_nHeadUnitCnt = 1;											// SN号的存储个数
		dataHead.m_nHeadRev = m_flashRev;										// FLASH版本
		dataHead.Save(ds);

		int nDataBufPos = buf.Pos();
		// 写入数据
		Write(ds, usChannel);

		dataHead.m_nHeadDataSize = buf.Size();
		// 写入MD5校验码
		STSMD5Context context;
		STSMD5_Init(&context);
		STSMD5_Update(&context, (unsigned char *)(buf.Data() + nDataBufPos), buf.Size() - nDataBufPos);
		STSMD5_Final(&context, (unsigned char *)(dataHead.m_cHeadCheckCode));

		buf.Seek(0);		// 回到初始位置
		dataHead.Save(ds);
		byDataWritten = (BYTE*)buf.Data();
		dwDataLength = buf.Size();

		nStartIndex = 0;
		dwDataLeft = dwDataLength;
		byCurPageNo = usChannel * CAL_INFO_PAGE_PER_CH;
		do
		{
			sCurWriteByte = dwDataLeft > 256 ? 256 : (short)dwDataLeft;

			if (0 != m_pHardwareFunction->WriteFlash(bySectorNo, byCurPageNo, 0, sCurWriteByte, &byDataWritten[nStartIndex]))			// 写FLASH失败，返回-1，冗余1次
			{
				nFailChannel |= 1 << usChannel;
				break;
			}

			++byCurPageNo;
			if (byCurPageNo - usChannel * CAL_INFO_PAGE_PER_CH == CAL_INFO_PAGE_PER_CH)
			{
				//The calibration information can't be all saved in flash.

				nFailChannel |= 1 << usChannel;
				break;
			}
			dwDataLeft -= sCurWriteByte;
			nStartIndex += sCurWriteByte;
		} while (0 < dwDataLeft);
	}
	return nFailChannel;
}

int CCalibrationInfo::ReadFromFlash(DWORD dwGetChannel)
{
	char *pBuf = STS_NULL;
	DWORD dwDataLength = 0;
	DWORD dwDataLeft = 0;
	BYTE byCurPageNo = 0;
	int nStartIndex = 0;
	short sCurWriteByte = 0;
	BYTE bySectorNo = m_byCtrlNo + CAL_INFO_SECTOR_START;
	BYTE byOffset = 0;
	BYTE byStartChannel = 0;
	BYTE byStopChannel = 0;
	int nSaveIndex = 0;
	if (0xFFFF == dwGetChannel)
	{
		//Get the calibration information of all channels in this Controller
		byStopChannel = DCM_CHANNELS_PER_CONTROL - 1;
	}
	else
	{
		//Get the channel index whose calibration information will be read.
		byStartChannel = (BYTE)dwGetChannel;
		byStopChannel = (BYTE)dwGetChannel;
	}
	int nFailChannel = 0;
	for (int nChannelIndex = byStartChannel; nChannelIndex <= byStopChannel; ++nChannelIndex)
	{
		InitCalibrationInfo(m_pstruCalInfo[nChannelIndex]);

		FlashHead DataHead;
		DataHead.SetMark(m_cMark.c_str(), m_cMark.size() + 1);
		int nDataBufPos = 0;

		STSBuffer bufTemp;
		if (!bufTemp.Open(STSIODevice::ReadWrite))
		{
			nFailChannel |= 1 << nChannelIndex;
			continue;
		}

		STSDataStream ds(&bufTemp);
		DataHead.Save(ds);
		nDataBufPos = bufTemp.Pos();
		{
			STSBuffer StsBuff;
			bufTemp.Swap(StsBuff);					// 清空bufTemp
		}
		bufTemp.Open(STSIODevice::ReadWrite);

		bool sameFlag = true;
		pBuf = new char[nDataBufPos + 1];

		nStartIndex = 0;
		dwDataLeft = nDataBufPos;
		byCurPageNo = nChannelIndex * CAL_INFO_PAGE_PER_CH;
		do
		{
			sCurWriteByte = dwDataLeft > 256 ? 256 : (short)dwDataLeft;
			//Read the calibration data from flash.
			m_pHardwareFunction->ReadFlash(bySectorNo, byCurPageNo, 0, sCurWriteByte, (BYTE*)&pBuf[nStartIndex]);
			++byCurPageNo;
			dwDataLeft -= sCurWriteByte;
			nStartIndex += sCurWriteByte;
		} while (0 < dwDataLeft);

		bufTemp.SetData(pBuf, nDataBufPos + 1);
		delete[] pBuf;

		if ((!sameFlag) || (!DataHead.Check(ds)))					// 检查不同小板存储的信息是否相同，校验判断数据头的Mark
		{
			nFailChannel |= 1 << nChannelIndex;
			continue;
		}
		m_flashRev = (STS_FLASHINFO_REV)DataHead.m_nHeadRev;

		STSBuffer buf;
		if (!buf.Open(STSIODevice::ReadOnly))
		{
			nFailChannel |= 1 << nChannelIndex;
			continue;
		}

		char *pData = new char[DataHead.m_nHeadDataSize + 1];

		nStartIndex = 0;
		dwDataLeft = DataHead.m_nHeadDataSize;
		byCurPageNo = nChannelIndex * CAL_INFO_PAGE_PER_CH;

		do
		{
			sCurWriteByte = dwDataLeft > 256 ? 256 : (short)dwDataLeft;
			//Read the calibration data from flash.
			m_pHardwareFunction->ReadFlash(bySectorNo, byCurPageNo, 0, sCurWriteByte, (BYTE*)&pData[nStartIndex]);
			++byCurPageNo;
			dwDataLeft -= sCurWriteByte;
			nStartIndex += sCurWriteByte;
		} while (0 < dwDataLeft);
		
		buf.SetData(pData, DataHead.m_nHeadDataSize);
		delete[] pData;

		STSDataStream re_ds(&buf);
		DataHead.Check(re_ds);									// 重新读取为是使用游标达到指定位置

		unsigned char checkCode[16] = { 0 };
		STSMD5Context context;
		STSMD5_Init(&context);
		STSMD5_Update(&context, (unsigned char *)(buf.Data() + nDataBufPos), buf.Size() - nDataBufPos);
		STSMD5_Final(&context, checkCode);
		bool bCheckSuccess = true;
		for (int i = 0; i < STS_ARRAY_SIZE(checkCode); ++i)
		{
			if (DataHead.m_cHeadCheckCode[i] != checkCode[i])
			{
				bCheckSuccess = false;
				break;
			}
		}
		if (!bCheckSuccess)
		{
			nFailChannel |= 1 << nChannelIndex;
			continue;
		}

		if (byStartChannel + 1 == byStopChannel)
		{
			nSaveIndex = 0;
		}
		else
		{
			nSaveIndex = nChannelIndex;
		}
		if (!Read(re_ds, nSaveIndex))
		{
			nFailChannel |= 1 << nChannelIndex;
			continue;
		}
	}
	return nFailChannel;
}

void CCalibrationInfo::InitCalibrationInfo(STS_CALINFO& CalibInfo)
{
	CalibInfo.calSystem = STS_8300_SYSTEM;
	CalibInfo.calDate = 0;
	CalibInfo.temperature = 1e5;
	CalibInfo.humidity = 1e5;
	CalibInfo.calResult = 255;
	CalibInfo.logicRev = 255;
	CalibInfo.meterType = 255;
	CalibInfo.meterSn[0] = '\0';
	CalibInfo.meterSnSize = 0;
	CalibInfo.boxType = 255;
	CalibInfo.boxRev = 255;
	CalibInfo.calBdSn[0] = '\0';
	CalibInfo.calBdSnSize = 0;
	CalibInfo.calBdHardRev[0] = '\0';
	CalibInfo.calBdHardRevSize = 0;
	CalibInfo.calBdLogicRev = 0;
	CalibInfo.softwareRev[0] = '\0';
	CalibInfo.softwareRevSize = 0;
	CalibInfo.calSlotID = 0;
	CalibInfo.ATESn[0] = '\0';
	CalibInfo.ATESnSize = 0;
	CalibInfo.bridgeBdSn[0] = '\0';
	CalibInfo.bridgeBdSnSize = 0;
}

int CCalibrationInfo::SetCalibrationInfo(STS_CALINFO* pCalInfo, STS_FLASHINFO_REV FlashRev)
{
	m_pstruCalInfo = pCalInfo;
	m_flashRev = FlashRev;
	return WriteToFlash();	
}

int CCalibrationInfo::GetCalibrationInfo(DWORD dwGetChannel, STS_CALINFO *calInfo)
{	
	m_pstruCalInfo = calInfo;
	int nRetVal = ReadFromFlash(dwGetChannel);
	return nRetVal;
}