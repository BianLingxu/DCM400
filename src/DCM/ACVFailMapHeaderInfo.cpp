#include "ACVFailMapHeaderInfo.h"
#include <assert.h>
#include <windows.h>

#ifdef _DEBUG
#define __ASSERT__(x) assert(x)
#else
#define __ASSERT__(x)
#endif

enum 
{
	FAILMAP_VER_0 = 0, // 早期无版本
	//FAILMAP_VER_1 = 1, // 加入记录通道失效个数

	FAILMAP_VER_CUR
};

static DWORD GetSysAllocationGranularity()
{
    SYSTEM_INFO sinfo;
    ::GetSystemInfo(&sinfo);
    return sinfo.dwAllocationGranularity;
}

const DWORD ACVFailMapHeaderInfo::PageByteSize = GetSysAllocationGranularity();

ACVFailMapHeaderInfo::ACVFailMapHeaderInfo()
    : m_nRecordFileSize(0)
    , m_nChCount(0)
    , m_nChUnitCount(0)
    , m_nInfoStructSize(0)
    , m_nInfoDataSize(0)
    , m_nLineCountPerPage(0)
    , m_dataBuf(STS_NULL)
    , m_pOffsetRow(STS_NULL)
    , m_pChAddress(STS_NULL)
    , m_nChCursor(0)
    , m_nWriteByteSize(0)
	, m_nFailMapVer(FAILMAP_VER_CUR)
	, m_nRecordChFailCountOffsetInFile(0)
{
    for (int i = 0; i < ChannelIndexBufSize; ++i)
    {
        m_vecChannelIndex.Append(STS_NULL);
    }
}

ACVFailMapHeaderInfo::~ACVFailMapHeaderInfo()
{
    for each (auto var in m_vecChannelIndex)
    {
        ChannelIndex *pt = var;
        while (STS_NULL != pt)
        {
            ChannelIndex *pPrev = pt;
            pt = pt->next;
            delete pPrev;
        }
    }

    m_pOffsetRow = STS_NULL;
    m_pChAddress = STS_NULL;
    if (STS_NULL != m_dataBuf)
    {
        delete[] m_dataBuf;
        m_dataBuf = STS_NULL;
    }
}


void ACVFailMapHeaderInfo::Save(STSDataStream & ds)
{
	STSIODevice *pIODevice = ds.IODevice();

    ds << m_nRecordFileSize;
    ds << m_nTotalCount;
    ds << fileName;
    ds << fileID;
    ds << saveMark;
    ds << labelMark;

    int nChannelCount = channels.Size();
    ds << nChannelCount;

    for (int i = 0; i < nChannelCount; ++i)
    {
        ds << channels.At(i);
    }

	ds << m_nFailMapVer; // 加入版本号
	m_nRecordChFailCountOffsetInFile = pIODevice->Pos(); //缓存当前文件的位置
	// 记录通道失败个数 初始化和文件中占位操作
	int nChFailCount = 0;
	for (int i = 0; i < nChannelCount; ++i)
	{
		m_vecChFailCount.Append(nChFailCount);
		ds << nChFailCount;
	}
    
    unsigned long pos = pIODevice->Pos();
    __ASSERT__(PageByteSize > pos);
    unsigned long nCount = PageByteSize - pos;

    char c ='\0' ;
    for (unsigned long i = 0; i < nCount; ++i)
    {
        pIODevice->Write(&c, 1);
    }

    pos = pIODevice->Pos();
    __ASSERT__(PageByteSize == pos);
    Calc(true);
}

void ACVFailMapHeaderInfo::Load(STSDataStream & ds)
{
	STSIODevice *pIODevice = ds.IODevice();

    ds >> m_nRecordFileSize;
    ds >> m_nTotalCount;
    ds >> fileName;
    ds >> fileID;
    ds >> saveMark;
    ds >> labelMark;

    int nChannelCount = 0;
    ds >> nChannelCount;

    for (int i = 0; i < nChannelCount; ++i)
    {
        int nCh = -1;
        ds >> nCh;
        channels.Append(nCh);
    }

	ds >> m_nFailMapVer; // 加入版本号
	m_nRecordChFailCountOffsetInFile = pIODevice->Pos();
	// 为了保证后面兼容处理减少使用版本号判断，直接初始化为0 FAILMAP_VER_0
	for (int i = 0; i < nChannelCount; ++i)
	{
		m_vecChFailCount.Append(0);
	}

	// 兼容加载方式
	do 
	{
		if (FAILMAP_VER_0 == m_nFailMapVer)
		{
			break;
		}

		for (int i = 0; i < nChannelCount; ++i)
		{
			int nChFailCount = 0;
			ds >> nChFailCount;
			m_vecChFailCount[i] = nChFailCount;
		}
	} while (false);

    
    unsigned long pos = pIODevice->Pos();
    __ASSERT__(PageByteSize > pos);
    pIODevice->Seek(PageByteSize);

    Calc(false);
}



void ACVFailMapHeaderInfo::SetChannelValidState(char c)
{
	// 记录通道失败个数
	int nChCount = m_vecChFailCount.size();
	for (int i = 0, j = 8 * m_nChCursor; i < 8; ++i, ++j)
	{
		if (j >= nChCount)
		{
			break;
		}
		char v = 1 << i;
		if (v == (v & c))
		{
			++m_vecChFailCount[j];
		}
	}


	//
    *(m_pChAddress + m_nChCursor) = c;
    ++m_nChCursor;
    if (m_nChCursor == m_nChUnitCount)
    {
        m_nChCursor = 0;
    }
}

