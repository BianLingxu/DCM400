#ifndef __19BC3C35_951E_41DC_A28A_E28DFEC74BE9_HDDOCTORRESULT_H__
#define __19BC3C35_951E_41DC_A28A_E28DFEC74BE9_HDDOCTORRESULT_H__

#include <vector>

class HDDoctorResultContrl
{
public:
    int  contrlIndex;
    int  pass; //!< -1��ʾΪ�����, 1 ��ʾͨ��, 0��ʾʧ��

public:
    HDDoctorResultContrl()
        : contrlIndex(-1)
        , pass(-1)
    {}
};

class HDDoctorResultBoard
{
public:
    int                                slotIndex;
    int                                pass; //!< -1��ʾΪ�����, 1 ��ʾͨ��, 0��ʾʧ��
    std::vector<HDDoctorResultContrl>  controlResult;
public:
    HDDoctorResultBoard()
        : slotIndex(-1)
        , pass(-1)
    {}
};

class HDDoctorResult
{
public:
    int                                passBoardCount;
    int                                failBoardCount;
    int                                passControlCount;
    int                                failControlCount;

    std::vector<HDDoctorResultBoard>   boardResult;

public:
    HDDoctorResult()
        : passBoardCount(0)
        , failBoardCount(0)
        , passControlCount(0)
        , failControlCount(0)
    {

    }
};

#endif // __19BC3C35_951E_41DC_A28A_E28DFEC74BE9_HDDOCTORRESULT_H__