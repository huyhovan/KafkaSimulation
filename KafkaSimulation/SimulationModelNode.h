#pragma once
#include <OABase/OABase.h>
#include <OABase/OAString.h>
#include <OABase/OAVariant.h>
namespace OA
{
    namespace ModelDataAPI
    {
       class FepSimulationItemInfo;
    }
}


class SimulationModelNode
{
public:
    SimulationModelNode();
    virtual ~SimulationModelNode();

    OA::OAString GetKey() const;
    OA::ModelDataAPI::FepSimulationItemInfo* GetItemInfo() const;
    OA::OAVariant GetValue() const;

    void SetKey(OA::OAString key);
    void SetItemInfo(OA::ModelDataAPI::FepSimulationItemInfo* itemInfo);
    void SetValue(OA::OAVariant value);

protected:
    OA::OAString m_key;
    OA::ModelDataAPI::FepSimulationItemInfo* m_pItemInfo;
    OA::OAVariant m_Value;

    
};

