/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/08
*/

#ifndef _SQL_PREPARED_STATEMENT_H_
#define _SQL_PREPARED_STATEMENT_H_

#include "Common.h"


class Database;
class SqlConnection;
class SqlStatement;


union SqlStmtField
{
    bool    boolean;
    uint8   ui8;
    int8    i8;
    uint16  ui16;
    int16   i16;
    uint32  ui32;
    int32   i32;
    uint64  ui64;
    int64   i64;
    float   f;
    double  d;
};

enum SqlStmtFieldType
{
    FIELD_BOOL,
    FIELD_UI8,
    FIELD_UI16,
    FIELD_UI32,
    FIELD_UI64,
    FIELD_I8,
    FIELD_I16,
    FIELD_I32,
    FIELD_I64,
    FIELD_FLOAT,
    FIELD_DOUBLE,
    FIELD_STRING,
    FIELD_NONE,
};

class SqlStmtFieldData
{
private:
    SqlStmtFieldType    m_type;
    SqlStmtField        m_binaryData;
    std::string         m_szStringData;

public:
    SqlStmtFieldData()
        : m_type(FIELD_NONE)
    {
        m_binaryData.ui64 = 0;
    }

    ~SqlStmtFieldData() 
    {

    }

    template<typename T>
    SqlStmtFieldData(T param)
    {
        set(param);
    }

    template<typename T1>
    void set(T1 param1);


    bool toBool() const 
    {
        Jovi_ASSERT(m_type == FIELD_BOOL);
        return static_cast<bool>(m_binaryData.ui8);
    }

    uint8 toUInt8() const
    {
        Jovi_ASSERT(m_type == FIELD_UI8);
        return m_binaryData.ui8;
    }

    int8 toInt8() const
    {
        Jovi_ASSERT(m_type == FIELD_I8);
        return m_binaryData.i8;
    }

    uint16 toUInt16() const
    {
        Jovi_ASSERT(m_type == FIELD_UI16);
        return m_binaryData.ui16;
    }

    int16 toInt16() const
    {
        Jovi_ASSERT(m_type == FIELD_I16);
        return m_binaryData.i16;
    }

    uint32 toUInt32() const
    {
        Jovi_ASSERT(m_type == FIELD_UI32);
        return m_binaryData.ui32;
    }

    int32 toInt32() const
    {
        Jovi_ASSERT(m_type == FIELD_I32);
        return m_binaryData.i32;
    }

    uint64 toUInt64() const
    {
        Jovi_ASSERT(m_type == FIELD_UI64);
        return m_binaryData.ui64;
    }

    int64 toInt64() const
    {
        Jovi_ASSERT(m_type == FIELD_I64);
        return m_binaryData.i64;
    }

    float toFloat() const
    {
        Jovi_ASSERT(m_type == FIELD_I8);
        return m_binaryData.f;
    }

    double toDouble() const
    {
        Jovi_ASSERT(m_type == FIELD_DOUBLE);
        return m_binaryData.d;
    }

    const char* toStr() const
    {
        Jovi_ASSERT(m_type == FIELD_STRING);
        return m_szStringData.c_str();
    }

    // get type of data
    SqlStmtFieldType type() const 
    {
        return m_type;
    }

    // get underlying buffer type
    void* buff() const 
    {
        return m_type == FIELD_STRING ? (void*)m_szStringData.c_str() : (void*)&m_binaryData;
    }

    // get size of data
    size_t size() const
    {
        switch(m_type)
        {
        case FIELD_NONE:
            return 0;
        case FIELD_BOOL:
        case FIELD_UI8:
            return sizeof(uint8);
        case FIELD_I8:
            return sizeof(int8);
        case FIELD_UI16:
            return sizeof(uint16);
        case FIELD_I16:
            return sizeof(int16);
        case FIELD_UI32:
            return sizeof(uint32);
        case FIELD_I32:
            return sizeof(int32);
        case FIELD_UI64:
            return sizeof(uint64);
        case FIELD_I64:
            return sizeof(int64);
        case FIELD_FLOAT:
            return sizeof(float);
        case FIELD_DOUBLE:
            return sizeof(double);
        case FIELD_STRING:
            return m_szStringData.length();
        default:
            LOG(ERROR)<<"unrecognized type of SqlStmtFieldType obtained";
            return 0;
        }
    }
};

// template specialization
/**
 * @brief
 *
 * @param val
 */
