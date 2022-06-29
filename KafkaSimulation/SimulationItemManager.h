#pragma once

#include <vector>
#include <memory>
#include <OABase/OAString.h>

#include <OAModelDataAPI/Base/DataCursor.h>
#include <OABase/OAStatus.h>


namespace OA
{
    class OAStatus;
    class OAVariant;

    namespace ModelDataAPI
    {
        class FepSimulationItemInfo;
        class CompilationDataAPI;
        class CompilationFepItemInfo;

    }
}

class SimulationModelNode;

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
    void SetValueBaseOnFepSimulationItemInfo(OA::ModelDataAPI::FepSimulationItemInfo* fepSimulationItemInfo, OA::OAVariant& outValue);

protected:
    std::unique_ptr<OA::ModelDataAPI::CompilationDataAPI> m_pDataAPI;
    OA::ModelDataAPI::DataCursor<OA::ModelDataAPI::FepSimulationItemInfo> dataCursor;
    OA::ModelDataAPI::DataCursor<OA::ModelDataAPI::CompilationFepItemInfo> dataCursorFep;

    std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>> m_listItems;
    std::vector <std::unique_ptr<OA::ModelDataAPI::CompilationFepItemInfo>> m_listFepItems;
    std::vector <std::unique_ptr<SimulationModelNode>> m_listModelNodes;

    OA::OAString m_strHost;
    OA::OAString m_strProject;
    OA::OAString m_strUserName;
    OA::OAString m_strPassword;
};

