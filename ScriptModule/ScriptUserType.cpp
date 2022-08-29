
#include "ScriptUserType.h"
#include "ScriptExporter.h"

#include <OABase/StringUtility.h>


ScriptUserType::ScriptUserType(ScriptExporter* pExporter)
    : m_pExporter(pExporter)
{

}

ScriptUserType::~ScriptUserType()
{

}

void ScriptUserType::ExportUserTypeToSOL(ScriptExporter* pExporter)
{
    ScriptVariableType::ExportVariableTypeToSOL(pExporter);
    ScriptMethodType::ExportMethodTypeToSOL(pExporter);
}

std::unique_ptr<ScriptUserType> ScriptUserType::Create(const ScriptNodeStruct& scriptNodeStruct, ScriptExporter* pExporter)
{
    std::unique_ptr<ScriptUserType> pRet;

    std::string exportName = OA::StringUtility::Utf16ToUtf8(scriptNodeStruct.replacedString);

    if (scriptNodeStruct.nodeType == ScriptNodeType::Variable)
    {
        pRet = ScriptVariableType::CreateVariable(exportName, scriptNodeStruct, pExporter);        
    }
    else if (scriptNodeStruct.nodeType == ScriptNodeType::Method)
    {
        pRet = std::make_unique<ScriptMethodType>(exportName, pExporter);
    }
    else if (scriptNodeStruct.nodeType == ScriptNodeType::Object)
    {
        pRet = std::make_unique<ScriptObjectType>(exportName, pExporter);
    }

    return pRet;
}



ScriptVariableType::ScriptVariableType(const std::string& exportName, ScriptExporter* pExporter)
    : ScriptUserType(pExporter)
{
    m_pExporter->ExportObject(exportName, this);
}

ScriptNodeType ScriptVariableType::GetType() const
{
    return ScriptNodeType::Variable;
}

OA::OAUInt64 ScriptVariableType::GetQuality() const
{
    sol::state* lua = m_pExporter->SolState();
    if ((*lua)["quality"] == 0)
    {
        //return OA_StatusCode_Good;
    }
    else
    {
        //OA::OAUInt64 statuscode = (*lua)["quality"];
        //return  OA_StatusCode_Bad;
    }

    return m_statusCode;
}

OA::OAInt64 ScriptVariableType::GetTimestampInterval() const
{
    sol::state* lua = m_pExporter->SolState();
    //return (*lua)["timestamp"];
    return m_timestamp;
}

OA::OAString ScriptVariableType::GetKey() const
{
    //return _OAText("");
    return m_key;
}

OA::OAUInt16 ScriptVariableType::GetDataTypeId() const
{
    return m_dataTypeId;
}

bool ScriptVariableType::IsOutput() const
{
    return m_bIsOutput;
}

void ScriptVariableType::SetInitValue(const OA::OAVariant& value)
{
    m_value = value;
}

void ScriptVariableType::SetInitStatusCode(const OA::OAUInt64& initStatusCode)
{
    m_statusCode = initStatusCode;
}

void ScriptVariableType::SetInitTimestamp(const OA::OAInt64& initTimestamp)
{
    m_timestamp = initTimestamp;
}

void ScriptVariableType::SetDataTypeId(const OA::OAUInt16& dataTypeId)
{
    m_dataTypeId = dataTypeId;
}

void ScriptVariableType::SetOutput(bool isOutput)
{
    m_bIsOutput = isOutput;
}

void ScriptVariableType::UpdateWithSOL(const sol::object& val, OA::OAUInt64 statusCode, OA::OAInt64 timestamp)
{
    // Update value, quality, timestamp, default quality = good(0); timestamp = now
    sol::state* lua = m_pExporter->SolState();

    //(*lua)["quality"] = statusCode;
    m_statusCode = statusCode;

    //(*lua)["timestamp"] = timestamp;
    m_timestamp = timestamp;

    sol::type type = val.get_type();
    switch (type)
    {
    case sol::type::none:
        break;
    case sol::type::lua_nil:
        break;
    case sol::type::string:
        break;
    case sol::type::number:
        (*lua)["value"] = val.as<int>();
        m_value = val.as<int>();       
        break;
    case sol::type::thread:
        break;
    case sol::type::boolean:
        (*lua)["value"] = val.as<bool>();
        m_value = val.as<bool>();
        break;
    case sol::type::function:
        break;
    case sol::type::userdata:
        break;
    case sol::type::lightuserdata:
        break;
    case sol::type::table:
        break;
    case sol::type::poly:
        break;
    default:
        break;
    }
}

