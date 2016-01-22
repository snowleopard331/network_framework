/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/18
*/

#ifndef _WORLD_PACKET_H_
#define _WORLD_PACKET_H_

#include "opCodes.h"
#include "byteBuffer.h"

class WorldPacket
    : public ByteBuffer
{
    OpCodes         m_opCode;

public:

    WorldPacket()
        : ByteBuffer(0)
        , m_opCode(MSG_NULL_ACTION)
    {

    }

    explicit WorldPacket(OpCodes opcode, size_t res = 200)
        : ByteBuffer(res)
        , m_opCode(opcode)
    {

    }

    WorldPacket(const WorldPacket& packet)
        : ByteBuffer(packet)
        , m_opCode(packet.m_opCode)
    {

    }

    void initialize(OpCodes opcode, size_t newres = 200)
    {
        clear();
        _storage.reserve(newres);
        m_opCode = opcode;
    }

    OpCodes getOpcode() const
    {
        return m_opCode;
    }

    void setOpcode(OpCodes opcode)
    {
        m_opCode = opcode;
    }

    // ?
    /*inline const char* getOpcodeName() const
    {

    }*/
};

#endif//_WORLD_PACKET_H_