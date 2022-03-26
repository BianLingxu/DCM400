#ifndef __844B04FC_4BFB_4B0E_8674_3F89C604A0A1_HDDEBUGTOOLITEMPMUMEAS_H__
#define __844B04FC_4BFB_4B0E_8674_3F89C604A0A1_HDDEBUGTOOLITEMPMUMEAS_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemPmuMeas : public HDDebugToolItem
{
public:
	HDDebugToolItemPmuMeas(HDDebugTool * debugTool);
	~HDDebugToolItemPmuMeas();



    virtual int Type() const;
    virtual const char * Name() const;

    virtual int CanModifyData() const;

    virtual int GetData(int site, int loagicChannel, STSVariant & data, STSVariant & mark) const;
};


#endif /* __844B04FC_4BFB_4B0E_8674_3F89C604A0A1_HDDEBUGTOOLITEMPMUMEAS_H__ */