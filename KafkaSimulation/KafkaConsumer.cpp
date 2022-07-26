#include "KafkaConsumer.h"
#include "KafkaConfig.h"
#include "KafkaRecordInfo.h"
#include "KafkaProducer.h"
#include "SimulationItemManager.h"

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlConsequenceItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlScenarioItemInfo.h>
#include <OABase/StringUtility.h>

#include <chrono>
#include <time.h>
#include <algorithm>

constexpr int LOCAL_TIME_ZONE = 7;

using namespace nlohmann;


KafkaConsumer::KafkaConsumer(SimulationItemManager* pSimulatioMng, KafkaProducer* pProduct)
    : m_pSimullationItemManager(pSimulatioMng), m_pKafkaProducer(pProduct)
{
    std::unique_ptr<KafkaConfig> pKafkaConfig = std::make_unique<KafkaConfig>();

    m_strBroker = pKafkaConfig->GetBorker();
    m_strTopicControl = pKafkaConfig->GetTopicControl();
    m_nPartition = pKafkaConfig->GetPartition();

    if (!m_pKafkaProducer)
        m_pKafkaProducer = std::make_unique<KafkaProducer>().get();

    //m_pKafkaProducer->Initialize();
}

KafkaConsumer::~KafkaConsumer()
{
    Stop();
}

bool KafkaConsumer::Initialize()
{
    std::string errstr;

    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    if (!conf)
    {
        std::cerr << "Rdkafka create global config failed" << std::endl;
        return false;
    }

    // broker list
    if (conf->set("metadata.broker.list", m_strBroker, errstr) != RdKafka::Conf::CONF_OK)
    {
        std::cerr << "RdKafka conf set brokerlist failed :" << errstr.c_str() << std::endl;
    }

    std::string strfetch_num = "10240000000";
    // the maximum size of each pull message from a single partition
    if (conf->set("max.partition.fetch.bytes", strfetch_num, errstr) != RdKafka::Conf::CONF_OK)
    {
        std::cerr << "RdKafke conf set max.partition false :" << errstr.c_str() << std::endl;
    }

    // CREATE CONSUMER  USING ACCUMULATED GLOBAL CONFIGURATION
    m_pKafkaConsumer = RdKafka::Consumer::create(conf, errstr);
    if (!m_pKafkaConsumer)
    {
        std::cerr << "failed to create consumer" << std::endl;
    }
    std::cout << "Create consumer: " << m_pKafkaConsumer->name() << std::endl;
    delete conf;

    // Topic
    RdKafka::Conf* tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    if (!tconf)
    {
        std::cerr << "rdkafka create topics config failed" << std::endl;
        return false;
    }
    if (tconf->set("auto.offset.reset", "latest", errstr) != RdKafka::Conf::CONF_OK)
    {
        std::cerr << "RdKafke conf set auto.offset.reset false :" << errstr.c_str() << std::endl;
    }

    // create topic handle
    m_pTopic = RdKafka::Topic::create(m_pKafkaConsumer, m_strTopicControl, tconf, errstr);
    if (!m_pTopic)
    {
        std::cerr << "RdKafke create topic false :" << errstr.c_str() << std::endl;
    }
    delete tconf;

    //START CONSUMER FOR TOPIC + PARTITION AT START OFFSET
    RdKafka::ErrorCode resp = m_pKafkaConsumer->start(m_pTopic, m_nPartition, /*m_nCurrentOffset*/ RdKafka::Topic::OFFSET_END);
    if (resp != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "Failed to start consumer" << errstr.c_str() << std::endl;
    }

    return true;
}

void KafkaConsumer::Start(int timeout_ms)
{
    RdKafka::Message* msg = NULL;
    m_bRun = true;

    while (m_bRun)
    {

        msg = m_pKafkaConsumer->consume(m_pTopic, m_nPartition, timeout_ms);
        MsgConsumer(msg, NULL);
        delete msg;
        m_pKafkaConsumer->poll(0);
    }

    if (m_pTopic)
    {
        delete m_pTopic;
        m_pTopic = NULL;
    }

    if (m_pKafkaConsumer)
    {
        delete m_pKafkaConsumer;
        m_pKafkaConsumer = NULL;
    }

    // wait for RdKafka to decomission
    RdKafka::wait_destroyed(5000);
}

void KafkaConsumer::Stop()
{
    m_bRun = false;
}

