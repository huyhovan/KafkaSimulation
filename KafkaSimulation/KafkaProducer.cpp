#include "KafkaProducer.h"
#include "KafkaRecordInfo.h"
#include "KafkaConfig.h"

#include <nlohmann/json.hpp>
#include <tchar.h>
#include <vector>
#include <random>

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationInitializationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationRandomGeneratorItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlConsequenceItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlScenarioItemInfo.h>
#include <OABase/StringUtility.h>
#include <OAModelDataAPI/Compilation/CompilationDataAPI.h>


using namespace nlohmann;


KafkaProducer::KafkaProducer()
    :m_pProducer(nullptr)
{
    std::unique_ptr<KafkaConfig> pKafkaConfig = std::make_unique<KafkaConfig>();

    m_strBroker = pKafkaConfig->GetBorker();
    m_strTopics = pKafkaConfig->GetTopics();
    m_nPpartition = pKafkaConfig->GetPartition();
}

KafkaProducer::~KafkaProducer()
{
    Stop();
}

bool KafkaProducer::Initialize()
{
    OA::OAString err;

    std::string errstr = "";

    // Create configuration objects
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf* tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    // Set configuration properties
    /// broker list
    if (conf->set("metadata.broker.list", m_strBroker, errstr) != RdKafka::Conf::CONF_OK)
    {
        err = (_T("RdKafka conf set brokerlist failed : %s\n"), *errstr.c_str());
    }

    // Set delivery report callbak
    conf->set("dr_cb", &m_producerDeliveryReportCallBack, errstr);
    conf->set("event_cb", &m_producerEventCallBack, errstr);

    // CREATE PRODUCER USING ACCUMULATED GLOBAL CONFIGURATION
    m_pProducer = RdKafka::Producer::create(conf, errstr);
    if (!m_pProducer)
        return false;

    // Create topic handle
    m_pTopic = RdKafka::Topic::create(m_pProducer, m_strTopics, tconf, errstr);
    if (!m_pTopic)
    {
        err = (_T("Failed to create toppic:%s \n "), *errstr.c_str());
        return false;
    }

    return true;
}

