#pragma warning (disable:4786)
#include "HardInfo.h"
#include "STSCoreFx.h"
#include "FlashHead.h"
#include "FlashInformation.h"
#include <fstream>
#include "FlashInfo.h"

CHardInfo::CHardInfo(CHardwareFunction& HardwareFunction)
	:m_strMark("DCM100_Hard_Information")
{
	m_pHardwareFunction = &HardwareFunction;
}

CHardInfo::~CHardInfo()
{
	if (m_mapHardInfo.size())
	{
		m_mapHardInfo.clear();
	}
}

bool CHardInfo::Write(STSDataStream & ds, int nChannelIndex)
{
	bool bRet = false;
	do
	{
		std::map<STS_BOARD_MODULE, STS_MODULEINFO>::iterator iter = m_mapHardInfo.begin();
		for (; iter != m_mapHardInfo.end(); iter++)
		{
			ds << (int)iter->first;								// ģ�����

			int i = 0;
			ds << iter->second.nameSize;						// ģ������Ƶĳ���

			for (i = 0; i < iter->second.nameSize; i++)
			{
				ds << iter->second.moduleName[i];				// �忨������
			}

			ds << iter->second.snSize;							// ���кŵĳ���

			for (i = 0; i < iter->second.snSize; i++)
			{
				ds << iter->second.moduleSN[i];					// ���к�
			}

			ds << iter->second.revSize;							// Ӳ���汾�ŵĳ���

			for (i = 0; i < iter->second.revSize; i++)
			{
				ds << iter->second.moduleHardRev[i];			// Ӳ���汾��
			}
		}
		bRet = true;
	} while (false);
	return bRet;
}
bool CHardInfo::Read(STSDataStream & ds, int nChannelIndex)
{
	bool bRet = false;
	do
	{
		for (UINT uUnit = 0; uUnit < m_nUnitCnt; uUnit++)
		{
			STS_BOARD_MODULE bdModule;
			STS_MODULEINFO bdHardInfo;
			int tempBdUnit = -1;
			ds >> tempBdUnit;			// ��ȡ���ݵ�Model
			bdModule = (STS_BOARD_MODULE)tempBdUnit;

			int i = 0;
			char tempChar = 0;

			ds >> bdHardInfo.nameSize;						// ģ�����Ƶĳ���
			for (i = 0; i < bdHardInfo.nameSize; i++)
			{
				ds >> tempChar;								// ģ������
				bdHardInfo.moduleName[i] = tempChar;
			}

			ds >> bdHardInfo.snSize;						// ģ�����кŵĳ���
			for (i = 0; i < bdHardInfo.snSize; i++)
			{
				ds >> tempChar;								// ģ�����к�
				bdHardInfo.moduleSN[i] = tempChar;
			}

			ds >> bdHardInfo.revSize;						// Ӳ���汾�ŵĳ���
			for (i = 0; i < bdHardInfo.revSize; i++)
			{
				ds >> tempChar;								// Ӳ���汾��
				bdHardInfo.moduleHardRev[i] = tempChar;
			}
			m_mapHardInfo.insert(std::make_pair(bdModule, bdHardInfo));
		}
		bRet = true;
	} while (false);
	return bRet;
}

void CHardInfo::Clear()
{
	return;
}

int CHardInfo::WriteToFlash()
{
	BYTE* byDataWritten = STS_NULL;
	DWORD dwDataLength = 0;
	DWORD dwDataLeft = 0;
	BYTE byCurPageNo = 0;
	int nStartIndex = 0;
	short sCurWriteByte = 0;
	BYTE bySectorNo = HARD_INFO_SECTOR;
	//Erase the sector which save current Controller's calibration information.
	m_pHardwareFunction->EraseFlash(bySectorNo);

	int nRet = -1;
	int loopCnt = 0;
	do
	{
		if (!m_mapHardInfo.size())
		{
			return -1;
			break;
		}
		STSBuffer buf;
		if (!buf.Open(STSIODevice::WriteOnly))
		{
			return -1;
			break;
		}

		STSDataStream ds(&buf);

		// д�����ݵ�ͷ����ҪĿ����ռλ
		FlashHead dataHead;
		dataHead.SetMark(m_strMark.c_str(), m_strMark.size() + 1);					// ���ñ�־
		dataHead.m_nHeadUnitCnt = m_mapHardInfo.size();							// SN�ŵĴ洢����
		dataHead.m_nHeadRev = m_flashRev;										// FLASH�汾
		dataHead.Save(ds);

		int nDataBufPos = buf.Pos();
		// д������
		Write(ds);

// 		if ((0 >= buf.Size() - sizeof(DCMFLASHHead)) || buf.Size() > 4096)		// 1ҳ�Ĵ洢�ռ�Ϊ4K = 4096 Byte
// 		{
// 			break;
// 		}

		dataHead.m_nHeadDataSize = buf.Size();
		// д��MD5У����
		STSMD5Context context;
		STSMD5_Init(&context);
		STSMD5_Update(&context, (unsigned char *)(buf.Data() + nDataBufPos), buf.Size() - nDataBufPos);
		STSMD5_Final(&context, (unsigned char *)(dataHead.m_cHeadCheckCode));

		buf.Seek(0);		// �ص���ʼλ��
		dataHead.Save(ds);


		buf.Seek(0);		// �ص���ʼλ��
		dataHead.Save(ds);
		byDataWritten = (BYTE*)buf.Data();
		dwDataLength = buf.Size();

		nStartIndex = 0;
		dwDataLeft = dwDataLength;
		byCurPageNo = HARD_INFO_PAGE_START;
		do
		{
			sCurWriteByte = dwDataLeft > 256 ? 256 : (short)dwDataLeft;

			if (0 != m_pHardwareFunction->WriteFlash(bySectorNo, byCurPageNo, 0, sCurWriteByte, &byDataWritten[nStartIndex]))			// дFLASHʧ�ܣ�����-1������1��
			{
				return -1;
			}
			++byCurPageNo;
			dwDataLeft -= sCurWriteByte;
			nStartIndex += sCurWriteByte;
		} while (0 < dwDataLeft);

		nRet = 0;
	} while (false);
	return nRet;
}

