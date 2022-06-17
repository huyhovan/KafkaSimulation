#pragma once

#include <vector>
#include <memory>
#include <OABase/OAString.h>

#include <OAModelDataAPI/Base/DataCursor.h>
#include <OABase/OAStatus.h>


namespace OA
{
    class OAStatus;

    namespace ModelDataAPI
    {
        class FepSimulationItemInfo;
        class CompilationDataAPI;
        class CompilationFepItemInfo;

    }
}

class  SimulationItemManager
{
public:
    SimulationItemManager();
    virtual ~SimulationItemManager();

    void Initialize();

    OA::OAStatus LoadSimulationItems();
    OA::OAStatus LoadFepItem();
    const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& GetListItem() const;

protected:
    std::unique_ptr<OA::ModelDataAPI::CompilationDataAPI> m_pDataAPI;
    OA::ModelDataAPI::DataCursor<OA::ModelDataAPI::FepSimulationItemInfo> dataCursor;
    OA::ModelDataAPI::DataCursor<OA::ModelDataAPI::CompilationFepItemInfo> dataCursorFep;

    std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>> m_listItems;

    OA::OAString m_strHost;
    OA::OAString m_strProject;
    OA::OAString m_strUserName;
    OA::OAString m_strPassword;
};

