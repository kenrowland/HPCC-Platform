#include "ws_config2Service.hpp"
//#include "ConfiguratorAPI.hpp"
#include "jstring.hpp"
#include "ConfigItem.hpp"


Cws_config2Ex::Cws_config2Ex()
{
    //CONFIGURATOR_API::initialize();
    
    //std::shared_ptr<ConfigItem> pConfig = std::make_shared<ConfigItem>("root");
    //ConfigParser *pCfgParser = new XSDConfigParser("", pConfig);
    //m_envMgr.setConfig(pConfig);

    m_pEnvMgr = getEnvironmentMgrInstance("XML", "/opt/HPCCSystems/componentfiles/config2xml/");
    std::vector<std::string> cfgParms;
    cfgParms.push_back("newenv.xsd");
    cfgParms.push_back("buildset.xml");
    m_pEnvMgr->loadConfig(cfgParms);
    m_pEnvMgr->loadEnvironment("environment.xml");

}

Cws_config2Ex::~Cws_config2Ex()
{
}

bool Cws_config2Ex::ongetPath(IEspContext &context, IEspGetPathRequest &req, IEspGetPathResponse &resp)
{
    std::string path = req.getPath();
    std::shared_ptr<EnvironmentNode> pNode = m_pEnvMgr->getNodeFromPath(path);

    IArrayOf<IEspelementType> elements;
    

    //
    // Fill out the response headdre stuff
    resp.setInputPath(path.c_str());
    resp.setStatus("ok");

    //
    // Attributes?
    IArrayOf<IEspattributeType> nodeAttributes;
    if (pNode->hasAttributes())
    {
        std::vector<std::shared_ptr<EnvValue>> attributes = pNode->getAttributes();
        for (auto it=attributes.begin(); it!=attributes.end(); ++it)
        {
            std::shared_ptr<EnvValue> pAttr = *it;
            Owned<IEspattributeType> pAttribute = createattributeType();
            
			const std::shared_ptr<CfgValue> &pCfgValue = pAttr->getCfgValue();
            pAttribute->updateItemInfo().setName(pAttr->getName().c_str());
            pAttribute->updateItemInfo().setDisplayName(pCfgValue->getDisplayName().c_str());
            pAttribute->updateDoc().setTooltip(pCfgValue->getTooltip().c_str());
            
			const std::shared_ptr<CfgType> &pType = pCfgValue->getType();
            pAttribute->updateType().setName(pType->getName().c_str());
            pAttribute->updateType().updateLimits().setMin(pType->getLimits()->getMin());
            pAttribute->updateType().updateLimits().setMax(pType->getLimits()->getMin());
            StringArray excludeList;
            //excludeList.append("username");
            pAttribute->updateType().updateLimits().setDisallowList(excludeList);

            pAttribute->setCurrentValue(pAttr->getValue().c_str());
            pAttribute->setDefaultValue("");
            
			pAttribute->updateItemInfo().setStatus("ok");
            nodeAttributes.append(*pAttribute.getLink());
        }
    }
    resp.setAttributes(nodeAttributes); 

    //
    // Now the children
    if (pNode->hasChildren())
    {
        std::vector<std::shared_ptr<EnvironmentNode>> children = pNode->getChildren();
        for (auto it=children.begin(); it!=children.end(); ++it)
        {
            std::shared_ptr<EnvironmentNode> pChildNode = *it;
            std::shared_ptr<ConfigItem> pConfigItem = pChildNode->getConfigItem();
            Owned<IEspelementType> pElement = createelementType();
            pElement->updateItemInfo().setName(pChildNode->getName().c_str());
            pElement->updateItemInfo().setDisplayName(pConfigItem->getDisplayName().c_str());
            pElement->setClass(pConfigItem->getClassName().c_str());
            pElement->setNumAllowedInstances(pConfigItem->getMaxInstances());
            pElement->setNumRequiredInstances(pConfigItem->getMinInstances());
                        
            pElement->updateItemInfo().setStatus("ok");
            pElement->setPath(pChildNode->getPath().c_str());
            pElement->updateDoc().setTooltip("");
            pElement->setNumChildren(pChildNode->getNumChildren());
            elements.append(*pElement.getLink());
        }
    }
    resp.setChildren(elements); 


    //StringArray mods;
    //mods.append("mod1");
    //mods.append("mod2");

    //resp.setModxxx(mods);

    
    // StringBuffer path = req.getPath();

    //
    // Temp code
    //resp.setInputPath(path.c_str());
    //return(mockInterface(path, resp));


    // IArrayOf<IEspelementType> elements;
    // Owned<IEspelementType> pElement = createelementType();
    
    // pElement->setName("esp");
    // pElement->setDisplayName("ESP Service");
    // elements.append(*pElement.getLink()); 
    // resp.setElements(elements); 

    // resp.setStatus(status);
    return true;  
}


