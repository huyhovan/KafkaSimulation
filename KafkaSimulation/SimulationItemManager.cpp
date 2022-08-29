#include "SimulationItemManager.h"

#include <OABase/OAStatus.h>
#include <OABase/StringUtility.h>
#include <OABase/OAUniqueID.h>
#include <OABase/StringUtility.h>
#include <OABase/OAVariant.h>

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationInitializationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationRandomGeneratorItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlConsequenceItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlScenarioItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationTriggerScenarioItemInfo.h>
#include <OAModelDataAPI/Compilation/CompilationDataAPI.h>
#include <OAModelDataAPI/Compilation/CompilationFepItemInfo.h>
#include <tchar.h>

#include "KafkaConfig.h"
#include "KafkaRecordInfo.h"

using namespace OA;
using namespace OA::ModelDataAPI;


SimulationItemManager::SimulationItemManager()
{
    std::unique_ptr<KafkaConfig> pKafkaConfig = std::make_unique<KafkaConfig>();

    m_strHost = OA::StringUtility::Utf8ToUtf16(pKafkaConfig->GetHost());
    m_strProject = OA::StringUtility::Utf8ToUtf16(pKafkaConfig->GetProject());

    /* m_strHost = _T("127.0.0.1:27017");
     m_strProject = _T("TEST");*/
    m_strUserName = _T("Administrator");
    m_strPassword = _T("atsco");
}

SimulationItemManager::~SimulationItemManager()
{

}

void SimulationItemManager::Initialize()
{
    if (!m_pDataAPI)
    {
        m_pDataAPI.reset(new CompilationDataAPI);
    }

    OAStatus status = m_pDataAPI->Initialize(m_strHost, m_strProject, m_strUserName, m_strPassword);

    LoadFepItem();
    LoadSimulationItems();
}

OA::OAStatus SimulationItemManager::LoadFepItem()
{
    OAStatus status;

    if (!m_pDataAPI)
    {
        m_pDataAPI = std::make_unique<OA::ModelDataAPI::CompilationDataAPI>();
        status = m_pDataAPI->Initialize(m_strHost, m_strProject, m_strUserName, m_strPassword);
        if (status.IsNotGood())
            return status;
    }

    status = m_pDataAPI->LoadAppliedFepItemInfos(dataCursorFep);

    while (dataCursorFep.GetCount() > 0)
    {
        std::unique_ptr<OA::ModelDataAPI::CompilationFepItemInfo> pFepItemInfo = std::move(dataCursorFep.RetrieveData());
        m_listFepItems.emplace_back(std::move(pFepItemInfo));
    }

    CreateKafkaRecord(m_listFepItems);

    return status;
}

OA::OAStatus SimulationItemManager::LoadSimulationItems()
{
    OAStatus status;

    if (!m_pDataAPI)
    {
        m_pDataAPI = std::make_unique<OA::ModelDataAPI::CompilationDataAPI>();
        status = m_pDataAPI->Initialize(m_strHost, m_strProject, m_strUserName, m_strPassword);
        if (status.IsNotGood())
            return status;
    }

    status = m_pDataAPI->LoadFepSimulationItemInfos(dataCursor);

    while (dataCursor.GetCount() > 0)
    {
        std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo> pFepSimulationInfo = std::move(dataCursor.RetrieveData());

        m_listItems.emplace_back(std::move(pFepSimulationInfo));
    }

    for (auto& pItem : m_listItems)
    {
        UpdateMapKeySimulationItems(pItem.get());
    }

    CreateKafkaRecord(m_listItems);

    return status;
}

const std::vector<std::unique_ptr<FepSimulationItemInfo>>& SimulationItemManager::GetListItem() const
{
    return m_listItems;
}

void SimulationItemManager::UpdateMapKeySimulationItems(OA::ModelDataAPI::FepSimulationItemInfo* simulationItem)
{
    OA::OAString key = simulationItem->GetItemKey();

    auto it = m_mapKeySimulationItems.find(key);

    if (it == m_mapKeySimulationItems.end())
    {
        std::vector<FepSimulationItemInfo*> listRecord;
        listRecord.emplace_back(simulationItem);

        m_mapKeySimulationItems[key] = listRecord;
        return;
    }
    else
    {
        it->second.emplace_back(simulationItem);
    }
}

