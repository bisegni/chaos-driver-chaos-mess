/*
 *	DefaultCommand.h
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
#include "DefaultCommand.h"

#define CMDCU_ LAPP_ << "[DefaultCommand] - "


using namespace chaos;

using namespace chaos::common::data;

using namespace chaos::cu::control_manager::slow_command;
namespace chaos_batch = chaos::common::batch_command;

DefaultCommand::DefaultCommand():o_lct_delay(NULL) {
    setFeatures(chaos_batch::features::FeaturesFlagTypes::FF_SET_SCHEDULER_DELAY, (uint64_t)1000000);
}

DefaultCommand::~DefaultCommand() {
}

// return the implemented handler
uint8_t DefaultCommand::implementedHandler() {
    return  chaos_batch::HandlerType::HT_Set | chaos_batch::HandlerType::HT_Acquisition;
}

// Start the command execution
void DefaultCommand::setHandler(CDataWrapper *data) {
	o_lct_delay = getVariableValue(chaos_batch::IOCAttributeSharedCache::SVD_OUTPUT, (chaos_batch::VariableIndexType)0)->getCurrentValue<uint64_t>();
	*o_lct_delay = 0;
}

// Aquire the necessary data for the command
/*!
 The acquire handler has the purpose to get all necessary data need the by CC handler.
 \return the mask for the runnign state
 */
void DefaultCommand::acquireHandler() {
	CDataWrapper *acquiredData = getNewDataWrapper();
	acquiredData->addInt64Value("trx_delay", *o_lct_delay);
	pushDataSet(acquiredData);
}

