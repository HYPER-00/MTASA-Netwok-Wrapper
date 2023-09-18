#include "PlayerJoinDataPacket.h"
#include <iostream>

PyObject* CPlayerJoinDataPacket(NetBitStreamInterface& BitStream)
{
    PyObject* pTuple = PyTuple_New(8);


    unsigned short m_usNetVersion;
    unsigned short m_usMTAVersion;
    unsigned short m_usBitStreamVersion;
    unsigned char  m_ucGameVersion;
    bool           m_bOptionalUpdateInfoRequired;
    SString        m_strNick;
    MD5            m_Password;
    SString        m_strSerialUser;
    CMtaVersion    m_strPlayerVersion;

    printf("Called GET Function\n");

    if (!BitStream.Read(m_usNetVersion) || !BitStream.Read(m_usMTAVersion))
        return nullptr;

    if (!BitStream.Read(m_usBitStreamVersion))
        return nullptr;

    BitStream.ReadString(m_strPlayerVersion);

    m_bOptionalUpdateInfoRequired = BitStream.ReadBit();

    if (BitStream.Read(m_ucGameVersion) && BitStream.ReadStringCharacters(m_strNick, MAX_PLAYER_NICK_LENGTH) &&
        BitStream.Read(reinterpret_cast<char*>(&m_Password), 16) && BitStream.ReadStringCharacters(m_strSerialUser, MAX_SERIAL_LENGTH))
    {
        m_strNick = *m_strNick;
        m_strSerialUser = *m_strSerialUser;

    }
    const char* szPassword = reinterpret_cast<const char*>(&m_Password.data);

    printf("Writing Tuple...\n");
    PyTuple_SetItem(pTuple, 0, PyLong_FromLong(m_usNetVersion));
    PyTuple_SetItem(pTuple, 1, PyLong_FromLong(m_usMTAVersion));
    PyTuple_SetItem(pTuple, 2, PyLong_FromLong(m_usBitStreamVersion));
    PyTuple_SetItem(pTuple, 3, PyLong_FromLong(m_ucGameVersion));
    PyTuple_SetItem(pTuple, 4, m_bOptionalUpdateInfoRequired ? Py_True : Py_False);
    PyTuple_SetItem(pTuple, 5, PyUnicode_DecodeUTF8(m_strNick.c_str(), m_strNick.length(), "strict"));
    PyTuple_SetItem(pTuple, 6, PyUnicode_DecodeUTF8(szPassword, strlen(szPassword), "strict"));
    PyTuple_SetItem(pTuple, 7, PyUnicode_DecodeUTF8(m_strSerialUser.c_str(), m_strSerialUser.length(), "strict"));
    PyTuple_SetItem(pTuple, 8, PyLong_FromLong(m_usMTAVersion));

    printf("Finaly Tuple has been Writed\n");

    return pTuple;
}
