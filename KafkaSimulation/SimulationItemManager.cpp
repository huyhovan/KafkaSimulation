#include "SimulationItemManager.h"

#include <OABase/OAStatus.h>
#include <OABase/StringUtility.h>

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OAModelDataAPI/Compilation/CompilationDataAPI.h>
#include <OAModelDataAPI/Compilation/CompilationFepItemInfo.h>
#include <tchar.h>


using namespace OA;
using namespace OA::ModelDataAPI;


SimulationItemManager::SimulationItemManager()
{
    m_strHost = _T("127.0.0.1:27017");
    m_strProject = _T("TEST");
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

    return status;
}

const std::vector<std::unique_ptr<FepSimulationItemInfo>>& SimulationItemManager::GetListItem() const
{
    return m_listItems;
}