OA::ModelDataAPI::FepSimulationItemType KafkaConsumer::GetSimualtatioItemType(int nItemType)
{
    if (nItemType == 1)
    {
        return  OA::ModelDataAPI::FepSimulationItemType::Initialization;
    }

    if (nItemType == 2)
    {
        return  OA::ModelDataAPI::FepSimulationItemType::RandomGenerator;
    }

    if (nItemType == 3 )
    {
        return OA::ModelDataAPI::FepSimulationItemType::ControlConsequence;
    }

    if (nItemType == 4)
    {
        return OA::ModelDataAPI::FepSimulationItemType::ControlScenario;
    }

    if (nItemType == 5)
    {
        return  OA::ModelDataAPI::FepSimulationItemType::TriggerScenario;
    }    
}

OA::OADateTime KafkaConsumer::GetTimestampWithLocalTimeZone(RdKafka::MessageTimestamp& kafkaTimestamp)
{
    OA::OADateTime timestamp; 
    
    if (kafkaTimestamp.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE)
    {
        std::string timeprefix;
        if (kafkaTimestamp.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME)
        {
            timeprefix = "create time";
        }
        else if (kafkaTimestamp.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME) {
            timeprefix = "log append time";
        }

        unsigned long long milli = kafkaTimestamp.timestamp + (unsigned long long)LOCAL_TIME_ZONE * 60 * 60 * 1000;

        auto mTime = std::chrono::milliseconds(milli);

        auto tp = std::chrono::time_point<std::chrono::system_clock>(mTime);
        auto tt = std::chrono::system_clock::to_time_t(tp);

        tm timeinfo;
        ::gmtime_s(&timeinfo, &tt);

       /* char s[60]{ 0 };
        ::sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d:%03d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        std::string sTime = s;        */

        timestamp = OA::OADateTime::BuildDateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        OA::OAString strTimestamp = timestamp.ToShortStringWithLocalTimeZone();

        return timestamp;
    }
    else
    {
        OA::OAString strTimestamp = OA::OADateTime::GetNullTimeStringWithOAFormat();
        return NULL;
    }   
}

void KafkaConsumer::MsgConsumer(RdKafka::Message* message, void* opaque)
{
    switch (message->err())
    {
    case RdKafka::ERR__TIMED_OUT:
        break;

    case  RdKafka::ERR_NO_ERROR:
    {  // real message 
        std::cout << "read message at offset " << message->offset() << std::endl;
        if (message->key())
        {
            std::cout << "key: " << message->key() << std::endl;
        }
        printf("%.*s\n", static_cast<int>(message->len()), static_cast<const char*>(message->payload()));

        std::string strRecord = static_cast<const char*>(message->payload());

        // for timestamp        
        OA::OADateTime timestamp = GetTimestampWithLocalTimeZone(message->timestamp());       

        //ParseKafkaDetection(strRecord, time);
        CreateKafkaRecordControl(strRecord, timestamp);

        m_nLastOffset = message->offset();
        break;
    }
    case RdKafka::ERR__PARTITION_EOF:
        // Last message
        std::cout << "Reach the end of the queue, offset: " << m_nLastOffset << std::endl;
        // Stop()
        break;
    case  RdKafka::ERR__UNKNOWN_TOPIC:
    {
        std::cerr << "Unknow TOPIC " << message->errstr() << std::endl;
        Stop();
        break;
    }
    case RdKafka::ERR__UNKNOWN_PARTITION:
        std::cerr << "consume false " << message->errstr() << std::endl;
        Stop();
        break;

    default:
        // error
        std::cerr << "Consumer failed" << message->errstr() << std::endl;
        break;
    }
}

