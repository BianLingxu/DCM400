/*!
 * @file      ACVFailMapHeaderInfo.h
 *
 * Copyright (C) xg
 *
 * @author    xg
 * @date      2017/09/13
 * @version   v 1.0.0.0
 * @brief     
              1. 信息记录样式 : 相对标签偏移量(int) 通道单元是否有效的bit位标识(最小单元char) 计算信息结构大小
                 例如 通道数是13，相对label标签7处, 第2，8, 1通道失效 那么记录信息如下(忽略空格) (14 - 16没有通道失效值默认给0)
                 int  char      char
                 7    0100 0001 0000 0010 0000
              2. 文件记录信息头大小为系统提供的页最小值(64k)
              3. 当每页最后剩余容量不足通道信息数据长度时将剩余容量补0,那么将通道信息数据从下一页开始写入

          补充
              1. 每8个通道为1一个单元,不满8给通道,bit位补0
                 例如,有如下所示7个通道, 
                 通道值是     : 0   1   0   0    0   0  1
                 通道顺序是   : 15, 17, 14, 12,  55, 7, 8
                 bit排序顺序是:  x, 8,  7,  55,  12, 14, 17, 15
                 bit值是:        0, 1,  0,   0,   0,  0,  1,  0   


		      2. 2019/03/29 
			     0页记录数据头部信息,从第1页开始记录错误信息 
                 


              使用样例:
              void SaveFailMap()
              {
                  STSFile fl;
                  if (!fl.Open(strFilePath.c_str(), STSIODevice::WriteOnly))
                  {
                      // error;
                      return;
                  }

                  STSDataStream ds(&fl);
                  ACVFailMapHeaderInfo header;
                  header.fileName = "vectorFileName"; //!< 记录向量文件名
                  header.fileID = "";                 //!< 记录向量文件ID
                  header.saveMark = "";               //!< 记录向量文件保存标识
                  header.labelMark = "";              //!< 记录向量文件运行标签

                  for (int i = 0; i < channelCount; ++i)
                  {
                      int nCh = 0;
                      header.channels.Append(nCh);   //!< 添加实际物理通道
                  }
                  header.Save(ds); //!< 保存文件头信息

                  int nFailLineCount = 0; //!< 获取错误行数
                  if (0 == nFailLineCount)
                  {
                      header.EndSave(ds, nFailCount);   //!< 结束保存信息
                      return;
                  }

                  // 计算通道使用的字节个数
                  int nChStatusByteCount = header.channels.Size() / 8;
                  if (0 < (header.channels.Size() % 8))
                  {
                      ++nChStatusByteCount;
                  }
                  char *pChStatus = new char[nChStatusByteCount];

                  for (int i = 0; i < nFailCount; ++i)
                  {
                      // 遍历错误行
                      int nLineOffset = 0; //!< 获取相对labelMark的偏移行
                      header.SetLabelOffsetRow(nCurLineNo); //!< 记录偏移行

                      for (int j = 0; j < nChStatusByteCount; ++j)
                      {
                          pChStatus[j] = 0; //!< 获取相关通道的状态整合成一个字节
                          header.SetChannelValidState(pChStatus[j]);  // 记录一次状态值
                      }
                      
                      header.SaveData(ds); //!< 写入到文件中
                  }

                  header.EndSave(ds, nFailCount);   //!< 结束保存信息
              }
              
 */
#pragma once

#include "STSCoreFx.h"


class ACVFailMapHeaderInfo
{
public:
    enum
    {
        CHMinRecordUnit = 8 //!< 通道最小记录单元可容纳8个通道信息
    };

    static const DWORD PageByteSize;

public:
    
    STSString         fileName;
    STSString         fileID;
    STSString         saveMark;
    STSString         labelMark;
    STSVector<int>    channels; //!< 实际通道

    

public:
    ACVFailMapHeaderInfo();
    ~ACVFailMapHeaderInfo();

    void Save(STSDataStream & ds);
    void Load(STSDataStream & ds);

    void SetLabelOffsetRow(int offsetRow){
        *m_pOffsetRow = offsetRow;
    }
    void SetChannelValidState(char c);

    void SaveData(STSDataStream & ds);

    void EndSave(STSDataStream & ds, int totalCount);


    bool GenerateChannelQuickSearchIndex();


    DWORD RecoredFileSize() const{
        return m_nRecordFileSize;
    }

    bool HasFailInfo() const{
        return (0 != m_nTotalCount);
    }
	bool HasFailInfo(int channel) const;
   
    int ChannelUnitCount() const{
        return m_nChUnitCount;
	}

    int Linage() const{
        return m_nLineCountPerPage;
    }
	
    int Page(int row) const{
       // return (row * m_nInfoStructSize / PageByteSize) + 1;
        return (row / m_nLineCountPerPage) + 1;
    }
    int PageStartLine(int page) const{
        return ((page - 1) * m_nLineCountPerPage);
    }

    DWORD ChannelDataSize() const{
        return m_nInfoDataSize;
    }
    DWORD InfoStructSize() const{
        return m_nInfoStructSize;
    }

    int GetChannelIndex(int channel) const;


    // Test
    bool CanRead(STSDataStream & ds);
    bool ReadOneLineData(STSDataStream & ds);
    int LabelOffsetRow() const{
        return *m_pOffsetRow;
    }
    char ChannelValidState(int index) const{
        return *(m_pChAddress + index);
    }

    int TotalCount() const{
        return m_nTotalCount;
    }
private:    
    void Calc(bool bCreateBuf);


private:
    int            m_nRecordFileSize;
    int            m_nTotalCount;
    int            m_nChCount;
    int            m_nChUnitCount;
    DWORD          m_nInfoStructSize;
    DWORD          m_nInfoDataSize;
    int            m_nLineCountPerPage;
    char          *m_dataBuf;
    int           *m_pOffsetRow;
    char          *m_pChAddress;
    int            m_nChCursor;    
    DWORD          m_nWriteByteSize;

	int            m_nFailMapVer;
	unsigned long  m_nRecordChFailCountOffsetInFile; // 记录通道失败个数在文件中的偏移量
	STSVector<int> m_vecChFailCount; //通道失败的个数, 记录的顺序与channels 一一对应

private:
    struct ChannelIndex
    {
        int channel;
        int index;
		int failCount; // [版本1] 该通道失效次数 
        ChannelIndex *next;
    };

    enum 
    {
        ChannelIndexBufSize = 256
    };

    STSVector<ChannelIndex *> m_vecChannelIndex;
};
