#pragma once

#include "KafkaProducer.h"
#include "SimulationItemManager.h"
#include "KafkaRecordInfo.h"


#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>

std::unique_ptr<KafkaProducer> m_pKafkaProducer;
std::unique_ptr<SimulationItemManager> m_pSimullationItemMnager;



void Initialize()
{
    if(!m_pKafkaProducer)
        m_pKafkaProducer = std::make_unique<KafkaProducer>();

    m_pKafkaProducer->Initialize();

    m_pSimullationItemMnager = std::make_unique<SimulationItemManager>();
    if (m_pSimullationItemMnager)
    {
        m_pSimullationItemMnager->Initialize();
    }
}

