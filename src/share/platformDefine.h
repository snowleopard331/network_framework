/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/03/29
*/

#ifndef _PLATFORM_DEFINE_H_
#define _PLATFORM_DEFINE_H_

#include <boost/detail/endian.hpp>

#define EVIL_LITTLE_ENDIAN   0
#define EVIL_BIG_ENDIAN      1

#if !defined(EVIL_ENDIAN)
#   if defined(BOOST_LITTLE_ENDIAN)
#       define  EVIL_ENDIAN     EVIL_LITTLE_ENDIAN
#   elif defined(BOOST_BIG_ENDIAN)
#       define  EVIL_ENDIAN     EVIL_BIG_ENDIAN
#   else
#       error   "unable to determine system endianness"
#   endif
#endif//EVIL_ENDIAN

#endif//_PLATFORM_DEFINE_H_