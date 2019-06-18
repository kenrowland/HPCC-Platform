/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2017 HPCC Systems®.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
############################################################################## */

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "build-config.h"
#include "platform.h"
#include <exception>
#include <sys/types.h>
#include <sys/stat.h>
#include "EnvironmentMgr.hpp"
#include "Exceptions.hpp"
#include "mod_template_support/EnvModTemplate.hpp"
#include "mod_template_support/TemplateExecutionException.hpp"
#include "mod_template_support/Environment.hpp"
#include "jutil.hpp"

//
// Inputs file format (json)
//
// {
//   "input-name" : [ <array of values> ],
//   ...
// }
// -t master.json -o env160cluster.xml -d /home/rowlke01/hpcc/src/initfiles/componentfiles/configschema/xsd -d /home/rowlke01/hpcc/ln/initfiles/componentfiles/configschema/xsd -m environment.xsd -s /home/rowlke01/hpcc/src/initfiles/componentfiles/configschema/templates/schema/ModTemplateSchema.json

bool processOptions(int argc, char *vargv[]);
bool validate();
std::string getNextArg(int argc, char *argv[], int idx);
bool dirExists(const std::string &dirName);
bool fileExists(const std::string &fileName);
bool forceOutput = false;
std::vector<std::string> splitString(const std::string &input, const std::string &delim);

// Default configuration directories
EnvironmentType envType = XML;
std::string masterSchemaFile = "environment.xsd";
//std::string configSchemaDir = "";
std::string configSchemaRelativetDir = ".." PATHSEPSTR "componentfiles"  PATHSEPSTR "configschema" PATHSEPSTR "xsd" PATHSEPSTR;
std::string configSchemaDefaultDir;
std::string workingDir;
std::string modTemplateFile;
std::string modTemplateSchemaFile = COMPONENTFILES_DIR PATHSEPSTR "configschema" PATHSEPSTR "templates" PATHSEPSTR "schema" PATHSEPSTR "ModTemplateSchema.json";
//std::string configSchemaPluginsDir = "";

std::vector<std::string> configDirs;

std::string inputEnvFile, envOutputFile;
struct InputDef {
    std::string inputName;
    std::string inputValue;
};

std::vector<InputDef> userInputs;
bool listInputs = false;

std::shared_ptr<EnvironmentMgr> pEnvMgr;
EnvModTemplate *pTemplate = nullptr;

class CliException : public std::exception
{
    public:

        explicit CliException(std::string reason) :  m_reason(std::move(reason)) { }
        virtual const char *what() const noexcept override
        {
            return m_reason.c_str();
        }

    private:

        std::string m_reason;
};


void usage()
{
    //
    // usage below documents options
    std::cout << std::endl;
    std::cout << "envmod <options> envfile" << std::endl;
    std::cout << std::endl;
    std::cout << "  Configuration Options:" << std::endl;
    std::cout << "    -d --schema-dir <path>            : path to schema files (>1 allowed). default (" << configSchemaRelativetDir << ")" << std::endl;
    std::cout << "    -m --master-config <filename>     : name of master schema file, default (" << masterSchemaFile << ")" << std::endl;
    std::cout << "    -s --template-schema <fullpath>   : path to template schema file (" << modTemplateSchemaFile << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "  Execution Options:" << std::endl;
    std::cout << "    -t --template <filename>          : Name of template file. If the template file is not in the current directory" << std::endl;
    std::cout << "                                          Then use -w/--working-dir to define the path to the template file" << std::endl;
    std::cout << "    -e --env <fullpath>               : Optional filepath to environment file to modify" << std::endl;
    std::cout << "                                          If omitted, a new empty environment is created" << std::endl;
    std::cout << "       --list-inputs                  : List the template inputs. If this option is specified, the template" << std::endl;
    std::cout << "                                          inputs are listed and the command exits." << std::endl;
    std::cout << "       --input-file <fullpath>        : Optional full path to a file with defined inputs for the template" << std::endl;
    std::cout << "                                          These would be combined with any command line variable assignments" << std::endl;
    std::cout << "    -v --var <var=value>              : Assign the indicated value to the variable input." << std::endl;
    std::cout << "                                          value may be a comma separated list (no blanks) for multi-value inputs" << std::endl;
    std::cout << "                                          Inputs given on the command line override any in an input file (--input-file)" << std::endl;
    std::cout << "                                          Repeat this option as many times as needed" << std::endl;
    std::cout << "    -o --output [fullpath]            : Optional. If present, results of template execution is saved." << std::endl;
    std::cout << "                                          If fullpath is specified, results written to this file, " << std::endl;
    std::cout << "                                          If not specified, input environment file is overwritten" << std::endl;
    std::cout << "                                          If -o is omitted, no results are saved (validates application of the template)" << std::endl;
    std::cout << "       --ignore-errors                : Optional. If present, outputs are written regardless of errors." << std::endl;
    std::cout << "    -w --working-dir                  : Optional. If present, use this as the working directory for loading templates." << std::endl;
    std::cout << "                                          All included templates with a relative path are loaded from this directory" << std::endl;

    std::cout << std::endl;
}