void KafkaProducer::ProductMsg()
{   
    for (size_t i = 0; i < m_listRecords.size(); i++)
    {
        OA::ModelDataAPI::FepSimulationItemType itemType = m_listRecords[i]->GetItemType();

        json jsonRecord;

        int nDump = 4; // format for Json record

        switch (itemType)
        {
        case OA::ModelDataAPI::FepSimulationItemType::Initialization:
        {
            jsonRecord["key"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetKey());
            jsonRecord["value"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetValue().ToString());
            jsonRecord["timestamp"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetTimestamp().ToString());
            jsonRecord["status"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetQuality().ToString());

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::RandomGenerator:
        {
            jsonRecord["key"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetKey());
            jsonRecord["value"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetValue().ToString());
            jsonRecord["timestamp"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetTimestamp().ToString());
            jsonRecord["status"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetQuality().ToString());

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

        OA::OAString  stt;
        std::string s = jsonRecord.dump(nDump);

        // Produce Message
        RdKafka::ErrorCode resp = m_pProducer->produce(m_pTopic, m_nPpartition, RdKafka::Producer::RK_MSG_COPY /*copy payload*/, const_cast<char*>(s.c_str()), s.size(), NULL, NULL);
        if (resp != RdKafka::ERR_NO_ERROR)
        {
            std::cerr << "Produce failed: " << RdKafka::err2str(resp) << std::endl;
        }

        // block waiting for message
        m_pProducer->poll(0);

        // wait for message to be delivery  //firecat add
        while (m_pProducer->outq_len() > 0)
        {
            stt = (_T("Waiting for %d\n"), m_pProducer->outq_len());
            m_pProducer->poll(100);
        }
    }
}

void KafkaProducer::ProductMsg(const std::string& key, const std::string value, std::string& timestamp, std::string& status)
{
    OA::OAString  stt;

    {
        json jsonRecord;

        jsonRecord["key"] = key;
        jsonRecord["value"] = value;
        jsonRecord["timestamp"] = timestamp;
        jsonRecord["status"] = status;

        std::string s = jsonRecord.dump(4);

        // Produce Message
        RdKafka::ErrorCode resp = m_pProducer->produce(m_pTopic, m_nPpartition, RdKafka::Producer::RK_MSG_COPY /*copy payload*/, const_cast<char*>(s.c_str()), s.size(), NULL, NULL);
        if (resp != RdKafka::ERR_NO_ERROR)
        {
            std::cerr << "Produce failed: " << RdKafka::err2str(resp) << std::endl;
        }
    }

    // block waiting for message
    m_pProducer->poll(0);

    // wait for message to be delivery  //firecat add
    while (m_pProducer->outq_len() > 0)
    {
        stt = (_T("Waiting for %d\n"), m_pProducer->outq_len());
        m_pProducer->poll(100);
    }
}

void KafkaProducer::ProductMsg(KafkaRecordInfo* pRecord)
{
    OA::ModelDataAPI::FepSimulationItemType itemType = pRecord->GetItemType();

    json jsonRecord;

    int nDump = 0;

    nDump = 4;

    jsonRecord["key"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetKey());
    jsonRecord["value"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetValue().ToString());
    jsonRecord["timestamp"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetTimestamp().ToString());
    jsonRecord["status"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetQuality().ToString());    

    OA::OAString  stt;
    std::string s = jsonRecord.dump(nDump);

    // Produce Message
    RdKafka::ErrorCode resp = m_pProducer->produce(m_pTopic, m_nPpartition, RdKafka::Producer::RK_MSG_COPY /*copy payload*/, const_cast<char*>(s.c_str()), s.size(), NULL, NULL);
    if (resp != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "Produce failed: " << RdKafka::err2str(resp) << std::endl;
    }

    // block waiting for message
    m_pProducer->poll(0);

    // wait for message to be delivery  //firecat add
    while (m_pProducer->outq_len() > 0)
    {
        stt = (_T("Waiting for %d\n"), m_pProducer->outq_len());
        m_pProducer->poll(100);
    }
}

void KafkaProducer::CreateKafkaRecord(const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& listItems)
{
    m_listRecords.clear();
   
    // For Testing    
    //CreatInitializeRecord();
    //CreateSingleRandomRecord();   

    for (size_t i = 0; i < listItems.size(); i++)
    {
        OA::OAString key = listItems[i]->GetItemKey();

        OA::ModelDataAPI::FepSimulationItemType type = listItems[i]->GetItemType();

        std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();

        switch (type)
        {
        case OA::ModelDataAPI::FepSimulationItemType::Initialization:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationInitializationItemInfo*>(listItems[i].get());
            OA::OAVariant initValue = pItem->GetInitialValue();
            if (initValue == 0)
                initValue = rand() % 100 + 1;
            OA::OAUInt16 dataType = pItem->GetDataType();
            OA::OAString strDataType = OA::StringUtility::BuiltinDataTypeToString(dataType);

            pRecord->SetKey(key);
            pRecord->SetValue(initValue);
            pRecord->SetDataType(strDataType);
            pRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::Initialization);                                

            m_listRecords.emplace_back(std::move(pRecord));
            m_mapKeyValue.emplace(key, initValue);

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::RandomGenerator:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationRandomGeneratorItemInfo*>(listItems[i].get());

            std::unique_ptr<KafkaRandomGeneratorRecordInfo> pRandomRecord = std::make_unique<KafkaRandomGeneratorRecordInfo>();
     
            pRandomRecord->SetKey(key);

            OA::OAUInt16 dataType = pItem->GetDataType();
            OA::OAString strDataType = OA::StringUtility::BuiltinDataTypeToString(dataType);
            pRandomRecord->SetDataType(strDataType);

            OA::OAUInt32 interval = pItem->GetInterval();
            pRandomRecord->SetInterval(interval);            

            OA::OAVariant minValue = pItem->GetMinValue();
            pRandomRecord->SetMinvalue(minValue);          

            OA::OAVariant maxValue = pItem->GetMaxValue();
            pRandomRecord->SetMaxValue(maxValue);       

            pRandomRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::RandomGenerator);

            m_listRecords.emplace_back(std::move(pRandomRecord));

            break;
        }           
        case OA::ModelDataAPI::FepSimulationItemType::ControlConsequence:
        {       
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo*>(listItems[i].get());

            std::unique_ptr<KafkaControlConsequenceRecordInfo> pControlConsequence = std::make_unique<KafkaControlConsequenceRecordInfo>();

            pControlConsequence->SetKey(key);
            pControlConsequence->SetControlType(pItem->GetControlType());
            pControlConsequence->SetTarget(pItem->GetTarget());

            m_listRecords.emplace_back(std::move(pControlConsequence));

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::ControlScenario:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationControlScenarioItemInfo*>(listItems[i].get());

            std::unique_ptr<KafkaControlScenarioRecordInfo> pControlScenario = std::make_unique<KafkaControlScenarioRecordInfo>();

            pControlScenario->SetKey(key);
            pControlScenario->SetContent(pItem->GetContent());
            pControlScenario->SetParameters(pItem->GetParameters());
            pControlScenario->SetInputs(pItem->GetInputs());

            m_listRecords.emplace_back(std::move(pControlScenario));

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::TriggerScenario:
            break;
        default:
            break;
        }       
    }    
}

void KafkaProducer::AddKafkaRecord(std::unique_ptr<KafkaRecordInfo> pRecord)
{
    m_listRecords.emplace_back(std::move(pRecord));
}

void KafkaProducer::Stop()
{
    delete m_pTopic;
    delete m_pProducer;
}

const std::vector<std::unique_ptr<KafkaRecordInfo>>& KafkaProducer::GetListRecord() const
{
    return m_listRecords;
}

const std::map<OA::OAString, OA::OAVariant>& KafkaProducer::GetMapKeyValue() const
{
    return m_mapKeyValue;
}

void KafkaProducer::CreateSingleRandomRecord()
{
    std::cout <<  ".RandomGenerator Simulation" << std::endl;

    std::unique_ptr<KafkaRandomGeneratorRecordInfo> randomRecord = std::make_unique<KafkaRandomGeneratorRecordInfo>();

    OA::OAString keyRandomTest = _T("PointRandomTest");
    randomRecord->SetKey(keyRandomTest);
    randomRecord->SetDataType(OA::StringUtility::BuiltinDataTypeToString(OA_DataType_Float));
    randomRecord->SetInterval(5000);
    randomRecord->SetMinvalue(1);
    randomRecord->SetMaxValue(1000);

    /* std::random_device                  rand_dev;
     std::mt19937                        generator(rand_dev());
     std::uniform_int_distribution<int>  distr(1, 1000);

     OA::OAVariant value = distr(generator);
     randomRecord->SetValue(value);*/

    m_listRecords.emplace_back(std::move(randomRecord));
}

void KafkaProducer::CreateInitializeRecord()
{
    for (int i = 0; i < 3; i++)
    {
        std::cout << ("%d", i + 1) << " .Initialize Simulation" << std::endl;

        OA::OAString key = _T("PointInitTest");

        std::unique_ptr<KafkaRecordInfo> pTestRecord = std::make_unique<KafkaRecordInfo>();

        OA::OAVariant initValue = 123;// rand() % 100 + 1;
        OA::OAString strDataType = OA::StringUtility::BuiltinDataTypeToString(OA_DataType_Float);

        pTestRecord->SetKey(key);
        pTestRecord->SetValue(initValue);
        pTestRecord->SetDataType(strDataType);
        pTestRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::Initialization);

        m_listRecords.emplace_back(std::move(pTestRecord));        
    }
}

bool KafkaProducer::HasDataChange(KafkaRecordInfo* pRecord)
{
    auto it = m_mapKeyValue.find(pRecord->GetKey());
    if (it == m_mapKeyValue.end())
    {
        m_mapKeyValue.emplace(pRecord->GetKey(), pRecord->GetValue());

        return true;
    }
    else
    {
        OA::OAVariant oldValue = it->second;

        if (pRecord->GetValue() != oldValue)
        {           
            m_mapKeyValue[pRecord->GetKey()] = pRecord->GetValue();

            return true;
        }
    }

    return false;
}

void KafkaProducer::UpdateMapKeyValue(OA::OAString key, OA::OAVariant& value)
{
    m_mapKeyValue[key] = value;
}
