#include "SimulationModelNode.h"

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>

SimulationModelNode::SimulationModelNode()
{

}

SimulationModelNode::~SimulationModelNode()
{

}

OA::OAString SimulationModelNode::GetKey() const
{
    return m_key;
}

OA::ModelDataAPI::FepSimulationItemInfo* SimulationModelNode::GetItemInfo() const
{
    return m_pItemInfo;
}

OA::OAVariant SimulationModelNode::GetValue() const
{
    return m_Value;
}

OA::OADateTime SimulationModelNode::GetTimestamp() const
{
    return m_timestamp;
}

OA::OAStatus SimulationModelNode::GetQuality() const
{
    return m_status;
}

void SimulationModelNode::SetKey(OA::OAString key)
{
    m_key = key;
}

void SimulationModelNode::SetItemInfo(OA::ModelDataAPI::FepSimulationItemInfo* itemInfo)
{
    m_pItemInfo = itemInfo;
}

void SimulationModelNode::SetValue(OA::OAVariant value)
{
    m_Value = value;
}

void SimulationModelNode::SetTimestamp(OA::OADateTime timestamp)
{
    m_timestamp = timestamp;
}

void SimulationModelNode::SetQuality(OA::OAStatus status)
{
    m_status = status;
}
