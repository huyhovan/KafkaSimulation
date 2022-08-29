#include "KafkaConsumer.h"
#include "KafkaConfig.h"
#include "KafkaRecordInfo.h"
#include "KafkaProducer.h"
#include "SimulationItemManager.h"

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlConsequenceItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationControlScenarioItemInfo.h>
#include <OAModelDataAPI/FepSimulation/FepSimulationTriggerScenarioItemInfo.h>
#include <OABase/StringUtility.h>
#include <sol/sol.hpp>

#include <chrono>
#include <time.h>
#include <algorithm>
#include <tchar.h>

constexpr int LOCAL_TIME_ZONE = 7;

using namespace nlohmann;


KafkaConsumer::KafkaConsumer(SimulationItemManager* pSimulatioMng, KafkaProducer* pProduct)
    : m_pSimullationItemManager(pSimulatioMng), m_pKafkaProducer(pProduct)
{
    std::unique_ptr<KafkaConfig> pKafkaConfig = std::make_unique<KafkaConfig>();

    m_strBroker = pKafkaConfig->GetBorker();
    m_strTopicControl = pKafkaConfig->GetTopicControl();
    m_nPartition = pKafkaConfig->GetPartition();

    if (!m_pKafkaProducer)
        m_pKafkaProducer = std::make_unique<KafkaProducer>().get();

    //m_pKafkaProducer->Initialize();
}

KafkaConsumer::~KafkaConsumer()
{
    Stop();
}

bool KafkaConsumer::Initialize()
{
    std::string errstr;

    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    if (!conf)
    {
        std::cerr << "Rdkafka create global config failed" << std::endl;
        return false;
    }

    // broker list
    if (conf->set("metadata.broker.list", m_strBroker, errstr) != RdKafka::Conf::CONF_OK)
    {
        std::cerr << "RdKafka conf set brokerlist failed :" << errstr.c_str() << std::endl;
    }

    std::string strfetch_num = "1024000"; //1024000000
    // the maximum size of each pull message from a single partition
    if (conf->set("max.partition.fetch.bytes", strfetch_num, errstr) != RdKafka::Conf::CONF_OK)
    {
        std::cerr << "RdKafke conf set max.partition false :" << errstr.c_str() << std::endl;
    }

    // CREATE CONSUMER  USING ACCUMULATED GLOBAL CONFIGURATION
    m_pKafkaConsumer = RdKafka::Consumer::create(conf, errstr);
    if (!m_pKafkaConsumer)
    {
        std::cerr << "failed to create consumer" << std::endl;
    }
    std::cout << "Create consumer: " << m_pKafkaConsumer->name() << std::endl;
    delete conf;

    // Topic
    RdKafka::Conf* tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    if (!tconf)
    {
        std::cerr << "rdkafka create topics config failed" << std::endl;
        return false;
    }
    if (tconf->set("auto.offset.reset", "latest", errstr) != RdKafka::Conf::CONF_OK)
    {
        std::cerr << "RdKafke conf set auto.offset.reset false :" << errstr.c_str() << std::endl;
    }

    // create topic handle
    m_pTopic = RdKafka::Topic::create(m_pKafkaConsumer, m_strTopicControl, tconf, errstr);
    if (!m_pTopic)
    {
        std::cerr << "RdKafke create topic false :" << errstr.c_str() << std::endl;
    }
    delete tconf;

    //START CONSUMER FOR TOPIC + PARTITION AT START OFFSET
    RdKafka::ErrorCode resp = m_pKafkaConsumer->start(m_pTopic, m_nPartition, /*m_nCurrentOffset*/ RdKafka::Topic::OFFSET_END);
    if (resp != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "Failed to start consumer" << errstr.c_str() << std::endl;
    }

    return true;
}

void KafkaConsumer::Start(int timeout_ms)
{
    RdKafka::Message* msg = NULL;
    m_bRun = true;

    while (m_bRun)
    {
        msg = m_pKafkaConsumer->consume(m_pTopic, m_nPartition, timeout_ms);
        MsgConsumer(msg, NULL);
        delete msg;
        m_pKafkaConsumer->poll(0);
    }

    if (m_pTopic)
    {
        delete m_pTopic;
        m_pTopic = NULL;
    }

    if (m_pKafkaConsumer)
    {
        delete m_pKafkaConsumer;
        m_pKafkaConsumer = NULL;
    }

    // wait for RdKafka to decomission
    RdKafka::wait_destroyed(5000);
}

