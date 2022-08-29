#pragma once

#include <vector>
#include <memory>
#include <map>

#include <OAModelDataAPI/Base/DataCursor.h>
#include <OABase/OAStatus.h>
#include <OABase/OAString.h>
#include <OABase/OAUniqueID.h>

namespace OA
{
    class OAStatus;
    class OAVariant;

    namespace ModelDataAPI
    {
        class FepSimulationItemInfo;
        class CompilationDataAPI;
        class CompilationFepItemInfo;
        class FepSimulationTriggerScenarioInput;
    }
}

class SimulationModelNode;
class KafkaRecordInfo;
class KafkaRandomGeneratorRecordInfo;

struct MonitoredItem
{
    OA::OAString triggerScenario;
    OA::OAUniqueID id;
    
    MonitoredItem(const OA::OAString& _triggerScenario, OA::OAUniqueID _id)
        :triggerScenario(_triggerScenario), id(_id) {};
    MonitoredItem(OA::OAString&& _triggerScenario, OA::OAUniqueID _id)
        :triggerScenario(std::move(_triggerScenario)), id(_id) {};

    bool operator==(const MonitoredItem& rhs) const { return (this->triggerScenario == rhs.triggerScenario) && (this->id == rhs.id); }
};


class  SimulationItemManager
{
public:
    SimulationItemManager();
    virtual ~SimulationItemManager();

    void Initialize();

    OA::OAStatus LoadFepItem();
    OA::OAStatus LoadSimulationItems();
    
    const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& GetListItem() const;

    void UpdateMapKeySimulationItems(OA::ModelDataAPI::FepSimulationItemInfo* simulationItem);
    std::vector<OA::ModelDataAPI::FepSimulationItemInfo*> GetListSimulationItemByKey(OA::OAString key) ;
    void DeleteSimulationItem(OA::ModelDataAPI::FepSimulationItemInfo* simulationItem);
    const std::map<OA::OAString, std::vector<OA::ModelDataAPI::FepSimulationItemInfo*>>& GetMapKeySimulationItems() const;
    std::map<OA::OAString, std::vector<OA::ModelDataAPI::FepSimulationItemInfo*>>  GetMapKeySimulationItems();

    const std::map<OA::OAString, std::unique_ptr<KafkaRecordInfo>>& GetMapModel() const;
    void UpdateMapModel(OA::OAString& key, KafkaRecordInfo* pRecord);
    void UpdateMapModel(OA::OAString& key, bool isOutput);

    const std::vector<MonitoredItem>& GetListMonitorItem() const;
    bool IsMonitoredVariable(OA::OAString& key);
    void GetListParentTrigger(OA::OAString& keyMonitor, std::vector<OA::OAString>& listParentTriggerName);

    const std::map<OA::OAUInt16, std::vector<KafkaRandomGeneratorRecordInfo*>>& GetMapIntervalRandomRecord() const;

protected:
    void SetValueBaseOnFepSimulationItemInfo(OA::ModelDataAPI::FepSimulationItemInfo* fepSimulationItemInfo, OA::OAVariant& outValue);

    void CreateKafkaRecord(const std::vector<std::unique_ptr<OA::ModelDataAPI::CompilationFepItemInfo>>& listFepItem);
    void CreateKafkaRecord(const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& listSimulateItem);

    void CreateListMonitorItemId(const OA::OAString& triggerScenarioName, const std::vector<OA::ModelDataAPI::FepSimulationTriggerScenarioInput>& listInput);
    void UpdateMapIntervalRandomRecord(KafkaRandomGeneratorRecordInfo* pRandomRecord);

protected:
    std::unique_ptr<OA::ModelDataAPI::CompilationDataAPI> m_pDataAPI;
    OA::ModelDataAPI::DataCursor<OA::ModelDataAPI::FepSimulationItemInfo> dataCursor;
    OA::ModelDataAPI::DataCursor<OA::ModelDataAPI::CompilationFepItemInfo> dataCursorFep;

    std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>> m_listItems;
    std::vector <std::unique_ptr<OA::ModelDataAPI::CompilationFepItemInfo>> m_listFepItems;
    
    OA::OAString m_strHost;
    OA::OAString m_strProject;
    OA::OAString m_strUserName;                                                                                                                                      
    OA::OAString m_strPassword;
    
    std::map<OA::OAString, std::vector<OA::ModelDataAPI::FepSimulationItemInfo*>> m_mapKeySimulationItems;

    //std::map<OA::OAString, std::unique_ptr<KafkaRecordInfo>> m_mapKeyRecordControl; // MODEL using during run-time application or:
    std::map<OA::OAString, std::unique_ptr<KafkaRecordInfo>> m_MODEL;   
  
    std::vector<MonitoredItem> m_listMonitorItem;
    //std::map<OA::OAString, std::vector<OA::OAString>> m_mapTriggerMonitoredVars;
    std::map<OA::OAUInt16, std::vector<KafkaRandomGeneratorRecordInfo*>> m_mapIntervalRandomRecord;
};

