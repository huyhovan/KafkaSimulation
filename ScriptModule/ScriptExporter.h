#pragma once

#include "ScriptModuleExportHelper.h"
#include <sol/sol.hpp>


class SCRIPTMODULE_LIB_API ScriptExporter
{
public:
    ScriptExporter(sol::state* pSolState) : m_pSolState(pSolState) {};

    template<typename... Args>
    sol::table MakeTable(const std::string& tableName, Args&&... args);

    template<typename...Args>
    void ExportObject(Args&&... args);

    template<typename...Args>
    void ExportFunction(const std::string& funcName, Args&&... args);

    template<typename Class, typename...Args>
    void ExportObjectType(const std::string& typeName, Args&&... args);

    template<typename... Args>
    sol::object MakeVariant(Args&&... args);

    sol::state* SolState();
    void SetFieldToTable(sol::table& tb, const std::string& fieldName, const std::string& fieldKey);    

private:
    sol::state* m_pSolState;
};

template<typename... Args> 
sol::table ScriptExporter::MakeTable(const std::string& tableName, Args&&... args)
{
    return m_pSolState->create_named_table(tableName, std::forward<Args>(args)...);
}

template<typename... Args>
inline void ScriptExporter::ExportObject(Args&&... args)
{
    m_pSolState->set(std::forward<Args>(args)...);
}

template<typename... Args>
void ScriptExporter::ExportFunction(const std::string& funcName, Args&&... args)
{
    m_pSolState->set_function(funcName, std::forward<Args>(args)...);
}

template<typename Class, typename... Args>
void ScriptExporter::ExportObjectType(const std::string& typeName, Args&&... args)
{
    m_pSolState->new_usertype<Class>(typeName, std::forward<Args>(args)...);
}

template<typename... Args>
sol::object ScriptExporter::MakeVariant(Args&&... args)
{
    return sol::make_object(*m_pSolState, std::forward<Args>(args)...);
}