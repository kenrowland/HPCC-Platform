################################################################################
#    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems®.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
################################################################################


# Component: dfuXRefLib 

#####################################################
# Description:
# ------------
#    Cmake Input File for dfuXRefLib
#####################################################


project( dfuXRefLib ) 

set (    SRCS 
         dfurdir.cpp 
         dfuxreflib.cpp 
         XRefFilesNode.cpp 
         XRefNodeManager.cpp 
    )

include_directories ( 
         ./../../esp/bindings/SOAP/xpp 
         ./../../fs/dafsclient
         ./../server 
         ./../../system/mp 
         . 
         ./../base 
         ./../ft 
         ./../../system/include 
         ./../../system/jlib 
         ./../../common/environment 
         ./../../common/workunit
         ./../../system/security/shared
    )

ADD_DEFINITIONS( -D_USRDLL -DDFUXREFLIB_EXPORTS )

HPCC_ADD_LIBRARY( dfuXRefLib SHARED ${SRCS} )
install ( TARGETS dfuXRefLib RUNTIME DESTINATION ${EXEC_DIR} LIBRARY DESTINATION ${LIB_DIR} )
target_link_libraries ( dfuXRefLib
         jlib
         mp 
         hrpc 
         dafsclient 
         dalibase 
    )

if (NOT CONTAINERIZED)
    target_link_libraries ( dfuXRefLib environment )
endif()
