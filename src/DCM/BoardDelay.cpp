#include "BoardDelay.h"
#include "STSCoreFx.h"
#include "FlashInfo.h"

CBoardDelay::CBoardDelay(BYTE bySlotNo)
	: m_Hardware(bySlotNo, nullptr)
	, m_strMark("DCM delay information")
	, m_dTimesetDelay(0)
	, m_dTotalStartDelay(0)
{

}

int CBoardDelay::SaveDelay()
{
	///<Erase delay flash
	m_Hardware.EraseFlash(DELAY_DATA_SECTOR);

	m_Hardware.EraseFlash(BACKUP_DELAY_DATA_SECTOR);

	int nRetVal = 0;
	BOOL bNoBoard = TRUE;
	for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
	{
		m_Hardware.SetControllerIndex(byControllerIndex);
		if (!m_Hardware.IsControllerExist())
		{
			continue;
		}
		bNoBoard = FALSE;
		nRetVal = SaveDelay(byControllerIndex);
		if (0 != nRetVal)
		{
			nRetVal = -1;
			break;
		}
	}
	if (bNoBoard)
	{
		nRetVal = -2;
	}
	return nRetVal;
}

int CBoardDelay::ReadDelay(BYTE byControllerIndex)
{
	m_Hardware.SetControllerIndex(byControllerIndex);
	if (!m_Hardware.IsControllerExist())
	{
		return -1;
	}
	DWORD dwDataLength = 0;
	DWORD dwDataLeft = 0;
	BYTE byCurPageNo = 0;
	int nStartIndex = 0;
	short sCurWriteByte = 0;
	BYTE bySectorNo = DELAY_DATA_SECTOR;
	BYTE byOffset = 0;
	int nSaveIndex = 0;
	int nRetVal = -1;
	int nSector[2] = { DELAY_DATA_SECTOR, BACKUP_DELAY_DATA_SECTOR };
	for (int nSectorIndex = 0; nSectorIndex < 2; ++nSectorIndex)
	{
		bySectorNo = nSector[nSectorIndex];
		do
		{
			int nDataBufPos = 0;

			STSBuffer bufTemp;
			if (!bufTemp.Open(STSIODevice::ReadWrite))
			{
				nRetVal = -2;
				break;
			}

			STSDataStream DataStream(&bufTemp);
			SaveHead(DataStream);
			nDataBufPos = bufTemp.Pos();
			{
				STSBuffer StsBuff;
				bufTemp.Swap(StsBuff);					// 清空bufTemp
			}
			bufTemp.Open(STSIODevice::ReadWrite);
			bool sameFlag = true;
			char* pBuf = new char[nDataBufPos + 1];

			nStartIndex = 0;
			dwDataLeft = nDataBufPos;
			byCurPageNo = DELAY_DATA_PAGE_PER_CONTROL * byControllerIndex;
			do
			{
				sCurWriteByte = dwDataLeft > 256 ? 256 : (short)dwDataLeft;
				//Read the calibration data from flash.
				m_Hardware.ReadFlash(bySectorNo, byCurPageNo, 0, sCurWriteByte, (BYTE*)&pBuf[nStartIndex]);
				++byCurPageNo;
				dwDataLeft -= sCurWriteByte;
				nStartIndex += sCurWriteByte;
			} while (0 < dwDataLeft);

			bufTemp.SetData(pBuf, nDataBufPos + 1);

			delete[] pBuf;
			pBuf = nullptr;


			if ((!sameFlag) || (!HeadCheck(DataStream)))					// 检查不同小板存储的信息是否相同，校验判断数据头的Mark
			{
				nRetVal = -2;
				break;
			}
			if (0 >= m_uDataSize || 4096 < m_uDataSize)
			{
				nRetVal = -2;
				break;
			}

			STSBuffer buf;
			if (!buf.Open(STSIODevice::ReadOnly))
			{
				nRetVal = -2;
				break;
			}

			char* pData = new char[m_uDataSize + 1];

			nStartIndex = 0;
			dwDataLeft = m_uDataSize;
			byCurPageNo = DELAY_DATA_PAGE_PER_CONTROL * byControllerIndex;

			do
			{
				sCurWriteByte = dwDataLeft > 256 ? 256 : (short)dwDataLeft;
				//Read the calibration data from flash.
				m_Hardware.ReadFlash(bySectorNo, byCurPageNo, 0, sCurWriteByte, (BYTE*)&pData[nStartIndex]);
				++byCurPageNo;
				dwDataLeft -= sCurWriteByte;
				nStartIndex += sCurWriteByte;
			} while (0 < dwDataLeft);

			// 根据数据头中存储的数据长度，获取所有数据
			buf.SetData(pData, m_uDataSize);
			delete[] pData;

			STSDataStream re_ds(&buf);
			HeadCheck(re_ds);									// 重新读取为是使用游标达到指定位置

			unsigned char ucMD5[16] = { 0 };
			STSMD5Context context;
			STSMD5_Init(&context);
			STSMD5_Update(&context, (unsigned char*)(buf.Data() + nDataBufPos), buf.Size() - nDataBufPos);
			STSMD5_Final(&context, ucMD5);
			bool bCheckSuccess = true;
			if (0 != memcmp(m_ucCheckCode, ucMD5, sizeof(ucMD5)))
			{
				bCheckSuccess = false;
				break;
			}
			if (!bCheckSuccess)
			{
				nRetVal = -3;
				break;
			}
			GetDelay(re_ds);			
			nRetVal = 0;
		} while (false);
	}
	return nRetVal;
}

