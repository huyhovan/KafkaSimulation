#pragma once

#include <OABase/OABase.h>
#include <OABase/OAStatus.h>
#include <OABase/OAString.h>
#include <OABase/OAVariant.h>
#include <OABase/StringUtility.h>
#include <OABase/OAUniqueID.h>

#include <OAModelDataAPI/FepSimulation/FepSimulationControlConsequenceItemInfo.h>

namespace OA
{
    namespace ModelDataAPI
    {
        enum class FepSimulationItemType;
        class FepSimulationControlScenarioItemInfo;
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

class KafkaControlConsequenceRecordInfo : public KafkaRecordInfo
{
public:
    KafkaControlConsequenceRecordInfo();
    virtual ~KafkaControlConsequenceRecordInfo();

    OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType GetControlType() const;
    OA::OAString GetTarget() const;

    void SetControlType(OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType controlType);
    void SetTarget(OA::OAString target);

protected:
    OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType m_controlType;
    OA::OAString m_target;
   
};


class KafkaControlScenarioRecordInfo : public KafkaRecordInfo
{
public:
    KafkaControlScenarioRecordInfo();
    virtual ~KafkaControlScenarioRecordInfo();

    const std::vector<OA::OAUniqueID>& GetInputs() const;
    const OA::OAString& GetContent() const;
    const OA::OAString& GetParameters() const;

    void SetContent(const OA::OAString& content);
    void SetParameters(const OA::OAString& params);
    void SetInputs(const std::vector<OA::OAUniqueID>& listInputs);

protected:
    OA::OAString m_content;
    OA::OAString m_parameters;
    std::vector<OA::OAUniqueID> m_listInput;
};