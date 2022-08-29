#pragma once

#include "KafkaProducer.h"
#include "SimulationItemManager.h"
#include "KafkaRecordInfo.h"
#include "KafkaConsumer.h"

#include <regex>

#include <OAModelDataAPI/FepSimulation/FepSimulationItemInfo.h>
#include <OABase/OAUniqueID.h>

#include <tchar.h>

std::unique_ptr<KafkaProducer> m_pKafkaProducer;
std::unique_ptr<SimulationItemManager> m_pSimullationItemManager;
std::unique_ptr<KafkaConsumer> m_pKafkaConsumer;


void Initialize();

// UTILITY HANDLE STRING
bool IsInputNodeTerminationCharacters(OA::_OAChar ch);
std::vector<InputNodeInScript> FindInputNodesInScriptContent(const OA::OAString& content);
std::vector<InputNodeInScript> FindInputNodesInScriptContentByRegex(const OA::OAString& content);


//void HandleTriggerScenarioRecord(KafkaTriggerScenarioRecordInfo* pRecord);

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

std::vector<InputNodeInScript> FindInputNodesInScriptContentByRegex(const OA::OAString& content)
{
    std::vector<InputNodeInScript> listScriptNode;

    std::string s = OA::StringUtility::Utf16ToUtf8(content);
    //std::string regex = (R"(([^\s\\]*?(?=:))|([^\s\\]*?(?=\s:))|([^\s\\]*?(?=\s=))|(\S+(value|stVal)))");
    std::string regex = (R"(([^\s\\]*?(?=:))|([^\s\\]*?(?=\s:))|([^\s\\]*?(?=.value)))");
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

                if ((*iter)[i].str().size() > 0)
                {
                    OA::OAString scriptName = OA::StringUtility::Utf8ToUtf16((*iter)[i]);

                    InputNodeInScript inputNode(scriptName, pos);

                    bool existScripNode = (std::find(listScriptNode.begin(), listScriptNode.end(), inputNode) != listScriptNode.end());
                    if (!existScripNode)
                        listScriptNode.emplace_back(inputNode);
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