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

DefaultCommand::DefaultCommand() {
}

DefaultCommand::~DefaultCommand() {
}

// return the implemented handler
uint8_t DefaultCommand::implementedHandler() {
    return  chaos_batch::HandlerType::HT_Set | chaos_batch::HandlerType::HT_Acquisition;
}

// Start the command execution
void DefaultCommand::setHandler(CDataWrapper *data) {
}

// Aquire the necessary data for the command
/*!
 The acquire handler has the purpose to get all necessary data need the by CC handler.
 \return the mask for the runnign state
 */
void DefaultCommand::acquireHandler() {
}

