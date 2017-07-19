
#include <stdlib.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <exception>
#include <iostream>
namespace pt = boost::property_tree;

int main()
{
    pt::ptree xsdTree;

    try
    {
        pt::read_xml("dafilesrv.xsd", xsdTree);

        //
        // Let's try iterating
        // for (pt::ptree::const_iterator it=xsdTree.begin(); it!=xsdTree.end(); ++it)
        // {
        //     std::cout << "here: " << it->first << std::endl;
        // }

        auto it = xsdTree.find("xs:schema");
        bool isEnd = (it == xsdTree.not_found());
        std::cout << "here " << it->first << "  " << it->second.data() << std::endl;


        std::cout << "Success\n";

        // std::cout << std::endl << "Using Boost "     
        //   << BOOST_VERSION / 100000     << "."  // major version
        //   << BOOST_VERSION / 100 % 1000 << "."  // minor version
        //   << BOOST_VERSION % 100                // patch level
        //   << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << "Error: " << e.what() << "\n";
    }
    return 0;
}