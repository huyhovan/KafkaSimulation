#pragma once

#include <OABase/OABase.h>
#include <OABase/OAStatus.h>
#include <OABase/OAString.h>
#include <OABase/OAVariant.h>
#include <OABase/StringUtility.h>

namespace OA
{
    namespace ModelDataAPI
    {
        enum class FepSimulationItemType;
    }
}

class  KafkaRecordInfo
{
public:
    KafkaRecordInfo();
    virtual ~KafkaRecordInfo();

    OA::OAString GetKey() const;
    OA::OAVariant GetValue() const;
    OA::OADateTime GetTimestamp() const;
    OA::OAStatus GetQuality() const;
    OA::OAString GetDatatype() const;
    OA::ModelDataAPI::FepSimulationItemType GetItemType() const;

    void SetKey(OA::OAString key);
    void SetValue(OA::OAVariant value);
    void SetTimestamp(OA::OADateTime timestamp);
    void SetQuality(OA::OAStatus status);
    void SetDataType(OA::OAString dataType);
    void SetItemType(OA::ModelDataAPI::FepSimulationItemType itemType);

protected:
    OA::OAString m_key;
    OA::OAVariant m_value;
    OA::OADateTime m_timeStamp;
    OA::OAStatus m_status;
    OA::OAString m_strDataType;
    OA::ModelDataAPI::FepSimulationItemType m_itemType;
};

class KafkaRandomGeneratorRecordInfo : public KafkaRecordInfo
{
public:
    KafkaRandomGeneratorRecordInfo();
    virtual ~KafkaRandomGeneratorRecordInfo();
   
    void SetInterval(OA::OAUInt32 interval);
    void SetMinvalue(OA::OAVariant minValue);
    void SetMaxValue(OA::OAVariant maxValue);

    OA::OAUInt32 GetInterval() const;
    OA::OAVariant GetMinValue() const;
    OA::OAVariant GetMaxValue() const;

protected:
    OA::OAUInt32 m_interval;
    OA::OAVariant m_minValue;
    OA::OAVariant m_maxValue;
};
