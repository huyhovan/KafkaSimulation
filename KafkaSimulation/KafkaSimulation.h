#pragma once

#include "KafkaProducer.h"
#include "SimulationItemManager.h"
#include "KafkaRecordInfo.h"
#include "KafkaConsumer.h"

#include <regex>

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OABase/OAUniqueID.h>
#include <OAModelDataAPI/OAModelDataAPIUtility.h>

std::unique_ptr<KafkaProducer> m_pKafkaProducer;
std::unique_ptr<SimulationItemManager> m_pSimullationItemManager;
std::unique_ptr<KafkaConsumer> m_pKafkaConsumer;

struct InputNodeInScript
{
    OA::OAString str;
    size_t pos;

    InputNodeInScript(const OA::OAString& _str, size_t _pos)
        :str(_str), pos(_pos) {};
    InputNodeInScript(OA::OAString&& _str, size_t _pos)
        :str(std::move(_str)), pos(_pos) {};
};


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


// UTILITY HANDLE STRING

bool IsInputNodeTerminationCharacters(OA::_OAChar ch) 
{
    switch (ch)
    {
    case ' ':
        return true;
    case '=':
        return true;
    case '>':
        return true;
    case '<':
        return true;
    case ',':
        return true;
    case ';':
        return true;
    case '+':
        return true;
    case '-':
        return true;
    case '*':
        return true;
    case '/':
        return true;
    case '(':
        return true;
    case ')':
        return true;
    case '[':
        return true;
    case ']':
        return true;
    case '%':
        return true;
    case '~':
        return true;
    case '\\':
        return true;
    case '\t':
        return true;
    case '\n':
        return true;
    case '\r':
        return true;
    default:
        break;
    }

    return false;
}

std::vector<InputNodeInScript> FindInputNodesInScriptContent(const OA::OAString& content) 
{
    // Create list of InputScript
    std::vector<InputNodeInScript> inputScriptNodes;
    size_t pos = 0;
    while (true)
    {
        pos = content.find('.', pos);
        if (pos == OA::OAString::npos)
            break;

        size_t beginPos = 0;
        for (size_t i = pos; ; i--)
        {
            beginPos = i;
            OA::_OAChar c = content[i];
            if (IsInputNodeTerminationCharacters(c))
            {
                beginPos++;
                break;
            }

            if (i == 0)
                break;
        }

        size_t endPos = pos;
        for (size_t i = pos; i < content.size(); i++)
        {
            endPos = i;
            OA::_OAChar c = content[i];
            if (IsInputNodeTerminationCharacters(c))
                break;
        }

        OA::OAString nodeIdStr = content.substr(beginPos, endPos - beginPos);
        inputScriptNodes.emplace_back(std::move(nodeIdStr), beginPos);

        pos = endPos + 1;
        if (pos > content.size())
            break;
    }

    return inputScriptNodes;
}

std::vector<OA::OAString> FindInputNodesInScriptContentByRegex(const OA::OAString& content)
{
    std::vector<OA::OAString> listScriptNode;

    std::string s = OA::StringUtility::Utf16ToUtf8(content);
    std::string regex = (R"(([^\s\\]*?(?=:))|([^\s\\]*?(?=\s:))|([^\s\\]*?(?=\s=))|(\S+(value|stVal)))");
    
    try {
        std::regex e(regex);
        std::smatch match;

        std::sregex_iterator iter(s.begin(), s.end(), e);
        std::sregex_iterator end;
        while (iter != end)
        {
            for (unsigned i = 0; i < iter->size(); ++i)
            {
                size_t pos = iter->position();

                //std::cout << "the " << i + 1 << "th match" << ": " << (*iter)[i] << std::endl;
                if ((*iter)[i].str().size() > 0)
                {       
                    OA::OAString scriptName = OA::StringUtility::Utf8ToUtf16((*iter)[i]);
                    bool existScripNode = (std::find(listScriptNode.begin(), listScriptNode.end(), scriptName) != listScriptNode.end());
                    if(!existScripNode)
                        listScriptNode.emplace_back(OA::StringUtility::Utf8ToUtf16((*iter)[i]));
                }
            }
            ++iter;
        }

       /* while (std::regex_search(s, match, e))
        {
            for (auto key : match)
            {
                if (key.str().size() > 0)
                {
                    std::cout << key << std::endl;
                    listScriptNode.emplace_back(OA::StringUtility::Utf8ToUtf16(key));
                }            
            }

            s = match.suffix().str();
        }*/
    }
    catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
    }
   
    return listScriptNode;
}

void HandleControlScenarioRecord(KafkaControlScenarioRecordInfo* pRecord)
{
    std::map <std::string, OA::OAString> mapKeyScriptName;

    std::vector<OA::OAUniqueID> listId = pRecord->GetInputs();
    for (size_t i = 0; i < listId.size(); i++)
    {
        std::string identity = listId[i].GetIdentifier().IdentifierString();
        OA::OAString scriptName = OA::ModelDataAPI::OAModelDataAPIUtility::MakeScriptNameOfOutsideTemplateNode(listId[i]);

        mapKeyScriptName.emplace(identity, scriptName);
    }

    OA::OAString content = pRecord->GetContent();

    std::vector<InputNodeInScript> inputNodesInLine = FindInputNodesInScriptContent(content);
    std::vector<OA::OAString> inputNodesInLineByRegex = FindInputNodesInScriptContentByRegex(content);

    std::string s = OA::StringUtility::Utf16ToUtf8(content);

    //std::cout << "\nContent of control D03F87L.D03F87LCON.RBGGIO2.SPCSO13.Control:\n " << s << std::endl;
   
}

void HandleControlScenarioRecord(KafkaTriggerScenarioRecordInfo* pRecord)
{
    std::map <std::string, OA::OAString> mapKeyScriptName;   

    OA::OAString content = pRecord->GetContent();

    std::vector<InputNodeInScript> inputNodesInLine = FindInputNodesInScriptContent(content);
    std::vector<OA::OAString> inputNodesInLineByRegex = FindInputNodesInScriptContentByRegex(content);

    std::string s = OA::StringUtility::Utf16ToUtf8(content);

    //std::cout << "\nContent of control D03F87L.D03F87LCON.RBGGIO2.SPCSO13.Control:\n " << s << std::endl;

}