template<> inline void SqlStmtFieldData::set(bool val)
{
    m_type = FIELD_BOOL;
    m_binaryData.ui8 = val;
}

template<> inline void SqlStmtFieldData::set(uint8 val)
{
    m_type = FIELD_UI8;
    m_binaryData.ui8 = val;
}

template<> inline void SqlStmtFieldData::set(int8 val)
{
    m_type = FIELD_I8;
    m_binaryData.i8 = val;
}

template<> inline void SqlStmtFieldData::set(uint16 val)
{
    m_type = FIELD_UI16;
    m_binaryData.ui16 = val;
}

template<> inline void SqlStmtFieldData::set(int16 val)
{
    m_type = FIELD_I16;
    m_binaryData.i16 = val;
}

template<> inline void SqlStmtFieldData::set(uint32 val)
{
    m_type = FIELD_UI32;
    m_binaryData.ui32 = val;
}

template<> inline void SqlStmtFieldData::set(int32 val)
{
    m_type = FIELD_I32;
    m_binaryData.i32 = val;
}

template<> inline void SqlStmtFieldData::set(uint64 val)
{
    m_type = FIELD_UI64;
    m_binaryData.ui64 = val;
}

template<> inline void SqlStmtFieldData::set(int64 val)
{
    m_type = FIELD_I64;
    m_binaryData.i64 = val;
}

template<> inline void SqlStmtFieldData::set(float val)
{
    m_type = FIELD_FLOAT;
    m_binaryData.f = val;
}

template<> inline void SqlStmtFieldData::set(double val)
{
    m_type = FIELD_DOUBLE;
    m_binaryData.d = val;
}

template<> inline void SqlStmtFieldData::set(const char* val)
{
    m_type = FIELD_STRING;
    m_szStringData = val;
}


/// ---- PREPARED STATEMENT EXECUTOR ----

class SqlStmtParameters
{
public:
    typedef std::vector<SqlStmtFieldData>   ParameterContainer;

private:
    SqlStmtParameters& operator=(const SqlStmtParameters& obj);

    ParameterContainer m_params;    /**< statement parameter holder */

public:
    // reserve memory to contain all input parameters of stmt
    explicit SqlStmtParameters(uint32 params);

    ~SqlStmtParameters() 
    {

    }

    // get amount of bound parameters
    uint32 boundParams() const 
    {
        return m_params.size();
    }

    void addParam(const SqlStmtFieldData& data) 
    {
        m_params.push_back(data);
    }

    /**
    * @brief empty SQL statement parameters.
    *
    * In case nParams > 1 - reserve memory for parameters should help to
    * reuse the same object with batched SQL requests
    *
    * @param stmt
    */
    void reset(const SqlStatement& stmt);

    // swaps contents of internal param container
    void swap(SqlStmtParameters& obj);

    // get bound parameters
    const ParameterContainer& params() const 
    {
        return m_params;
    }
};


/**
 * @brief statement ID encapsulation logic
 *
 */
class SqlStatementID
{
private:
    friend class Database;

    int         m_index;
    uint32      m_arguments;
    bool        m_initialized;

public:
    SqlStatementID()
        : m_initialized(false)    
    {

    }

    int ID() const 
    {
        return m_index;
    }

    uint32 arguments() const 
    {
        return m_arguments;
    }

    bool initialized() const 
    {
        return m_initialized;
    }

private:
    void init(int id, uint32 args)
    {
        m_index = id;
        m_arguments = args;
        m_initialized = true;
    }
};


/**
 * @brief statement index
 *
 */
class SqlStatement
{
private:
    SqlStatementID      m_index;
    Database*           m_pDB;
    SqlStmtParameters*  m_pParams;

public:
    ~SqlStatement() 
    {
        delete m_pParams;
    }

    SqlStatement(const SqlStatement& index)
        : m_index(index.m_index)
        , m_pDB(index.m_pDB)
        , m_pParams(NULL)
    {
        if(index.m_pParams)
        {
            m_pParams = new SqlStmtParameters(*(index.m_pParams));
        }
    }

    SqlStatement& operator=(const SqlStatement& index);

    int ID() const  
    {
        return m_index.ID();
    }

    uint32 arguments() const 
    {
        return m_index.arguments();
    }

    bool Execute();

    bool DirectExecute();