void KafkaConsumer::Stop()
{
    m_bRun = false;
}

OA::ModelDataAPI::FepSimulationItemType KafkaConsumer::GetSimualtatioItemType(int nItemType)
{
    if (nItemType == 1)
    {
        return  OA::ModelDataAPI::FepSimulationItemType::Initialization;
    }

    if (nItemType == 2)
    {
        return  OA::ModelDataAPI::FepSimulationItemType::RandomGenerator;
    }

    if (nItemType == 3)
    {
        return OA::ModelDataAPI::FepSimulationItemType::ControlConsequence;
    }

    if (nItemType == 4)
    {
        return OA::ModelDataAPI::FepSimulationItemType::ControlScenario;
    }

    if (nItemType == 5)
    {
        return  OA::ModelDataAPI::FepSimulationItemType::TriggerScenario;
    }
}

OA::OADateTime KafkaConsumer::GetTimestampWithLocalTimeZone(RdKafka::MessageTimestamp& kafkaTimestamp)
{
    OA::OADateTime timestamp;

    if (kafkaTimestamp.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE)
    {
        std::string timeprefix;
        if (kafkaTimestamp.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME)
        {
            timeprefix = "create time";
        }
        else if (kafkaTimestamp.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME) {
            timeprefix = "log append time";
        }

        unsigned long long milli = kafkaTimestamp.timestamp + (unsigned long long)LOCAL_TIME_ZONE * 60 * 60 * 1000;

        auto mTime = std::chrono::milliseconds(milli);

        auto tp = std::chrono::time_point<std::chrono::system_clock>(mTime);
        auto tt = std::chrono::system_clock::to_time_t(tp);

        tm timeinfo;
        ::gmtime_s(&timeinfo, &tt);

        /* char s[60]{ 0 };
         ::sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d:%03d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

         std::string sTime = s;        */

        timestamp = OA::OADateTime::BuildDateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        OA::OAString strTimestamp = timestamp.ToShortStringWithLocalTimeZone();

        return timestamp;
    }
    else
    {
        OA::OAString strTimestamp = OA::OADateTime::GetNullTimeStringWithOAFormat();
        return NULL;
    }
}


void KafkaConsumer::MsgConsumer(RdKafka::Message* message, void* opaque)
{
    switch (message->err())
    {
    case RdKafka::ERR__TIMED_OUT:
        break;

    case  RdKafka::ERR_NO_ERROR:
    {  // real message 
        std::cout << "read message at offset " << message->offset() << std::endl;
        if (message->key())
        {
            std::cout << "key: " << message->key() << std::endl;
        }
        printf("%.*s\n", static_cast<int>(message->len()), static_cast<const char*>(message->payload()));

        std::string strRecord = static_cast<const char*>(message->payload());

        // for timestamp        
        OA::OADateTime timestamp = GetTimestampWithLocalTimeZone(message->timestamp());

        //ParseKafkaDetection(strRecord, time);
        CreateKafkaRecordControl(strRecord, timestamp);

        m_nLastOffset = message->offset();
        break;
    }
    case RdKafka::ERR__PARTITION_EOF:
        // Last message
        std::cout << "Reach the end of the queue, offset: " << m_nLastOffset << std::endl;
        // Stop()
        break;
    case  RdKafka::ERR__UNKNOWN_TOPIC:
    {
        std::cerr << "Unknow TOPIC " << message->errstr() << std::endl;
        Stop();
        break;
    }
    case RdKafka::ERR__UNKNOWN_PARTITION:
        std::cerr << "consume false " << message->errstr() << std::endl;
        Stop();
        break;

    default:
        // error
        std::cerr << "Consumer failed" << message->errstr() << std::endl;
        break;
    }
}