int CBoardDelay::GetControllerDelay(double* pdTimesetDelay, double* pdTotalStartDelay)
{
	if (nullptr == pdTimesetDelay || nullptr == pdTotalStartDelay)
	{
		return -1;
	}
	*pdTimesetDelay = m_dTimesetDelay;
	*pdTotalStartDelay = m_dTotalStartDelay;
	return 0;
}

int CBoardDelay::GetIODelay(USHORT usChannel, IO_DELAY* pIODelay)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	if (nullptr == pIODelay)
	{
		return -2;
	}
	if (m_vecIODelay.size() <= usChannel)
	{
		return -3;
	}
	*pIODelay = m_vecIODelay[usChannel];
	return 0;
}

int CBoardDelay::SaveDelay(BYTE byControllerIndex)
{
	BYTE* byDataWritten = STS_NULL;
	DWORD dwDataLength = 0;
	DWORD dwDataLeft = 0;
	BYTE byCurPageNo = 0;
	int nStartIndex = 0;
	short sCurWriteByte = 0;
	m_Hardware.SetControllerIndex(byControllerIndex);
	if (!m_Hardware.IsControllerExist())
	{
		return -1;
	}
	int nRet = -1;
	int loopCnt = 0;
	do
	{
		STSBuffer buf;
		if (!buf.Open(STSIODevice::WriteOnly))
		{
			return -1;
			break;
		}

		STSDataStream DataStream(&buf);

		SaveHead(DataStream);
		int nDataBufPos = buf.Pos();
		// 写入数据
		WriteDelay(DataStream);

		byDataWritten = (BYTE*)buf.Data();

		m_uDataSize = buf.Size();
		// 写入MD5校验码
		STSMD5Context context;
		STSMD5_Init(&context);
		STSMD5_Update(&context, (unsigned char*)(buf.Data() + nDataBufPos), buf.Size() - nDataBufPos);
		STSMD5_Final(&context, (unsigned char*)(m_ucCheckCode));

		buf.Seek(0);		// 回到初始位置
		SaveHead(DataStream);

		byDataWritten = (BYTE*)buf.Data();
		dwDataLength = buf.Size();

		nStartIndex = 0;
		dwDataLeft = dwDataLength;
		byCurPageNo = DELAY_DATA_PAGE_PER_CONTROL * byControllerIndex;
		do
		{
			sCurWriteByte = dwDataLeft > 256 ? 256 : (short)dwDataLeft;

			if (0 != m_Hardware.WriteFlash(DELAY_DATA_SECTOR, byCurPageNo, 0, sCurWriteByte, &byDataWritten[nStartIndex]))			// 写FLASH失败，返回-1，冗余1次
			{
				return -2;
			}

			if (0 != m_Hardware.WriteFlash(BACKUP_DELAY_DATA_SECTOR, byCurPageNo, 0, sCurWriteByte, &byDataWritten[nStartIndex]))			// 写FLASH失败，返回-1，冗余1次
			{
				return -2;
			}
			++byCurPageNo;
			dwDataLeft -= sCurWriteByte;
			nStartIndex += sCurWriteByte;
		} while (0 < dwDataLeft);

		nRet = 0;
	} while (false);
	return nRet;
}

