#pragma once
#include "../MTAStuff/sdk/net/CNetServer.h"
#include "../MTAStuff/sdk/net/Packets.h"
#include <Python.h>

PyObject* CPlayerJoinDataPacket(NetBitStreamInterface& BitStream);