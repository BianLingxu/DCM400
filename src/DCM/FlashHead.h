#ifndef __QVMEFLASHHEAD_H__
#define __QVMEFLASHHEAD_H__

class STSDataStream;

class FlashHead
{
public:
	FlashHead();
	~FlashHead();

	void Save(STSDataStream & ds) const;
	bool Check(STSDataStream & ds);
	void SetMark(const char *mark, int markSize);

public:
	// 数据头
	unsigned int	m_nHeadDataSize;				// 数据长度（所有数据总长度，包含数据头）
	unsigned int	m_nHeadRev;						// 存储数据的版本信息
	unsigned int	m_nHeadUnitCnt;					// 当存储的SN号及硬件版本号时，记录其个数，其它情况下为默认为1
	unsigned char	m_cHeadCheckCode[16];			// MD5校验码
	unsigned long	m_lHeadReserved;				// 保留字段

private:
	char *			m_pcDCMMark;					// Mark标志
	int				m_nMarkSize;					// Mark标志的Size
};

#endif /*__QVMEFLASHHEAD_H__*/