std::vector<FepSimulationItemInfo*> SimulationItemManager::GetListSimulationItemByKey(OA::OAString key)
{
    auto it = m_mapKeySimulationItems.find(key);

    if (it != m_mapKeySimulationItems.end())
    {
        const std::vector<FepSimulationItemInfo*>& listRecord = it->second;
        return listRecord;
    }

    return {};
}

void SimulationItemManager::DeleteSimulationItem(FepSimulationItemInfo* simulationItem)
{
    OA::OAString keyMethod = simulationItem->GetItemKey();

    std::vector<FepSimulationItemInfo*> listRecord = GetListSimulationItemByKey(keyMethod);

    if (listRecord.size() > 0)
    {
        listRecord.erase(std::remove(listRecord.begin(), listRecord.end(), simulationItem), listRecord.end());

        m_mapKeySimulationItems[keyMethod] = listRecord;
    }
}

const std::map<OA::OAString, std::vector<FepSimulationItemInfo*>>& SimulationItemManager::GetMapKeySimulationItems() const
{
    return m_mapKeySimulationItems;
}

std::map<OA::OAString, std::vector<OA::ModelDataAPI::FepSimulationItemInfo*>> SimulationItemManager::GetMapKeySimulationItems()
{
    return m_mapKeySimulationItems;
}

const std::map<OA::OAString, std::unique_ptr<KafkaRecordInfo>>& SimulationItemManager::GetMapModel() const
{
    return m_MODEL;
}

void SimulationItemManager::UpdateMapModel(OA::OAString& key, KafkaRecordInfo* pRecord)
{
    //std::unique_ptr<KafkaRecordInfo> pOldRecord = std::move(m_MODEL[key]); // = std::move(pRecord);
    //if (pOldRecord)
    //{
    //    pOldRecord->SetValue(pRecord->GetValue());
    //    pOldRecord->SetTimeInterval(pRecord->GetTimeInterval());
    //    pOldRecord->SetQuality(pRecord->GetQuality());

    //    if (pOldRecord->GetItemType() == FepSimulationItemType::RandomGenerator)
    //    {
    //        auto pOldRecordConvert = static_cast<KafkaRandomGeneratorRecordInfo*>(pOldRecord.get());
    //        auto pRecordConvert = static_cast<KafkaRandomGeneratorRecordInfo*>(pRecord);
    //        pOldRecordConvert->SetIsGenning(pRecordConvert->IsGenning());

    //        std::unique_ptr<KafkaRandomGeneratorRecordInfo> pRecordPtr = std::make_unique<KafkaRandomGeneratorRecordInfo>(*pOldRecordConvert);

    //        m_MODEL[key] = std::move(pRecordPtr);
    //        return;
    //    }
    //}
    //m_MODEL[key] = std::move(pOldRecord);

    m_MODEL.at(key)->SetValue(pRecord->GetValue());
    m_MODEL.at(key)->SetTimeInterval(pRecord->GetTimeInterval());
    m_MODEL.at(key)->SetQuality(pRecord->GetQuality());

    if (m_MODEL.at(key)->GetItemType() == FepSimulationItemType::RandomGenerator)
    {
        auto pRecordConvert = static_cast<KafkaRandomGeneratorRecordInfo*>(pRecord);
        static_cast<KafkaRandomGeneratorRecordInfo*>(m_MODEL.at(key).get())->SetIsGenning(pRecordConvert->IsGenning());    
        return;
    }
}

void SimulationItemManager::UpdateMapModel(OA::OAString& key, bool isOutput)
{
    /*std::unique_ptr<KafkaRecordInfo> pOldRecord = std::move(m_MODEL[key]);
    if (pOldRecord && pOldRecord->GetItemType() == FepSimulationItemType::RandomGenerator)
    {
        auto pOldRecordConvert = static_cast<KafkaRandomGeneratorRecordInfo*>(pOldRecord.get());
        pOldRecordConvert->SetIsGenning(isOutput);

        std::unique_ptr<KafkaRandomGeneratorRecordInfo> pRecordPtr = std::make_unique<KafkaRandomGeneratorRecordInfo>(*pOldRecordConvert);

        m_MODEL[key] = std::move(pRecordPtr);
    }   */

    if (m_MODEL.at(key)->GetItemType() == FepSimulationItemType::RandomGenerator)
    {       
        static_cast<KafkaRandomGeneratorRecordInfo*>(m_MODEL.at(key).get())->SetIsGenning(isOutput);               
    }
}

const std::vector<MonitoredItem>& SimulationItemManager::GetListMonitorItem() const
{
    return m_listMonitorItem;
}

