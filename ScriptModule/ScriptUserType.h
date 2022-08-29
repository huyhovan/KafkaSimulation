#pragma once
#include "ScriptModuleExportHelper.h"

#include <OABase/OABase.h>
#include <OABase/OAStatus.h>
#include <OABase/OAUniqueID.h>
#include <OABase/OAVariant.h>

#include "ScriptExporter.h"
#include <sol/sol.hpp>

//class ScriptExporter;

enum class ScriptNodeType
{
    Variable, Method, Object
};

typedef struct _ScriptNodeStruct
{
    ScriptNodeType nodeType;
    OA::OAUniqueID uniqueId;
    OA::OAString nodeIdString;
    OA::OAString replacedString;
    OA::OAUInt16 dataType =0;   
    OA::OAVariant initValue;
    OA::OAUInt64 initStatusCode;
    OA::OAInt64 initTimeStamp;    
    bool isOutput = false;
} ScriptNodeStruct;


class  ScriptUserType
{
public:
    ScriptUserType(ScriptExporter* pExporter);
    virtual ~ScriptUserType();

    virtual ScriptNodeType GetType() const = 0;

    static void ExportUserTypeToSOL(ScriptExporter* pExporter);
    static std::unique_ptr<ScriptUserType> Create(const ScriptNodeStruct& scriptNodeStruct, ScriptExporter* pExporter);
protected:
    ScriptExporter* m_pExporter;

};

class  ScriptVariableType : public ScriptUserType
{
public:
    ScriptVariableType(const std::string& exportName, ScriptExporter* pExporter);

    ScriptNodeType GetType() const override final;

    virtual sol::object GetValue() const = 0;

    OA::OAUInt64 GetQuality() const;
    OA::OAInt64 GetTimestampInterval() const;
    OA::OAString GetKey() const;
    OA::OAUInt16 GetDataTypeId() const;
    bool IsOutput() const;

    // Set Init value
    void SetInitValue(const OA::OAVariant& value);
    void SetInitStatusCode(const OA::OAUInt64& initStatusCode);
    void SetInitTimestamp(const OA::OAInt64& initTimestamp);
    void SetDataTypeId(const OA::OAUInt16& dataTypeId);
    void SetOutput(bool isOutput);

    void UpdateWithSOL(const sol::object& val, OA::OAUInt64 statusCode, OA::OAInt64 timestamp);
    void Update(const sol::object& val);
    void Force();
    void ForceValue(const sol::object val);
    void ForceWithSOLArgs(const sol::object val, OA::OAStatus quality, OA::OAInt64 timestamp);
    void Simulate();
    void Resetlatch();   

    /*OA::OAStatus Write(const sol::object& val);
    void Enable();
    void Reset();
    void SetInterval(int interval);*/

    static std::unique_ptr<ScriptVariableType> CreateVariable(const std::string& exportName,const ScriptNodeStruct& scriptNodeStruct, ScriptExporter* pExporter);
    static void ExportVariableTypeToSOL(ScriptExporter* pExporter);

protected:
    OA::OAString m_key;
    OA::OAVariant m_value;
    OA::OAUInt64 m_statusCode;
    OA::OAInt64 m_timestamp;
    OA::OAUInt16 m_dataTypeId;
    bool m_bIsOutput;
};

class  ScriptVariableTypeBoolean : public ScriptVariableType
{
public:
    ScriptVariableTypeBoolean(const std::string& exportName, ScriptExporter* pExporter);
    virtual ~ScriptVariableTypeBoolean();

    sol::object GetValue() const override final;
};

class  ScriptVariableTypeNumber : public ScriptVariableType
{
public:
    ScriptVariableTypeNumber(const std::string& exportName, ScriptExporter* pExporter);
    virtual ~ScriptVariableTypeNumber();

    sol::object GetValue() const override final;
};

class ScriptVariableTypeString : public ScriptVariableType
{
public:
    ScriptVariableTypeString(const std::string& exportName, ScriptExporter* pExporter);
    virtual ~ScriptVariableTypeString();

    sol::object GetValue() const override final;
};


class  ScriptMethodType : public ScriptUserType
{
public:
    ScriptMethodType(const std::string& exportName, ScriptExporter* pExporter);
    virtual ~ScriptMethodType();

    ScriptNodeType GetType() const override final;

    OA::OAString GetKey() const;
    OA::OAStatus Call(const sol::variadic_args& va);

    static void ExportMethodTypeToSOL(ScriptExporter* pExporter);
};


class  ScriptObjectType : public ScriptUserType
{
public:
    ScriptObjectType(const std::string& exportName, ScriptExporter* pExporter);
    virtual ~ScriptObjectType();

    ScriptNodeType GetType() const override final;

    OA::OAString GetKey() const;
    OA::OAStatus ResetLatch();

    void SetFieldToTable(const OA::OAString& fieldName, const OA::OAString& fieldKey);

private:
    sol::table m_soltable;
};