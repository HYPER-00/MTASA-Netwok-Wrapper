#include "NetworkWrapper.h"

uint16_t MTANetworkWrapper::nextId;
std::map<uint16_t, MTANetworkWrapper*> MTANetworkWrapper::netWrappers;
std::map<NetServerPlayerID, MTANetworkWrapper*> MTANetworkWrapper::netWrappersPerSocket;

std::mutex mtx;
std::condition_variable cv;
std::queue<PacketHandler> messageQueue;

PacketHandler g_PacketHandler = nullptr;

bool staticPacketHandler(unsigned char ucPacketID, const NetServerPlayerID& Socket, NetBitStreamInterface* pBitStream, SNetExtraInfo* pNetExtraInfo)
{
    return MTANetworkWrapper::GetNetWrapper(Socket)->StaticPacketHandler(ucPacketID, Socket, pBitStream, pNetExtraInfo);
}

bool MTANetworkWrapper::StaticPacketHandler(unsigned char ucPacketID, const NetServerPlayerID& player, NetBitStreamInterface* pBitStream, SNetExtraInfo* pNetExtraInfo)
{
    m_Players[player.GetBinaryAddress()] = player;
    if (m_bRunning)
    {
        m_uiPacket = ucPacketID;
        m_ulPlayerListAddress = player.GetBinaryAddress();
        uint uiByteCount = pBitStream->GetNumberOfBytesUsed();

        char* szBuffer = new char[uiByteCount];
        printf("in\n");
        pBitStream->Read(szBuffer, uiByteCount);
        printf("out\n");

        m_szPacketBuffer = (const char*)szBuffer;
        m_uiPacketIndex++;

        //if (m_uiPacket == 3)
        //{
        //    printf("\n=============== C++ =======================\n[");
        //    for (int i = 0; i <= strlen(m_szPacketBuffer); i++)
        //    {
        //        printf("%x, ", (int)m_szPacketBuffer[i]);
        //    }
        //    printf("]\n");
        //}


        //bool hasPing = false;
        //unsigned int ping = 0;
        //if (pNetExtraInfo != nullptr && pNetExtraInfo->m_bHasPing) {
        //    hasPing = true;
        //    ping = pNetExtraInfo->m_uiPing;
        //}
        delete szBuffer;
    }
    return true;
}

MTANetworkWrapper::MTANetworkWrapper() {
    this->usID = MTANetworkWrapper::nextId++;
    MTANetworkWrapper::netWrappers[this->usID] = this;
}

bool MTANetworkWrapper::Setup(const char* szServerIdPath, const char* szNetLibPath, const char* szIP, unsigned short usPort,
    unsigned int uiPlayerCount, const char* szServerName, unsigned long* pulMtaVersionType)
{
    if (!m_NetworkLibLoader.Load(szNetLibPath))
    {
        std::stringstream description;
        description << "Couldn't Load Network Lib (Given Path: \"" << szNetLibPath << "\")";
        Utils::Error(description.str().c_str());
    }

    typedef unsigned long (*PFNCHECKCOMPATIBILITY)(unsigned long, unsigned long*);
    PFNCHECKCOMPATIBILITY pfnCheckCompatibility = reinterpret_cast<PFNCHECKCOMPATIBILITY>(m_NetworkLibLoader.GetProcedureAddress("CheckCompatibility"));

    if (!pfnCheckCompatibility || !pfnCheckCompatibility(0x0AB, pulMtaVersionType))
    {
        unsigned long ulNetModuleVersion = 0;
        if (pfnCheckCompatibility)
        {
            pfnCheckCompatibility(0x0AB, pulMtaVersionType);
            Utils::Error("Network module not compatible! (Expected 0x%x, got 0x%x)", 0x0AB, (unsigned int)ulNetModuleVersion);
        }
    }

    typedef CNetServer* (*InitNetServerInterface)();
    InitNetServerInterface pfnInitNetServerInterface = (InitNetServerInterface)(m_NetworkLibLoader.GetProcedureAddress("InitNetServerInterface"));
    if (!pfnInitNetServerInterface)
    {
        Utils::Error("Couldn't Find Network Library Initializer (Mabye Corrupted DLL Or An Old Version, Try to update your package)");
    }

    m_pNetwork = pfnInitNetServerInterface();

    if (!m_pNetwork->InitServerId(szServerIdPath))
    {
        Utils::Error("Couldn't Init Server Network With Server Id File");
    }

    m_pNetwork->RegisterPacketHandler(staticPacketHandler);
    if (!m_pNetwork->StartNetwork(szIP, usPort, uiPlayerCount, szServerName))
    {
        Utils::Error("Coudln't Start Network");
    }
    m_PulseThread = std::thread(&MTANetworkWrapper::PulseLoop, this);
    return true;
}