void KafkaConsumer::CreateKafkaRecordControl(std::string msg, OA::OADateTime& timestamp)
{
    json jsonObj = json::parse(msg);   

    const std::map<OA::OAString, std::vector<OA::ModelDataAPI::FepSimulationItemInfo*>>& mapKeySimulationItems = m_pSimullationItemManager->GetMapKeySimulationItems();
    const std::map <OA::OAString, std::unique_ptr<KafkaRecordInfo>>& mapMODEL = m_pSimullationItemManager->GetMapModel();

    OA::OAString keyControl = OA::StringUtility::Utf8ToUtf16(jsonObj["key"]);

    std::vector<bool> listInputArgument;
    //if (jsonObj["inputArgument"].is_null())
    //{
    //   // check keyControl is TriigerKey or not
    //   //if(isTrigger) HandleTriggerScenario(keyControl);
    //}
    json data = jsonObj["inputArgument"];   
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        listInputArgument.emplace_back(*it);
    }

    if (HandleDataChangedMonitorVar(keyControl, listInputArgument))
        return;
        
    int nItemType = jsonObj["itemType"];
    OA::ModelDataAPI::FepSimulationItemType itemType = GetSimualtatioItemType(nItemType);   

    auto it = mapKeySimulationItems.find(keyControl); // should using mapMODEL, using vector inscase 1 OAString have many FepSimulation
    if (it != mapKeySimulationItems.end())
    {
        std::vector<OA::ModelDataAPI::FepSimulationItemInfo*> listItem = it->second;
        for (auto& pItem : listItem)
        {
            if (itemType == pItem->GetItemType())
            {
                const std::map<OA::OAString, OA::OAVariant>& m_mapKeyValue = m_pKafkaProducer->GetMapKeyValue();

                switch (itemType)
                {
                case OA::ModelDataAPI::FepSimulationItemType::Initialization:
                    break;
                case OA::ModelDataAPI::FepSimulationItemType::RandomGenerator:
                    break;
                case OA::ModelDataAPI::FepSimulationItemType::ControlConsequence:
                {
                    std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();

                    OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo* pControlConsequence = static_cast<OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo*>(pItem);

                    OA::OAString targetKey = pControlConsequence->GetTarget();
                    pRecord->SetKey(targetKey);

                    pRecord->SetTimeInterval(timestamp);

                    pRecord->SetQuality(OA_StatusCode_Good);

                    OA::OAVariant value;
                    auto iter = m_mapKeyValue.find(targetKey);
                    if (iter != m_mapKeyValue.end())
                    {
                        value = iter->second;

                        pRecord->SetDataTypeId(value.GetDataType());
                    }

                    OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType controlType = pControlConsequence->GetControlType();
                    switch (controlType)
                    {
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::RB:
                    {
                        if (listInputArgument[0])
                            value = !value;

                        break;
                    }
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::CB:
                    {
                        value = (listInputArgument[0]) ? 2 : 1;
                        break;
                    }
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::SP:
                    {
                        value = (listInputArgument[0]) ? 1 : 0;
                        break;
                    }
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::TapChgUp:
                    {
                        OA::OAInt16 tempValue;
                        OA::OAStatus stt = value.GetInt16(tempValue);
                        if (stt == OA_StatusCode_Good)
                        {
                            if (listInputArgument[0])
                                tempValue++;
                            value = tempValue;
                        }

                        break;
                    }
                    case OA::ModelDataAPI::FepSimulationControlConsequenceItemInfo::ControlType::TapChgDn:
                    {
                        OA::OAInt16 tempValue;
                        OA::OAStatus stt = value.GetInt16(tempValue);
                        if (stt == OA_StatusCode_Good)
                        {
                            if (listInputArgument[0])
                                tempValue--;
                            value = tempValue;
                        }

                        break;
                    }
                    default:
                        break;
                    }
                    pRecord->SetValue(value);

                    m_pKafkaProducer->ProductMsg(pRecord.get());
                    m_pKafkaProducer->UpdateMapKeyValue(targetKey, pRecord->GetValue());
                    m_pSimullationItemManager->UpdateMapModel(targetKey, pRecord.get());

                    break;
                }
                case OA::ModelDataAPI::FepSimulationItemType::ControlScenario:
                {
                    OA::ModelDataAPI::FepSimulationControlScenarioItemInfo* pControlScenario = static_cast<OA::ModelDataAPI::FepSimulationControlScenarioItemInfo*>(pItem);

                    KafkaControlScenarioRecordInfo* pKafkaConfig = static_cast<KafkaControlScenarioRecordInfo*>(mapMODEL.at(keyControl).get());
                    if (pKafkaConfig != nullptr)
                    {
                        HandleControlScenarioRecord(pKafkaConfig, listInputArgument);
                    }
                    break;
                }

                case OA::ModelDataAPI::FepSimulationItemType::TriggerScenario:
                {
                    KafkaTriggerScenarioRecordInfo* pTriggerRecord = static_cast<KafkaTriggerScenarioRecordInfo*>(mapMODEL.at(keyControl).get());
                    HandleTriggerScenario(pTriggerRecord);

                    // Situation: Change TriggerScenario
                    break;
                }
                default:                  
                    break;
                }
            }
        }
    }
    else
    {
        std::cout << "The Method been called is not Simulate Control" << std::endl;
    }
}