void KafkaConsumer::CreateKafkaRecordControl(std::string msg, OA::OADateTime& timestamp)
{
    json jsonObj = json::parse(msg);

    // Determine which Method been called by (key- itemType)
    OA::OAString key = OA::StringUtility::Utf8ToUtf16(jsonObj["key"]);

    int nItemType = jsonObj["itemType"];

    json data = jsonObj["inputArgument"];
    std::vector<bool> listArgument;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        listArgument.emplace_back(*it);
    }

    OA::ModelDataAPI::FepSimulationItemType itemType = GetSimualtatioItemType(nItemType);

    const std::map<OA::OAString, std::vector<OA::ModelDataAPI::FepSimulationItemInfo*>>& mapKeySimulationItems = m_pSimullationItemManager->GetMapKeySimulationItems();

    auto it = mapKeySimulationItems.find(key);
    if (it != mapKeySimulationItems.end())
    {
        std::vector<OA::ModelDataAPI::FepSimulationItemInfo*> listItem = it->second;
        for (auto& pItem : listItem)
        {
            std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();

            if (itemType == pItem->GetItemType())
            {
                const std::map<OA::OAString, OA::OAVariant>& m_mapKeyValue = m_pKafkaProducer->GetMapKeyValue();

                switch (itemType)
                {
                case OA::ModelDataAPI::FepSimulationItemType::Initialization:
                    break;
                case OA::ModelDataAPI::FepSimulationItemType::RandomGenerator:
                    break;
                case OA::ModelDataAPI::FepSimulationItemType::ControlConsequence:
                {
                    OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo* pControlConsequence = static_cast<OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo*>(pItem);

                    OA::OAString targetKey = pControlConsequence->GetTarget();
                    pRecord->SetKey(targetKey);

                    pRecord->SetTimestamp(timestamp);

                    pRecord->SetQuality(OA_StatusCode_Good);

                    OA::OAVariant value;
                    auto iter = m_mapKeyValue.find(targetKey);
                    if (iter != m_mapKeyValue.end())
                    {
                        value = iter->second;

                        pRecord->SetDataType(OA::StringUtility::BuiltinDataTypeToString(value.GetDataType()));
                    }

                    OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType controlType = pControlConsequence->GetControlType();
                    switch (controlType)
                    {
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::RB:
                    {
                        if (listArgument[0])
                            value = !value;                     

                        break;
                    }
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::CB:
                    {
                        value = (listArgument[0]) ? 2 : 1;
                        break;
                    }                      
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::SP:
                    {
                        value = (listArgument[0]) ? 1 : 0;                       
                        break;
                    }
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::TapChgUp:
                    {
                        OA::OAInt16 tempValue;
                        OA::OAStatus stt = value.GetInt16(tempValue);
                        if (stt == OA_StatusCode_Good)
                        {
                            if (listArgument[0])
                                tempValue++;
                            value = tempValue;
                        }

                        break;
                    }
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::TapChgDn:
                    {
                        OA::OAInt16 tempValue;
                        OA::OAStatus stt = value.GetInt16(tempValue);
                        if (stt == OA_StatusCode_Good)
                        {
                            if (listArgument[0])
                                tempValue--;
                            value = tempValue;
                        }

                        break;
                    }
                    default:
                        break;
                    }
                    pRecord->SetValue(value);

                    m_pKafkaProducer->ProductMsg(pRecord.get());
                    m_pKafkaProducer->UpdateMapKeyValue(targetKey, pRecord->GetValue());
                    
                    break;
                }                  
                case OA::ModelDataAPI::FepSimulationItemType::ControlScenario:
                {
                    OA::ModelDataAPI::FepSimulationControlScenarioItemInfo* pControlScenario = static_cast<OA::ModelDataAPI::FepSimulationControlScenarioItemInfo*>(pItem);

                    // Create number records = number of input
                    break;
                }
                                 
                case OA::ModelDataAPI::FepSimulationItemType::TriggerScenario:
                    break;
                default:
                    break;
                }

                break;
            }
        }
    }
    else
    {
        std::cout << "The Method been called is not Simulate Control" << std::endl;
    }
}