void MTANetworkWrapper::Start() {
    m_bRunning = true;
    m_PulseThread.join();
    m_MainThread = std::thread(&MTANetworkWrapper::MainLoop, this);
}

void MTANetworkWrapper::RegisterPacketHandler(PacketHandler packetHandler)
{
    g_PacketHandler = packetHandler;
    m_PacketHandler(0, 0, 0);
}

PyPacket MTANetworkWrapper::GetLastPackets()
{
    return { m_uiPacketIndex, m_uiPacket, m_ulPlayerListAddress, m_szPacketBuffer };
}

void MTANetworkWrapper::ListenerThread()
{
    if (m_PacketHandler != nullptr)
    {
        while (1)
        {
            printf("here\n");
            const char* szBuffer = const_cast<const char*>(m_szPacketBuffer);
            printf("szBuffer\n");
            if (g_PacketHandler(m_uiPacket, m_ulPlayerListAddress, "szBuffer"))
            {
                printf("here2\n");
                m_uiPacket = 0U;
                m_ulPlayerListAddress = 0UL;
            }
            printf("here3\n");
        }
    }
}

void MTANetworkWrapper::Send(unsigned long address, unsigned char packetId, unsigned short bitStreamVersion, const char* payload, unsigned long payloadSize, unsigned char priority, unsigned char reliability)
{
    NetBitStreamInterface* bitStream = m_pNetwork->AllocateNetServerBitStream(bitStreamVersion);
    if (bitStream)
    {
        bitStream->Write(payload, payloadSize);
        NetServerPlayerID& socket = m_Players[address];
        mutex.lock();
        m_Packets.push(Packet(socket, packetId, bitStream, priority, reliability));
        mutex.unlock();
    }
}

void MTANetworkWrapper::SetNetworkVersion(unsigned long address, unsigned short version)
{
    m_pNetwork->SetClientBitStreamVersion(m_Players[address], version);
}

void MTANetworkWrapper::SetAntiCheatChecks(const char* szDisableComboACMap, const char* szDisableACMap, const char* szEnableSDMap,
    int iEnableClientChecks, bool bHideAC, const char* szImgMods)
{
    this->m_pNetwork->SetChecks(szDisableComboACMap, szDisableACMap, szEnableSDMap, iEnableClientChecks, bHideAC, szImgMods);
}

void MTANetworkWrapper::GetAntiCheatInfo(unsigned long address)
{
    m_pNetwork->ResendACPackets(m_Players[address]);
}

SerialExtraAndVersion MTANetworkWrapper::GetClientData(unsigned long address)
{
    auto socket = m_Players[address];

    SFixedString<32> strSerialTemp;
    SFixedString<64> strExtraTemp;
    SFixedString<32> strPlayerVersionTemp;
    m_pNetwork->GetClientSerialAndVersion(socket, strSerialTemp, strExtraTemp, strPlayerVersionTemp);

    std::string serial = (std::string)SStringX(strSerialTemp);
    std::string extra = (std::string)SStringX(strExtraTemp);
    std::string version = (std::string)SStringX(strPlayerVersionTemp);

    SerialExtraAndVersion result = SerialExtraAndVersion(serial, extra, version);
    return result;
}