void KafkaConsumer::HandleControlScenarioRecord(KafkaControlScenarioRecordInfo* pRecord, const std::vector<bool>& listInputArg)
{
    // Initialize ScriptModule
    OA::OAString errMsg;
    ScriptModule* pScript = new ScriptModule();
    bool ret = pScript->Initialize(errMsg);
    if (ret != true)
        return;

    pScript->ExportUserTypeToSOL();

    // Declare Params in SOL
    OA::OAString param = pRecord->GetParameters();
    bool isMatch = pScript->ConvertMethodParamsToSOL(param, listInputArg);
    OA_ASSERT(isMatch);

    const std::map<OA::OAString, std::unique_ptr<KafkaRecordInfo>>& mapKeyRecord = m_pSimullationItemManager->GetMapModel();  

    std::map <std::string, OA::OAString> mapKeyScriptName;

    OA::OAString keyMethod = pRecord->GetKey();
    ScriptNodeStruct nodeMethod;   

    // InputNode
    std::vector<OA::OAUniqueID> listId = pRecord->GetInputs();
    for (size_t i = 0; i < listId.size(); i++)
    {   
        std::string identity = listId[i].GetIdentifier().IdentifierString(); //DE name       
        OA::OAString scriptName = OA::ModelDataAPI::OAModelDataAPIUtility::MakeScriptNameOfOutsideTemplateNode(listId[i]); //Lua name

        mapKeyScriptName.emplace(identity, scriptName);

        // Make ScriptUserType to replace in content
        ScriptNodeStruct nodeStruct;
        auto it = mapKeyRecord.find(OA::StringUtility::Utf8ToUtf16(identity));
        if (it != mapKeyRecord.end())
        {
            if (it->second->GetDataTypeId() != 0)
            {
                nodeStruct.nodeType = ScriptNodeType::Variable;
                nodeStruct.dataType = it->second->GetDataTypeId();
                nodeStruct.initValue = it->second->GetValue();
                nodeStruct.initStatusCode = it->second->GetQuality();
                nodeStruct.initTimeStamp = it->second->GetTimeInterval();
            }
          
            nodeStruct.uniqueId = listId[i];
            nodeStruct.nodeIdString = OA::StringUtility::Utf8ToUtf16(identity);
            nodeStruct.replacedString = scriptName;                    
        }

        if (nodeStruct.nodeType == ScriptNodeType::Variable/* && nodeStruct.dataType == OA_DataType_Boolean*/)
        {
            pScript->ExportScriptNode(nodeStruct);
        }
    }

    std::string codetest = R"(      
       if node404d_4796eb50.value ==true and ctlVal then
		node404d_4796eb50:Update(false)
		node404d_767ef1cd:Update(true)
		node404d_d009fa79:Update(true)
		node404d_15aec4f7:Update(true)
		node404d_b3d9cf43:Update(true)
		node404d_8231d5de:Update(true)
		node40ee_8a57cac7:Update(122, 0, 456) --SNMP.SERET1.P01St
    end	
     
    )";

    //pScript->RunScript(OA::StringUtility::Utf8ToUtf16(codetest), errMsg);
    pScript->RunScript(pRecord->GetContent(), errMsg);
     
    for (size_t i = 0; i < listId.size(); i++)
    {
        std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();

        OA::OAString key;
        OA::OAVariant value;
        OA::OAInt64 timeInterval;
        OA::OAUInt64 statusCode;
        OA::OAUInt16 dataTypeId =0;
        bool isOutput = true;
        pRecord->SetKey(OA::StringUtility::Utf8ToUtf16(listId[i].GetIdentifier().IdentifierString()));
        
        auto it = mapKeyScriptName.find(listId[i].GetIdentifier().IdentifierString());
        if (it != mapKeyScriptName.end())
        {
            pScript->GetValueScriptUserTypeByKey(it->second, value, dataTypeId, statusCode, timeInterval, isOutput);
            pRecord->SetValue(value);
            pRecord->SetQuality((OA::OAUInt32)statusCode);
            pRecord->SetDataTypeId(dataTypeId);
            pRecord->SetTimeInterval(timeInterval);
        }          
        if (m_pKafkaProducer->HasDataChange(pRecord.get()))
        {
            m_pKafkaProducer->ProductMsg(pRecord.get());
        }
        m_pSimullationItemManager->UpdateMapModel(pRecord->GetKey(), pRecord.get());
    }
}

