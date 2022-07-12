#pragma once

#include <iostream>
#include <map>

#include <Kafka/rdkafkacpp.h>
#include <OABase/OAString.h>
#include <OABase/OAVariant.h>

#include <nlohmann/json.hpp>

class SimulationItemManager;
class KafkaControlConsequenceRecordInfo;
class KafkaProducer;
class KafkaRecordInfo;

namespace OA
{
    class OADateTime;

    namespace ModelDataAPI
    {
       class FepSimulationItemInfo;
       class FepSimulationControlConsequenceItemInfo;
       enum class FepSimulationItemType;
    }
}

class KafkaConsumer
{
public: 
    KafkaConsumer(SimulationItemManager* pSimulationMng, KafkaProducer* pProduct);
    virtual ~KafkaConsumer();

    bool Initialize();
    void Start(int timeout_ms);
    void Stop();    
  
protected:
    void MsgConsumer(RdKafka::Message* message, void* opaque);   
    void CreateKafkaRecordControl(std::string msg, OA::OADateTime& timestamp);
    OA::ModelDataAPI::FepSimulationItemType GetSimualtatioItemType(int nItemType);
    OA::OADateTime GetTimestampWithLocalTimeZone(RdKafka::MessageTimestamp& kafkaTimestamp); // KafkaTimestamp start by 1653 < 1601 of OADatetime
    //void ParseKafkaDetection(std::string msg, OA::OADateTime timestamp);    

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

    //std::unique_ptr<KafkaProducer> m_pKafkaProducer;
    KafkaProducer* m_pKafkaProducer;
    SimulationItemManager* m_pSimullationItemManager;

    std::map<OA::OAString, std::unique_ptr<KafkaRecordInfo>> m_mapKeyRecord; // using m_mapKeyRecord for record posted to topic    
};