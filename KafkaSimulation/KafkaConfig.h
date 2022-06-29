#pragma once

#include <iostream>
#include <OABase/OAString.h>

class KafkaConfig
{
public:
    KafkaConfig();
    virtual ~KafkaConfig();

    std::string GetBorker() const;
    std::string GetTopics() const;
    std::string GetTopicControl() const;
    int GetPartition() const;

    std::string GetHost() const;
    std::string GetProject() const;

protected:
    void InitSettingInfoXML();
    void LoadSettingInfoFromXML();


protected:
    std::string m_strTopics;
    std::string m_strBroker;
    std::string m_strTopicControl;
    int m_nPartition;

    std::string m_strHost;
    std::string m_strProject;

};

