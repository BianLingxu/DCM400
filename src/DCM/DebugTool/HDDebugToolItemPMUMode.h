#ifndef __12C48B11_0887_4FFE_B101_5582054E80AE_HDDEBUGTOOLITEMPMUMODE_H__
#define __12C48B11_0887_4FFE_B101_5582054E80AE_HDDEBUGTOOLITEMPMUMODE_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemPmuMode : public HDDebugToolItem
{
public :
	HDDebugToolItemPmuMode(HDDebugTool * debugTool);
	~HDDebugToolItemPmuMode();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__12C48B11_0887_4FFE_B101_5582054E80AE_HDDEBUGTOOLITEMPMUMODE_H__*/