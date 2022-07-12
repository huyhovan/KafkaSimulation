#pragma once

#include <vector>
#include <memory>
#include <map>

#include <OAModelDataAPI/Base/DataCursor.h>
#include <OABase/OAStatus.h>
#include <OABase/OAString.h>


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
class KafkaRecordInfo;

class  SimulationItemManager
{
public:
    SimulationItemManager();
    virtual ~SimulationItemManager();

    void Initialize();

    OA::OAStatus LoadSimulationItems();
    OA::OAStatus LoadFepItem();
    const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& GetListItem() const;

    void UpdateMapKeySimulationItems(OA::ModelDataAPI::FepSimulationItemInfo* simulationItem);
    std::vector<OA::ModelDataAPI::FepSimulationItemInfo*> GetListSimulationItemByKey(OA::OAString key) ;
    void DeleteSimulationItem(OA::ModelDataAPI::FepSimulationItemInfo* simulationItem);
    const std::map<OA::OAString, std::vector<OA::ModelDataAPI::FepSimulationItemInfo*>>& GetMapKeySimulationItems() const;
    std::map<OA::OAString, std::vector<OA::ModelDataAPI::FepSimulationItemInfo*>>  GetMapKeySimulationItems();


protected:
    void SetValueBaseOnFepSimulationItemInfo(OA::ModelDataAPI::FepSimulationItemInfo* fepSimulationItemInfo, OA::OAVariant& outValue);

protected:
    std::unique_ptr<OA::ModelDataAPI::CompilationDataAPI> m_pDataAPI;
    OA::ModelDataAPI::DataCursor<OA::ModelDataAPI::FepSimulationItemInfo> dataCursor;
    OA::ModelDataAPI::DataCursor<OA::ModelDataAPI::CompilationFepItemInfo> dataCursorFep;

    std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>> m_listItems;
    std::vector <std::unique_ptr<OA::ModelDataAPI::CompilationFepItemInfo>> m_listFepItems;
    std::vector <std::unique_ptr<SimulationModelNode>> m_listModelNodes; /*not use*/

    OA::OAString m_strHost;
    OA::OAString m_strProject;
    OA::OAString m_strUserName;
    OA::OAString m_strPassword;

    //std::map<OA::OAString, std::unique_ptr<KafkaRecordInfo>> m_mapKeyRecordControl; // using m_mapmodel for control consequence, xxscenario
    std::map<OA::OAString, std::vector<OA::ModelDataAPI::FepSimulationItemInfo*>> m_mapKeySimulationItems;
};

