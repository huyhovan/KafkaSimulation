#include "KafkaSimulation.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <tchar.h>

int main()
{
    std::cout << "*****START RUN KAFKA SIMULATOR******" << std::endl;

    std::cout << RdKafka::version_str() << std::endl;

    Initialize();

    const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& lisItems = m_pSimullationItemManager->GetListItem();

    m_pKafkaProducer->CreateKafkaRecord(lisItems);

    const std::vector<std::unique_ptr<KafkaRecordInfo>>& listRecord = m_pKafkaProducer->GetListRecord();

    // for handle 1 Control Scenario
    for (auto& record : listRecord)
    {
        if (record->GetKey() == _T("(SAS) Simulate Data by Live Status"))
        {
            KafkaTriggerScenarioRecordInfo* controlScenarioRecord = static_cast<KafkaTriggerScenarioRecordInfo*>(record.get());

            HandleControlScenarioRecord(controlScenarioRecord);
        }
    }


    std::thread threadConsumer([]() {
        m_pKafkaConsumer->Start(500);
        });

    int nLoop = 0;
    OA::OAUInt32 interval;


    std::thread threadProducer([&nLoop, &listRecord, &interval]() {

        while (true)
        {
            for (auto& record : listRecord)
            {
                OA::ModelDataAPI::FepSimulationItemType type = record->GetItemType();
                if (type == OA::ModelDataAPI::FepSimulationItemType::Initialization)
                {
                    if (m_pKafkaProducer->HasDataChange(record.get()))
                    {
                        m_pKafkaProducer->ProductMsg(record.get());
                    }
                    continue;
                }

                if (type == OA::ModelDataAPI::FepSimulationItemType::RandomGenerator)
                {
                    KafkaRandomGeneratorRecordInfo* pRecord = static_cast<KafkaRandomGeneratorRecordInfo*>(record.get());
                    if (pRecord->GetKey() != _T("AT1BCU.AT1BCUANN.AINZGGIO29.AnIn01.mag.f")) /*for testing 1 point generate random, control by ControlConsequence AT1BCU.AT1BCUCON.RBGGIO1.SPCSO02.Control*/
                    {
                        interval = pRecord->GetInterval();
                        int minValue, maxValue;
                        pRecord->GetMinValue().GetInt32(minValue);
                        pRecord->GetMaxValue().GetInt32(maxValue);
                        int i = 0;
                        while (i < 1)
                        {
                            std::random_device                  rand_dev;
                            std::mt19937                        generator(rand_dev());
                            std::uniform_int_distribution<int>  distr(minValue, maxValue);

                            OA::OAVariant value = distr(generator);
                            pRecord->SetValue(value);
                            if (m_pKafkaProducer->HasDataChange(record.get()))
                            {
                                m_pKafkaProducer->ProductMsg(pRecord);
                            }

                            i++;
                        }
                    }
                }
            }

            std::chrono::milliseconds timespan(interval);
            std::this_thread::sleep_for(timespan);

            nLoop++;
        }

        });


    threadProducer.join();
    threadConsumer.join();

    return 0;
}
