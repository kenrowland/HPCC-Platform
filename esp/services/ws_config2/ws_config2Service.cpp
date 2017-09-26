#include "ws_config2Service.hpp"
//#include "ConfiguratorAPI.hpp"
#include "jstring.hpp"
#include "ConfigItem.hpp"


Cws_config2Ex::Cws_config2Ex()
{
    //CONFIGURATOR_API::initialize();
    
    std::shared_ptr<ConfigItem> pConfig = std::make_shared<ConfigItem>("root");
    ConfigParser *pCfgParser = new XSDConfigParser("", pConfig);
    m_envMgr.setConfig(pConfig);

}

Cws_config2Ex::~Cws_config2Ex()
{
}

bool Cws_config2Ex::ongetPath(IEspContext &context, IEspGetPathRequest &req, IEspGetPathResponse &resp)
{
    StringBuffer status = "ok";

    StringArray mods;
    mods.append("mod1");
    mods.append("mod2");

    //resp.setModxxx(mods);

    std::string path = req.getPath();
    // StringBuffer path = req.getPath();

    //
    // Temp code
    resp.setInputPath(path.c_str());
    return(mockInterface(path, resp));


    // IArrayOf<IEspelementType> elements;
    // Owned<IEspelementType> pElement = createelementType();
    
    // pElement->setName("esp");
    // pElement->setDisplayName("ESP Service");
    // elements.append(*pElement.getLink()); 
    // resp.setElements(elements); 

    // resp.setStatus(status);
    // return true;
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


