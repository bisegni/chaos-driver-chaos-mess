/*	
 *	ControlUnitTest.cpp
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *	
 *    	Copyright 2012 INFN, National Institute of Nuclear Physics
 *
 *    	Licensed under the Apache License, Version 2.0 (the "License");
 *    	you may not use this file except in compliance with the License.
 *    	You may obtain a copy of the License at
 *
 *    	http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */


#include "ChaosMESS.h"
#include "DummyDriver.h"

#include <chaos/common/chaos_constants.h>
#include <chaos/cu_toolkit/ChaosCUToolkit.h>

#include <iostream>
#include <string>

/*! 
 
 */
using namespace std;
using namespace chaos;
using namespace chaos::cu;

namespace common_plugin = chaos::common::plugin;
namespace common_utility = chaos::common::utility;
namespace cu_driver_manager = chaos::cu::driver_manager;

#define OPT_CUSTOM_DEVICE_ID "device_id"


int main (int argc, char* argv[] ) {
    string tmp_device_id;
    try {
    ChaosCUToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_CUSTOM_DEVICE_ID, po::value<string>(), "device id for the");
	MATERIALIZE_INSTANCE_AND_INSPECTOR(DummyDriver)
	cu_driver_manager::DriverManager::getInstance()->registerDriver(DummyDriverInstancer, DummyDriverInspector);
		
    ChaosCUToolkit::getInstance()->init(argc, argv);
    if(ChaosCUToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_CUSTOM_DEVICE_ID)){
        tmp_device_id = ChaosCUToolkit::getInstance()->getGlobalConfigurationInstance()->getOption<string>(OPT_CUSTOM_DEVICE_ID);
        ChaosCUToolkit::getInstance()->addControlUnit(new MESS(tmp_device_id));
    }
    ChaosCUToolkit::getInstance()->start();
    } catch (CException& e) {
        cerr<<"Exception::"<<endl;
        std::cerr<< "in:"<<e.errorDomain << std::endl;
        std::cerr<< "cause:"<<e.errorMessage << std::endl;
    } catch (program_options::error &e){
        cerr << "Unable to parse command line: " << e.what() << endl;
    } catch (...){
        cerr << "unexpected exception caught.. " << endl;
    }

    return 0;
}
