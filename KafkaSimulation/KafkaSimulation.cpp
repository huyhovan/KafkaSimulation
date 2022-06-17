#include "KafkaSimulation.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <random>

int main()
{
    std::cout << "*****START RUN KAFKA SIMULATOR******" << std::endl;

    Initialize();

    const std::vector<std::unique_ptr<OA::ModelDataAPI::FepSimulationItemInfo>>& lisItems = m_pSimullationItemMnager->GetListItem(); 

    m_pKafkaProducer->CreateKafkaRecord(lisItems);

    const std::vector<std::unique_ptr<KafkaRecordInfo>>& listRecord = m_pKafkaProducer->GetListRecord();
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

            OA::OAUInt32 interval = pRecord->GetInterval();
            int minValue, maxValue;
            pRecord->GetMinValue().GetInt32(minValue);
            pRecord->GetMaxValue().GetInt32(maxValue);
            int i = 0;
            while (i < 20)
            {
                std::random_device                  rand_dev;
                std::mt19937                        generator(rand_dev());
                std::uniform_int_distribution<int>  distr(1, 1000);

                OA::OAVariant value = distr(generator);
                pRecord->SetValue(value);
                if (m_pKafkaProducer->HasDataChange(record.get()))
                {
                    m_pKafkaProducer->ProductMsg(pRecord);
                }                

                std::chrono::milliseconds timespan(interval);
                std::this_thread::sleep_for(timespan);

                i++;
            }
        }        
    }
   
    if (lisItems.size() > 0)
    {
        m_pKafkaProducer->CreateKafkaRecord(lisItems);

        m_pKafkaProducer->ProductMsg();
    }


    // For Test kafka topic    
   /* const std::string key = ("TestKey");
    std::string strValue = ("123");
    std::string strTimestamp = ("Timestamp");
    std::string strQuality = ("Good");
    m_pKafkaProducer->ProductMsg(key, strValue, strTimestamp, strQuality);*/

    return 0;
}