    // templates to simplify 1-4 parameter bindings
    template<typename ParamType1, typename ParamType2, typename ParamType3, typename ParamType4>
    bool PExecute(ParamType1 param1, ParamType2 param2, ParamType3 param3, ParamType4 param4)
    {
        arg(param1);
        arg(param2);
        arg(param3);
        arg(param4);
        return Execute();
    }

    template<typename ParamType1, typename ParamType2, typename ParamType3>
    bool PExecute(ParamType1 param1, ParamType2 param2, ParamType3 param3)
    {
        arg(param1);
        arg(param2);
        arg(param3);
        return Execute();
    }

    template<typename ParamType1, typename ParamType2>
    bool PExecute(ParamType1 param1, ParamType2 param2)
    {
        arg(param1);
        arg(param2);
        return Execute();
    }

    template<typename ParamType1>
    bool PExecute(ParamType1 param1)
    {
        arg(param1);
        return Execute();
    }

    // bind parameters with specified type
    
    void addBool(bool val)  {arg(val);}
    void addUInt8(uint8 val) {arg(val);}
    void addInt8(int8 val)  {arg(val);}
    void addUInt16(uint16 val) {arg(val);}
    void addInt16(int16 val)  {arg(val);}
    void addUInt32(uint32 val) {arg(val);}
    void addInt32(int32 val)  {arg(val);}
    void addUInt64(uint64 val) {arg(val);}
    void addInt64(int64 val)  {arg(val);}
    void addFloat(float val)    {arg(val);}
    void addDouble(double val)  {arg(val);}
    void addString(std::ostringstream& ss)
    {
        arg(ss.str().c_str());
        ss.str(std::string());
    }

protected:
    // don't allow anyone except Database class to create static SqlStatement objects
    friend class Database;

    SqlStatement(const SqlStatementID& index, Database& db)
        : m_index(index)
        , m_pDB(&db)
        , m_pParams(NULL)   
    {

    }

private:
    
    SqlStmtParameters* get()
    {
        if(!m_pParams)
        {
            m_pParams = new SqlStmtParameters(arguments());
        }

        return m_pParams;
    }

    SqlStmtParameters* detach()
    {
        SqlStmtParameters* p = m_pParams ? m_pParams : new SqlStmtParameters(0);
        m_pParams = NULL;
        return p;
    }


    // helper function
    // use appropriate add* functions to bind specific data type
    template<typename ParamType>
    void arg(ParamType val)
    {
        SqlStmtParameters* p = get();
        p->addParam(SqlStmtFieldData(val));
    }

};


class SqlPreparedStatement
{
protected:
    SqlPreparedStatement(const std::string& fmt, SqlConnection& pConn)
        : m_params(0)
        , m_columns(0)
        , m_isQuery(false)
        , m_prepared(false)
        , m_szFmt(fmt)
        , m_pConn(pConn)    {}
    
    uint32          m_params;
    uint32          m_columns;
    bool            m_isQuery;
    bool            m_prepared;
    std::string     m_szFmt;
    SqlConnection&  m_pConn;

public:
    virtual ~SqlPreparedStatement() 
    {

    }

    bool isPrepared() const 
    {
        return m_prepared;
    }

    bool isQuery() const 
    {
        return m_isQuery;
    }

    uint32 params() const 
    {
        return m_params;
    }

    uint32 columns() const 
    {
        return isQuery() ? m_columns : 0;
    }

    /**
    * @brief initialize internal structures of prepared statement
    *
    * upon success m_bPrepared should be true
    *
    * @return bool
    */
    virtual bool prepare() = 0;

    /**
    * @brief bind parameters for prepared statement from parameter placeholder
    *
    * @param holder
    */
    virtual void bind(const SqlStmtParameters& holder) = 0;

    /**
    * @brief execute statement w/o result set
    *
    * @return bool
    */
    virtual bool execute() = 0;
};


/**
 * @brief prepared statements via plain SQL string requests
 *
 */
class SqlPlainPreparedStatement
    : public SqlPreparedStatement
{
public:
    SqlPlainPreparedStatement(const std::string& fmt, SqlConnection& pConn);

    ~SqlPlainPreparedStatement() {}

    // this statement is always prepared
    virtual bool prepare() override {return true;}

    // we should replace all '?' symbols with substrings with proper format
    virtual void bind(const SqlStmtParameters& holder) override;

    virtual bool execute() override;

protected:
    void DataToString(const SqlStmtFieldData& data, std::ostringstream& fmt);

    std::string m_szPlainRequest;
};


#endif//_SQL_PREPARED_STATEMENT_H_