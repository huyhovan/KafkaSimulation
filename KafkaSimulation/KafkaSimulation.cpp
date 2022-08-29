#include "KafkaSimulation.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <tchar.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <mutex>
#include <condition_variable>


class HandleTimeout
{
public:
    HandleTimeout(boost::asio::io_service& io_service, KafkaProducer* pProducer, SimulationItemManager* pSimulationMng)
        :m_timer(io_service), m_pProducer(pProducer), m_pSimulationMng(pSimulationMng)
    {
        OA_ASSERT(m_pProducer);
        //Wait();
    }

    virtual ~HandleTimeout() { Cancel(); }

    void SetInterval(OA::OAUInt16& interval)
    {
        m_interval = interval;
        m_listRecord = m_pSimulationMng->GetMapIntervalRandomRecord().at(m_interval);
        Wait();
    }

    void Run()
    {
        Wait();
    }

    void Callback_Timeout(boost::system::error_code const& cError)
    {
        if (cError)
        {
            std::cout << "Error:" << cError.message() << std::endl;
            return;
        }

        // Handle RandomGerator here, split case by interval        

        for (auto& pRecord : m_listRecord)
        {
            if (/*false*/true/*pRecord->GetKey() == _T("SAS_SIM.S1.A1.MEASF.P3") || pRecord->GetKey() == _T("SAS_SIM.S1.A1.MEASF.P4")*/)
            {
                if (pRecord->GetDataTypeId() == OA_DataType_Boolean)
                {
                    bValue = !bValue;
                    OA::OAVariant value = bValue;
                    pRecord->SetValue(value);
                    if (pRecord->IsGenning() == true && m_pProducer->HasDataChange(pRecord))
                    {
                        std::lock_guard<std::mutex> guard(m_mutex);

                        m_pProducer->ProductMsg(pRecord);
                        m_pSimulationMng->UpdateMapModel(pRecord->GetKey(), pRecord);
                    }
                }
                else
                {
                    OA::OAFloat minValue, maxValue;
                    pRecord->GetMinValue().GetFloat(minValue);
                    pRecord->GetMaxValue().GetFloat(maxValue);

                    std::random_device                  rand_dev;
                    std::mt19937                        generator(rand_dev());
                    std::uniform_real_distribution<OA::OAFloat>  distr(minValue, maxValue);

                    OA::OAVariant value = distr(generator);
                    pRecord->SetValue(value);
                    if (pRecord->IsGenning() == true && m_pProducer->HasDataChange(pRecord))
                    {
                        std::lock_guard<std::mutex> guard(m_mutex);

                        m_pProducer->ProductMsg(pRecord);
                        m_pSimulationMng->UpdateMapModel(pRecord->GetKey(), pRecord);
                    }
                }
            }           
        }

        Wait();
    }

private:
    void Wait()
    {
        m_timer.expires_from_now(boost::posix_time::millisec(m_interval));
        m_timer.async_wait(boost::bind(&HandleTimeout::Callback_Timeout, this, boost::asio::placeholders::error));
    }

    void Cancel() { m_timer.cancel();  }

private:
    boost::asio::deadline_timer m_timer;
    std::mutex m_mutex;
 
    OA::OAUInt16 m_interval;
    KafkaProducer* m_pProducer;
    SimulationItemManager* m_pSimulationMng;
    std::vector<KafkaRandomGeneratorRecordInfo*> m_listRecord;
    OA::OABoolean bValue = true;
    //static std::vector <boost::asio::deadline_timer> m_listTimer;

};
//std::vector<boost::asio::deadline_timer> HandleTimeout::m_listTimer{};


void Initialize()
{
    if (!m_pKafkaProducer)
        m_pKafkaProducer = std::make_unique<KafkaProducer>();

    m_pKafkaProducer->Initialize();

    m_pSimullationItemManager = std::make_unique<SimulationItemManager>();
    if (m_pSimullationItemManager)
    {
        m_pSimullationItemManager->Initialize();
    }

    if (!m_pKafkaConsumer)
        m_pKafkaConsumer = std::make_unique<KafkaConsumer>(m_pSimullationItemManager.get(), m_pKafkaProducer.get());
    m_pKafkaConsumer->Initialize();
}