int CHardInfo::ReadFromFlash(DWORD dwGetChannel)
{
	DWORD dwDataLength = 0;
	DWORD dwDataLeft = 0;
	BYTE byCurPageNo = 0;
	int nStartIndex = 0;
	short sCurWriteByte = 0;
	BYTE bySectorNo =  HARD_INFO_SECTOR;
	BYTE byOffset = 0;
	int nSaveIndex = 0;
	int nRet = -1;
	do
	{
		FlashHead dataHead;
		dataHead.SetMark(m_strMark.c_str(), m_strMark.size() + 1);
		int nDataBufPos = 0;
	
		STSBuffer bufTemp;
		if (!bufTemp.Open(STSIODevice::ReadWrite))
		{
			return -1;
			break;
		}

		STSDataStream ds(&bufTemp);
		dataHead.Save(ds);
		nDataBufPos = bufTemp.Pos();
		{
			STSBuffer StsBuff;
			bufTemp.Swap(StsBuff);					// ���bufTemp
		}
		bufTemp.Open(STSIODevice::ReadWrite);
		bool sameFlag = true;
		char *pBuf = new char[nDataBufPos + 1];

		nStartIndex = 0;
		dwDataLeft = nDataBufPos;
		byCurPageNo =  HARD_INFO_PAGE_START;
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
		
		if ((!sameFlag) || (!dataHead.Check(ds)))					// ��鲻ͬС��洢����Ϣ�Ƿ���ͬ��У���ж�����ͷ��Mark
		{
			return -1;
			break;
		}
		m_nUnitCnt = dataHead.m_nHeadUnitCnt;		// �洢���ݵĵ�Ԫ��
		m_flashRev = (STS_FLASHINFO_REV)dataHead.m_nHeadRev;

		if (0 >= dataHead.m_nHeadDataSize || 4096 < dataHead.m_nHeadDataSize)
		{
			return -1;
			break;
		}

		STSBuffer buf;
		if (!buf.Open(STSIODevice::ReadOnly))
		{
			return -1;
			break;
		}

		char *pData = new char[dataHead.m_nHeadDataSize + 1];

		nStartIndex = 0;
		dwDataLeft = dataHead.m_nHeadDataSize;
		byCurPageNo = HARD_INFO_PAGE_START;

		do
		{
			sCurWriteByte = dwDataLeft > 256 ? 256 : (short)dwDataLeft;
			//Read the calibration data from flash.
			m_pHardwareFunction->ReadFlash(bySectorNo, byCurPageNo, 0, sCurWriteByte, (BYTE*)&pData[nStartIndex]);
			++byCurPageNo;
			dwDataLeft -= sCurWriteByte;
			nStartIndex += sCurWriteByte;
		} while (0 < dwDataLeft);

		// ��������ͷ�д洢�����ݳ��ȣ���ȡ��������
		buf.SetData(pData, dataHead.m_nHeadDataSize);
		delete[] pData;
		
		STSDataStream re_ds(&buf);
		dataHead.Check(re_ds);									// ���¶�ȡΪ��ʹ���α�ﵽָ��λ��

		unsigned char checkCode[16] = { 0 };
		STSMD5Context context;
		STSMD5_Init(&context);
		STSMD5_Update(&context, (unsigned char *)(buf.Data() + nDataBufPos), buf.Size() - nDataBufPos);
		STSMD5_Final(&context, checkCode);
		bool bCheckSuccess = true;
		for (int i = 0; i < STS_ARRAY_SIZE(checkCode); ++i)
		{
			if (dataHead.m_cHeadCheckCode[i] != checkCode[i])
			{
				bCheckSuccess = false;
				break;
			}
		}
		if (!bCheckSuccess)
		{
			return -1;
			break;
		}

		if (!Read(re_ds))
		{
			return -1;
			break;
		}
		nRet = 0;
	} while (false);
	return nRet;
}

bool CHardInfo::SetHardInfo(std::map<STS_BOARD_MODULE, STS_MODULEINFO> hardInfo,
							   STS_FLASHINFO_REV flashRev)
{
	bool ret = true;
	if (hardInfo.size())
	{
		m_mapHardInfo = hardInfo;
		m_flashRev = flashRev;
		if (WriteToFlash())
		{
			ret = false;
		}
	}
	else
	{
		ret = false;
	}
	return ret;
}
std::map<STS_BOARD_MODULE, STS_MODULEINFO> CHardInfo::GetHardInfo()
{
	ReadFromFlash();
	return m_mapHardInfo;
}