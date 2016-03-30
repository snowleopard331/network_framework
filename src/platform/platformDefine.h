/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/03/29
*/

#ifndef _PLATFORM_DEFINE_H_
#define _PLATFORM_DEFINE_H_

#include <boost/detail/endian.hpp>

#define JOVI_LITTLE_ENDIAN   0
#define JOVI_BIG_ENDIAN      1

#if !defined(JOVI_ENDIAN)
#   if defined(BOOST_LITTLE_ENDIAN)
#       define  JOVI_ENDIAN     JOVI_LITTLE_ENDIAN
#   elif defined(BOOST_BIG_ENDIAN)
#       define  JOVI_ENDIAN     JOVI_BIG_ENDIAN
#   else
#       error   "unable to determine system endianness"
#   endif
#endif//JOVI_ENDIAN

#endif//_PLATFORM_DEFINE_H_