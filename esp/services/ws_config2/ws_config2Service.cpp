#include "ws_config2Service.hpp"
//#include "ConfiguratorAPI.hpp"
#include "jstring.hpp"


Cws_config2Ex::Cws_config2Ex()
{
    //CONFIGURATOR_API::initialize();
    int x;
    x = 4;
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

    if (path == ".")
    {
        Owned<IEspelementType> pElement = createelementType();
        pElement->setName("hw");
        pElement->setDisplayName("Hardware Configuration");
        pElement->setClass("category");
        pElement->setPath(".hw");
        pElement->updateDoc().setTooltip("Defines the hardware configuration for the cluster");
        pElement->setNumChildren(5);
        pElement->updateElementData().setNumAllowedInstances(0);
        pElement->updateElementData().setNumRequiredInstances(0);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->setName("sw");
        pElement->setDisplayName("Software Configuration");
        pElement->setClass("category");
        pElement->setPath(".sw");
        pElement->updateDoc().setTooltip("Defines the software configuration for the cluster");
        pElement->setNumChildren(4);
        pElement->updateElementData().setNumAllowedInstances(0);
        pElement->updateElementData().setNumRequiredInstances(0);
        elements.append(*pElement.getLink());

        
    }
    else if (path == ".sw")
    {
        Owned<IEspelementType> pElement = createelementType();
        pElement->setName("esp");
        pElement->setDisplayName("ESP Processes");
        pElement->setClass("instanceSet");
        pElement->setPath(".sw.esp");
        pElement->updateDoc().setTooltip("Defines ESP processes that run on various systems in the cluster");
        pElement->setNumChildren(2);
        pElement->updateElementData().setNumAllowedInstances(0);
        pElement->updateElementData().setNumRequiredInstances(1);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->setName("dropzones");
        pElement->setDisplayName("Dropzones");
        pElement->setClass("instanceSet");
        pElement->setPath(".sw.dropzones");
        pElement->updateDoc().setTooltip("Define dropzones for file transfer activities");
        pElement->setNumChildren(2);
        pElement->updateElementData().setNumAllowedInstances(0);
        pElement->updateElementData().setNumRequiredInstances(0);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->setName("loggingagents");
        pElement->setDisplayName("Loggin Agents");
        pElement->setClass("instanceSet");
        pElement->setPath(".sw.loggingagents");
        pElement->updateDoc().setTooltip("Define logging agents for use across ESP and other services");
        pElement->setNumChildren(0);
        pElement->updateElementData().setNumAllowedInstances(0);
        pElement->updateElementData().setNumRequiredInstances(0);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->setName("xyzservice");
        pElement->setDisplayName("XYZ Service");
        pElement->setClass("component");
        pElement->setPath(".sw.xyzservice");
        pElement->updateDoc().setTooltip("Configure XYZ service");
        pElement->setNumChildren(3);
        pElement->updateElementData().setNumAllowedInstances(0);
        pElement->updateElementData().setNumRequiredInstances(1);
        elements.append(*pElement.getLink());        

    }
    else if (path == ".sw.xyzservice")
    {
        Owned<IEspelementType> pElement = createelementType();
        pElement->setName("attributes");
        pElement->setDisplayName("Attributes");
        pElement->setClass("valuesetSet");
        pElement->setPath(".sw.xyzservice.attributes");
        pElement->updateDoc().setTooltip("Attributes");
        pElement->setNumChildren(3);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->setName("otherstuff");
        pElement->setDisplayName("Other Stuff");
        pElement->setClass("valueSet");
        pElement->setPath(".sw.xyzservice.otherstuff");
        pElement->updateDoc().setTooltip("Some other settings");
        pElement->setNumChildren(4);
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->setName("instances");
        pElement->setDisplayName("System Bindings");
        pElement->setClass("instanceSet");
        pElement->setPath(".sw.xyzservice.instances");
        pElement->updateDoc().setTooltip("Computer bindings for the xyz service (5 allowed)");
        pElement->setNumChildren(0);  // current number defined (must compare with numXXXInstances below to see if can create or if any required)
        pElement->updateElementData().setNumAllowedInstances(5);
        pElement->updateElementData().setNumRequiredInstances(1);
        elements.append(*pElement.getLink());
    }
    else if (path == ".sw.xyzservice.attributes")
    {
        Owned<IEspelementType> pElement = createelementType();
        pElement->setName("username");
        pElement->setDisplayName("Username");
        pElement->setClass("value");
        pElement->updateDoc().setTooltip("Username for all operations");
        pElement->updateElementData().updateType().setName("string");
        pElement->updateElementData().updateType().updateLimits().setMin(5);
        pElement->updateElementData().updateType().updateLimits().setMax(15);
        StringArray excludeList;
        excludeList.append("username");
        pElement->updateElementData().updateType().updateLimits().setDisallowList(excludeList);
        pElement->updateElementData().setCurrentValue("bobtheuser");
        pElement->updateElementData().setDefaultValue("username");
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->setName("password");
        pElement->setDisplayName("Password");
        pElement->setClass("value");
        pElement->updateDoc().setTooltip("Password for the user");
        pElement->updateElementData().updateType().setName("string");
        pElement->updateElementData().updateType().updateLimits().setMin(5);
        pElement->updateElementData().updateType().updateLimits().setMax(15);
        StringArray modifiers;
        modifiers.append("password");
        pElement->updateElementData().updateType().setModifiers(modifiers);
        pElement->updateElementData().setCurrentValue("mypassword");
        pElement->updateElementData().setDefaultValue("");
        elements.append(*pElement.getLink());

        pElement.setown(createelementType());
        pElement->setName("maxopenfiles");
        pElement->setDisplayName("Max Open Files");
        pElement->setClass("value");
        pElement->updateDoc().setTooltip("Password for the user");
        pElement->updateElementData().updateType().setName("integer");
        pElement->updateElementData().updateType().updateLimits().setMin(1);
        pElement->updateElementData().updateType().updateLimits().setMax(20);
        pElement->updateElementData().setCurrentValue("6");
        pElement->updateElementData().setDefaultValue("3");
        elements.append(*pElement.getLink());


    }


    // just add the elements we made
    resp.setElements(elements); 
    resp.setStatus("ok");

    return(true);
}