//void KafkaConsumer::ParseKafkaDetection(std::string msg, OA::OADateTime timestamp)
//{
//    json jsonObj = json::parse(msg);
//
//    // Determine which Method been called by (key- itemType)
//    std::string key = jsonObj["key"];
//    int itemType = jsonObj["itemType"];
//    OA::OAString targetKey = OA::StringUtility::Utf8ToUtf16(jsonObj["target"]);
//
//    auto it = m_mapKeyRecord.find(targetKey);
//    if (it == m_mapKeyRecord.end())
//    {
//        std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();
//
//        pRecord->SetKey(targetKey);
//
//        OA::OAString strTimestamp = timestamp.ToStringWithLocalTimeZone();
//        pRecord->SetTimestamp(timestamp);
//
//        json data = jsonObj["inputArgument"];
//        std::vector<bool> listArgument;
//        for (auto it = data.begin(); it != data.end(); ++it)
//        {
//            listArgument.emplace_back(*it);
//        }
//
//        OA::OAVariant varValue;
//        int value = jsonObj["value"];
//
//        if (itemType == 2 /*Control Consequence*/ && listArgument.size() == 1)
//        {
//            pRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::ControlConsequence);
//            KafkaControlConsequenceRecordInfo* pTemp = static_cast<KafkaControlConsequenceRecordInfo*>(pRecord.get());
//            auto pControlConsequence = std::make_unique<KafkaControlConsequenceRecordInfo>(*pTemp);       
//
//            OA::OAUInt8 ctrlType;
//            ctrlType = jsonObj["controltype"];
//
//            switch (ctrlType)
//            {
//            case 0:/*RB: reverse bit*/
//            {
//                bool tempValue = (value) ? true : false;
//
//                if (listArgument[0])
//                    tempValue = !tempValue;
//
//                varValue = tempValue;
//
//                break;
//            }
//            case 1: /*CB*/
//            {
//                value = (listArgument[0]) ? 2 : 1;
//                varValue = value;
//                break;
//            }
//            case 2: /*SP*/
//            {
//                value = (listArgument[0]) ? 1 : 0;
//                varValue = value;
//                break;
//            }
//            case 3: /*TapChgup*/
//            {
//                if (listArgument[0])
//                    value++;
//
//                varValue = value;
//
//                break;
//            }
//            case 4: /*TapChgDown*/
//            {
//                if (listArgument[0])
//                    value--;
//
//                varValue = value;
//
//                break;
//            }
//
//            default:
//                break;
//            }
//
//            pRecord->SetValue(varValue);
//            pRecord->SetQuality(OA_StatusCode_Good);
//        }
//
//        if (itemType == 3 /*Control Scenario*/)
//        {
//            KafkaControlScenarioRecordInfo* pTemp = static_cast<KafkaControlScenarioRecordInfo*>(pRecord.get());
//            pTemp->SetItemType(OA::ModelDataAPI::FepSimulationItemType::ControlScenario);
//            auto pControlScenario = std::make_unique<KafkaControlScenarioRecordInfo>(*pTemp);
//
//            pControlScenario->SetKey(OA::StringUtility::Utf8ToUtf16(key));
//
//        }
//
//        if (itemType == 4 /*Trigger Scenario*/)
//        {
//            pRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::TriggerScenario);
//        }
//
//        m_pKafkaProducer->ProductMsg(pRecord.get());
//
//        m_mapKeyRecord.emplace(targetKey, std::move(pRecord));
//    }
//
//    else
//    {
//        auto pOldRecord = std::move(it->second);
//
//        OA::OAString strTimestamp = timestamp.ToStringWithLocalTimeZone();
//        pOldRecord->SetTimestamp(timestamp);
//
//        OA::OAVariant oldValue = pOldRecord->GetValue();
//        OA::OAVariant newValue;
//
//        json data = jsonObj["inputArgument"];
//        std::vector<bool> listArgument;
//        for (auto it = data.begin(); it != data.end(); ++it)
//        {
//            listArgument.emplace_back(*it);
//        }
//
//        if (itemType == 2 /*Control Consequence*/ && listArgument.size() == 1)
//        {
//            pOldRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::ControlConsequence);
//
//            OA::OAUInt8 ctrlType;
//            ctrlType = jsonObj["controltype"];
//
//            switch (ctrlType)
//            {
//            case 0:/*RB: reverse bit*/
//            {
//                bool tempValue = (oldValue) ? true : false;
//
//                if (listArgument[0])
//                    tempValue = !tempValue;
//
//                newValue = tempValue;
//
//                break;
//            }
//            case 1: /*CB*/
//            {
//                newValue = (listArgument[0]) ? 2 : 1;
//                break;
//            }
//            case 2: /*SP*/
//            {
//                newValue = (listArgument[0]) ? 1 : 0;
//                break;
//            }
//            case 3: /*TapChgup*/
//            {
//                OA::OAUInt16 temp;
//                oldValue.GetUInt16(temp);
//
//                if (listArgument[0])
//                    temp++;
//
//                newValue = temp;
//                break;
//            }
//            case 4: /*TapChgDown*/
//            {
//                OA::OAUInt16 temp;
//                oldValue.GetUInt16(temp);
//
//                if (listArgument[0])
//                    temp--;
//
//                newValue = temp;
//                break;
//            }
//
//            default:
//                break;
//            }
//
//            pOldRecord->SetValue(newValue);
//            pOldRecord->SetQuality(OA_StatusCode_Good);
//        }
//
//        if (itemType == 3 /*Control Scenario*/)
//        {
//            pOldRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::ControlScenario);
//        }
//
//        if (itemType == 4 /*Trigger Scenario*/)
//        {
//            pOldRecord->SetItemType(OA::ModelDataAPI::FepSimulationItemType::TriggerScenario);
//        }
//
//        m_pKafkaProducer->ProductMsg(pOldRecord.get());
//
//        m_mapKeyRecord[targetKey] = std::move(pOldRecord);
//    }
//}
