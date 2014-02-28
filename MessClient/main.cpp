/*
 *	UIToolkitCMDLineExample.cpp
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
#include <iostream>
#include <string>
#include <vector>
#include <chaos/common/global.h>
#include <chaos/common/chaos_constants.h>
#include <chaos/common/network/CNodeNetworkAddress.h>
#include <chaos/ui_toolkit/ChaosUIToolkit.h>
#include <chaos/ui_toolkit/LowLevelApi/LLRpcApi.h>
#include <chaos/ui_toolkit/HighLevelApi/HLDataApi.h>
#include <stdio.h>
#include <chaos/common/bson/bson.h>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

using namespace std;
using namespace chaos;
using namespace chaos::ui;
using namespace bson;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::date_time;
namespace chaos_batch = chaos::common::batch_command;

#define OPT_MESS_DID				"mess_device_id"
#define OPT_MESS_PHASES_START		"start"
#define OPT_MESS_PHASES_STOP		"stop"
#define OPT_MESS_PHASES_INIT		"init"
#define OPT_MESS_PHASES_DEINIT		"deinit"
#define OPT_MAKE_TRX_TEST			"trx_delay_test"
#define OPT_MAKE_ROUND_TRIP_TEST	"round_trip_test"
#define OPT_GET_TRX_TEST			"get_trx_delay"
#define OPT_TIMEOUT					"timeout"

int main (int argc, char* argv[] ) {
	try {
		int err = 0;
		uint32_t timeout = 0;
		uint64_t command_id = 0;
		string mess_device_id;
		uint64_t schedule_delay = 0;
		CDataWrapper *wrapped_data = NULL;
		typedef std::vector<int>::iterator OpcodeSequenceIterator;
		std::vector<int> opcodes_sequence;
		CDeviceNetworkAddress deviceNetworkAddress;
		
		ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption<string>(OPT_MESS_DID, "The host of the mess monitor", &mess_device_id);
		ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MESS_PHASES_INIT, "Initialize the monitor");
		ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MESS_PHASES_START, "Start the monitor");
		ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MESS_PHASES_STOP, "Stop the monitor");
		ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MESS_PHASES_DEINIT, "Deinitilize the monitor");
		ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MAKE_TRX_TEST, "Execute the test of the transmission delay");
		ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MAKE_ROUND_TRIP_TEST, "Execute the round trip delay test");
		ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_GET_TRX_TEST, "Execute the test of the transmission delay");
		ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption<uint32_t>(OPT_TIMEOUT, "Timeout rpc in milliseconds", 2000, &timeout);
		
		
		
		ChaosUIToolkit::getInstance()->init(argc, argv);
		
		if(!ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_DID)) throw CException(1, "invalid device identification string", "check param");
		if(mess_device_id.size()==0) throw CException(1, "invalid device identification string", "check param");
		
		if(ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_PHASES_INIT)){
			opcodes_sequence.push_back(1);
		}
		if(ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_PHASES_START)){
			opcodes_sequence.push_back(2);
		}
		
		if(ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MAKE_TRX_TEST)){
			opcodes_sequence.push_back(6);
		}
		
		if(ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_GET_TRX_TEST)){
			opcodes_sequence.push_back(7);
		}
		
		if(ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MAKE_ROUND_TRIP_TEST)){
			opcodes_sequence.push_back(8);
		}
		
		if(ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_PHASES_STOP)){
			opcodes_sequence.push_back(3);
		}
		if(ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_PHASES_DEINIT)){
			opcodes_sequence.push_back(4);
		}
		
		DeviceController *controller = HLDataApi::getInstance()->getControllerForDeviceID(mess_device_id, timeout);
		if(!controller) throw CException(4, "Error allcoating decive controller", "device controller creation");
		
		for(OpcodeSequenceIterator iter = opcodes_sequence.begin();
			iter != opcodes_sequence.end();
			iter++) {
			switch (*iter) {
				case 1:
					LAPP_ << "Initialize " << mess_device_id;
					err = controller->initDevice();
					if(err == ErrorCode::EC_TIMEOUT) throw CException(6, "Time out on connection", "Set device to init state");
					break;
				case 2:
					LAPP_ << "Start " << mess_device_id;
					err = controller->startDevice();
					if(err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Set device to start state");
					break;
				case 3:
					LAPP_ << "Stop " << mess_device_id;
					err = controller->stopDevice();
					if(err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Set device to stop state");
					break;
				case 4:
					LAPP_ << "Deinitialize " << mess_device_id;
					err = controller->deinitDevice();
					if(err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Set device to deinit state");
					break;
				case 5:
					LAPP_ << "Set schedule delay to " << schedule_delay;
					err = controller->setScheduleDelay(schedule_delay);
					if(err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Set device to deinit state");
					break;
				case 6: {//send delay test
					LAPP_ << "Execute transmission delay test to " << mess_device_id;
					CDataWrapper test_delay_param_data;
					boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
					boost::posix_time::time_duration duration( time.time_of_day() );
					
					test_delay_param_data.addInt64Value("sts", duration.total_microseconds());
					err = controller->submitSlowControlCommand("trx_delay",
															   chaos_batch::SubmissionRuleType::SUBMIT_AND_Stack,
															   100,
															   command_id,
															   0,
															   0,
															   0,
															   &test_delay_param_data);
					
					//read answer
					if(err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Test transmission delay");
					//std::cout << "Command submitted successfully his command idedentification number(cidn) is= " << command_id << std::endl;
				}
				case 7: {//read last command trx delay
					//getLastTrxDelay
					//trx_delay
					LAPP_ << "Request transmission delay result to " << mess_device_id;
					CDataWrapper *test_delay_result;
					err = controller->sendCustomRequest("getLastTrxDelay", NULL, &test_delay_result);
					if(err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Read last transmisison delay");
					if(test_delay_result) {
						uint64_t delay = test_delay_result->getUInt64Value("trx_delay");
                        std::cout << "Last transmission delay = " << delay << std::endl;
					}
					break;
				}
					
				case 8: {
					//send
					uint64_t cur_ts;
					uint64_t got_ts;
					std::cout << "start roundtrip test.." << mess_device_id << std::endl;
					CDataWrapper test_delay_param_data;
					boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
					boost::posix_time::time_duration duration( time.time_of_day() );
					
					test_delay_param_data.addInt64Value("sts", cur_ts = duration.total_microseconds());
					err = controller->submitSlowControlCommand("trx_delay",
															   chaos_batch::SubmissionRuleType::SUBMIT_AND_Stack,
															   100,
															   command_id,
															   0,
															   0,
															   0,
															   &test_delay_param_data);
					
					//read answer
					if(err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Test transmission delay");
					std::cout << "Fetch data from data proxy" << std::endl;
					do{
						controller->fetchCurrentDeviceValue();
						wrapped_data = controller->getCurrentData();
						if(wrapped_data)
							got_ts = wrapped_data->getUInt64Value("trx_ts");
					}while (got_ts < cur_ts);
					if(cur_ts == cur_ts) {
						boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
						boost::posix_time::time_duration duration( time.time_of_day() );
						std::cout << "Round trip delay is" << std::endl;
					} else {
						std::cout << "Data is not from my test" << duration.total_microseconds()-cur_ts << std::endl;
					}
					break;
				}
			}
		}
		
	} catch (CException& e) {
		std::cerr << e.errorCode << " - "<< e.errorDomain << " - " << e.errorMessage << std::endl;
	}
	try {
		ChaosUIToolkit::getInstance()->deinit();
	} catch (CException& e) {
		std::cerr << e.errorCode << " - "<< e.errorDomain << " - " << e.errorMessage << std::endl;
	}
	return 0;
}

