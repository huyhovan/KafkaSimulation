#pragma once

#include <iostream>

#include <Kafka/rdkafkacpp.h>


class KafkaControlConsequenceRecordInfo;
class KafkaProducer;

namespace OA
{
    class OADateTime;

    namespace ModelDataAPI
    {
       class FepSimulationControlConsequenceItemInfo;
    }
}

class KafkaConsumer
{
public: 
    KafkaConsumer();
    virtual ~KafkaConsumer();

    bool Initialize();
    void Start(int timeout_ms);
    void Stop();

protected:
    void MsgConsumer(RdKafka::Message* message, void* opaque);
    void ParseKafkaDetection(std::string msg, OA::OADateTime timestamp);

protected:
    std::string m_strBroker;
    std::string m_strTopicControl;
    std::string m_strGroupId;
    int64_t m_nLastOffset;
    int64_t m_nCurrentOffset;
    int32_t m_nPartition;

    RdKafka::Consumer* m_pKafkaConsumer;
    RdKafka::Topic* m_pTopic;
  
    bool m_bRun;

    std::unique_ptr<KafkaProducer> m_pKafkaProducer;
};

