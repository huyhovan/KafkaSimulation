
#include "KafkaRecordInfo.h"

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlScenarioItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationTriggerScenarioItemInfo.h>

KafkaRecordInfo::KafkaRecordInfo()
{
    
}

KafkaRecordInfo::~KafkaRecordInfo()
{

}

bool operator==(const KafkaRecordInfo& lhs, const KafkaRecordInfo& rhs)
{
    if ((lhs.m_value == rhs.m_value)
        && (lhs.m_statusCode == rhs.m_statusCode)
        && (lhs.m_timeInterval == rhs.m_timeInterval)
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool operator!=(const KafkaRecordInfo& lhs, const KafkaRecordInfo& rhs)
{
    return !operator==(lhs, rhs);
}


OA::OAString KafkaRecordInfo::GetKey() const
{
    return m_key;
}

OA::OAVariant KafkaRecordInfo::GetValue() const
{
    return m_value;
}

OA::OAInt64 KafkaRecordInfo::GetTimeInterval() const
{
    return m_timeInterval;
}

OA::OAUInt32 KafkaRecordInfo::GetQuality() const
{
    return m_statusCode;
}

OA::OAUInt16 KafkaRecordInfo::GetDataTypeId() const
{
    return m_dataTypeId;
}

std::vector<OA::OAMethodParameter> KafkaRecordInfo::GetListMethodParam() const
{
    return m_listParam;
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

void KafkaRecordInfo::SetTimeInterval(OA::OAInt64 timeInterval)
{
    m_timeInterval = timeInterval;
}

void KafkaRecordInfo::SetQuality(OA::OAUInt32 statusCode)
{
    m_statusCode = statusCode;
}

void KafkaRecordInfo::SetDataTypeId(OA::OAUInt16 dataTypeId)
{
    m_dataTypeId = dataTypeId;
}

void KafkaRecordInfo::SetItemType(OA::ModelDataAPI::FepSimulationItemType itemType)
{
    m_itemType = itemType;
}


void KafkaRecordInfo::SetListMethodParam(const std::vector<OA::OAMethodParameter>& listMethodParam)
{
    m_listParam.resize(listMethodParam.size());
    m_listParam = listMethodParam;
}

// Random Generator RecordInfo 

KafkaRandomGeneratorRecordInfo::KafkaRandomGeneratorRecordInfo()
    : m_bGenning(true)
{
    SetItemType(OA::ModelDataAPI::FepSimulationItemType::RandomGenerator);
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

void KafkaRandomGeneratorRecordInfo::SetIsGenning(bool isGenning)
{
    m_bGenning = isGenning;
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

bool KafkaRandomGeneratorRecordInfo::IsGenning() const
{
    return m_bGenning;
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


// TriggerScenario
KafkaTriggerScenarioRecordInfo::KafkaTriggerScenarioRecordInfo()
{
    
    SetItemType(OA::ModelDataAPI::FepSimulationItemType::TriggerScenario);
}

KafkaTriggerScenarioRecordInfo::~KafkaTriggerScenarioRecordInfo()
{

}

const OA::OAString& KafkaTriggerScenarioRecordInfo::GetContent() const
{
    return m_content;
}

std::vector<OA::ModelDataAPI::FepSimulationTriggerScenarioInput> KafkaTriggerScenarioRecordInfo::GetInputs() const
{
    return m_inputs;
}

bool KafkaTriggerScenarioRecordInfo::GetRepeatEnabled() const
{
    return m_repeatEnabled;
}

int KafkaTriggerScenarioRecordInfo::GetRepeatInterval() const
{
    return m_repeatInterval;
}

void KafkaTriggerScenarioRecordInfo::SetContent(const OA::OAString& content)
{
    m_content = content;
}

void KafkaTriggerScenarioRecordInfo::SetInputs(const std::vector<OA::ModelDataAPI::FepSimulationTriggerScenarioInput>& listInputs)
{
    m_inputs = listInputs;
}

void KafkaTriggerScenarioRecordInfo::SetRepeatEnable(bool bEnable)
{
    m_repeatEnabled = bEnable;
}

void KafkaTriggerScenarioRecordInfo::SetRepeatInterval(int interval)
{
    m_repeatInterval = interval;
}
