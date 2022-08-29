
#include "ScriptModule.h"
#include "ScriptExporter.h"

#include <OABase/StringUtility.h>

#include <sol/sol.hpp>

ScriptModule::ScriptModule()
{

}

ScriptModule::~ScriptModule()
{

}

bool ScriptModule::Initialize(OA::OAString& errMsg)
{
    errMsg.clear();

    if (!m_pSolState)
    {
        if (Init(errMsg) == false)
        {
            return false;
        }
    }

    return true;
}

void ScriptModule::ExportScriptNode(const ScriptNodeStruct& inputScriptNode)
{
    OA_ASSERT(m_pSolState);

    if (m_mapScriptNodes.find(inputScriptNode.replacedString) == m_mapScriptNodes.end())
    {
        auto pScriptNode = ScriptUserType::Create(inputScriptNode, m_pExporter.get());
        OA_ASSERT(pScriptNode);

        if (pScriptNode)
        {
            m_mapScriptNodes.emplace(inputScriptNode.replacedString, std::move(pScriptNode));
        }
    }
}

void ScriptModule::ExportScriptStructure(const OA::OAString& tableKey, const OA::OAString& fieldName, const OA::OAString& fieldKey)
{
    auto it = m_mapScriptNodes.find(tableKey);
    if (it != m_mapScriptNodes.end())
    {
        if (it->second->GetType() == ScriptNodeType::Object)
        {
            static_cast<ScriptObjectType*>(it->second.get())->SetFieldToTable(fieldName, fieldKey);
        }
    }
    else
    {
        OA_ASSERT(false);
    }
}

ScriptExporter* ScriptModule::GetScriptExporter()
{
    return m_pExporter.get();
}

bool ScriptModule::RunScript(const OA::OAString& scriptContent, OA::OAString& errMsg)
{
    if (!m_pSolState)
    {
        if (Init(errMsg) == false)
        {
            return false;
        }
    }

    try
    {
        lua_State* L = m_pSolState->lua_state();

        if (!L)
            return false;

      /*  int nResult = luaL_dostring(L, OA::StringUtility::Utf16ToUtf8(scriptContent).c_str());
        if (nResult != 0)
        {
            errMsg = OA::StringUtility::Utf8ToUtf16(lua_tostring(L, -1));
            return false;
        }*/

        m_pSolState->script(OA::StringUtility::Utf16ToUtf8(scriptContent));
    }
    catch(std::exception& e)
    {
        errMsg = OA::StringUtility::Utf8ToUtf16(e.what());
        return false;
    }

    return true;
}

void ScriptModule::ExportUserTypeToSOL()
{
    ScriptUserType::ExportUserTypeToSOL(m_pExporter.get());
}

std::unique_ptr<ScriptUserType> ScriptModule::CreateScriptVariableTypeBoolean(const ScriptNodeStruct& scriptNodeStruct, ScriptExporter* pExporter)
{
    return ScriptUserType::Create(scriptNodeStruct, pExporter);
}

bool ScriptModule::ConvertMethodParamsToSOL(const OA::OAString& functionParamStr, const std::vector<bool>& listInputArg)
{
    std::vector<OA::OAString> methodParamArr = GetFunctionParameters(functionParamStr);

    if (listInputArg.size() != methodParamArr.size())
    {
        //std::cout << "No Match input Parameter" << std::endl;
        return false;
    }
    for (size_t i = 0; i < listInputArg.size(); i++)
    {
        std::string exportMethodParameterStr;
        if (listInputArg[i] == true)
        {
            exportMethodParameterStr += /*std::string("local ") +*/ (OA::StringUtility::Utf16ToUtf8(methodParamArr[i]) + std::string(" = true\n"));
        }
        else
        {
            exportMethodParameterStr += /*std::string("local ") +*/ (OA::StringUtility::Utf16ToUtf8(methodParamArr[i]) + std::string(" = false\n"));
        }
       
        m_pSolState->script(exportMethodParameterStr.c_str());
    }  

    return true;
}

void ScriptModule::GetValueScriptUserTypeByKey(OA::OAString luaKey, OA::OAVariant& outValue, OA::OAUInt16& dataTypeId, OA::OAUInt64& outStatusCode, OA::OAInt64& outTimestampInterVal, bool& isOutput) const
{
    auto it = m_mapScriptNodes.find(luaKey);
    if (it != m_mapScriptNodes.end())
    {
        if ((it->second)->GetType() == ScriptNodeType::Variable)
        {
            ScriptVariableType* pUserType = static_cast<ScriptVariableType*>(it->second.get());
            sol::object valueSOL = pUserType->GetValue();

            dataTypeId = pUserType->GetDataTypeId();
            outStatusCode = pUserType->GetQuality();
            outTimestampInterVal = pUserType->GetTimestampInterval();
                        
            OA::OAString key = pUserType->GetKey();

            sol::type type = valueSOL.get_type();
            if (type == sol::type::boolean)
            {
                outValue = valueSOL.as<bool>();
            }
            else if (type == sol::type::number)
            {
                outValue = valueSOL.as<int>();
                OA::OAString strValue = outValue.ToString();
            }
            
            isOutput = pUserType->IsOutput();
        }      
    }
}

bool ScriptModule::Init(OA::OAString& errMsg)
{
    m_pSolState.reset(new sol::state());
    m_pSolState->open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::os, sol::lib::string, sol::lib::table, sol::lib::io);

    lua_State* L = m_pSolState->lua_state();
    if (!L)
    {
        assert(false);
        errMsg = _OAText("Initialize script module failed");
        return false;
    }

    m_pExporter = std::make_unique<ScriptExporter>(m_pSolState.get());

    return true;
}

std::vector<OA::OAString> ScriptModule::GetFunctionParameters(const OA::OAString& functionParamStr)
{
    std::vector<OA::OAString> retArr;

    if (functionParamStr.length())
    {
        size_t idx = functionParamStr.find(',');
        size_t lastIdx = 0;
        while (idx != std::string::npos)
        {
            OA::OAString parameterStr = functionParamStr.substr(lastIdx, idx - lastIdx);
            retArr.emplace_back(std::move(parameterStr));

            lastIdx = idx + 1;
            idx = functionParamStr.find(',', lastIdx);
        }

        OA::OAString lastParameterStr = functionParamStr.substr(lastIdx, functionParamStr.length() - lastIdx);
        retArr.emplace_back(std::move(lastParameterStr));
    }

    return retArr;
}