void MTANetworkWrapper::GetModPackets(unsigned long address)
{
    m_pNetwork->ResendModPackets(m_Players[address]);
}

const char* MTANetworkWrapper::GetNetRoute()
{
    if (m_bRunning)
    {
        SFixedString<32> pstrRoute;
        m_pNetwork->GetNetRoute(&pstrRoute);
        return (const char*)pstrRoute;
    }
    else
    {
        Utils::Error("Cannot Get Net Route And The Server Network didn't Started");
    }
}

const char* MTANetworkWrapper::GetPingStatus()
{
    if (m_bRunning)
    {
        SFixedString<32> pstrPing;
        m_pNetwork->GetPingStatus(&pstrPing);
        return (const char*)pstrPing;
    }
    else
    {
        Utils::Error("Cannot Get Net Route And The Server Network didn't Started");
    }
}

SBandwidthStatistics MTANetworkWrapper::GetBandwidthStatistics()
{
    if (m_bRunning)
    {
        SBandwidthStatistics liveStats;
        m_pNetwork->GetBandwidthStatistics(&liveStats);
        return liveStats;
    }
    else
    {
        Utils::Error("Cannot Get Bandwidth Statistics And The Server Network didn't Started");
    }
}

SPacketStat MTANetworkWrapper::GetPacketStats()
{
    if (m_bRunning)
    {
        SPacketStat stats;
        return *(m_pNetwork->GetPacketStats());
    }
    else
    {
        Utils::Error("Cannot Get Bandwidth Statistics And The Server Network didn't Started");
    }
}

void MTANetworkWrapper::PulseLoop()
{
    while (!m_bRunning)
    {
        m_pNetwork->DoPulse();
        m_pNetwork->GetHTTPDownloadManager(EDownloadMode::ASE)->ProcessQueuedFiles();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void MTANetworkWrapper::MainLoop()
{
    while (m_bRunning)
    {
        mutex.lock();
        while (!m_Packets.empty()) {
            Packet& entry = m_Packets.front();
            m_pNetwork->SendPacket(entry.ucPacketID, entry.player, entry.pBitStream, false, static_cast<NetServerPacketPriority>(entry.ucPriority), static_cast<NetServerPacketReliability>(entry.ucReliability));
            m_pNetwork->DeallocateNetServerBitStream(entry.pBitStream);
            m_Packets.pop();
        }
        mutex.unlock();

        m_pNetwork->DoPulse();
        m_pNetwork->GetHTTPDownloadManager(EDownloadMode::ASE)->ProcessQueuedFiles();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


void MTANetworkWrapper::Stop() {
    m_bRunning = false;
    m_MainThread.join();
    m_PulseThread = std::thread(&MTANetworkWrapper::PulseLoop, this);
}

void MTANetworkWrapper::Destroy()
{

    MTANetworkWrapper::netWrappers.erase(this->usID);

    for (auto kvPair : m_Players) {
        MTANetworkWrapper::netWrappersPerSocket.erase(kvPair.second);
    }
}

ushort MTANetworkWrapper::GetId() {
    return this->usID;
}

bool MTANetworkWrapper::IsValidSocket(NetServerPlayerID id)
{
    return this->m_pNetwork->IsValidSocket(id);
}

MTANetworkWrapper* MTANetworkWrapper::GetNetWrapper(int id)
{
    return MTANetworkWrapper::netWrappers[id];
}
MTANetworkWrapper* MTANetworkWrapper::GetNetWrapper(NetServerPlayerID id)
{
    if (MTANetworkWrapper::netWrappersPerSocket.find(id) == MTANetworkWrapper::netWrappersPerSocket.end()) {
        for (auto wrapper : netWrappers) {
            if (wrapper.second->IsValidSocket(id)) {
                MTANetworkWrapper::netWrappersPerSocket[id] = wrapper.second;
                return wrapper.second;
            }
        }
        return nullptr;
    }

    return MTANetworkWrapper::netWrappersPerSocket[id];
}