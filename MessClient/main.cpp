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
#include <fstream>
#include <functional>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace chaos;
using namespace chaos::ui;
using namespace bson;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::date_time;
namespace chaos_batch = chaos::common::batch_command;

#define OPT_MESS_DID                        "mess_device_id"
#define OPT_MESS_PHASES_START               "start"
#define OPT_MESS_PHASES_STOP                "stop"
#define OPT_MESS_PHASES_INIT                "init"
#define OPT_MESS_PHASES_DEINIT              "deinit"

#define OPT_PERFORM_TEST                    "report"

#define OPT_MAKE_TRX_TEST                   "trx_delay_test"
#define OPT_MAKE_ROUND_TRIP_TEST            "round_trip_test"
#define OPT_MAKE_ROUND_TRIP_TEST_ITERATION  "round_trip_test_iteration"
#define OPT_GET_TRX_TEST                    "get_trx_delay"
#define OPT_TIMEOUT                         "timeout"
#define OPT_SCHED_DELAY                     "scheduler_delay"

class perform_test {
    
protected:
    boost::shared_ptr<ofstream> fs;
    string test_name;
    string fradix_name;
    DeviceController* controller;
    string filename;
public:
   
    perform_test(string _test_name, string _fradix_name, DeviceController*ctrl) : test_name(_test_name), fradix_name(_fradix_name), controller(ctrl) {
        filename = _fradix_name + "_" + _test_name + ".csv";
        fs.reset(new ofstream(filename.c_str(), ios::out));
        cout << "opening "<<filename<<" for write"<<endl;
        int err;
        if ( fs == NULL || !fs->good()) {
            throw CException(1, string(__FUNCTION__), "## cannot open " + filename);
        }
        
        err = controller->initDevice();
        if (err == ErrorCode::EC_TIMEOUT) throw CException(6,string(__FUNCTION__), "Set device to init state");
           
        err = controller->startDevice();
        if (err == ErrorCode::EC_TIMEOUT) throw CException(2, string(__FUNCTION__), "Set device to start state");
    }

    ~perform_test() {
        int err;
        fs->close();
        fs.reset(); 
        err = controller->stopDevice();
        if (err == ErrorCode::EC_TIMEOUT) throw CException(2,  string(__FUNCTION__), "Set device to stop state");
        
        err = controller->deinitDevice();
        if (err == ErrorCode::EC_TIMEOUT) throw CException(2,  string(__FUNCTION__), "Set device to deinit state");
         std::cout << "Wrote  " << filename<<endl;
         
    }

    int operator()(int i) {
        return test(i);
    };

    virtual int test(int i) = 0;
};

class perform_rt_test : public perform_test {
public:
    perform_rt_test(string _fradix_name, DeviceController*ctrl) : perform_test(string("rt_test"), _fradix_name, ctrl) {
        *fs << "Iterations,max rt(us), min rt(us), mean rt(us),max set(us), min set(us), mean set(us)"<<endl;
    }

    int test(int iterations) {

        int err;
        uint64_t got_ts = 0, got_delay = 0, cur_ts = 0, command_id = 0, calc_rt_delay = 0, got_max_rt_delay, got_min_rt_delay, got_mean_rt_delay, got_max_set_delay, got_min_set_delay, got_mean_set_delay;
        CDataWrapper *wrapped_data = NULL;
         std::cout << "Performing  " << test_name<< "Iterations :"<<iterations<<endl;
        //set scehdule delay to 500 micro seconds
        controller->setScheduleDelay(500);
        for (int idx = 0; idx < iterations; idx++) {
            CDataWrapper test_delay_param_data;
            boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
            boost::posix_time::time_duration duration(time.time_of_day());

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
            if (err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Test transmission delay");

            do {
                controller->fetchCurrentDeviceValue();
                wrapped_data = controller->getCurrentData();
                if (wrapped_data) {
                    got_ts = wrapped_data->getUInt64Value("trx_ts");
                    got_delay = wrapped_data->getUInt64Value("trx_delay");
                }
            } while (got_ts < cur_ts);

            if (got_ts == cur_ts) {
                boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
                boost::posix_time::time_duration duration(time.time_of_day());
                calc_rt_delay = duration.total_microseconds() - cur_ts;
                if (!idx) {
                    got_max_rt_delay = calc_rt_delay;
                    got_min_rt_delay = calc_rt_delay;
                    got_mean_set_delay = got_delay;
                    got_mean_rt_delay = calc_rt_delay;

                    got_max_set_delay = got_delay;
                    got_min_set_delay = got_delay;
                } else {
                    got_max_rt_delay = std::max(got_max_rt_delay, calc_rt_delay);
                    got_min_rt_delay = std::min(got_min_rt_delay, calc_rt_delay);
                    got_mean_set_delay += got_delay;
                    got_mean_rt_delay += calc_rt_delay;

                    got_max_set_delay = std::max(got_max_set_delay, got_delay);
                    got_min_set_delay = std::min(got_min_set_delay, got_delay);
                }


            }
        }
        *fs << iterations << "," << got_min_rt_delay << "," << got_max_rt_delay << "," << got_mean_rt_delay/iterations << "," << got_min_set_delay << "," << got_max_set_delay << "," << got_mean_set_delay/iterations << endl;

        return 0;
    }

};


