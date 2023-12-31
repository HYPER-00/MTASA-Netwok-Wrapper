﻿#include "NetworkWrapper.h"
#include <cstring>

#if defined _WIN32
#define MTAEXPORT extern "C" __declspec(dllexport)
#else
#define MTAEXPORT extern "C" __attribute__ ((visibility ("default")))
#endif

MTAEXPORT short Setup(const char* szServerIdPath, const char* szNetLibPath, const char* szIP, unsigned short usPort,
    unsigned int uiPlayerCount, const char* szServerName, unsigned long* pulMtaVersionType, PyObject* pFunction)
{

    MTANetworkWrapper* pNetworkWrapper = new MTANetworkWrapper();
    if (!pNetworkWrapper->Setup(szServerIdPath, szNetLibPath, szIP, usPort, uiPlayerCount, szServerName, pulMtaVersionType, pFunction))
        return -1;
    Py_Initialize();

    return pNetworkWrapper->GetId();
}

MTAEXPORT void Start(unsigned short usID)
{
    auto wrapper = MTANetworkWrapper::GetNetWrapper(usID);
    MTANetworkWrapper::GetNetWrapper(usID)->Start();
}

MTAEXPORT PyObject* Test()
{
    return PyTuple_New(0);
}

MTAEXPORT PyObject* GetLastPackets(unsigned short usID)
{
    return MTANetworkWrapper::GetNetWrapper(usID)->GetLastPackets();
}

MTAEXPORT void Send(unsigned short usID, unsigned long ulAddress, unsigned int uiPacketId, unsigned short usBitStreamVersion, const char* szData, unsigned long ulDataSize, unsigned char ucPriority, unsigned char ucReliability)
{
    MTANetworkWrapper::GetNetWrapper(usID)->Send(ulAddress, uiPacketId, usBitStreamVersion, szData, ulDataSize, ucPriority, ucReliability);
}

MTAEXPORT bool IsValidSocket(unsigned short usID, unsigned long ulAddress)
{
    return MTANetworkWrapper::GetNetWrapper(usID)->IsValidSocket(ulAddress);
}

MTAEXPORT PlayerAddress GetPlayerAddress(unsigned short usID, unsigned long ulAddress)
{
    return MTANetworkWrapper::GetNetWrapper(usID)->GetPlayerAddress(ulAddress);
}

MTAEXPORT void SetClientBitStreamVersion(unsigned short usID, unsigned long address, unsigned short version)
{
    MTANetworkWrapper::GetNetWrapper(usID)->SetClientBitStreamVersion(address, version);
}

MTAEXPORT void SetAntiCheatChecks(unsigned short usID, const char* szDisableComboACMap, const char* szDisableACMap, const char* szEnableSDMap,
    int iEnableClientChecks, bool bHideAC, const char* szImgMods)
{
    MTANetworkWrapper::GetNetWrapper(usID)->SetAntiCheatChecks(szDisableComboACMap, szDisableACMap, szEnableSDMap, iEnableClientChecks, bHideAC, szImgMods);
}

MTAEXPORT void GetAntiCheatInfo(unsigned short usID, unsigned long address)
{
    MTANetworkWrapper::GetNetWrapper(usID)->GetAntiCheatInfo(address);
}

MTAEXPORT void GetClientData(unsigned short usID, unsigned long ulAddress, char* szSerial, char* szExtra, char* szvVersion)
{                                                       
    auto result = MTANetworkWrapper::GetNetWrapper(usID)->GetClientData(ulAddress);

    strcpy(szSerial, result.serial.c_str());
    strcpy(szExtra, result.extra.c_str());
    strcpy(szvVersion, result.version.c_str());
}

MTAEXPORT void GetModPackets(unsigned short usID, unsigned long address)
{
    MTANetworkWrapper::GetNetWrapper(usID)->GetModPackets(address);
}

MTAEXPORT const char* GetNetRoute(unsigned short usID)
{
    return MTANetworkWrapper::GetNetWrapper(usID)->GetNetRoute();
}

MTAEXPORT const char* GetPingStatus(unsigned short usID)
{
    return MTANetworkWrapper::GetNetWrapper(usID)->GetPingStatus();
}

MTAEXPORT SBandwidthStatistics GetBandwidthStatistics(unsigned short usID)
{
    return MTANetworkWrapper::GetNetWrapper(usID)->GetBandwidthStatistics();
}

MTAEXPORT SPacketStat GetPacketStat(unsigned short usID)
{
    return MTANetworkWrapper::GetNetWrapper(usID)->GetPacketStats();
}

MTAEXPORT void Stop(unsigned short usID)
{
    MTANetworkWrapper::GetNetWrapper(usID)->Stop();
}

MTAEXPORT void Destroy(unsigned short usID)
{
    MTANetworkWrapper* pNetworkWrapper = MTANetworkWrapper::GetNetWrapper(usID);
    pNetworkWrapper->Destroy();
    delete pNetworkWrapper;
}