int main(int argc, char *argv[])
{
    //
    // Build the default directory for the schema files, in case none supplied
    std::string processPath(queryCurrentProcessPath());
    configSchemaDefaultDir = processPath.substr(0, processPath.find_last_of(PATHSEPSTR)) + PATHSEPSTR + configSchemaRelativetDir;

    if (argc == 1)
    {
        usage();
        return 0;
    }

    //
    // Read options and validate
    if (!processOptions(argc, argv))
    {
        return 1;   // get out now
    }

    //
    // If no config dirs specified, build the default
    if (configDirs.empty())
    {
        configDirs.emplace_back(configSchemaDefaultDir);
    }

    //
    // Validate all the inputs
    if (!validate())
    {
        usage();
        return 1;
    }


    //
    // Create the modification template
    try
    {
        std::shared_ptr<Environment> pEnv;
        //
        // Create the environment for the template (if specified)
        if (!masterSchemaFile.empty())
        {
            pEnv = std::make_shared<Environment>(pEnvMgr);
            if (!inputEnvFile.empty())
            {
                pEnv->setLoadName(inputEnvFile);
            }
            else
            {
                pEnv->setInitializeEmpty(true);
            }

            if (!envOutputFile.empty())
            {
                pEnv->setOutputName(envOutputFile);
            }

            pEnv->m_masterCfgSchemaFile = masterSchemaFile;

            for (auto &path: configDirs)
            {
                pEnv->m_configPaths.emplace_back(path);
            }
        }

        pTemplate = new EnvModTemplate(pEnv, modTemplateSchemaFile, workingDir);
        pTemplate->loadTemplateFromFile(modTemplateFile);
    }
    catch (const TemplateException &te)
    {
        std::cout << "There was a problem loading the modification template: " << te.what() << std::endl;
        return 1;
    }


    //
    // If list inputs was given, list them and get out
    if (listInputs)
    {
        std::cout << "Template inputs:" << std::endl;
        auto templateInputs = pTemplate->getInputs();
        for (auto &pInput : templateInputs)
        {
            std::cout << "Name: " << pInput->getName() << std::endl <<  "  Type-" << pInput->getType() << std::endl << "  Description: " << pInput->getDescription() << std::endl;
        }
        return 0;  // get out
    }

    //
    // Process the input file if given here

    //
    // If the user provided any inputs, assign them
    if (!userInputs.empty())
    {
        for (auto &userInput: userInputs)
        {
            try
            {
                auto pInput = pTemplate->getVariable(userInput.inputName);
                auto values = splitString(userInput.inputValue, ",");
                for (auto &value: values)
                {
                    pInput->addValue(value);
                }
            }
            catch (const TemplateException &te)
            {
                std::cout << "There was a problem: " << te.what() << std::endl;
                return 0;  // get out
            }
        }
    }

    //
    // Execute
    try
    {
        pTemplate->execute(true);
    }
    catch (const TemplateExecutionException &te)
    {
        std::cout << std::endl << te.what() << std::endl;
        return 1;
    }

    //
    // Validate the results
    std::map<std::string, Status> envStatuses;
    pTemplate->validateEnvironments(envStatuses);
    bool errorsFound = false;

    //
    // This is where the error messages are printed out
//    bool errorHeader = false;
//    for (auto const &envStatus: envStatuses)
//    {
//        if (envStatus.second.isError())
//        {
//            errorsFound = true;
//            std::vector<statusMsg> msgs = envStatus.second.getMessages(statusMsg::error, false);
//            if (!errorHeader)
//            {
//                std::cerr << "Errors were detected in one or more environments generated" << std::endl;
//                errorHeader = true;
//            }
//            std::cerr << "Environment: " << envStatus.first << std::endl;
//            std::cerr << "  Errors:" << std::endl;
//            for (auto const &msg: msgs)
//            {
//                std::cerr << "    Message: " << msg.msg << "    Path: " << msg.path << std::endl;
//            }
//        }
//    }


    //
    // If no errors or forced output, save the environments.
    if (!errorsFound || forceOutput)
    {
        pTemplate->saveEnvironments();
    }


    std::cout << "Done" << std::endl;
    return 0;
}