bool Cws_config2Ex::onsetValues(IEspContext &context, IEspSetValuesRequest &req, IEspGetPathResponse &resp)
{
    std::string path = req.getPath();
    std::shared_ptr<EnvironmentNode> pNode = m_pEnvMgr->getNodeFromPath(path);
    if (pNode)
    {
        bool forceCreate = req.getForceCreate();
        bool allowInvalid = req.getAllowInvalid();
        IArrayOf<IConstattributeValueType> &attrbuteValues = req.getAttributeValues();
        ForEachItemIn(i, attrbuteValues)
        {
            IConstattributeValueType& attrVal = attrbuteValues.item(i);
            std::string name = attrVal.getName();
            std::string value = attrVal.getValue();
            pNode->setAttributeValue(name, value, allowInvalid, forceCreate);
        }
    }
    else
    {
        resp.setStatus("error");
        resp.updateDoc().setMsg("Invalid path");
    }
   
    return true;
}


bool Cws_config2Ex::mockInterface(const std::string &path, IEspGetPathResponse &resp)
{
    IArrayOf<IEspelementType> elements;
    IArrayOf<IEspattributeType> attributes;

    if (path == ".")
    {
        Owned<IEspelementType> pElement = createelementType();
        pElement->updateItemInfo().setName("hw");
        pElement->updateItemInfo().setDisplayName("Hardware Configuration");
        pElement->updateItemInfo().setStatus("ok");
        pElement->setClass("category");
        pElement->setPath(".hw");
        pElement->updateDoc().setTooltip("Defines the hardware configuration for the cluster");
        pElement->setNumChildren(5);
        pElement->setNumAllowedInstances(0);
        pElement->setNumRequiredInstances(0);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->updateItemInfo().setName("sw");
        pElement->updateItemInfo().setDisplayName("Software Configuration");
        pElement->updateItemInfo().setStatus("ok");
        pElement->setClass("category");
        pElement->setPath(".sw");
        pElement->updateDoc().setTooltip("Defines the software configuration for the cluster");
        pElement->setNumChildren(4);
        pElement->setNumAllowedInstances(0);
        pElement->setNumRequiredInstances(0);
        elements.append(*pElement.getLink());
    }
    else if (path == ".sw")
    {
        Owned<IEspelementType> pElement = createelementType();
        pElement->updateItemInfo().setName("esp");
        pElement->updateItemInfo().setDisplayName("ESP Processes");
        pElement->updateItemInfo().setStatus("ok");
        pElement->setClass("instanceSet");
        pElement->setPath(".sw.esp");
        pElement->updateDoc().setTooltip("Defines ESP processes that run on various systems in the cluster");
        pElement->setNumChildren(2);
        pElement->setNumAllowedInstances(0);
        pElement->setNumRequiredInstances(1);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->updateItemInfo().setName("dropzones");
        pElement->updateItemInfo().setDisplayName("Dropzones");
        pElement->updateItemInfo().setStatus("ok");
        pElement->setClass("instanceSet");
        pElement->setPath(".sw.dropzones");
        pElement->updateDoc().setTooltip("Define dropzones for file transfer activities");
        pElement->setNumChildren(2);
        pElement->setNumAllowedInstances(0);
        pElement->setNumRequiredInstances(0);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->updateItemInfo().setName("loggingagents");
        pElement->updateItemInfo().setDisplayName("Logging Agents");
        pElement->updateItemInfo().setStatus("ok");
        pElement->setClass("instanceSet");
        pElement->setPath(".sw.loggingagents");
        pElement->updateDoc().setTooltip("Define logging agents for use across ESP and other services");
        pElement->setNumChildren(0);
        pElement->setNumAllowedInstances(0);
        pElement->setNumRequiredInstances(0);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->updateItemInfo().setName("xyzservice");
        pElement->updateItemInfo().setDisplayName("XYZ Service");
        pElement->updateItemInfo().setStatus("ok");
        pElement->setClass("component");
        pElement->setPath(".sw.xyzservice");
        pElement->updateDoc().setTooltip("Configure XYZ service");
        pElement->setNumChildren(3);
        pElement->setNumAllowedInstances(0);
        pElement->setNumRequiredInstances(1);
        elements.append(*pElement.getLink());        

    }
    else if (path == ".sw.xyzservice")
    {
        Owned<IEspelementType> pElement = createelementType();

        pElement->updateItemInfo().setName("otherstuff");
        pElement->updateItemInfo().setDisplayName("Other Stuff");
        pElement->updateItemInfo().setStatus("ok");
        pElement->setClass("valueSet");
        pElement->setPath(".sw.xyzservice.otherstuff");
        pElement->updateDoc().setTooltip("Some other settings");
        pElement->setNumChildren(4);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->updateItemInfo().setName("instances");
        pElement->updateItemInfo().setDisplayName("System Bindings");
        pElement->updateItemInfo().setStatus("ok");
        pElement->setClass("instanceSet");
        pElement->setPath(".sw.xyzservice.instances");
        pElement->updateDoc().setTooltip("Computer bindings for the xyz service (5 allowed)");
        pElement->setNumChildren(0);  // current number defined (must compare with numXXXInstances below to see if can create or if any required)
        pElement->setNumAllowedInstances(5);
        pElement->setNumRequiredInstances(1);
        elements.append(*pElement.getLink());

        Owned<IEspattributeType> pAttribute = createattributeType();
        pAttribute->updateItemInfo().setName("username");
        pAttribute->updateItemInfo().setDisplayName("Username");
        pAttribute->updateItemInfo().setStatus("ok");
        pAttribute->updateDoc().setTooltip("Username for all operations");
        pAttribute->updateType().setName("string");
        pAttribute->updateType().updateLimits().setMin(5);
        pAttribute->updateType().updateLimits().setMax(15);
        StringArray excludeList;
        excludeList.append("username");
        pAttribute->updateType().updateLimits().setDisallowList(excludeList);
        pAttribute->setCurrentValue("bobtheuser");
        pAttribute->setDefaultValue("username");
        attributes.append(*pAttribute.getLink());

        pAttribute.setown(createattributeType());
        pAttribute->updateItemInfo().setName("password");
        pAttribute->updateItemInfo().setDisplayName("Password");
        pAttribute->updateItemInfo().setStatus("ok");
        pAttribute->updateDoc().setTooltip("User's password");
        pAttribute->updateType().setName("string");
        pAttribute->updateType().updateLimits().setMin(5);
        pAttribute->updateType().updateLimits().setMax(15);
        StringArray modifiers;
        modifiers.append("mask");
        modifiers.append("verify");
        pAttribute->updateType().updateLimits().setDisallowList(excludeList);
        attributes.append(*pAttribute.getLink());

        pAttribute.setown(createattributeType());
        pAttribute->updateItemInfo().setName("maxopenfiles");
        pAttribute->updateItemInfo().setDisplayName("Max Open Files");
        pAttribute->updateItemInfo().setStatus("ok");
        pAttribute->updateDoc().setTooltip("Password for the user");
        pAttribute->updateType().setName("integer");
        pAttribute->updateType().updateLimits().setMin(1);
        pAttribute->updateType().updateLimits().setMax(20);
        pAttribute->setCurrentValue("6");
        pAttribute->setDefaultValue("3");
        attributes.append(*pAttribute.getLink());



    }
    // else if (path == ".sw.xyzservice.attributes")
    // {
    //     Owned<IEspelementType> pElement = createelementType();
    //     pElement->updateItemInfo().setName("username");
    //     pElement->updateItemInfo().setDisplayName("Username");
    //     pElement->setClass("value");
    //     pElement->updateDoc().setTooltip("Username for all operations");
    //     pElement->updateElementData().updateType().setName("string");
    //     pElement->updateElementData().updateType().updateLimits().setMin(5);
    //     pElement->updateElementData().updateType().updateLimits().setMax(15);
    //     StringArray excludeList;
    //     excludeList.append("username");
    //     pElement->updateElementData().updateType().updateLimits().setDisallowList(excludeList);
    //     pElement->updateElementData().setCurrentValue("bobtheuser");
    //     pElement->updateElementData().setDefaultValue("username");
    //     elements.append(*pElement.getLink());

    //     pElement.setown(createelementType());
    //     pElement->updateItemInfo().setName("password");
    //     pElement->updateItemInfo().setDisplayName("Password");
    //     pElement->setClass("value");
    //     pElement->updateDoc().setTooltip("Password for the user");
    //     pElement->updateElementData().updateType().setName("string");
    //     pElement->updateElementData().updateType().updateLimits().setMin(5);
    //     pElement->updateElementData().updateType().updateLimits().setMax(15);
    //     StringArray modifiers;
    //     modifiers.append("password");
    //     pElement->updateElementData().updateType().setModifiers(modifiers);
    //     pElement->updateElementData().setCurrentValue("mypassword");
    //     pElement->updateElementData().setDefaultValue("");
    //     elements.append(*pElement.getLink());

    //     pElement.setown(createelementType());
    //     pElement->updateItemInfo().setName("maxopenfiles");
    //     pElement->updateItemInfo().setDisplayName("Max Open Files");
    //     pElement->setClass("value");
    //     pElement->updateDoc().setTooltip("Password for the user");
    //     pElement->updateElementData().updateType().setName("integer");
    //     pElement->updateElementData().updateType().updateLimits().setMin(1);
    //     pElement->updateElementData().updateType().updateLimits().setMax(20);
    //     pElement->updateElementData().setCurrentValue("6");
    //     pElement->updateElementData().setDefaultValue("3");
    //     elements.append(*pElement.getLink());


    // }


    // just add the elements we made
    resp.setChildren(elements); 
    resp.setAttributes(attributes); 
    resp.setStatus("ok");

    return(true);
}


