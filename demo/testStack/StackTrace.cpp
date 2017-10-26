/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/09
*/

#include "StackTrace.h"

// linux
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>

const char StackTrace::UNSUPPORTED[] = "<stack traces unsupported platform>";
const char StackTrace::UNABLE_TO_GET_TRACE[] = "<unable to get trace>";

StackTrace::StackTrace(ssize_t starting_frame_offset /* = 0 */, size_t num_frames /* = 0 */)
    : m_buflen(0)
{
    // cannot initialize arrays, so we must assign.
    this->m_buf[0] = '\0';
    this->genetate_tace(starting_frame_offset, num_frames);
}

const char* StackTrace::c_str() const
{
    return this->m_buf;
}

static inline size_t determine_starting_frame(ssize_t initial_frame, ssize_t offset)
{
    return MAX(initial_frame + offset, static_cast<ssize_t>(0));
}

// about platforms linux
void StackTrace::genetate_tace(ssize_t starting_frame_offset, size_t num_frames)
{
    const size_t    MAX_FRAMES = 128;
    const ssize_t   INITIAL_FRAME = 3;

    void*   stack[MAX_FRAMES];
    size_t  stack_size = 0;
    char**  stack_syms;

    if(0 == num_frames)
    {
        num_frames = MAX_FRAMES;
    }

    size_t starting_frame = determine_starting_frame(INITIAL_FRAME, starting_frame_offset);

    /**
    * @brief 
    *
    * @param stack, the buffer of getting stack info
    * @return size_t, the depth of stack info
    */
    stack_size = ::backtrace(stack, sizeof(stack)/sizeof(stack[0]));
    if(stack_size)
    {
        stack_syms = ::backtrace_symbols(stack, stack_size);

        for(size_t i = starting_frame; i < stack_size && num_frames > 0; ++i, --num_frames)
        {
            char* symp = &stack_syms[i][0];
            while(this->m_buflen < SYMBUFSIZ - 2 && *symp != '\0')
            {
                this->m_buf[this->m_buflen++] = *symp++;
            }
            this->m_buf[this->m_buflen++] = '\n';
        }
        this->m_buf[this->m_buflen] = '\0';

        ::free(stack_syms);
    }
    else
    {
        strncpy(&this->m_buf[0], UNABLE_TO_GET_TRACE, sizeof(UNABLE_TO_GET_TRACE));
    }
}



