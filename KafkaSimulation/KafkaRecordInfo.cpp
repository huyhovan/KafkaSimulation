#include "KafkaRecordInfo.h"
#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>

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

