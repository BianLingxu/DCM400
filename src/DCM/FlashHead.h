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
	// ����ͷ
	unsigned int	m_nHeadDataSize;				// ���ݳ��ȣ����������ܳ��ȣ���������ͷ��
	unsigned int	m_nHeadRev;						// �洢���ݵİ汾��Ϣ
	unsigned int	m_nHeadUnitCnt;					// ���洢��SN�ż�Ӳ���汾��ʱ����¼����������������ΪĬ��Ϊ1
	unsigned char	m_cHeadCheckCode[16];			// MD5У����
	unsigned long	m_lHeadReserved;				// �����ֶ�

private:
	char *			m_pcDCMMark;					// Mark��־
	int				m_nMarkSize;					// Mark��־��Size
};

#endif /*__QVMEFLASHHEAD_H__*/