int main()
{
    std::cout << "*****START RUN KAFKA SIMULATOR******" << std::endl;

    std::cout << "Kafka Ver." << RdKafka::version_str() << std::endl;

    Initialize();

    const std::map<OA::OAString, std::unique_ptr<KafkaRecordInfo>>& mapKeyRecord = m_pSimullationItemManager->GetMapModel();    
    const std::map<OA::OAUInt16, std::vector<KafkaRandomGeneratorRecordInfo*>>& mapIntervalRecord = m_pSimullationItemManager->GetMapIntervalRandomRecord();

    boost::asio::io_service io_service;

    std::vector<OA::OAUInt16> listInterval;
    for (auto& iter : mapIntervalRecord)
    {
        listInterval.push_back(iter.first);
    }

    std::vector<std::unique_ptr<HandleTimeout>> listHandleTimeout;
    for (size_t i = 0; i < mapIntervalRecord.size(); i++)
    {
        listHandleTimeout.push_back(std::make_unique<HandleTimeout>(io_service, m_pKafkaProducer.get(), m_pSimullationItemManager.get()));
    }

    OA_ASSERT(listInterval.size() == listHandleTimeout.size());

    for (size_t i = 0; i < listHandleTimeout.size(); i++)
    {
        listHandleTimeout[i]->SetInterval(listInterval[i]);
    }

    //for (auto& it : mapKeyRecord)
    //{
    //    OA::ModelDataAPI::FepSimulationItemType type = it.second->GetItemType();
    //    if (type == OA::ModelDataAPI::FepSimulationItemType::Initialization)
    //    {
    //        if (m_pKafkaProducer->HasDataChange(it.second.get()))
    //        {
    //            //m_pKafkaProducer->ProductMsg(record.get());
    //        }
    //    }
    //}


    std::thread threadIOService([&io_service]() {      
        io_service.run();
        });


    std::thread threadConsumer([]() {
        m_pKafkaConsumer->Start(500);
        });

    
    //OA::OAUInt32 interval;
    //bool bRun = true;
    //
    //OA::OABoolean bValue = true;  
    //
    //std::thread threadProducer([&mapKeyRecord, &interval, &bRun, &bValue]() {
    //    while (bRun)
    //    {
    //        for (auto& it : mapKeyRecord)
    //        {
    //            OA::ModelDataAPI::FepSimulationItemType type = it.second->GetItemType();
    //            if (type == OA::ModelDataAPI::FepSimulationItemType::Initialization)
    //            {
    //                if (m_pKafkaProducer->HasDataChange(it.second.get()))
    //                {
    //                    //m_pKafkaProducer->ProductMsg(record.get());
    //                }
    //                continue;
    //            }
    //
    //            if (type == OA::ModelDataAPI::FepSimulationItemType::RandomGenerator)
    //            {
    //                KafkaRandomGeneratorRecordInfo* pRecord = static_cast<KafkaRandomGeneratorRecordInfo*>(it.second.get());
    //                if (/*false*//*true*/pRecord->GetKey() == _T("SAS_SIM.S1.A1.MEASF.P3") || pRecord->GetKey() == _T("SAS_SIM.S1.A1.MEASF.P4")) /*for testing 1 point generate random, control by ControlConsequence AT1BCU.AT1BCUCON.RBGGIO1.SPCSO02.Control*/
    //                {
    //                    interval = pRecord->GetInterval();
    //
    //                    if (pRecord->GetDataTypeId() == OA_DataType_Boolean)
    //                    {
    //                        bValue = !bValue;
    //                        OA::OAVariant value = bValue;
    //                        pRecord->SetValue(value);
    //                        if (pRecord->IsGenning() == true && m_pKafkaProducer->HasDataChange(it.second.get()))
    //                        {
    //                            m_pKafkaProducer->ProductMsg(pRecord);
    //                            m_pSimullationItemManager->UpdateMapModel(pRecord->GetKey(), pRecord);
    //                        }
    //                    }
    //                    else
    //                    {
    //                        OA::OAFloat minValue, maxValue;
    //                        pRecord->GetMinValue().GetFloat(minValue);
    //                        pRecord->GetMaxValue().GetFloat(maxValue);
    //
    //                        std::random_device                  rand_dev;
    //                        std::mt19937                        generator(rand_dev());
    //                        std::uniform_real_distribution<OA::OAFloat>  distr(minValue, maxValue);
    //
    //                        OA::OAVariant value = distr(generator);
    //                        pRecord->SetValue(value);
    //                        if (pRecord->IsGenning() == true && m_pKafkaProducer->HasDataChange(it.second.get()))
    //                        {
    //                            m_pKafkaProducer->ProductMsg(pRecord);
    //                            m_pSimullationItemManager->UpdateMapModel(pRecord->GetKey(), pRecord);
    //                        }
    //                    }                       
    //                }
    //            }
    //        }
    //
    //        std::chrono::milliseconds timespan(interval);
    //        std::this_thread::sleep_for(timespan);
    //    }
    //
    //});
   

    //threadProducer.join();

    threadIOService.join();
    threadConsumer.join();

    return 0;
}