bool KafkaConsumer::HandleDataChangedMonitorVar(OA::OAString& keyControl, std::vector<bool> listArgControl)
{
    // check data change of point is monitored point or not -> Update m_MODEL
    // check key of variable belong which trigger scenario (std::map key- pRecord??)
    // HandleTriggerScenario(pRecord)    

    OA::OAString triggerParentName;

    if (m_pSimullationItemManager->IsMonitoredVariable(keyControl) && listArgControl.size() == 1)
    {
        // Update value of monitoredVar in MODEL
        OA::OAVariant valueCtrl = listArgControl[0];
        std::unique_ptr<KafkaRecordInfo> pRecord = std::make_unique<KafkaRecordInfo>();
        pRecord->SetKey(keyControl);
        pRecord->SetValue(valueCtrl);
        pRecord->SetQuality(OA_StatusCode_Good);
        pRecord->SetTimeInterval(OA::OAInt64(OA::OADateTime::Now()));

        m_pSimullationItemManager->UpdateMapModel(keyControl, pRecord.get());

        // Check keyControl belong which triggerScenario -> Get TriggerSceanrio Key        
        std::vector<OA::OAString> listParentTrigger;
        m_pSimullationItemManager->GetListParentTrigger(keyControl, listParentTrigger);
        if(!listParentTrigger.size())
            return false;
        OA::OAString triggerKey = listParentTrigger[0]; // assume only one TriggerScenario

        //
        //OA::OAString triggerKey = _T("(SAS) Simulate Data by Live Status");
        const std::map <OA::OAString, std::unique_ptr<KafkaRecordInfo>>& mapMODEL = m_pSimullationItemManager->GetMapModel();
        KafkaTriggerScenarioRecordInfo* pTriggerRecord = static_cast<KafkaTriggerScenarioRecordInfo*>(mapMODEL.at(triggerKey).get());
        HandleTriggerScenario(pTriggerRecord);       

        return true;
    }

    return false;
}

