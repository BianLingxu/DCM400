#ifndef __F9006EEE_C3CD_4090_92A3_5E0CCB3B75A5_HDDEBUGTOOL_H__
#define __F9006EEE_C3CD_4090_92A3_5E0CCB3B75A5_HDDEBUGTOOL_H__

#include <vector>
#include "ATGlobal.cs"
#include "IHDDebugTool.h"
#include "IHDDebugToolCommon.h"

class HDModule;

class HDDebugTool : public IHDDebugTool, public IHDDebugToolCommon
{
public:
    HDDebugTool(HDModule * parent);
    ~HDDebugTool();

    virtual int QueryInterface(const char * interfaceName, void ** ptr);
    virtual int SetCallBackHelp(IHDDebugToolCallBackHelp * help);
    

    virtual int GetSites(int & sites) const;
    virtual int GetValidLogicChannels(int site, STSVector<LoagicChannel> & channelsBuf) const;

    virtual int GetItems(STSVector<IHDDebugToolItem *> & items) const;

    virtual int ApplayRun();
    virtual int ApplayRun(int logicChannel, int site);

    IHDDebugToolCallBackHelp * CallBackHelp() const{
        return m_pCallBackHelp;
    }

    AccoTest::HardwareType Type() ;

private:
    void Init();

private:
    HDModule *m_pParent;
    IHDDebugToolCallBackHelp *m_pCallBackHelp;

    std::vector<IHDDebugToolItem *> m_vecItems;
};


#endif /* __F9006EEE_C3CD_4090_92A3_5E0CCB3B75A5_HDDEBUGTOOL_H__ */