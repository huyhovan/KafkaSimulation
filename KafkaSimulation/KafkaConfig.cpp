#include "KafkaConfig.h"

#include <XMLParser/tinyxml2.h>
#include <XMLParser/tinyxml2.cpp>

using namespace tinyxml2;


KafkaConfig::KafkaConfig()
{
    LoadSettingInfoFromXML();
}

KafkaConfig::~KafkaConfig()
{

}

void KafkaConfig::InitSettingInfoXML()
{
    tinyxml2::XMLDocument xmlDoc;

    XMLElement* pRoot = xmlDoc.NewElement("SettingKafkaInfo");
    pRoot->SetAttribute("Version", "");
    pRoot->SetAttribute("Platform", "");
    xmlDoc.InsertFirstChild(pRoot);

    XMLElement* host = xmlDoc.NewElement("HostDataBase");
    host->SetText("127.0.0.1:27017");
    pRoot->InsertEndChild(host);

    XMLElement* project = xmlDoc.NewElement("ProjectDataBase");
    project->SetText("TEST");
    pRoot->InsertEndChild(project);

    XMLElement* address = xmlDoc.NewElement("Address");
    address->SetText("localhost:9092");
    pRoot->InsertEndChild(address);

    XMLElement* pjName = xmlDoc.NewElement("ProjectName");
    pjName->SetText("oa-ds-data-input");
    pRoot->InsertEndChild(pjName);

    XMLElement* topicControl = xmlDoc.NewElement("TopicControl");
    topicControl->SetText("oa-fep-simulate-control-test");
    pRoot->InsertEndChild(topicControl);

    xmlDoc.SaveFile("SettingInfo.xml");
}

void KafkaConfig::LoadSettingInfoFromXML()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.LoadFile("SettingInfo.xml");
    if (err != XML_SUCCESS)
    {
        InitSettingInfoXML();
        LoadSettingInfoFromXML();

        return;
    }

    m_strHost = doc.FirstChildElement("SettingKafkaInfo")->FirstChildElement("HostDataBase")->GetText();
    m_strProject = doc.FirstChildElement("SettingKafkaInfo")->FirstChildElement("ProjectDataBase")->GetText();

    m_strBroker = doc.FirstChildElement("SettingKafkaInfo")->FirstChildElement("Address")->GetText();
    m_strTopics = doc.FirstChildElement("SettingKafkaInfo")->FirstChildElement("ProjectName")->GetText();
    m_strTopicControl = doc.FirstChildElement("SettingKafkaInfo")->FirstChildElement("TopicControl")->GetText();
    m_nPartition = 0;
}

std::string KafkaConfig::GetBorker() const
{
    return m_strBroker;
}

std::string KafkaConfig::GetTopics() const
{
    return m_strTopics;
}

std::string KafkaConfig::GetTopicControl() const
{
    return m_strTopicControl;
}

int KafkaConfig::GetPartition() const
{
    return m_nPartition;
}

std::string KafkaConfig::GetHost() const
{
    return m_strHost;
}

std::string KafkaConfig::GetProject() const
{
    return m_strProject;
}
