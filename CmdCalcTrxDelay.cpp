//
//  CmdCalcTrxDelay.cpp
//  chaos-mess
//
//  Created by Claudio Bisegni on 27/02/14.
//  Copyright (c) 2014 INFN. All rights reserved.
//

#include "CmdCalcTrxDelay.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace chaos;

using namespace chaos::common::data;

using namespace chaos::cu::control_manager::slow_command;
namespace chaos_batch = chaos::common::batch_command;


CmdCalcTrxDelay::CmdCalcTrxDelay() {
	
}

CmdCalcTrxDelay::~CmdCalcTrxDelay() {
	
}

// return the implemented handler
uint8_t CmdCalcTrxDelay::implementedHandler() {
	return  chaos_batch::HandlerType::HT_Set;
}

// Start the command execution
void CmdCalcTrxDelay::setHandler(CDataWrapper *data) {
	if(!data) return;
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration( time.time_of_day() );
	
	uint64_t *o_lct_delay = getVariableValue(chaos_batch::IOCAttributeSharedCache::SVD_OUTPUT, (chaos_batch::VariableIndexType)0)->getCurrentValue<uint64_t>();
	if(data->hasKey(CmdCalcTrxDelay_TS_PARAM_KEY)) {
		*o_lct_delay = duration.total_milliseconds() - data->getUInt64Value(CmdCalcTrxDelay_TS_PARAM_KEY);
	}
}