void ScriptVariableType::Update(const sol::object& val)
{
    sol::state* lua = m_pExporter->SolState();

    sol::type type = val.get_type();
    switch (type)
    {
    case sol::type::none:
        break;
    case sol::type::lua_nil:
        break;
    case sol::type::string:
        break;
    case sol::type::number:
    {
        (*lua)["value"] = val.as<int>();
        m_value = val.as<int>();
         break;
    }
    case sol::type::thread:
        break;
    case sol::type::boolean:
        (*lua)["value"] = val.as<bool>();
        m_value = val.as<bool>();
        break;
    case sol::type::function:
        break;
    case sol::type::userdata:
        break;
    case sol::type::lightuserdata:
        break;
    case sol::type::table:
        break;
    case sol::type::poly:
        break;
    default:
        break;
    }
 
    (*lua)["quality"] = 0;
    m_statusCode = 0;

    (*lua)["timestamp"] = OA::OAInt64(OA::OADateTime::Now());
    m_timestamp = OA::OAInt64(OA::OADateTime::Now());
}

void ScriptVariableType::Force()
{
    // if(item.itemType == RandomGenerator) => call Force item-> DISABLE randomGenerator
    if (IsOutput())
    {        
        SetOutput(false);
    }
}

void ScriptVariableType::ForceValue(const sol::object val)
{
    if (IsOutput())
    {
        SetOutput(false);

        sol::type type = val.get_type();
        switch (type)
        {
        case sol::type::none:
            break;
        case sol::type::lua_nil:
            break;
        case sol::type::string:
            break;
        case sol::type::number:
        {
            m_value = val.as<int>();
            break;
        }
        case sol::type::thread:
            break;
        case sol::type::boolean:
            m_value = val.as<bool>();
            break;
        case sol::type::function:
            break;
        case sol::type::userdata:
            break;
        case sol::type::lightuserdata:
            break;
        case sol::type::table:
            break;
        case sol::type::poly:
            break;
        default:
            break;
        }
    }
    
}

void ScriptVariableType::ForceWithSOLArgs(const sol::object val, OA::OAStatus quality, OA::OAInt64 timestamp)
{
    // if(item.itemType == RandomGenerator) => call Force(value, timestamp, quality) -> item.value = value; item.timestamp = timestamp; item.quality = quality;
}

void ScriptVariableType::Simulate()
{
    // if(item.itemType == RandomGenerator) => item.Force(): DISABLE simulation -> call SIMULATE(): resume simulate
    if ( ! IsOutput())
    {
        // Update Data to create new record
        SetOutput(true);
    }
}

void ScriptVariableType::Resetlatch()
{

}

