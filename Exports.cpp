#include "NetworkWrapper.h"

#if defined _WIN32
#define MTAEXPORT extern "C" __declspec(dllexport)
#else
#define MTAEXPORT extern "C" __attribute__ ((visibility ("default")))
#endif

MTAEXPORT short Setup(const char* szServerIdPath, const char* szNetLibPath, const char* szIP, unsigned short usPort,
    unsigned int uiPlayerCount, const char* szServerName, unsigned long* pulMtaVersionType)
{
    auto wrapper = new MTANetworkWrapper();
    if (!wrapper->Setup(szServerIdPath, szNetLibPath, szIP, usPort, uiPlayerCount, szServerName, pulMtaVersionType))
    {
        return -1;
    }
    return wrapper->GetId();
}

MTAEXPORT void Start(unsigned short usID)
{
    auto wrapper = MTANetworkWrapper::GetNetWrapper(usID);
    MTANetworkWrapper::GetNetWrapper(usID)->Start();
}

MTAEXPORT void RegisterPacketHandler(unsigned short usID, PacketHandler packetHandler)
{
    MTANetworkWrapper::GetNetWrapper(usID)->RegisterPacketHandler(packetHandler);
}

MTAEXPORT void RegisterPacketQueue(unsigned short usID, PyPacket* pPackets)
{
    printf("Called\n");
    std::cout << "EXPORTS: PacketQueue Size: " << sizeof pPackets << "\n";
    pPackets[0].ucPacketID *= 5;

    MTANetworkWrapper::GetNetWrapper(usID)->RegisterPacketQueue(pPackets);
}

MTAEXPORT bool StartListening(ushort usId, PacketHandler packetHandler)
{
    return MTANetworkWrapper::GetNetWrapper(usId)->StartListening(packetHandler);
}

MTAEXPORT void Send(unsigned short usID, unsigned long address, unsigned int packetId, unsigned short bitStreamVersion, const char* payload, unsigned long payloadSize, unsigned char priority, unsigned char reliability)
{
    MTANetworkWrapper::GetNetWrapper(usID)->Send(address, packetId, bitStreamVersion, payload, payloadSize, priority, reliability);
}

MTAEXPORT void SetNetworkVersion(unsigned short usID, unsigned long address, unsigned short version)
{
    MTANetworkWrapper::GetNetWrapper(usID)->SetNetworkVersion(address, version);
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

MTAEXPORT void GetClientData(unsigned short usID, unsigned long address, char* serial, char* extra, char* version)
{                                                       
    auto result = MTANetworkWrapper::GetNetWrapper(usID)->GetClientData(address);

    strcpy(serial, result.serial.c_str());
    strcpy(extra, result.extra.c_str());
    strcpy(version, result.version.c_str());
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

MTAEXPORT void Stop(unsigned short usID) {
    MTANetworkWrapper::GetNetWrapper(usID)->Stop();
}

MTAEXPORT void Destroy(unsigned short usID)
{
    auto wrapper = MTANetworkWrapper::GetNetWrapper(usID);
    wrapper->Destroy();
    delete wrapper;
}