int CBoardDelay::WriteDelay(STSDataStream& DataStream)
{
	if (!m_Hardware.IsControllerExist())
	{
		return -1;
	}
	///<Save the timeset delay
	double dDelay = m_Hardware.GetTimesetDelay();
	DataStream << dDelay;

	///<Save total start delay
	dDelay = m_Hardware.GetTotalStartDelay();
	DataStream << dDelay;

	///<Save the IO delay of each channel
	const int nIODelayCount = 4;
	double dIODelay[nIODelayCount] = { 0 };
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		m_Hardware.GetIODelay(usChannel, &dIODelay[0], &dIODelay[1], &dIODelay[2], &dIODelay[3]);
		for (int nDelayIndex = 0; nDelayIndex < nIODelayCount; ++nDelayIndex)
		{
			DataStream << dIODelay[nDelayIndex];
		}
	}
	return 0;
}

void CBoardDelay::SaveHead(STSDataStream& DataStream)
{
	STSIODevice* pDevice = DataStream.IODevice();
	pDevice->Write(m_strMark.c_str(), m_strMark.size() + 1);
	pDevice->Write((const char*)&m_uDataSize, sizeof(m_uDataSize));
	pDevice->Write((const char*)&m_ucCheckCode, sizeof(m_ucCheckCode));
}

bool CBoardDelay::HeadCheck(STSDataStream& DataStream)
{
	bool bRet = false;
	do
	{
		STSIODevice* pDevice = DataStream.IODevice();

		int nMarkSize = m_strMark.size() + 1;
		char* lpszMark = new char[nMarkSize];
		memset(lpszMark, 0, nMarkSize);
		pDevice->Read(lpszMark, nMarkSize);
		bool bOk = true;
		if (0 != m_strMark.compare(lpszMark))			// 读取的标志与之前写入的如否相同
		{
			bOk = false;
		}
		if (nullptr != lpszMark)
		{
			delete[] lpszMark;
			lpszMark = nullptr;
		}
		if (!bOk)
		{
			break;
		}
		pDevice->Read((char*)&m_uDataSize, sizeof(m_uDataSize));
		pDevice->Read((char*)&m_ucCheckCode, STS_ARRAY_SIZE(m_ucCheckCode));
		bRet = true;
	} while (false);
	return bRet;
}

void CBoardDelay::GetDelay(STSDataStream& DataStream)
{
	m_vecIODelay.clear();
	///<Get the timeset delay
	DataStream >> m_dTimesetDelay;

	///<Get total start delay
	DataStream >> m_dTotalStartDelay;

	///<Get the IO delay of each channel
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		IO_DELAY Delay;
		DataStream >> Delay.m_dData;
		DataStream >> Delay.m_dDataEn;
		DataStream >> Delay.m_dHigh;
		DataStream >> Delay.m_dLow;
		m_vecIODelay.push_back(Delay);
	}
}