std::unique_ptr<ScriptVariableType> ScriptVariableType::CreateVariable(const std::string& exportName, const ScriptNodeStruct& scriptNodeStruct, ScriptExporter* pExporter)
{
    std::unique_ptr<ScriptVariableType> pRet;

    switch (scriptNodeStruct.dataType)
    {
    case 0:
    case OA_DataType_Boolean:
    {
        pRet = std::make_unique<ScriptVariableTypeBoolean>(exportName, pExporter);
        break;
    }
    case OA_DataType_Int8:
    { 
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_UInt8:
    {
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_Int16:
    {
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_UInt16:
    {
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_Int32:
    {
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_UInt32:
    {
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_Int64:
    {
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_UInt64:
    {
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_Float:
    {
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_Double:
    {
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    }
    case OA_DataType_DateTime:
        pRet = std::make_unique<ScriptVariableTypeNumber>(exportName, pExporter);
        break;
    case OA_DataType_TON:
        pRet = std::make_unique<ScriptVariableTypeBoolean>(exportName, pExporter);
        break;
    case OA_DataType_String:
        pRet = std::make_unique<ScriptVariableTypeString>(exportName, pExporter);
        break;
    default:
        OA_ASSERT(false);
        break;
    }

    pRet->SetInitValue(scriptNodeStruct.initValue);
    pRet->SetInitStatusCode(scriptNodeStruct.initStatusCode);
    pRet->SetInitTimestamp(scriptNodeStruct.initTimeStamp);
    pRet->SetDataTypeId(scriptNodeStruct.dataType);
    pRet->SetOutput(scriptNodeStruct.isOutput);

    return pRet;
}

void ScriptVariableType::ExportVariableTypeToSOL(ScriptExporter* pExporter)
{
    pExporter->ExportObjectType<ScriptVariableType>("ScriptVariableType",
        "value", sol::property(&ScriptVariableType::GetValue),
        "quality", sol::property(&ScriptVariableType::GetQuality),
        "timestamp", sol::property(&ScriptVariableType::GetTimestampInterval),
        "Update", sol::overload(&ScriptVariableType::UpdateWithSOL, &ScriptVariableType::Update),
        "Force", sol::overload(&ScriptVariableType::Force, &ScriptVariableType::ForceWithSOLArgs, &ScriptVariableType::ForceValue),
        "Simulate", &ScriptVariableType::Simulate,
        "GetKey", &ScriptVariableType::GetKey
        );
}



ScriptVariableTypeBoolean::ScriptVariableTypeBoolean(const std::string& exportName, ScriptExporter* pExporter)
    :ScriptVariableType(exportName, pExporter)
{

}

ScriptVariableTypeBoolean::~ScriptVariableTypeBoolean()
{

}

sol::object ScriptVariableTypeBoolean::GetValue() const
{
    bool initValue = false;
    m_value.GetBoolean(initValue);   
    return m_pExporter->MakeVariant(initValue);
}


ScriptVariableTypeNumber::ScriptVariableTypeNumber(const std::string& exportName, ScriptExporter* pExporter)
    :ScriptVariableType(exportName, pExporter)
{

}

ScriptVariableTypeNumber::~ScriptVariableTypeNumber()
{

}

sol::object ScriptVariableTypeNumber::GetValue() const
{
    OA::OAInt64 value = 0;
    m_value.GetInt64(value);
    return m_pExporter->MakeVariant((int)value);

    //sol::state* lua = m_pExporter->SolState();
    //return   (*lua)["value"];
    //return m_pExporter->MakeVariant((int)0);
}


ScriptVariableTypeString::ScriptVariableTypeString(const std::string& exportName, ScriptExporter* pExporter)
    : ScriptVariableType(exportName, pExporter)
{

}

ScriptVariableTypeString::~ScriptVariableTypeString()
{

}

sol::object ScriptVariableTypeString::GetValue() const
{
    return m_pExporter->MakeVariant("");
}



ScriptMethodType::ScriptMethodType(const std::string& exportName, ScriptExporter* pExporter)
    :ScriptUserType(pExporter)
{
    m_pExporter->ExportObject(exportName, this);
}

ScriptMethodType::~ScriptMethodType()
{

}

ScriptNodeType ScriptMethodType::GetType() const
{
    return ScriptNodeType::Method;
}

OA::OAString ScriptMethodType::GetKey() const
{
    return _OAText("");
}

OA::OAStatus ScriptMethodType::Call(const sol::variadic_args& va)
{
    return 0;
}

void ScriptMethodType::ExportMethodTypeToSOL(ScriptExporter* pExporter)
{
    pExporter->ExportObjectType<ScriptMethodType>("ScriptMethodType",
        "Call", &ScriptMethodType::Call,
        "GetKey", &ScriptMethodType::GetKey);
}



ScriptObjectType::ScriptObjectType(const std::string& exportName, ScriptExporter* pExporter)
    :ScriptUserType(pExporter)
{
    m_soltable = m_pExporter->MakeTable(
        exportName,
        "GetKey", sol::as_function_reference(&ScriptObjectType::GetKey, this),
        "ResetLatch", sol::as_function_reference(&ScriptObjectType::ResetLatch, this)
    );
}

ScriptObjectType::~ScriptObjectType()
{

}

ScriptNodeType ScriptObjectType::GetType() const
{
    return ScriptNodeType::Object;
}

OA::OAString ScriptObjectType::GetKey() const
{
    return _OAText("");
}

OA::OAStatus ScriptObjectType::ResetLatch()
{
    return 0;
}

void ScriptObjectType::SetFieldToTable(const OA::OAString& _fieldName, const OA::OAString& _fieldKey)
{
    std::string fieldName = OA::StringUtility::Utf16ToUtf8(_fieldName);
    std::string fieldKey = OA::StringUtility::Utf16ToUtf8(_fieldKey);

    m_pExporter->SetFieldToTable(m_soltable, fieldName, fieldKey);
}
