#pragma once

#include "ScriptModuleExportHelper.h"
#include "ScriptUserType.h"

#include <lua.hpp>
#include <sol/sol.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

#include <OABase/OAString.h>

#include "KafkaSimulation/KafkaRecordInfo.h"

namespace sol
{
    class state;
}

class ScriptExporter;

class SCRIPTMODULE_LIB_API ScriptModule
{
public:
    ScriptModule();
    virtual ~ScriptModule();

    bool Initialize(OA::OAString& errMsg);

    void ExportScriptNode(const ScriptNodeStruct& inputScriptNode);
    void ExportScriptStructure(const OA::OAString& tableKey, const OA::OAString& fieldName, const OA::OAString& fieldKey);

    ScriptExporter* GetScriptExporter();
    bool RunScript(const OA::OAString& scriptContent, OA::OAString& errMsg);

    // utility for dllimport class
    void ExportUserTypeToSOL();
    std::unique_ptr<ScriptUserType> CreateScriptVariableTypeBoolean(const ScriptNodeStruct& scriptNodeStruct, ScriptExporter* pExporter);
    bool ConvertMethodParamsToSOL(const OA::OAString& functionParamStr, const std::vector<bool>& listInputArg);

   void GetValueScriptUserTypeByKey(OA::OAString luaKey, OA::OAVariant& outValue, OA::OAUInt16& dataTypeId, OA::OAUInt64& outStatusCode, OA::OAInt64& outTimestampInterVal, bool& isOutput) const;

private:
    bool Init(OA::OAString& errMsg);
    std::vector<OA::OAString> GetFunctionParameters(const OA::OAString& functionParamStr);

protected:
    sol::state lua{};

    std::unique_ptr<sol::state> m_pSolState;
    std::unique_ptr<ScriptExporter> m_pExporter;

    std::unordered_map<OA::OAString, std::unique_ptr<ScriptUserType>> m_mapScriptNodes;
};