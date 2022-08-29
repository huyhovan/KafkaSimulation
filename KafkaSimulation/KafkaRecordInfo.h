#pragma once

#include <OABase/OABase.h>
#include <OABase/OAStatus.h>
#include <OABase/OAString.h>
#include <OABase/OAVariant.h>
#include <OABase/StringUtility.h>
#include <OABase/OAUniqueID.h>
#include <OABase/OAMethodParameter.h>

#include <OAModelDataAPI/FepSimulation/FepSimulationControlConsequenceItemInfo.h>


namespace OA
{
    namespace ModelDataAPI
    {
        enum class FepSimulationItemType;
        class FepSimulationControlScenarioItemInfo;
        class FepSimulationTriggerScenarioInput;
        class FepSimulationTriggerScenarioItemInfo;
    }
}

class  KafkaRecordInfo
{
public:
    KafkaRecordInfo();
    virtual ~KafkaRecordInfo();

    friend bool operator==(const KafkaRecordInfo& lhs, const KafkaRecordInfo& rhs);
    friend bool operator!=(const KafkaRecordInfo& lhs, const KafkaRecordInfo& rhs);

    OA::OAString GetKey() const;
    OA::OAVariant GetValue() const;
    OA::OAInt64 GetTimeInterval() const;
    OA::OAUInt32 GetQuality() const;
    OA::OAUInt16 GetDataTypeId() const;
    std::vector<OA::OAMethodParameter> GetListMethodParam() const;
    OA::ModelDataAPI::FepSimulationItemType GetItemType() const;

    void SetKey(OA::OAString key);
    void SetValue(OA::OAVariant value);
    void SetTimeInterval(OA::OAInt64 timeInterval);
    void SetQuality(OA::OAUInt32 statusCode);
    void SetDataTypeId(OA::OAUInt16 dataTypeId);
    void SetItemType(OA::ModelDataAPI::FepSimulationItemType itemType);
    void SetListMethodParam(const std::vector<OA::OAMethodParameter>& listMethodParam);

protected:
    OA::OAString m_key;
    OA::OAVariant m_value;
    OA::OAInt64 m_timeInterval;
    OA::OAUInt32 m_statusCode;
    OA::OAUInt16 m_dataTypeId;
    std::vector<OA::OAMethodParameter> m_listParam;
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
    void SetIsGenning(bool isGenning);

    OA::OAUInt32 GetInterval() const;
    OA::OAVariant GetMinValue() const;
    OA::OAVariant GetMaxValue() const;
    bool IsGenning() const;

protected:
    OA::OAUInt32 m_interval;
    OA::OAVariant m_minValue;
    OA::OAVariant m_maxValue;
    bool m_bGenning;
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

class KafkaTriggerScenarioRecordInfo : public KafkaRecordInfo
{
public:
    KafkaTriggerScenarioRecordInfo();
    virtual ~KafkaTriggerScenarioRecordInfo();

    const OA::OAString& GetContent() const;    
    std::vector<OA::ModelDataAPI::FepSimulationTriggerScenarioInput> GetInputs() const;
    bool GetRepeatEnabled() const;
    int GetRepeatInterval() const;

    void SetContent(const OA::OAString& content);
    void SetInputs(const std::vector<OA::ModelDataAPI::FepSimulationTriggerScenarioInput>& listInputs);
    void SetRepeatEnable(bool bEnable);
    void SetRepeatInterval(int interval);

protected:
    OA::OAString m_content;  
    bool m_repeatEnabled;
    int m_repeatInterval;

    std::vector<OA::ModelDataAPI::FepSimulationTriggerScenarioInput> m_inputs;
};