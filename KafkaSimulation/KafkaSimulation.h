#pragma once

#include "KafkaProducer.h"
#include "SimulationItemManager.h"
#include "KafkaRecordInfo.h"
#include "KafkaConsumer.h"

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>

std::unique_ptr<KafkaProducer> m_pKafkaProducer;
std::unique_ptr<SimulationItemManager> m_pSimullationItemManager;
std::unique_ptr<KafkaConsumer> m_pKafkaConsumer;


void Initialize()
{
    if(!m_pKafkaProducer)
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

