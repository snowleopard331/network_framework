/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/14
*/

#ifndef _DATABASE_ENV_H_
#define _DATABASE_ENV_H_

#include "Common.h"
#include "Field.h"
#include "QueryResult.h"
#include "QueryNamedResult.h"

#include "Database.h"
#include "DatabaseMysql.h"
#include "QueryResultMysql.h"



typedef DatabaseMysql DatabaseType;  // diff database interface

#endif//_DATABASE_ENV_H_