void KafkaConsumer::HandleTriggerScenario(KafkaTriggerScenarioRecordInfo* pRecord)
{   
    const std::vector<OA::ModelDataAPI::FepSimulationTriggerScenarioInput>& listInput = pRecord->GetInputs();

    // Initialize ScriptModule
    OA::OAString errMsg;
    ScriptModule* pScript = new ScriptModule();
    bool ret = pScript->Initialize(errMsg);
    if (ret != true)
        return;

    pScript->ExportUserTypeToSOL();

    // Export ScriptNode
    const std::map<OA::OAString, std::unique_ptr<KafkaRecordInfo>>& mapKeyRecord = m_pSimullationItemManager->GetMapModel();
    std::map <std::string, OA::OAString> mapKeyScriptName;

    std::vector<OA::OAString> listInputKey;

    for (auto& input : listInput)
    {
        OA::OAUniqueID inputId = input.GetInputNode();
        OA::OAString key = OA::StringUtility::Utf8ToUtf16(inputId.GetIdentifier().IdentifierString());
        listInputKey.push_back(key);

        // Export ScriptNode
        std::string identity = inputId.GetIdentifier().IdentifierString(); //DE name       
        OA::OAString scriptName = OA::ModelDataAPI::OAModelDataAPIUtility::MakeScriptNameOfOutsideTemplateNode(inputId); //Lua name

        mapKeyScriptName.emplace(identity, scriptName);
        
        ScriptNodeStruct nodeStruct;
        auto it = mapKeyRecord.find(OA::StringUtility::Utf8ToUtf16(identity));
        if (it != mapKeyRecord.end())
        {
            if (it->second->GetDataTypeId() != 0)
            {
                nodeStruct.nodeType = ScriptNodeType::Variable;
                nodeStruct.dataType = it->second->GetDataTypeId();
                nodeStruct.initValue = it->second->GetValue();
                nodeStruct.initStatusCode = it->second->GetQuality();
                nodeStruct.initTimeStamp = it->second->GetTimeInterval();
                //nodeStruct.isOutput = (it->second->GetItemType() == OA::ModelDataAPI::FepSimulationItemType::RandomGenerator) ? true : false;
                if (it->second->GetItemType() == OA::ModelDataAPI::FepSimulationItemType::RandomGenerator)
                {
                    bool isGenning = static_cast<KafkaRandomGeneratorRecordInfo*>(it->second.get())->IsGenning();
                    nodeStruct.isOutput = isGenning;
                }              
            }

            nodeStruct.uniqueId = inputId;
            nodeStruct.nodeIdString = OA::StringUtility::Utf8ToUtf16(identity);
            nodeStruct.replacedString = scriptName;
        }

        if (nodeStruct.nodeType == ScriptNodeType::Variable/* && nodeStruct.dataType == OA_DataType_Boolean*/)
        {
            pScript->ExportScriptNode(nodeStruct);
        }
    } 

    std::string codeTest = R"(
     if node4111_5b4fa88f.value then
	    node4111_c2d12b4c:Simulate()
	    node4111_5cb5beef:Simulate()	
    else
	    node4111_c2d12b4c:Force(0)
	    node4111_5cb5beef:Force(0)	
    end
        )";

    //pScript->RunScript(OA::StringUtility::Utf8ToUtf16(codeTest), errMsg);
    pScript->RunScript(pRecord->GetContent(), errMsg);

    for (size_t i = 0; i < listInputKey.size(); i++)
    {
        if (mapKeyRecord.at(listInputKey[i])->GetItemType() == OA::ModelDataAPI::FepSimulationItemType::RandomGenerator)
        {
            std::unique_ptr<KafkaRandomGeneratorRecordInfo> pRecord = std::make_unique<KafkaRandomGeneratorRecordInfo>();

            OA::OAString key;
            OA::OAVariant value;
            OA::OAInt64 timeInterval;
            OA::OAUInt64 statusCode;
            OA::OAUInt16 dataTypeId = 0;
            bool isOutput = true;
            pRecord->SetKey(listInputKey[i]);

            auto it = mapKeyScriptName.find(OA::StringUtility::Utf16ToUtf8(listInputKey[i]));
            if (it != mapKeyScriptName.end())
            {
                pScript->GetValueScriptUserTypeByKey(it->second, value, dataTypeId, statusCode, timeInterval, isOutput);
                pRecord->SetValue(value);
                pRecord->SetQuality((OA::OAUInt32)statusCode);
                pRecord->SetDataTypeId(dataTypeId);
                pRecord->SetTimeInterval(timeInterval);
                pRecord->SetIsGenning(isOutput);
            }
            if (!isOutput && m_pKafkaProducer->HasDataChange(pRecord.get()) ) // Forced with Arg
            {
                m_pKafkaProducer->ProductMsg(pRecord.get());
                m_pSimullationItemManager->UpdateMapModel(pRecord->GetKey(), pRecord.get());
            }

            if (isOutput && m_pKafkaProducer->HasDataChange(pRecord.get()) /*&& pRecord->GetKey() == _T("SAS_SIM.S1.A1.MEASF.P1")*/)
            {
                //m_pKafkaProducer->ProductMsg(pRecord.get());
                m_pSimullationItemManager->UpdateMapModel(pRecord->GetKey(), /*pRecord.get()*/isOutput);
            }
            //m_pSimullationItemManager->UpdateMapModel(pRecord->GetKey(), pRecord.get());
        }        
    }
}

