#include "KafkaConsumer.h"
#include "KafkaConfig.h"
#include "KafkaRecordInfo.h"
#include "KafkaProducer.h"

#include <nlohmann/json.hpp>

#include <OAModelDataAPI/FepSimulation/FepSimulationControlConsequenceItemInfo.h>
#include <OABase/StringUtility.h>


using namespace nlohmann;


KafkaConsumer::KafkaConsumer()
{
    std::unique_ptr<KafkaConfig> pKafkaConfig = std::make_unique<KafkaConfig>();

    m_strBroker = pKafkaConfig->GetBorker();
    m_strTopicControl = pKafkaConfig->GetTopicControl();
    m_nPartition = pKafkaConfig->GetPartition();

    if (!m_pKafkaProducer)
        m_pKafkaProducer = std::make_unique<KafkaProducer>();

    m_pKafkaProducer->Initialize();
}

KafkaConsumer::~KafkaConsumer()
{

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
        RdKafka::MessageTimestamp timestamp = message->timestamp();
        OA::OADateTime time(timestamp.timestamp);
        OA::OAString strTimestamp = time.ToStringWithLocalTimeZone();

        ParseKafkaDetection(strRecord, time);

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

void KafkaConsumer::ParseKafkaDetection(std::string msg, OA::OADateTime timestamp)
{
    std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();

    OA::OAString strTimestamp = timestamp.ToStringWithLocalTimeZone();
    pRecord->SetTimestamp(timestamp);

    json jsonObj = json::parse(msg);

    std::string key = jsonObj["key"];
   
    OA::OAString targetKey = OA::StringUtility::Utf8ToUtf16(jsonObj["target"]);    
    pRecord->SetKey(targetKey);

    OA::OAVariant varValue;
    int value = jsonObj["value"];

    OA::OAUInt8 ctrlType;
    ctrlType = jsonObj["controltype"];

    int itemType = jsonObj["itemType"];

    json data = jsonObj["inputArgument"];
    std::vector<bool> listArgument;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        listArgument.emplace_back(*it);
    }
            
    
    if (itemType == 2 /*Control Consequence*/ && listArgument.size()==1)
    {
        switch (ctrlType)
        {
        case 0:/*RB: reverse bit*/
        {
            bool tempValue = (value) ? true : false;

            if (listArgument[0])
                tempValue = !tempValue;

            varValue = tempValue;

            break;
        }
        case 1: /*CB*/
        {
            value = (listArgument[0]) ? 2 : 1;
            varValue = value;
            break;
        }
        case 2: /*SP*/
        {
            value = (listArgument[0]) ? 1 : 0;     
            varValue = value;
            break;
        }
        case 3: /*TapChgup*/
        {
            if (listArgument[0])
                value++;

            varValue = value;

            break;
        }
        case 4: /*TapChgDown*/
        {
            if (listArgument[0])
                value--;

            varValue = value;

            break;
        }
       
        default:
            break;
        }
       
    }
   
    pRecord->SetValue(varValue);
    pRecord->SetQuality(OA_StatusCode_Good);
    
    m_pKafkaProducer->ProductMsg(pRecord.get());    
}
