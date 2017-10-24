// CAENv1718.cpp
// 2014.10.22
// T. Yamaga
// 2017.10.8
// modifiled by Hidemitsu Asano

#include "CAENv1718.h"

CAENv1718::CAENv1718()
{
}

CAENv1718::CAENv1718(CVBoardTypes BdType, short Link, short BdNum, int32_t* pHandle)
{
    this->BdType = BdType;
    this->Link = Link;
    this->BdNum = BdNum;
    this->pHandle = pHandle;
}

CAENv1718::~CAENv1718()
{
}

CVErrorCodes CAENv1718::Initialize()
{
    CVErrorCodes ret;
    ret = CAENVME_Init(BdType,Link,BdNum,pHandle);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::Initialize]\n" << std::endl;

    SetBusy(1);
    return ret;
}

CVErrorCodes CAENv1718::Finalize()
{
    SetBusy(1);

    CVErrorCodes ret;
    ret = CAENVME_End(*pHandle);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::Finalize]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::IRQCheck(CAEN_BYTE* pMask)
{
    CVErrorCodes ret;
    ret = CAENVME_IRQCheck(*pHandle,pMask);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::IRQCheck]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::SetPuls(CVPulserFreq Freq)
{
    SetOutputConf(cvOutput0,cvDirect,cvActiveHigh,cvMiscSignals);

    CVErrorCodes ret;
    switch(Freq){
        case cvPulser1200 :
            ret = SetPulserConf(cvPulserA,2,1,cvUnit410us);
            break;
        case cvPulser3000 :
            ret = SetPulserConf(cvPulserA,208,1,cvUnit1600ns);
            break;
        case cvPulser4000 :
            ret = SetPulserConf(cvPulserA,156,1,cvUnit1600ns);
            break;
        case cvPulser8000 :
            ret = SetPulserConf(cvPulserA,78,1,cvUnit1600ns);
            break;
        case cvPulser10k :
            ret = SetPulserConf(cvPulserA,62,1,cvUnit1600ns);
            break;
        default :
            break;
    }
    return ret;
}

//Asano memo
//Period: The period of the pulse in time units.
//Width : The width of the pulse in time unitis

CVErrorCodes CAENv1718::SetPulserConf(CVPulserSelect PulSel,
        unsigned char Period, unsigned char Width, CVTimeUnits Unit,
        unsigned char PulseNo, CVIOSources Start, CVIOSources Reset)
{
    CVErrorCodes ret;
    ret = CAENVME_SetPulserConf(*pHandle,PulSel,Period,Width,Unit,PulseNo,Start,Reset);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::SetPulserConf]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::SetOutputConf(CVOutputSelect OutSel,
        CVIOPolarity OutPol, CVLEDPolarity LEDPol, CVIOSources Source)
{
    CVErrorCodes ret;
    ret = CAENVME_SetOutputConf(*pHandle,OutSel,OutPol,LEDPol,Source);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::SetOutputConf]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::SetOutputRegister(unsigned short Mask)
{
    CVErrorCodes ret;
    ret = CAENVME_SetOutputRegister(*pHandle,Mask);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::SetOutputRegister]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::ClearOutputRegister(unsigned short Mask)
{
    CVErrorCodes ret;
    ret = CAENVME_ClearOutputRegister(*pHandle,Mask);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::ClearOutputRegister]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::SystemReset()
{
    CVErrorCodes ret;
    ret = CAENVME_SystemReset(*pHandle);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::SystemReset]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::StartPulser(CVPulserSelect PulSel)
{
    CVErrorCodes ret;
    ret = CAENVME_StartPulser(*pHandle,PulSel);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::StartPulser]\n" << std::endl;

    return ret;
}

//added by H.Asano
CVErrorCodes CAENv1718::OnePulse(CVPulserSelect PulSel)
{
    SetOutputConf(cvOutput0,cvDirect,cvActiveHigh,cvMiscSignals);
    
    CVErrorCodes ret;
    ret = SetPulserConf(cvPulserA,1,1,cvUnit25ns,1);
    ret = CAENVME_StartPulser(*pHandle,PulSel);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::OnePulse]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::StopPulser(CVPulserSelect PulSel)
{
    CVErrorCodes ret;
    ret = CAENVME_StopPulser(*pHandle,PulSel);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::StopPulser]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::IRQWait(uint32_t Mask, uint32_t Timeout)
{
    CVErrorCodes ret;
    ret = CAENVME_IRQWait(*pHandle,Mask,Timeout);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::IRQWait]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::ReadCycle(uint32_t Address, void* pData,
        CVAddressModifier AM, CVDataWidth DW)
{
    //printf("Address : %08X\n", Address);

    CVErrorCodes ret;
    ret = CAENVME_ReadCycle(*pHandle,Address,pData,AM,DW);
    if(ret == cvCommError)
        ret = CAENVME_ReadCycle(*pHandle,Address,pData,AM,DW);
    if(ret != cvSuccess) {
        std::cout << "\n\n Error !!! [CAENv1718::ReadCycle]\n" << std::endl;
    }
    return ret;
}

CVErrorCodes CAENv1718::WriteCycle(uint32_t Address, void* pData,
        CVAddressModifier AM, CVDataWidth DW)
{
    CVErrorCodes ret;
    ret = CAENVME_WriteCycle(*pHandle,Address,pData,AM,DW);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::WriteCycle]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::BLTReadCycle(uint32_t Address, void* Buffer
        ,int Size, CVAddressModifier AM, CVDataWidth DW)
{
    int nb;
    CVErrorCodes ret;
    ret = CAENVME_BLTReadCycle(*pHandle,Address,(char *)Buffer,Size,AM,DW,&nb);
    if(ret != cvSuccess) {
        std::cout << "\n\n Error !!! [CAENv1718::BLTReadCycle]\n" << std::endl;
        std::cout << "Error code : " << ret << std::endl;
    }

    return ret;
}

CVErrorCodes CAENv1718::BLTWriteCycle(uint32_t Address, void* pBuffer
        ,int Size, CVAddressModifier AM, CVDataWidth DW)
{
    int nb;
    CVErrorCodes ret;
    ret = CAENVME_BLTWriteCycle(*pHandle,Address,(char *)pBuffer,Size,AM,DW,&nb);
    if(ret != cvSuccess)
        std::cout << "\n\n Error !!! [CAENv1718::BLTWriteCycle]\n" << std::endl;

    return ret;
}

CVErrorCodes CAENv1718::SetBusy(int i)
{
    SetOutputConf(cvOutput1,cvDirect,cvActiveHigh,cvManualSW);
    switch(i){
        case 0 : 
            ClearOutputRegister(cvOut1Bit);
            break;
        case 1 :
            SetOutputRegister(cvOut1Bit);
            break;
        default :
            break;
    }
}

