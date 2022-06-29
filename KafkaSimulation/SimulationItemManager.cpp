#include "SimulationItemManager.h"
#include "SimulationModelNode.h"

#include <OABase/OAStatus.h>
#include <OABase/StringUtility.h>
#include <OABase/OAUniqueID.h>
#include <OABase/StringUtility.h>
#include <OABase/OAVariant.h>

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationInitializationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationRandomGeneratorItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlConsequenceItemInfo.h>
#include <OAModelDataAPI/Compilation/CompilationDataAPI.h>
#include <OAModelDataAPI/Compilation/CompilationFepItemInfo.h>
#include <tchar.h>

#include "KafkaConfig.h"


using namespace OA;
using namespace OA::ModelDataAPI;


SimulationItemManager::SimulationItemManager()
{
    std::unique_ptr<KafkaConfig> pKafkaConfig = std::make_unique<KafkaConfig>();

    m_strHost =OA::StringUtility::Utf8ToUtf16(pKafkaConfig->GetHost());
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

    LoadSimulationItems();
    LoadFepItem();
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

    return status;
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

        if (pFepItemInfo->GetLinkTargets().size() > 0)
        {
            // Create Simulation Model
            auto pSimulationModelNode = std::make_unique<SimulationModelNode>();

            OA::OAUniqueID id = pFepItemInfo->GetLinkTargets().at(0).GetTargetUniqueId();
            OA::OAString keyTarget = OA::StringUtility::Utf8ToUtf16(id.GetIdentifier().IdentifierString());
            pSimulationModelNode->SetKey(keyTarget);

            OA::OAString key = OA::StringUtility::Utf8ToUtf16(pFepItemInfo->GetId().GetIdentifier().IdentifierString());

            for (auto& fepSimulationItem : m_listItems)
            {
                if (key == fepSimulationItem->GetItemKey())
                {
                    pSimulationModelNode->SetItemInfo(fepSimulationItem.get());
                    OA::OAVariant value;
                    SetValueBaseOnFepSimulationItemInfo(fepSimulationItem.get(), value);
                    pSimulationModelNode->SetValue(value);

                    break;
                }
            }

            m_listModelNodes.emplace_back(std::move(pSimulationModelNode));

            m_listFepItems.emplace_back(std::move(pFepItemInfo));
        }         
    }    

    return status;
}

const std::vector<std::unique_ptr<FepSimulationItemInfo>>& SimulationItemManager::GetListItem() const
{
    return m_listItems;
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
