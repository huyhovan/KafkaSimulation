
#include "ScriptExporter.h"

sol::state* ScriptExporter::SolState()
{
    return m_pSolState;
}

void ScriptExporter::SetFieldToTable(sol::table& tb, const std::string& fieldName, const std::string& fieldKey)
{
    tb[fieldName] = (*m_pSolState)[fieldKey];
}
