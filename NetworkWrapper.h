#include <iostream>
#include <sstream>

#include "MTAStuff/sdk/SharedUtil.h"
#include "MTAStuff/core/CDynamicLibrary.h"
#include "MTAStuff/sdk/SharedUtil.h"
#include "MTAStuff/sdk/net/Packets.h"
#include "MTAStuff/sdk/net/bitstream.h"
#include "MTAStuff/sdk/net/CNetServer.h"

#include "Utils.h"

#ifndef WIN32
#define __stdcall
#endif

typedef bool(__stdcall* PacketHandler)(unsigned char, unsigned long, const char*);
typedef bool(*PacketCallback)(unsigned short, PacketHandler, bool, bool);

struct PyPacket
{
	unsigned int  uiPacketIndex;
	unsigned int  uiPacketID;
	unsigned long ulPlayerBinaryAddress;
	const char*   szPacketBuffer;
};

struct Packet
{
	NetServerPlayerID player;
	unsigned char ucPacketID;
	NetBitStreamInterface* pBitStream;
	unsigned char ucPriority;
	unsigned char ucReliability;

	Packet(NetServerPlayerID socket, unsigned char packetId, NetBitStreamInterface* bitStream, unsigned char priority, unsigned char reliability)
		: player(socket), ucPacketID(packetId), pBitStream(bitStream), ucPriority(priority), ucReliability(reliability) {}
};

struct SerialExtraAndVersion
{
	std::string serial;
	std::string extra;
	std::string version;

	SerialExtraAndVersion(std::string serial, std::string extra, std::string version)
		: serial(serial), extra(extra), version(version) {}
};

class MTANetworkWrapper
{
private:
	unsigned short usID;
	static uint16_t nextId;
	static std::map<uint16_t, MTANetworkWrapper*> netWrappers;
	static std::map<NetServerPlayerID, MTANetworkWrapper*> netWrappersPerSocket;

	bool m_QueuedPacket = false;
	PacketHandler m_PacketHandler;
	CDynamicLibrary m_NetworkLibLoader;

	unsigned int  m_uiPacketIndex;
	unsigned int  m_uiPacket = 0;
	unsigned long m_ulPlayerListAddress = 0;
	const char*         m_szPacketBuffer;

	CNetServer* m_pNetwork;
	std::map<ulong, NetServerPlayerID> m_Players;
	bool m_bRunning;
	std::thread m_MainThread;
	std::thread m_PulseThread;
	std::thread m_PacketHandlerThread;
	std::queue<Packet> m_Packets;
	std::mutex mutex;

	void PulseLoop();
	void MainLoop();
public:
	MTANetworkWrapper();

	bool Setup(const char* szServerIdPath, const char* szNetLibPath, const char* szIP, unsigned short usPort, unsigned int uiPlayerCount,
		const char* szServerName, unsigned long* pulMtaVersionType);

	void Start();

	PyPacket GetLastPackets();

	void Send(unsigned long address, unsigned char packetId, unsigned short bitStreamVersion, const char* payload, unsigned long payloadSize, unsigned char priority, unsigned char reliability);

	void SetNetworkVersion(unsigned long address, unsigned short version);

	void SetAntiCheatChecks(const char* szDisableComboACMap, const char* szDisableACMap, const char* szEnableSDMap, int iEnableClientChecks, bool bHideAC, const char* szImgMods);

	void GetAntiCheatInfo(unsigned long address);

	SerialExtraAndVersion GetClientData(unsigned long address);

	void  GetModPackets(unsigned long address);

	const char* GetNetRoute();

	const char* GetPingStatus();

	SBandwidthStatistics GetBandwidthStatistics();
	SPacketStat GetPacketStats();

	void Stop();

	void Destroy();

	ushort GetId();
	bool StaticPacketHandler(unsigned char ucPacketID, const NetServerPlayerID& Socket, NetBitStreamInterface* pBitStream, SNetExtraInfo* pNetExtraInfo);
	bool IsValidSocket(NetServerPlayerID id);


	static MTANetworkWrapper* GetNetWrapper(int id);
	static MTANetworkWrapper* GetNetWrapper(NetServerPlayerID id);
};