void ACVFailMapHeaderInfo::SaveData(STSDataStream & ds)
{
    STSIODevice *pIODevice = ds.IODevice();
    unsigned long nWriteSize = pIODevice->Write(m_dataBuf, m_nInfoStructSize);
    m_nWriteByteSize += m_nInfoStructSize;

    DWORD nSize = PageByteSize - m_nWriteByteSize;
    if (nSize == 0)
    {
        m_nWriteByteSize = 0;
    }
    else if (nSize < m_nInfoStructSize)
    {
        char c = 0;
        for (DWORD i = 0; i < nSize; ++i)
        {
            pIODevice->Write(&c, 1);
        }
        m_nWriteByteSize = 0;

        __ASSERT__(0 == pIODevice->Pos() % PageByteSize);
    }
}

void ACVFailMapHeaderInfo::EndSave(STSDataStream & ds, int totalCount)
{
    this->m_nTotalCount = totalCount;
    STSIODevice *pIODevice = ds.IODevice();
    m_nRecordFileSize = pIODevice->Pos();
    pIODevice->Seek(0);
    ds << m_nRecordFileSize;
    ds << totalCount;

	// 记录通道失败个数
	pIODevice->Seek(m_nRecordChFailCountOffsetInFile);
	int nChannelCount = channels.Size();
	for (int i = 0; i < nChannelCount; ++i)
	{
		ds << m_vecChFailCount.At(i);
	}
}

bool ACVFailMapHeaderInfo::GenerateChannelQuickSearchIndex()
{
    bool bRet = false;
    do 
    {

        if (channels.IsEmpty())
        {
            break;
        }

        bRet = true;
        int nSize = channels.Size();
        for (int i = 0; i < nSize; ++i)
        {
            int nCh = channels.At(i);
            int nPos = nCh % ChannelIndexBufSize;
			int nChFailCount = m_vecChFailCount.At(i);

            if (STS_NULL == m_vecChannelIndex[nPos])
            {
                ChannelIndex *p = new ChannelIndex();
                p->next = STS_NULL;
                p->index = i;
                p->channel = nCh;
				p->failCount = nChFailCount;
                m_vecChannelIndex[nPos] = p;
                continue;
            }
            
            ChannelIndex *pPrev = STS_NULL;
            ChannelIndex *pt = m_vecChannelIndex[nPos];
            while (STS_NULL != pt)
            {
                if (pt->channel == nCh)
                {
                    bRet = false;
                    break;
                }
                pPrev = pt;
                pt = pt->next;
            }

            if (!bRet)
            {
                break;
            }
            ChannelIndex *p = new ChannelIndex();
            p->next = STS_NULL;
            p->index = i;
            p->channel = nCh;
			p->failCount = nChFailCount;
            pPrev->next = p;
        }

    } while (false);
    return bRet;
}

int ACVFailMapHeaderInfo::GetChannelIndex(int channel) const
{
    int nPos = channel % ChannelIndexBufSize;
    ChannelIndex *pChIndex = m_vecChannelIndex.At(nPos);
    while (STS_NULL != pChIndex)
    {
        if (channel == pChIndex->channel)
        {
            return pChIndex->index;
        }
        pChIndex = pChIndex->next;
    }

    return -1;
}

bool ACVFailMapHeaderInfo::HasFailInfo(int channel) const
{
	int nPos = channel % ChannelIndexBufSize;
	ChannelIndex *pChIndex = m_vecChannelIndex.At(nPos);
	while (STS_NULL != pChIndex)
	{
		if (channel == pChIndex->channel)
		{
			return (0 < pChIndex->failCount);
		}
		pChIndex = pChIndex->next;
	}

	return false;
}

bool ACVFailMapHeaderInfo::CanRead(STSDataStream & ds)
{
    STSIODevice *pIODevice = ds.IODevice();
    if (pIODevice->AtEnd())
    {
        return false;
    }
    return true;
}

bool ACVFailMapHeaderInfo::ReadOneLineData(STSDataStream & ds)
{
    STSIODevice *pIODevice = ds.IODevice();
    if (pIODevice->AtEnd())
    {
        return false;
    }

    unsigned long nReadSize = pIODevice->Read(m_dataBuf, m_nInfoStructSize);
    assert(nReadSize == m_nInfoStructSize);

    m_nWriteByteSize += m_nInfoStructSize;
    DWORD nSize = PageByteSize - m_nWriteByteSize;
    if (nSize == 0)
    {
        m_nWriteByteSize = 0;
    }
    else if (nSize < m_nInfoStructSize)
    {
        // 可以seek
        char c = 0;
        for (DWORD i = 0; i < nSize; ++i)
        {
            pIODevice->Read(&c, 1);
        }
        m_nWriteByteSize = 0;

        __ASSERT__(0 == pIODevice->Pos() % PageByteSize);
    }
    return true;
}

void ACVFailMapHeaderInfo::Calc(bool bCreateBuf)
{
    int nChCount = channels.Size();
    m_nChUnitCount = nChCount / CHMinRecordUnit;
    if (0 < nChCount % CHMinRecordUnit)
    {
        ++m_nChUnitCount;
    }

    m_nInfoStructSize = sizeof(int) + m_nChUnitCount;
    __ASSERT__(0 < m_nInfoStructSize);
    m_nInfoDataSize = m_nTotalCount * m_nInfoStructSize;

    if (bCreateBuf)
    {
        m_dataBuf = new char[m_nInfoStructSize];
        m_pOffsetRow = reinterpret_cast<int *>(m_dataBuf);
        m_pChAddress = m_dataBuf + sizeof(int);
    }
    
    m_nLineCountPerPage = PageByteSize / m_nInfoStructSize;
 
}

