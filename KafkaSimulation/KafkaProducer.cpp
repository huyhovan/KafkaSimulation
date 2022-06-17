#include "KafkaProducer.h"
#include "KafkaRecordInfo.h"

#include <nlohmann/json.hpp>
#include <XMLParser/tinyxml2.h>
#include <XMLParser/tinyxml2.cpp>
#include <tchar.h>
#include <vector>
#include <random>

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationInitializationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationRandomGeneratorItemInfo.h>
#include <OABase/StringUtility.h>

using namespace nlohmann;
using namespace tinyxml2;

KafkaProducer::KafkaProducer()
    :m_pProducer(nullptr)
{
    LoadSettingInfoFromXML();
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
   /* const std::string jsonRecord = R"(
            {
            "key" : "point1",
            "value": 123,
            "timestamp" : 123456789,
            "status": 1
            }        
     )";*/
  
    for (size_t i = 0; i < m_listRecords.size(); i++)
    {
        OA::ModelDataAPI::FepSimulationItemType itemType = m_listRecords[i]->GetItemType();

        json jsonRecord;

        int nDump = 0;

        switch (itemType)
        {
        case OA::ModelDataAPI::FepSimulationItemType::Initialization:
        {
            nDump = 4;

            jsonRecord["key"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetKey());
            jsonRecord["value"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetValue().ToString());
            jsonRecord["timestamp"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetTimestamp().ToString());
            jsonRecord["status"] = OA::StringUtility::Utf16ToUtf8(m_listRecords[i]->GetQuality().ToString());

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::RandomGenerator:
        {
            nDump = 4;

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

    /* json jsonRecord;

     jsonRecord["key"] = listRecord[i]->GetKey();
     jsonRecord["value"] = listRecord;
     jsonRecord["timestamp"] = timestamp;
     jsonRecord["status"] = status;*/
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

    switch (itemType)
    {
    case OA::ModelDataAPI::FepSimulationItemType::Initialization:
    {
        nDump = 4;

        jsonRecord["key"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetKey());
        jsonRecord["value"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetValue().ToString());
        jsonRecord["timestamp"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetTimestamp().ToString());
        jsonRecord["status"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetQuality().ToString());

        break;
    }
    case OA::ModelDataAPI::FepSimulationItemType::RandomGenerator:
    {
        nDump = 4;

        jsonRecord["key"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetKey());
        jsonRecord["value"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetValue().ToString());
        jsonRecord["timestamp"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetTimestamp().ToString());
        jsonRecord["status"] = OA::StringUtility::Utf16ToUtf8(pRecord->GetQuality().ToString());

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

void KafkaProducer::CreateKafkaRecord(const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& lisItems)
{

    // For Testing
    m_listRecords.clear();
    CreatInitializeRecord();
    CreateSingleRandomRecord();


    for (size_t i = 0; i < lisItems.size(); i++)
    {
        OA::OAString key = lisItems[i]->GetItemKey();

        OA::ModelDataAPI::FepSimulationItemType type = lisItems[i]->GetItemType();

        std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();

        switch (type)
        {
        case OA::ModelDataAPI::FepSimulationItemType::Initialization:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationInitializationItemInfo*>(lisItems[i].get());
            OA::OAVariant initValue = pItem->GetInitialValue();
            if (initValue == 0)
                initValue = rand() % 100 + 1;
            OA::OAUInt16 dataType = pItem->GetDataType();
            OA::OAString strDataType = OA::StringUtility::BuiltinDataTypeToString(dataType);

            pRecord->SetKey(key);
            pRecord->SetValue(initValue);
            pRecord->SetDataType(strDataType);
            pRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::Initialization);                                

            break;
        }
        case OA::ModelDataAPI::FepSimulationItemType::RandomGenerator:
        {
            auto pItem = static_cast<OA::ModelDataAPI::FepSimulationRandomGeneratorItemInfo*>(lisItems[i].get());

            KafkaRandomGeneratorRecordInfo* pRandomRecord = static_cast<KafkaRandomGeneratorRecordInfo*>(pRecord.get());         

            OA::OAUInt16 dataType = pItem->GetDataType();
            OA::OAString strDataType = OA::StringUtility::BuiltinDataTypeToString(dataType);
            
            OA::OAUInt32 interval = pItem->GetInterval();
            pRandomRecord->SetInterval(interval);            

            OA::OAVariant minValue = pItem->GetMinValue();
            pRandomRecord->SetMinvalue(minValue);          

            OA::OAVariant maxValue = pItem->GetMaxValue();
            pRandomRecord->SetMaxValue(maxValue);                     

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

void KafkaProducer::InitSettingInfoXML()
{
    tinyxml2::XMLDocument xmlDoc;

    XMLElement* pRoot = xmlDoc.NewElement("SettingKafkaInfo");
    pRoot->SetAttribute("Version", "");
    pRoot->SetAttribute("Platform", "");
    xmlDoc.InsertFirstChild(pRoot);

    XMLElement* address = xmlDoc.NewElement("Address");
    address->SetText("localhost:9092");
    pRoot->InsertEndChild(address);

    XMLElement* pjName = xmlDoc.NewElement("ProjectName");
    pjName->SetText("oa-ds-data-input");
    pRoot->InsertEndChild(pjName);

    xmlDoc.SaveFile("SettingInfo.xml");
}

void KafkaProducer::CreateSingleRandomRecord()
{
    std::cout <<  ".RandomGenerator Simulation" << std::endl;

    std::unique_ptr<KafkaRandomGeneratorRecordInfo> randomRecord = std::make_unique<KafkaRandomGeneratorRecordInfo>();;

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

void KafkaProducer::CreatInitializeRecord()
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

void KafkaProducer::LoadSettingInfoFromXML()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.LoadFile("SettingInfo.xml");
    if (err != XML_SUCCESS)
    {
        InitSettingInfoXML();
        LoadSettingInfoFromXML();

        return;
    }

    m_strBroker = doc.FirstChildElement("SettingKafkaInfo")->FirstChildElement("Address")->GetText();
    m_strTopics = doc.FirstChildElement("SettingKafkaInfo")->FirstChildElement("ProjectName")->GetText();
    m_nPpartition = 0;
}