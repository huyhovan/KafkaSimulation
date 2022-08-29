#pragma once

#include <iostream>

#include <Kafka/rdkafkacpp.h>

#include <OABase/OAString.h>
#include <OABase/OABase.h>
#include <OABase/OADateTime.h>
#include <OABase/OAVariant.h>
#include <OABase/OAStatus.h>

#include <any>
#include <memory>
#include <map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "KafkaRecordInfo.h"

namespace OA
{
    namespace ModelDataAPI
    {
        class FepSimulationItemInfo;
    }
}


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

    void HandleRandomGeneration(boost::asio::io_service& io_service, const std::vector<KafkaRandomGeneratorRecordInfo*>& listRecord);

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
    void ProductRandomMsg(const std::vector<KafkaRandomGeneratorRecordInfo*>& listRecord);
    void Callback_Timeout(boost::system::error_code const& cError);

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


//#include <random>
//
//
//class HandleTimeout
//{
//public:
//    HandleTimeout(boost::asio::io_service& io_service, KafkaProducer* pProducer, const std::vector<KafkaRandomGeneratorRecordInfo*>& listRecord)
//        :m_timer(io_service), m_pProducer(pProducer), m_listRecord(listRecord)
//    {
//        OA_ASSERT(m_pProducer);
//        m_interval = m_listRecord[0]->GetInterval();
//
//        Wait();
//    }
//
//    virtual ~HandleTimeout() { Cancel(); }
//
//    void Callback_Timeout(boost::system::error_code const& cError)
//    {
//        if (cError)
//        {
//            std::cout << "Error:" << cError.message() << std::endl;
//            return;
//        }
//
//        // Handle RandomGerator here, split case by interval
//
//        OA::OABoolean bValue = true;
//
//        for (auto& pRecord : m_listRecord)
//        {
//            if (pRecord->GetDataTypeId() == OA_DataType_Boolean)
//            {
//                bValue = !bValue;
//                OA::OAVariant value = bValue;
//                pRecord->SetValue(value);
//                if (pRecord->IsGenning() == true && m_pProducer->HasDataChange(pRecord))
//                {
//                    m_pProducer->ProductMsg(pRecord);
//                    //m_pSimullationItemManager->UpdateMapModel(pRecord->GetKey(), pRecord);
//                }
//            }
//            else
//            {
//                OA::OAFloat minValue, maxValue;
//                pRecord->GetMinValue().GetFloat(minValue);
//                pRecord->GetMaxValue().GetFloat(maxValue);
//
//                std::random_device                  rand_dev;
//                std::mt19937                        generator(rand_dev());
//                std::uniform_real_distribution<OA::OAFloat>  distr(minValue, maxValue);
//
//                OA::OAVariant value = distr(generator);
//                pRecord->SetValue(value);
//                if (pRecord->IsGenning() == true && m_pProducer->HasDataChange(pRecord))
//                {
//                    m_pProducer->ProductMsg(pRecord);
//                    //m_pSimullationItemManager->UpdateMapModel(pRecord->GetKey(), pRecord);
//                }
//            }
//        }
//
//
//        Wait();
//    }
//
//private:
//    void Wait()
//    {
//        m_timer.expires_from_now(boost::posix_time::millisec(m_interval));
//        m_timer.async_wait(boost::bind(&HandleTimeout::Callback_Timeout, this, boost::asio::placeholders::error));
//    }
//
//    void Cancel() { m_timer.cancel(); }
//
//private:
//    boost::asio::deadline_timer m_timer;
//    OA::OAUInt16 m_interval;
//    KafkaProducer* m_pProducer;
//    const std::vector<KafkaRandomGeneratorRecordInfo*>& m_listRecord;
//};