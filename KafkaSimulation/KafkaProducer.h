#pragma once

#include <iostream>

#include <Kafka/rdkafkacpp.h>

#include <OABase/OAString.h>
#include <OABase/OABase.h>
#include <OABase/OADateTime.h>
#include <OABase/OAVariant.h>
#include <OABase/OAStatus.h>

#include <memory>
#include <map>

namespace OA
{
    namespace ModelDataAPI
    {
        class FepSimulationItemInfo;
    }
}
class KafkaRecordInfo;
class KafkaRandomGeneratorRecordInfo;

class  KakfkaProducerDeliveryReportCallBack : public RdKafka::DeliveryReportCb
{
public:
    void dr_cb(RdKafka::Message& message)
    {
        std::cout << "Message delivery for (" << message.len() << " bytes): " << message.errstr() << std::endl;

        if (message.key())
        {
            std::cout << "Key: " << *(message.key()) << ";" << std::endl;
        }
    }
};

class  KafkaProducerEventCallBack : public RdKafka::EventCb
{
public:
    void event_cb(RdKafka::Event& event) override
    {
        switch (event.type())
        {
        case RdKafka::Event::EVENT_ERROR:
            std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " << event.str() << std::endl;
            if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
                break;
        case RdKafka::Event::EVENT_STATS:
            std::cerr << "\"STATS\": " << event.str() << std::endl;
            break;
        case RdKafka::Event::EVENT_LOG:
            //m_strErr = OA::OAString(_T("LOG-%i-%s: %s\n", event.severity(), event.fac().c_str(), event.str().c_str()));
            fprintf(stderr, "LOG-%i-%s: %s\n", event.severity(), event.fac().c_str(), event.str().c_str());
            break;
        default:
            std::cerr << "EVENT " << event.type() << " (" << RdKafka::err2str(event.err()) << "); " << event.str() << std::endl;
            break;
        }
    }
};


class  KafkaProducer
{
public:
    KafkaProducer();
    virtual ~KafkaProducer();

    bool Initialize();
    void ProductMsg();
    void ProductMsg(KafkaRecordInfo* pRecord);
    void ProductMsg(const std::string& key, const std::string value, std::string& timestamp, std::string& status);
    void CreateKafkaRecord(const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& listItems);
    void AddKafkaRecord(std::unique_ptr<KafkaRecordInfo> pRecord);
    void Stop();

    const std::vector<std::unique_ptr<KafkaRecordInfo>>& GetListRecord() const;    
    const std::map<OA::OAString, OA::OAVariant>& GetMapKeyValue() const;
    bool HasDataChange(KafkaRecordInfo* pRecord);

    void UpdateMapKeyValue(OA::OAString key, OA::OAVariant& value);

protected:    
    void CreateSingleRandomRecord();
    void CreateInitializeRecord();


protected:    
    std::string m_strTopics;
    std::string m_strBroker;
    std::string m_strTopicControl;
    int m_nPpartition;

    RdKafka::Producer* m_pProducer;
    RdKafka::Topic* m_pTopic;
    KakfkaProducerDeliveryReportCallBack m_producerDeliveryReportCallBack;
    KafkaProducerEventCallBack m_producerEventCallBack;

    std::unique_ptr<KafkaRecordInfo> m_pRecord;

    std::vector<std::unique_ptr<KafkaRecordInfo>> m_listRecords;
    std::map<OA::OAString, OA::OAVariant> m_mapKeyValue;
};