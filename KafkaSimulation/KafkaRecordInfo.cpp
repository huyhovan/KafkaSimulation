#include "KafkaRecordInfo.h"
#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlScenarioItemInfo.h>


KafkaRecordInfo::KafkaRecordInfo()
{

}

KafkaRecordInfo::~KafkaRecordInfo()
{

}

OA::OAString KafkaRecordInfo::GetKey() const
{
    return m_key;
}

OA::OAVariant KafkaRecordInfo::GetValue() const
{
    return m_value;
}

OA::OADateTime KafkaRecordInfo::GetTimestamp() const
{
    return m_timeStamp;
}

OA::OAStatus KafkaRecordInfo::GetQuality() const
{
    return m_status;
}

OA::OAString KafkaRecordInfo::GetDatatype() const
{
    return m_strDataType;
}

OA::ModelDataAPI::FepSimulationItemType KafkaRecordInfo::GetItemType() const
{
    return m_itemType;
}

void KafkaRecordInfo::SetKey(OA::OAString key)
{
    m_key = key;
}

void KafkaRecordInfo::SetValue(OA::OAVariant value)
{
    m_value = value;
}

void KafkaRecordInfo::SetTimestamp(OA::OADateTime timestamp)
{
    m_timeStamp = timestamp;
}

void KafkaRecordInfo::SetQuality(OA::OAStatus status)
{
    m_status = status;
}

void KafkaRecordInfo::SetDataType(OA::OAString dataType)
{
    m_strDataType = dataType;
}

void KafkaRecordInfo::SetItemType(OA::ModelDataAPI::FepSimulationItemType itemType)
{
    m_itemType = itemType;
}


// Random Generator RecordInfo 

KafkaRandomGeneratorRecordInfo::KafkaRandomGeneratorRecordInfo()    
{
    //SetItemType(OA::ModelDataAPI::FepSimulationItemType::RandomGenerator);
}

KafkaRandomGeneratorRecordInfo::~KafkaRandomGeneratorRecordInfo()
{
   
}

void KafkaRandomGeneratorRecordInfo::SetInterval(OA::OAUInt32 interval)
{
    m_interval = interval;
}

void KafkaRandomGeneratorRecordInfo::SetMinvalue(OA::OAVariant minValue)
{
    m_minValue = minValue;
}

void KafkaRandomGeneratorRecordInfo::SetMaxValue(OA::OAVariant maxValue)
{
    m_maxValue = maxValue;
}

OA::OAUInt32 KafkaRandomGeneratorRecordInfo::GetInterval() const
{
    return m_interval;
}

OA::OAVariant KafkaRandomGeneratorRecordInfo::GetMinValue() const
{
    return m_minValue;
}

OA::OAVariant KafkaRandomGeneratorRecordInfo::GetMaxValue() const
{
    return m_maxValue;
}


// Control Consequence

KafkaControlConsequenceRecordInfo::KafkaControlConsequenceRecordInfo()
{
    SetItemType(OA::ModelDataAPI::FepSimulationItemType::ControlConsequence);
}

KafkaControlConsequenceRecordInfo::~KafkaControlConsequenceRecordInfo()
{

}

OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType KafkaControlConsequenceRecordInfo::GetControlType() const
{
    return m_controlType;
}

OA::OAString KafkaControlConsequenceRecordInfo::GetTarget() const
{
    return m_target;
}

void KafkaControlConsequenceRecordInfo::SetControlType(OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType controlType)
{
    m_controlType = controlType;
}

void KafkaControlConsequenceRecordInfo::SetTarget(OA::OAString target)
{
    m_target = target;
}


// Control Scenario

KafkaControlScenarioRecordInfo::KafkaControlScenarioRecordInfo()
{
    SetItemType(OA::ModelDataAPI::FepSimulationItemType::ControlScenario);
}

KafkaControlScenarioRecordInfo::~KafkaControlScenarioRecordInfo()
{
   
}

const std::vector<OA::OAUniqueID>& KafkaControlScenarioRecordInfo::GetInputs() const
{
    return m_listInput;
}

const OA::OAString& KafkaControlScenarioRecordInfo::GetContent() const
{
    return m_content;
}

const OA::OAString& KafkaControlScenarioRecordInfo::GetParameters() const
{
    return m_parameters;
}

void KafkaControlScenarioRecordInfo::SetContent(const OA::OAString& content)
{
    m_content = content;
}

void KafkaControlScenarioRecordInfo::SetParameters(const OA::OAString& params)
{
    m_parameters = params;
}

void KafkaControlScenarioRecordInfo::SetInputs(const std::vector<OA::OAUniqueID>& listInputs)
{
    m_listInput = listInputs;
}