bool SimulationItemManager::IsMonitoredVariable(OA::OAString& key)
{
    for (auto& item : m_listMonitorItem)
    {
        if (key == OA::StringUtility::Utf8ToUtf16(item.id.GetIdentifier().IdentifierString()))
            return true;
    }

    return false;
}

void SimulationItemManager::GetListParentTrigger(OA::OAString& keyMonitor, std::vector<OA::OAString>& listParentTriggerName)
{
    for (auto& item : m_listMonitorItem)
    {
        if (OA::StringUtility::Utf8ToUtf16(item.id.GetIdentifier().IdentifierString()) == keyMonitor)
        {
            listParentTriggerName.push_back(item.triggerScenario);
        }
    }
}

const std::map<OA::OAUInt16, std::vector<KafkaRandomGeneratorRecordInfo*>>& SimulationItemManager::GetMapIntervalRandomRecord() const
{
    return m_mapIntervalRandomRecord;
}

void SimulationItemManager::SetValueBaseOnFepSimulationItemInfo(OA::ModelDataAPI::FepSimulationItemInfo* fepSimulationItemInfo, OA::OAVariant& outValue)
{
    OA::ModelDataAPI::FepSimulationItemType itemType = fepSimulationItemInfo->GetItemType();

    switch (itemType)
    {
    case OA::ModelDataAPI::FepSimulationItemType::Initialization:
    {
        auto pItem = static_cast<OA::ModelDataAPI::FepSimulationInitializationItemInfo*>(fepSimulationItemInfo);
        outValue = pItem->GetInitialValue();

        break;
    }
    case OA::ModelDataAPI::FepSimulationItemType::RandomGenerator:
    {

        break;
    }
    case OA::ModelDataAPI::FepSimulationItemType::ControlConsequence:
        break;
    case OA::ModelDataAPI::FepSimulationItemType::ControlScenario:
        break;
    case OA::ModelDataAPI::FepSimulationItemType::TriggerScenario:
        break;
    default:
        break;
    }

}

void SimulationItemManager::CreateKafkaRecord(const std::vector<std::unique_ptr<OA::ModelDataAPI::CompilationFepItemInfo>>& listFepItem)
{
    m_MODEL.clear();

    for (auto& fepItem : listFepItem)
    {
        std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();

        OA::OAString key = fepItem->GetId().GetIdentifier().IdentifierStringW();
        pRecord->SetKey(key);

        if (fepItem->GetItemType() == CompilationFepItemInfo::ItemType::MonitoredItem)
        {
            OA::ModelDataAPI::CompilationFepMonitoredItemInfo* pMonitoredItem = static_cast<OA::ModelDataAPI::CompilationFepMonitoredItemInfo*>(fepItem.get());
            OA::OAUInt32 dataType = pMonitoredItem->GetDataType();
            pRecord->SetDataTypeId(dataType);

            pRecord->SetValue(0);
            pRecord->SetQuality(0);
        }

        if (fepItem->GetItemType() == CompilationFepItemInfo::ItemType::MethodItem)
        {
            //OA::ModelDataAPI::CompilationFepMethodItemInfo* pMethodItem = static_cast<OA::ModelDataAPI::CompilationFepMethodItemInfo*>(fepItem.get());
            //pRecord->SetListMethodParam(pMethodItem->GetParameters());
        }

        m_MODEL.emplace(key, std::move(pRecord));
    }

}

