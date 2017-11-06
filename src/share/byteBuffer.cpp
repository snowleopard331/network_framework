/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/18
*/

#include "byteBuffer.h"
#include "glogHelper.h"
#include "StackTrace.h"

void ByteBufferException::printPosError() const
{
    StackTrace trace;

    LOG(ERROR)<<"Attempted to"<<(m_isAdd ? "put" : "get")
        <<"in ByteBuffer (pos: " <<m_pos
        <<" size: "<<m_size
        <<"value with size: "<<m_esize;
    LOG(ERROR)<<trace.c_str();
}