bool processOptions(int argc, char *argv[])
{
    bool rc = true;
    int idx = 1;
    std::string optName, optVal;
    bool checkDir = false;

    try
    {
        while (idx < argc)
        {
            optName = getNextArg(argc, argv, idx++);

            if (optName == "-d" || optName == "--schema-dir")
            {
                std::string schemaDir = getNextArg(argc, argv, idx++);
                if (schemaDir.back() != PATHSEPCHAR)
                {
                    schemaDir.append(PATHSEPSTR);
                }
                configDirs.emplace_back(schemaDir);
            }

            else if (optName == "-m" || optName == "--master-config")
            {
                masterSchemaFile = getNextArg(argc, argv, idx++);
            }

            else if (optName == "-t" || optName == "--template")
            {
                modTemplateFile = getNextArg(argc, argv, idx++);
            }
            else if (optName == "-e" || optName == "--env")
            {
                inputEnvFile = getNextArg(argc, argv, idx++);
            }
            else if (optName == "-s" || optName == "--template-schema")
            {
                modTemplateSchemaFile = getNextArg(argc, argv, idx++);
            }
            else if (optName == "-o" || optName == "--output")
            {
                // this is the default if no filename given
                envOutputFile = inputEnvFile;
                if (idx < argc)
                {
                    envOutputFile = getNextArg(argc, argv, idx++);
                }
            }
            else if (optName == "-v" || optName == "--var")
            {
                std::string assign = getNextArg(argc, argv, idx++);
                std::size_t equalPos = assign.find_first_of('=');
                if (equalPos != std::string::npos)
                {
                    InputDef input;
                    input.inputName = assign.substr(0, equalPos);
                    input.inputValue = assign.substr(equalPos+1);
                    userInputs.emplace_back(input);
                }
                else
                {
                    throw CliException("Invalid input variable assignement: " + assign);
                }
            }
            else if (optName == "--list-inputs")
            {
                listInputs = true;
            }
            else if (optName == "--ignore-inputs")
            {
                forceOutput = true;
            }

            else if (optName == "-w" || optName == "--working-dir")
            {
                std::string dir = getNextArg(argc, argv, idx++);
                if (dir.back() != PATHSEPCHAR)
                {
                    dir.append(PATHSEPSTR);
                }
                workingDir = dir;
            }

        }
    }
    catch(const CliException &e)
    {
        std::cout << "There was an issue processing option: " << e.what() << std::endl;
        rc = false;
    }
    return rc;
}


bool validate()
{
    for (auto const &dir: configDirs)
    {
        if (!dirExists(dir))
        {
            std::cout << "Schema directory " << dir << " does not exist" << std::endl;
            return false;
        }
    }

    if (!fileExists(configDirs[0] + masterSchemaFile))
    {
        std::cout << "The master config file " << masterSchemaFile << " does not exist" << std::endl;
        return false;
    }

    if (!fileExists(modTemplateFile) && !fileExists(workingDir + modTemplateFile))
    {
        std::cout << "The modification template " << modTemplateFile << " does not exist" << std::endl;
        return false;
    }

    if (!fileExists(modTemplateSchemaFile))
    {
        std::cout << "The modification template schema file " << modTemplateSchemaFile << " does not exist" << std::endl;
        return false;
    }

    if (!inputEnvFile.empty())
    {
        if (!fileExists(inputEnvFile))
        {
            std::cout << "The environment file " << inputEnvFile << " does not exist" << std::endl;
            return false;
        }
    }
    return true;
}


std::string getNextArg(int argc, char *argv[], int idx)
{
    if (idx < argc)
    {
        return std::string(argv[idx]);
    }
    throw CliException("Arguments exhausted when more expected");
}


bool dirExists(const std::string &dirName)
{
    bool rc = true;
    struct stat info;
    if (stat(dirName.c_str(), &info) != 0)
    {
        rc = false;
    }
    rc = ((info.st_mode&S_IFMT)==S_IFDIR);
    return rc;
}


bool fileExists(const std::string &fileName)
{
    struct stat info;
    return stat(fileName.c_str(), &info) == 0;
}


std::vector<std::string> splitString(const std::string &input, const std::string &delim)
{
    size_t  start = 0, end = 0, delimLen = delim.length();
    std::vector<std::string> list;

    while (end != std::string::npos)
    {
        end = input.find(delim, start);
        std::string item = input.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
        if (!item.empty())
            list.push_back(item);
        start = ((end > (std::string::npos - delimLen)) ? std::string::npos : end + delimLen);
    }
    return list;
}