    int main(int argc, char* argv[]) {
        try {
            int err = 0;
            uint32_t timeout = 0;
            uint32_t iteration = 0;
            uint64_t command_id = 0;
            string mess_device_id;
            uint64_t schedule_delay = 0;
            string report_name;
            CDataWrapper *wrapped_data = NULL;
            typedef std::vector<int>::iterator OpcodeSequenceIterator;
            std::vector<int> opcodes_sequence;
            CDeviceNetworkAddress deviceNetworkAddress;

            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MESS_DID, po::value<string>(&mess_device_id), "The host of the mess monitor");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MESS_PHASES_INIT, "Initialize the monitor");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MESS_PHASES_START, "Start the monitor");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MESS_PHASES_STOP, "Stop the monitor");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_PERFORM_TEST, po::value<string>(&report_name)->default_value(""), "Perform full test and generate CSV <report> file");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MESS_PHASES_DEINIT, "Deinitilize the monitor");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MAKE_TRX_TEST, "Execute the test of the transmission delay");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MAKE_ROUND_TRIP_TEST, "Execute the round trip delay test");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_MAKE_ROUND_TRIP_TEST_ITERATION, po::value<uint32_t>(&iteration)->default_value(1), "Number of iteration of the roundtrip test");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_GET_TRX_TEST, "Execute the test of the transmission delay");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_TIMEOUT, po::value<uint32_t>(&timeout)->default_value(2000), "Timeout rpc in milliseconds");
            ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_SCHED_DELAY, po::value(&schedule_delay), "Scheduler delay");


            ChaosUIToolkit::getInstance()->init(argc, argv);

            if (!ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_DID)) throw CException(1, "invalid device identification string", "check param");
            if (mess_device_id.size() == 0) throw CException(1, "invalid device identification string", "check param");

            if (ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_PHASES_INIT)) {
                opcodes_sequence.push_back(1);
            }
            if (ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_PHASES_START)) {
                opcodes_sequence.push_back(2);
            }
            if (ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_SCHED_DELAY)) {
                opcodes_sequence.push_back(5);
            }
            if (ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MAKE_TRX_TEST)) {
                opcodes_sequence.push_back(6);
            }

            if (ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_GET_TRX_TEST)) {
                opcodes_sequence.push_back(7);
            }

            if (ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MAKE_ROUND_TRIP_TEST)) {
                opcodes_sequence.push_back(8);
            }

            if (ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_PHASES_STOP)) {
                opcodes_sequence.push_back(3);
            }
            if (ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_MESS_PHASES_DEINIT)) {
                opcodes_sequence.push_back(4);
            }
            if (ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_SCHED_DELAY)) {
                opcodes_sequence.push_back(5);
            }


            DeviceController *controller = HLDataApi::getInstance()->getControllerForDeviceID(mess_device_id, timeout);
            if (!controller) throw CException(4, "Error allcoating decive controller", "device controller creation");

            if (report_name.length()) {
                std::vector<int> repeat_test;
                std::vector<int>::iterator i;
                perform_rt_test rt_test(report_name,controller);
                repeat_test.push_back(5);
                repeat_test.push_back(10);
                repeat_test.push_back(100);
                repeat_test.push_back(1000);
                for_each<std::vector<int>::iterator,perform_rt_test&>(repeat_test.begin(),repeat_test.end(),rt_test);
                //for_each(repeat_test.begin(),repeat_test.end(),rt_test);
                return 0;
            }
            for (OpcodeSequenceIterator iter = opcodes_sequence.begin();
                    iter != opcodes_sequence.end();
                    iter++) {
                switch (*iter) {
                    case 1:
                        LAPP_ << "Initialize " << mess_device_id;
                        err = controller->initDevice();
                        if (err == ErrorCode::EC_TIMEOUT) throw CException(6, "Time out on connection", "Set device to init state");
                        break;
                    case 2:
                        LAPP_ << "Start " << mess_device_id;
                        err = controller->startDevice();
                        if (err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Set device to start state");
                        break;
                    case 3:
                        LAPP_ << "Stop " << mess_device_id;
                        err = controller->stopDevice();
                        if (err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Set device to stop state");
                        break;
                    case 4:
                        LAPP_ << "Deinitialize " << mess_device_id;
                        err = controller->deinitDevice();
                        if (err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Set device to deinit state");
                        break;
                    case 5:
                        LAPP_ << "Set schedule delay to " << schedule_delay;
                        err = controller->setScheduleDelay(schedule_delay);
                        if (err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Set device to deinit state");
                        break;
                    case 6:
                    {//send delay test
                        LAPP_ << "Execute transmission delay test to " << mess_device_id;
                        CDataWrapper test_delay_param_data;
                        boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
                        boost::posix_time::time_duration duration(time.time_of_day());

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
                        if (err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Test transmission delay");
                        //std::cout << "Command submitted successfully his command idedentification number(cidn) is= " << command_id << std::endl;
                    }
                    case 7:
                    {//read last command trx delay
                        //getLastTrxDelay
                        //trx_delay
                        LAPP_ << "Request transmission delay result to " << mess_device_id;
                        CDataWrapper *test_delay_result;
                        err = controller->sendCustomRequest("getLastTrxDelay", NULL, &test_delay_result);
                        if (err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Read last transmisison delay");
                        if (test_delay_result) {
                            uint64_t delay = test_delay_result->getUInt64Value("trx_delay");
                            std::cout << "Last transmission delay = " << delay << std::endl;
                        }
                        break;
                    }

                    case 8:
                    {
                        //send
                        uint64_t cur_ts;
                        uint64_t got_ts;
                        uint64_t got_delay;
                        uint64_t calc_rt_delay;
                        uint64_t got_min_set_delay;
                        uint64_t got_max_set_delay;
                        uint64_t got_min_rt_delay;
                        uint64_t got_max_rt_delay;
                        std::cout << "start roundtrip test.." << mess_device_id << std::endl;
                        for (int idx = 0; idx < iteration; idx++) {
                            CDataWrapper test_delay_param_data;
                            boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
                            boost::posix_time::time_duration duration(time.time_of_day());

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
                            if (err == ErrorCode::EC_TIMEOUT) throw CException(2, "Time out on connection", "Test transmission delay");
                            //set scehdule delay to 500 micro seconds
                            controller->setScheduleDelay(500);
                            do {
                                controller->fetchCurrentDeviceValue();
                                wrapped_data = controller->getCurrentData();
                                if (wrapped_data) {
                                    got_ts = wrapped_data->getUInt64Value("trx_ts");
                                    got_delay = wrapped_data->getUInt64Value("trx_delay");
                                }
                            } while (got_ts < cur_ts);

                            if (got_ts == cur_ts) {
                                boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
                                boost::posix_time::time_duration duration(time.time_of_day());
                                calc_rt_delay = duration.total_microseconds() - cur_ts;
                                if (!idx) {
                                    got_max_rt_delay = calc_rt_delay;
                                    got_min_rt_delay = calc_rt_delay;

                                    got_max_set_delay = got_delay;
                                    got_min_set_delay = got_delay;
                                } else {
                                    got_max_rt_delay = std::max(got_max_rt_delay, calc_rt_delay);
                                    got_min_rt_delay = std::min(got_min_rt_delay, calc_rt_delay);

                                    got_max_set_delay = std::max(got_max_set_delay, got_delay);
                                    got_min_set_delay = std::min(got_min_set_delay, got_delay);
                                }


                            }
                        }
                        std::cout << "Round trip test result for " << iteration << "Iteration" << std::endl;
                        std::cout << "Minimum rt delay->" << got_min_rt_delay << std::endl;
                        std::cout << "Maximum rt delay->" << got_max_rt_delay << std::endl;
                        std::cout << "Minimum set delay->" << got_min_set_delay << std::endl;
                        std::cout << "Maximum set delay->" << got_max_set_delay << std::endl;
                        break;
                    }
                }
            }

        } catch (CException& e) {
            std::cerr << e.errorCode << " - " << e.errorDomain << " - " << e.errorMessage << std::endl;
        }
        try {
            ChaosUIToolkit::getInstance()->deinit();
        } catch (CException& e) {
            std::cerr << e.errorCode << " - " << e.errorDomain << " - " << e.errorMessage << std::endl;
        }
        return 0;
    }

