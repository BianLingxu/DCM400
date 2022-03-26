#ifndef __QVMEHARDINFO_H__
#define __QVMEHARDINFO_H__

#include <string>
#include <map>
#include "BaseDataOp.h"
#include "HardwareFunction.h"
#include "FlashInformation.h"

class STSDataStream;

class CHardInfo : public BaseDataOp
{
public:
	CHardInfo(CHardwareFunction& HardwareFunction);
	~CHardInfo();
private:
	bool Write(STSDataStream & ds, int nChannelIndex = 0);
	bool Read(STSDataStream & ds, int nChannelIndex = 0);
	void Clear();	

	int WriteToFlash();
	int ReadFromFlash(DWORD dwGetChannel = 0);

public:
	bool SetHardInfo(std::map<STS_BOARD_MODULE, STS_MODULEINFO> hardInfo,
					 STS_FLASHINFO_REV flashRev = STS_FLASH_REV_1);
	std::map<STS_BOARD_MODULE, STS_MODULEINFO> GetHardInfo();

private:
	std::string m_strMark;					// 标志
	unsigned int m_nUnitCnt;				// 硬件板卡的数量（大板+子模块）
	STS_FLASHINFO_REV m_flashRev;			// 数据在FLASH中的存储版本
	std::map<STS_BOARD_MODULE, STS_MODULEINFO> m_mapHardInfo;
	CHardwareFunction* m_pHardwareFunction;
};


#endif /*__QVMEHARDINFO_H__*/