void SimulationItemManager::CreateKafkaRecord(const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& listSimulateItem)
{
    assert(m_MODEL.size() > 0);

    for (size_t i = 0; i < listSimulateItem.size(); i++)
    {
        OA::OAString key = listSimulateItem[i]->GetItemKey();

        OA::ModelDataAPI::FepSimulationItemType type = listSimulateItem[i]->GetItemType();

        std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();

        switch (type)
        {
        case OA::ModelDataAPI::FepSimulationItemType::Initialization:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationInitializationItemInfo*>(listSimulateItem[i].get());
            OA::OAVariant initValue = pItem->GetInitialValue();
            if (initValue == 0)
                initValue = rand() % 100 + 1;
            OA::OAUInt16 dataType = pItem->GetDataType();
            OA::OAString strDataType = OA::StringUtility::BuiltinDataTypeToString(dataType);

            pRecord->SetKey(key);
            pRecord->SetValue(initValue);
            pRecord->SetDataTypeId(pItem->GetDataType());
            pRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::Initialization);

            //m_listRecords.emplace_back(std::move(pRecord));
            m_MODEL[key] = std::move(pRecord);

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::RandomGenerator:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationRandomGeneratorItemInfo*>(listSimulateItem[i].get());

            std::unique_ptr<KafkaRandomGeneratorRecordInfo> pRandomRecord = std::make_unique<KafkaRandomGeneratorRecordInfo>();

            pRandomRecord->SetKey(key);

            //OA::OAUInt16 dataType = pItem->GetDataType();
            //OA::OAString strDataType = OA::StringUtility::BuiltinDataTypeToString(dataType);
            pRandomRecord->SetDataTypeId(pItem->GetDataType());

            OA::OAUInt32 interval = pItem->GetInterval();
            pRandomRecord->SetInterval(interval);

            OA::OAVariant minValue = pItem->GetMinValue();
            pRandomRecord->SetMinvalue(minValue);

            OA::OAVariant maxValue = pItem->GetMaxValue();
            pRandomRecord->SetMaxValue(maxValue);

            pRandomRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::RandomGenerator);

            UpdateMapIntervalRandomRecord(pRandomRecord.get());

            //m_listRecords.emplace_back(std::move(pRandomRecord));
            m_MODEL[key] = std::move(pRandomRecord);

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::ControlConsequence:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo*>(listSimulateItem[i].get());

            std::unique_ptr<KafkaControlConsequenceRecordInfo> pControlConsequence = std::make_unique<KafkaControlConsequenceRecordInfo>();

            pControlConsequence->SetKey(key);
            pControlConsequence->SetControlType(pItem->GetControlType());
            pControlConsequence->SetTarget(pItem->GetTarget());

            //m_listRecords.emplace_back(std::move(pControlConsequence));
            m_MODEL[key] = std::move(pControlConsequence);

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::ControlScenario:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationControlScenarioItemInfo*>(listSimulateItem[i].get());

            std::unique_ptr<KafkaControlScenarioRecordInfo> pControlScenario = std::make_unique<KafkaControlScenarioRecordInfo>();

            pControlScenario->SetKey(key);
            pControlScenario->SetContent(pItem->GetContent());
            pControlScenario->SetParameters(pItem->GetParameters());
            pControlScenario->SetInputs(pItem->GetInputs());

            //m_listRecords.emplace_back(std::move(pControlScenario));
            m_MODEL[key] = std::move(pControlScenario);

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::TriggerScenario:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationTriggerScenarioItemInfo*>(listSimulateItem[i].get());

            std::unique_ptr<KafkaTriggerScenarioRecordInfo> pTriggerScenario = std::make_unique<KafkaTriggerScenarioRecordInfo>();

            pTriggerScenario->SetKey(key);
            pTriggerScenario->SetContent(pItem->GetContent());
            pTriggerScenario->SetInputs(pItem->GetInputs());
            pTriggerScenario->SetRepeatEnable(pItem->GetRepeatEnabled());
            pTriggerScenario->SetRepeatInterval(pItem->GetRepeatInterval());

            //m_listRecords.emplace_back(std::move(pTriggerScenario));
            CreateListMonitorItemId(key, pItem->GetInputs());
            m_MODEL[key] = std::move(pTriggerScenario);

            break;
        }
        default:
            break;
        }
    }
}

void SimulationItemManager::CreateListMonitorItemId(const OA::OAString& triggerScenarioName, const std::vector<OA::ModelDataAPI::FepSimulationTriggerScenarioInput>& listInput)
{
    for (auto& input : listInput)
    {        
        OA::OAUniqueID inputId = input.GetInputNode();
        MonitoredItem item = MonitoredItem(triggerScenarioName, inputId);
        bool bExist = (std::find(m_listMonitorItem.begin(), m_listMonitorItem.end(), item) != m_listMonitorItem.end());
        if (input.IsMonitored() && !bExist)
        {
            m_listMonitorItem.push_back(item);
        }
    }
}

void SimulationItemManager::UpdateMapIntervalRandomRecord(KafkaRandomGeneratorRecordInfo* pRandomRecord)
{
    OA::OAUInt16 interval = pRandomRecord->GetInterval();

    auto it = m_mapIntervalRandomRecord.find(interval);
    if (it == m_mapIntervalRandomRecord.end())
    {
        m_mapIntervalRandomRecord[interval].push_back(pRandomRecord);
    }
    else
    {
        it->second.push_back(pRandomRecord);
    }
}
