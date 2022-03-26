
#pragma warning (disable:4786)
#include "FlashHead.h"
#include "STSCoreFx.h"


FlashHead::FlashHead()
	: m_nHeadDataSize(0)
	, m_nHeadRev(0)
	, m_nHeadUnitCnt(1)
	, m_lHeadReserved(0)
	, m_pcDCMMark(nullptr)
	, m_nMarkSize(0)
{
	memset(m_cHeadCheckCode, 'a', STS_ARRAY_SIZE(m_cHeadCheckCode));
}

FlashHead::~FlashHead()
{
	if (m_pcDCMMark != nullptr)
	{
		delete[] m_pcDCMMark;
		m_pcDCMMark = nullptr;
	}
}

void FlashHead::Save(STSDataStream & ds) const
{
	STSIODevice *pDevice = ds.IODevice();
	if (m_pcDCMMark != nullptr)
	{
		pDevice->Write(m_pcDCMMark, m_nMarkSize);
	}
	pDevice->Write((const char *)&m_nHeadDataSize, sizeof(m_nHeadDataSize));
	pDevice->Write((const char *)&m_nHeadRev, sizeof(m_nHeadRev));
	pDevice->Write((const char *)&m_nHeadUnitCnt, sizeof(m_nHeadUnitCnt));
	pDevice->Write((const char *)&m_cHeadCheckCode, STS_ARRAY_SIZE(m_cHeadCheckCode));
	pDevice->Write((const char *)&m_lHeadReserved, sizeof(m_lHeadReserved));
}

bool FlashHead::Check(STSDataStream & ds)
{
	bool bRet = false;
	do
	{
		STSIODevice *pDevice = ds.IODevice();

		char *DCMMark = new char[m_nMarkSize];
		pDevice->Read(DCMMark, m_nMarkSize);
		bool bOk = true;
		for (int i = 0; i < m_nMarkSize; ++i)
		{
			if (m_pcDCMMark[i] != DCMMark[i])			// 读取的标志与之前写入的如否相同
			{
				bOk = false;
				break;
			}
		}
		if (!bOk)
		{
			break;
		}
		pDevice->Read((char *)&m_nHeadDataSize, sizeof(m_nHeadDataSize));
		pDevice->Read((char *)&m_nHeadRev, sizeof(m_nHeadRev));
		pDevice->Read((char *)&m_nHeadUnitCnt, sizeof(m_nHeadUnitCnt));
		pDevice->Read((char *)&m_cHeadCheckCode, STS_ARRAY_SIZE(m_cHeadCheckCode));
		pDevice->Read((char *)&m_lHeadReserved, sizeof(m_lHeadReserved));
		bRet = true;
	} while (false);
	return bRet;
}

void FlashHead::SetMark(const char *mark, int markSize)
{
	if (markSize > 0)
	{
		m_nMarkSize = markSize;
		if (m_pcDCMMark != nullptr)
		{
			delete[] m_pcDCMMark;
			m_pcDCMMark = nullptr;
		}
		m_pcDCMMark = new char[m_nMarkSize + 1];
		memset(m_pcDCMMark, 0, m_nMarkSize + 1);
		sts_strcpy(m_pcDCMMark, markSize + 1, mark);
	}
}