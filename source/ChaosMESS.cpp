//
//  ChaosMESS.cpp
//  ControlUnitTest
//
//  Created by Claudio Bisegni on 7/20/13.
//  Copyright (c) 2013 INFN. All rights reserved.
//

#include "ChaosMESS.h"
#include "DefaultCommand.h"
#include "CmdCalcTrxDelay.h"
#include <chaos/common/configuration/GlobalConfiguration.h>

#include <boost/format.hpp>

using namespace chaos::common::data;

using namespace chaos::cu::control_manager::slow_command;
using namespace chaos::cu::driver_manager::driver;


/*
 Construct a new CU with an identifier
 */
ChaosMESS::ChaosMESS() {
    _deviceID = "to find";
}

ChaosMESS::~ChaosMESS() {
	
}

/*
 Return the default configuration
 */
void ChaosMESS::unitDefineActionAndDataset() throw(CException) {
    //create the mess virtual device identifier
	_deviceID = boost::str( boost::format("%1%_mess_monitor") % GlobalConfiguration::getInstance()->getLocalServerAddress());
	
    //add managed device di
    setDeviceID(_deviceID);
    
    //install a command
    installCommand<DefaultCommand>("DefaultCommand");
	installCommand<CmdCalcTrxDelay>(CmdCalcTrxDelay_CMD_ALIAS);
	
	addAttributeToDataSet("trx_delay",
                          "Last command transmission delay in microseconds",
                          DataType::TYPE_INT32,
                          DataType::Output);
	
	setDefaultCommand("DefaultCommand");
}

void ChaosMESS::defineSharedVariable() {
	//uint32_t quit = false;
	//here are defined the custom shared variable
    //addCustomSharedVariable("quit", 1, chaos::DataType::TYPE_BOOLEAN);
    //setVariableValue(chaos_batch::IOCAttributeSharedCache::SVD_CUSTOM, "quit", &quit, sizeof(bool));
}

void ChaosMESS::unitDefineDriver(std::vector<cu_driver::DrvRequestInfo>& neededDriver) {
	cu_driver::DrvRequestInfo drv1 = {"DummyDriver","1.0.0","url_host:port"};
	neededDriver.push_back(drv1);
}

// Abstract method for the initialization of the control unit
void ChaosMESS::unitInit() throw(CException) {
	
}

// Abstract method for the start of the control unit
void ChaosMESS::unitStart() throw(CException) {
	
}

// Abstract method for the stop of the control unit
void ChaosMESS::unitStop() throw(CException) {
	
}

// Abstract method for the deinit of the control unit
void ChaosMESS::unitDeinit() throw(CException) {
	
}
