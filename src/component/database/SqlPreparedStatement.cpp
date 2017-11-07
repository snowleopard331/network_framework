/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/14
*/

#include "DatabaseEnv.h"

SqlStmtParameters::SqlStmtParameters(uint32 params)
{
    if(params > 0)
    {
        m_params.reserve(params);
    }
}

void SqlStmtParameters::reset(const SqlStatement& stmt)
{
    m_params.clear();
    // reserve memory if needed
    if(stmt.arguments() > 0)
    {
        m_params.reserve(stmt.arguments());
    }
}


//////////////////////////////////////////////////////////////////

SqlStatement& SqlStatement::operator =(const SqlStatement& index)
{
    if(this != &index)
    {
        m_index = index.m_index;
        m_pDB = index.m_pDB;

        SafeDelete(m_pParams);

        if(index.m_pParams)
        {
            m_pParams = new SqlStmtParameters(*(index.m_pParams));
        }
    }

    return *this;
}

bool SqlStatement::Execute()
{
    SqlStmtParameters* args = detach();

    if(args->boundParams() != arguments())
    {
        // TODO, "SQL ERROR: wrong amount of parameters (%i instead of %i)", args->boundParams(), arguments()
        LOG(ERROR)<<"SQL ERROR: wrong amount of parameters ("<<args->boundParams()<<" instead of "<<arguments()<<")";
        // TODO, "SQL ERROR: statement: %s", m_pDB->GetStmtString(ID()).c_str()
        LOG(ERROR)<<"SQL ERROR: statement: "<<m_pDB->GetStmtString(ID()).c_str();
        Evil_ASSERT(false);
        return false;
    }

    return m_pDB->ExecuteStmt(m_index, args);
}

bool SqlStatement::DirectExecute()
{
    SqlStmtParameters* args = detach();

    if(args->boundParams() != arguments())
    {
        // TODO, "SQL ERROR: wrong amount of parameters (%i instead of %i)", args->boundParams(), arguments()
        LOG(ERROR)<<"SQL ERROR: wrong amount of parameters ("<<args->boundParams()<<" instead of "<<arguments()<<")";
        // TODO, "SQL ERROR: statement: %s", m_pDB->GetStmtString(ID()).c_str()
        LOG(ERROR)<<"SQL ERROR: statement: "<<m_pDB->GetStmtString(ID()).c_str();
        Evil_ASSERT(false);
        return false;
    }

    return m_pDB->DirectExecuteStmt(m_index, args);
}


/////////////////////////////////////////////////////////

SqlPlainPreparedStatement::SqlPlainPreparedStatement(const std::string& fmt, SqlConnection& pConn)
    : SqlPreparedStatement(fmt, pConn)
{
    m_prepared = true;
    m_params = std::count(m_szFmt.begin(), m_szFmt.end(), '?');
    m_isQuery = strncasecmp(m_szFmt.c_str(), "select", 6) == 0;
}

void SqlPlainPreparedStatement::bind(const SqlStmtParameters& holder)
{
    if(m_params != holder.boundParams())
    {
        Evil_ASSERT(false);
        return;
    }

    // reset resulting plain SQL request
    m_szPlainRequest = m_szFmt;
    size_t nLastPos = 0;

    SqlStmtParameters::ParameterContainer const& _args = holder.params();

    SqlStmtParameters::ParameterContainer::const_iterator iter_last = _args.end();
    for(SqlStmtParameters::ParameterContainer::const_iterator iter = _args.begin(); iter != iter_last; ++iter)
    {
        // bind parameter
        const SqlStmtFieldData& data = *iter;

        std::ostringstream fmt;
        DataToString(data, fmt);

        nLastPos = m_szPlainRequest.find('?', nLastPos);
        if(nLastPos != std::string::npos)
        {
            std::string tmp = fmt.str();
            m_szPlainRequest.replace(nLastPos, 1, tmp);
            nLastPos += tmp.length();
        }
    }
}

bool SqlPlainPreparedStatement::execute()
{
    if(m_szPlainRequest.empty())
    {
        return false;
    }

    return m_pConn.Execute(m_szPlainRequest.c_str());
}

void SqlPlainPreparedStatement::DataToString(const SqlStmtFieldData& data, std::ostringstream& fmt)
{
    switch(data.type())
    {
    case FIELD_BOOL:
        {
            fmt<<"'"<<uint32(data.toBool())<<"'";
            break;
        }
    case FIELD_UI8:
        {
            fmt<<"'"<<uint32(data.toUInt8())<<"'";
            break;
        }
    case FIELD_I8:
        {
            fmt<<"'"<<uint32(data.toInt8())<<"'";
            break;
        }
    case FIELD_UI16:
        {
            fmt<<"'"<<uint32(data.toUInt16())<<"'";
            break;
        }
    case FIELD_I16:
        {
            fmt<<"'"<<uint32(data.toInt16())<<"'";
            break;
        }
    case FIELD_UI32:
        {
            fmt<<"'"<<data.toUInt32()<<"'";
            break;
        }
    case FIELD_I32:
        {
            fmt<<"'"<<data.toInt32()<<"'";
            break;
        }
    case FIELD_UI64:
        {
            fmt<<"'"<<data.toUInt64()<<"'";
            break;
        }
    case FIELD_I64:
        {
            fmt<<"'"<<data.toInt64()<<"'";
            break;
        }
    case FIELD_FLOAT:
        {
            fmt<<"'"<<data.toFloat()<<"'";
            break;
        }
    case FIELD_DOUBLE:
        {
            fmt<<"'"<<data.toDouble()<<"'";
            break;
        }
    case FIELD_STRING:
        {
            std::string tmp = data.toStr();
            m_pConn.DB().escape_string(tmp);
            fmt<<"'"<<tmp<<"'";
            break;
        }
    case FIELD_NONE:
    default:
        break;
    }
}

