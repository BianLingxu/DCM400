/*!
 * @file      ACVFailMapHeaderInfo.h
 *
 * Copyright (C) xg
 *
 * @author    xg
 * @date      2017/09/13
 * @version   v 1.0.0.0
 * @brief     
              1. ��Ϣ��¼��ʽ : ��Ա�ǩƫ����(int) ͨ����Ԫ�Ƿ���Ч��bitλ��ʶ(��С��Ԫchar) ������Ϣ�ṹ��С
                 ���� ͨ������13�����label��ǩ7��, ��2��8, 1ͨ��ʧЧ ��ô��¼��Ϣ����(���Կո�) (14 - 16û��ͨ��ʧЧֵĬ�ϸ�0)
                 int  char      char
                 7    0100 0001 0000 0010 0000
              2. �ļ���¼��Ϣͷ��СΪϵͳ�ṩ��ҳ��Сֵ(64k)
              3. ��ÿҳ���ʣ����������ͨ����Ϣ���ݳ���ʱ��ʣ��������0,��ô��ͨ����Ϣ���ݴ���һҳ��ʼд��

          ����
              1. ÿ8��ͨ��Ϊ1һ����Ԫ,����8��ͨ��,bitλ��0
                 ����,��������ʾ7��ͨ��, 
                 ͨ��ֵ��     : 0   1   0   0    0   0  1
                 ͨ��˳����   : 15, 17, 14, 12,  55, 7, 8
                 bit����˳����:  x, 8,  7,  55,  12, 14, 17, 15
                 bitֵ��:        0, 1,  0,   0,   0,  0,  1,  0   


		      2. 2019/03/29 
			     0ҳ��¼����ͷ����Ϣ,�ӵ�1ҳ��ʼ��¼������Ϣ 
                 


              ʹ������:
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
                  header.fileName = "vectorFileName"; //!< ��¼�����ļ���
                  header.fileID = "";                 //!< ��¼�����ļ�ID
                  header.saveMark = "";               //!< ��¼�����ļ������ʶ
                  header.labelMark = "";              //!< ��¼�����ļ����б�ǩ

                  for (int i = 0; i < channelCount; ++i)
                  {
                      int nCh = 0;
                      header.channels.Append(nCh);   //!< ���ʵ������ͨ��
                  }
                  header.Save(ds); //!< �����ļ�ͷ��Ϣ

                  int nFailLineCount = 0; //!< ��ȡ��������
                  if (0 == nFailLineCount)
                  {
                      header.EndSave(ds, nFailCount);   //!< ����������Ϣ
                      return;
                  }

                  // ����ͨ��ʹ�õ��ֽڸ���
                  int nChStatusByteCount = header.channels.Size() / 8;
                  if (0 < (header.channels.Size() % 8))
                  {
                      ++nChStatusByteCount;
                  }
                  char *pChStatus = new char[nChStatusByteCount];

                  for (int i = 0; i < nFailCount; ++i)
                  {
                      // ����������
                      int nLineOffset = 0; //!< ��ȡ���labelMark��ƫ����
                      header.SetLabelOffsetRow(nCurLineNo); //!< ��¼ƫ����

                      for (int j = 0; j < nChStatusByteCount; ++j)
                      {
                          pChStatus[j] = 0; //!< ��ȡ���ͨ����״̬���ϳ�һ���ֽ�
                          header.SetChannelValidState(pChStatus[j]);  // ��¼һ��״ֵ̬
                      }
                      
                      header.SaveData(ds); //!< д�뵽�ļ���
                  }

                  header.EndSave(ds, nFailCount);   //!< ����������Ϣ
              }
              
 */
#pragma once

#include "STSCoreFx.h"


class ACVFailMapHeaderInfo
{
public:
    enum
    {
        CHMinRecordUnit = 8 //!< ͨ����С��¼��Ԫ������8��ͨ����Ϣ
    };

    static const DWORD PageByteSize;

public:
    
    STSString         fileName;
    STSString         fileID;
    STSString         saveMark;
    STSString         labelMark;
    STSVector<int>    channels; //!< ʵ��ͨ��

    

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
	unsigned long  m_nRecordChFailCountOffsetInFile; // ��¼ͨ��ʧ�ܸ������ļ��е�ƫ����
	STSVector<int> m_vecChFailCount; //ͨ��ʧ�ܵĸ���, ��¼��˳����channels һһ��Ӧ

private:
    struct ChannelIndex
    {
        int channel;
        int index;
		int failCount; // [�汾1] ��ͨ��ʧЧ���� 
        ChannelIndex *next;
    };

    enum 
    {
        ChannelIndexBufSize = 256
    };

    STSVector<ChannelIndex *> m_vecChannelIndex;
};
