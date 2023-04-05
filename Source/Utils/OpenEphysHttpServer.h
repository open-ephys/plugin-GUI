/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __PROCESSORGRAPHHTTPSERVER_H_124F8B50__
#define __PROCESSORGRAPHHTTPSERVER_H_124F8B50__

#include "../Processors/Parameter/Parameter.h"
#include "../Processors/GenericProcessor/GenericProcessor.h"

#include <sstream>
#include "httplib.h"
#include "json.hpp"

#include "../MainWindow.h"
#include "../AccessClass.h"
#include "../UI/ProcessorList.h"
#include "../UI/EditorViewport.h"

#include "Utils.h"

using json = nlohmann::json;

#define PORT 37497


/**
 * HTTP server thread for controlling Processor Parameters via an HTTP API. This starts an HTTP server on port 37497
 * (== "EPHYS" on a phone keypad) and allows remote manipulation of parameters of processors currently in the graph.
 *
 * The API is "RESTful", such that the resource URLs are:
 * 
 * - GET /api/status : 
 *          returns a JSON string with the GUI's current mode (IDLE, ACQUIRE, RECORD)
 * 
 * - PUT /api/status : 
 *          sets the GUI's mode, e.g.: {"mode" : "ACQUIRE"}
 * 
 * - PUT /api/message :
 *          sends a broadcast message to all processors, e.g.: {"text" : "Message content"}
 *          only works while acquisition is active
 * 
 * - GET /api/recording :
 *          returns a JSON string with the following information:
 *          - default recording directory ("directory")
 *          - default data format ("format")
 *          - default directory name prepend + append text
 *          - default directory name string
 *          - available Record Nodes
 *          - directory, format, experiment #, and recording #, for each Record Node
 * 
 * - PUT /api/recording :
 *          used to set the default recording options
 * 
 * - PUT /api/recording/<processor_id> :
 *          used to set the options for a given Record Node
 * 
 * - GET /api/processors :
 * - GET /api/processors/<processor_id>
 * - GET /api/processors/<processor_id>/parameters
 * - GET /api/processors/<processor_id>/parameters/<parameter_name>
 * - PUT /api/processors/<processor_id>/parameters/<parameter_name>
 * - GET /api/processors/<processor_id>/streams/<stream_index>
 * - GET /api/processors/<processor_id>/streams/<stream_index>/parameters
 * - GET /api/processors/<processor_id>/streams/<stream_index>/parameters/<parameter_name>
 * - PUT /api/processors/<processor_id>/streams/<stream_index>/parameters/<parameter_name>
 * - PUT /api/processors/<processor_id>/config
 * - PUT /api/processors/add
 * - PUT /api/processors/delete
 * - PUT /api/window
 *
 * All endpoints are JSON endpoints. The PUT endpoint expects two parameters: "channel" (an integer), and "value",
 * which should have a type matching the type of the parameter.
 */
class OpenEphysHttpServer : juce::Thread {
public:

    explicit OpenEphysHttpServer(ProcessorGraph* graph) :
        graph_(graph),
        juce::Thread("HttpServer") {}

    void run() override {
        
        svr_->Get("/api/status", [this](const httplib::Request&, httplib::Response& res) {
            json ret;
            status_to_json(graph_, &ret);
            res.set_content(ret.dump(), "application/json");
            });
        
        svr_->Put("/api/status", [this](const httplib::Request& req, httplib::Response& res) 
        {
            std::string desired_mode;

            LOGD("Received PUT request with content: ", req.body);
            try {
                LOGD("Trying to decode request");
                json request_json;
                request_json = json::parse(req.body);
                LOGD("Successfully parsed body");
                desired_mode = request_json["mode"];
                LOGD("Found 'mode': ", desired_mode);
            }
            catch (json::exception& e) {
                LOGD("Hit exception: ", String(e.what()));
                res.set_content(e.what(), "text/plain");
                res.status = 400;
                return;
            }

            if (desired_mode == "RECORD" && !CoreServices::getRecordingStatus()) {
                const MessageManagerLock mmLock;
                CoreServices::setRecordingStatus(true);
            }
            else if (desired_mode == "ACQUIRE") {
                const MessageManagerLock mmLock;
                CoreServices::setRecordingStatus(false);
                CoreServices::setAcquisitionStatus(true);
            }
            else if (desired_mode == "IDLE") {
                const MessageManagerLock mmLock;
                CoreServices::setAcquisitionStatus(false);
            }

            json ret;
            status_to_json(graph_, &ret);
            res.set_content(ret.dump(), "application/json");
            });

        svr_->Get("/api/recording", [this](const httplib::Request&, httplib::Response& res) {
            json ret;
            recording_info_to_json(graph_, &ret);
            res.set_content(ret.dump(), "application/json");
            });

        svr_->Put("/api/recording", [this](const httplib::Request& req, httplib::Response& res)
            {
                
                json request_json;

                LOGD("Received PUT request at /api/recording with content: ", req.body);

                try 
                {
                    LOGD("Trying to decode request");
                    request_json = json::parse(req.body);
                    LOGD("Successfully parsed body");
                }
                catch (json::exception& e) 
                {
                    LOGD("Could not parse input.");
                    res.set_content(e.what(), "text/plain");
                    res.status = 400;
                    return;
                }
               
                try {
                    std::string parent_directory = request_json["parent_directory"];
                    LOGD("Found 'parent_directory': ", parent_directory);
                    const MessageManagerLock mml;
                    CoreServices::setRecordingParentDirectory(String(parent_directory));
                }
                catch (json::exception& e) {
                    LOGD("'parent_directory' not specified'");
                }

                try {
                    std::string prepend_text = request_json["prepend_text"];
                    LOGD("Found 'prepend_text': ", prepend_text);
                    const MessageManagerLock mml;
                    CoreServices::setRecordingDirectoryPrependText(String(prepend_text));
                }
                catch (json::exception& e) {
                    LOGD("'prepend_text' not specified'");
                }

                try {
                    std::string base_text = request_json["base_text"];
                    LOGD("Found 'base_text': ", base_text);
                    const MessageManagerLock mml;
                    CoreServices::setRecordingDirectoryBaseText(String(base_text));
                }
                catch (json::exception& e) {
                    LOGD("'base_text' not specified'");
                }

                try {
                    std::string append_text = request_json["append_text"];
                    LOGD("Found 'append_text': ", append_text);
                    const MessageManagerLock mml;
                    CoreServices::setRecordingDirectoryAppendText(String(append_text));
                }
                catch (json::exception& e) {
                    LOGD("'append_text' not specified'");
                }

                try {
                    std::string default_record_engine = request_json["default_record_engine"];
                    LOGD("Found 'default_record_engine': ", default_record_engine);
                    const MessageManagerLock mml;
                    CoreServices::setDefaultRecordEngine(String(default_record_engine));
                }
                catch (json::exception& e) {
                    LOGD("'default_record_engine' not specified'");
                }

                try {
                    std::string start_new_directory = request_json["start_new_directory"];
                    LOGD("Found 'start_new_directory': ", start_new_directory);
                    const MessageManagerLock mml;
                    if (start_new_directory == "true")
                        CoreServices::createNewRecordingDirectory();
                }
                catch (json::exception& e) {
                    LOGD("'start_new_directory' not specified'");
                }

                json ret;
                recording_info_to_json(graph_, &ret);
                res.set_content(ret.dump(), "application/json");
            });

        svr_->Put("/api/recording/([0-9]+)", [this](const httplib::Request& req, httplib::Response& res)
            {

                json request_json;

                int id = juce::String(req.matches[1]).getIntValue();

                LOGD("Received PUT request at /api/recording/", id, ":", req.body);

                try
                {
                    LOGD("Trying to decode request");
                    request_json = json::parse(req.body);
                    LOGD("Successfully parsed body");
                }
                catch (json::exception& e)
                {
                    LOGD("Could not parse input.");
                    res.set_content(e.what(), "text/plain");
                    res.status = 400;
                    return;
                }

                try {
                    std::string parent_directory = request_json["parent_directory"];
                    LOGD("Found 'parent_directory': ", parent_directory);
                    const MessageManagerLock mml;
                    CoreServices::RecordNode::setRecordingDirectory(String(parent_directory), id);
                }
                catch (json::exception& e) {
                    LOGD("'parent_directory' not specified'");
                }

                try {
                    std::string record_engine = request_json["record_engine"];
                    LOGD("Found 'record_engine': ", record_engine);
                    const MessageManagerLock mml;
                    CoreServices::RecordNode::setRecordEngine(String(record_engine), id);
                }
                catch (json::exception& e) {
                    LOGD("'record_engine' not specified'");
                }
                
                json ret;
                recording_info_to_json(graph_, &ret);
                res.set_content(ret.dump(), "application/json");
            });
        
        svr_->Put("/api/message", [this](const httplib::Request& req, httplib::Response& res) {
            std::string message_str;
            LOGD("Received PUT request");
            try {
                LOGD( "Trying to decode" );
                json request_json;
                request_json = json::parse(req.body);
                LOGD( "Parsed" );
                message_str = request_json["text"];
                LOGD( "Message string: ", message_str );
            }
            catch (json::exception& e) {
                LOGD( "Hit exception" );
                res.set_content(e.what(), "text/plain");
                res.status = 400;
                return;
            }

            graph_->broadcastMessage(String(message_str));

            json ret;
            status_to_json(graph_, &ret);
            res.set_content(ret.dump(), "application/json");
            });

        svr_->Put("/api/load", [this](const httplib::Request& req, httplib::Response& res) {
            
            std::string message_str;
            LOGD("Received PUT request");

            try {
                LOGD("Trying to decode");
                json request_json;
                request_json = json::parse(req.body);
                LOGD("Parsed");
                message_str = request_json["path"];
                LOGD("Message string: ", message_str);
            }
            catch (json::exception& e) {
                LOGD("Hit exception");
                res.set_content(e.what(), "text/plain");
                res.status = 400;
                return;
            }

            const MessageManagerLock mml;
            CoreServices::loadSignalChain(message_str);

            json ret;
            status_to_json(graph_, &ret);
            res.set_content(ret.dump(), "application/json");
            });

        svr_->Get("/api/processors/list", [this](const httplib::Request&, httplib::Response& res) {
            auto listOfProc = AccessClass::getProcessorList()->getItemList();

            std::vector<json> processors_json;
            for(const auto& p : listOfProc) {
                json processor_json;
                processor_json["name"] = p.toStdString();
                processors_json.push_back(processor_json);
            }
            json ret;
            ret["processors"] = processors_json;

            res.set_content(ret.dump(), "application/json");
            });
        
        svr_->Get("/api/processors", [this](const httplib::Request&, httplib::Response& res) {
            Array<GenericProcessor*> processors = graph_->getListOfProcessors();

            std::vector<json> processors_json;
            for (const auto& processor : processors) {
                json processor_json;
                processor_to_json(processor, &processor_json);
                processors_json.push_back(processor_json);
            }
            json ret;
            ret["processors"] = processors_json;

            res.set_content(ret.dump(), "application/json");
            });
        
        svr_->Get(R"(/api/processors/([0-9]+))", [this](const httplib::Request& req, httplib::Response& res) {
            auto processor = find_processor(req.matches[1]);
            if (processor == nullptr) {
                res.status = 404;
                return;
            }
            json processor_json;
            processor_to_json(processor, &processor_json);
            res.set_content(processor_json.dump(), "application/json");
            });
        
        svr_->Get(R"(/api/processors/([0-9]+)/parameters)", [this](const httplib::Request& req, httplib::Response& res) {
            auto processor = find_processor(req.matches[1]);
            if (processor == nullptr) {
                res.status = 404;
                return;
            }
            std::vector<json> parameters_json;
            parameters_to_json(processor, &parameters_json);
            json ret;
            ret["parameters"] = parameters_json;
            res.set_content(ret.dump(), "application/json");
            });
        
        svr_->Get(R"(/api/processors/([0-9]+)/parameters/([A-Za-z0-9_\.\-]+))",
            [this](const httplib::Request& req, httplib::Response& res) {
                auto processor = find_processor(req.matches[1]);
                if (processor == nullptr) {
                    res.status = 404;
                    return;
                }

                auto parameter = find_parameter(processor, req.matches[2]);
                if (parameter == nullptr) {
                    res.status = 404;
                    return;
                }

                json ret;
                parameter_to_json(parameter, &ret);
                res.set_content(ret.dump(), "application/json");
            });

        svr_->Get(R"(/api/processors/([0-9]+)/streams/([0-9]+))",
            [this](const httplib::Request& req, httplib::Response& res) {
                auto processor = find_processor(req.matches[1]);
                if (processor == nullptr) {
                    res.status = 404;
                    return;
                }

                auto stream = find_stream(processor, req.matches[2]);
                if (stream == nullptr) {
                    res.status = 404;
                    return;
                }

                json ret;
                stream_to_json(processor, stream, &ret);
                res.set_content(ret.dump(), "application/json");
            });
        
        svr_->Get(R"(/api/processors/([0-9]+)/streams/([0-9]+)/parameters)",
        [this](const httplib::Request& req, httplib::Response& res) {
            auto processor = find_processor(req.matches[1]);
            if (processor == nullptr) {
                res.status = 404;
                return;
            }

            auto stream = find_stream(processor, req.matches[2]);
            if (stream == nullptr) {
                res.status = 404;
                return;
            }

            json ret;
            
            std::vector<json> parameters_json;
            parameters_to_json(processor, stream->getStreamId(), &parameters_json);
            ret["parameters"] = parameters_json;
            res.set_content(ret.dump(), "application/json");
        });
        
        svr_->Get(R"(/api/processors/([0-9]+)/streams/([0-9]+)/parameters/([A-Za-z0-9_\.\-]+))",
        [this](const httplib::Request& req, httplib::Response& res) {
            auto processor = find_processor(req.matches[1]);
            if (processor == nullptr) {
                res.status = 404;
                return;
            }

            auto stream = find_stream(processor, req.matches[2]);
            if (stream == nullptr) {
                res.status = 404;
                return;
            }

            auto parameter = find_parameter(processor, stream->getStreamId(), req.matches[3]);
            if (parameter == nullptr) {
               res.status = 404;
               return;
             }

             json ret;
             parameter_to_json(parameter, &ret);
             res.set_content(ret.dump(), "application/json");
        });
        
        svr_->Put("/api/processors/([0-9]+)/config", [this](const httplib::Request& req, httplib::Response& res) {
            std::string message_str;
            LOGD( "Received PUT request" );
            auto processor = find_processor(req.matches[1]);
            if (processor == nullptr) {
                LOGD( "Could not find processor" );
                res.status = 404;
                return;
            }
            try {
                LOGD( "Trying to decode" );
                json request_json;
                request_json = json::parse(req.body);
                LOGD( "Parsed" );
                message_str = request_json["text"];
                LOGD( "Message string: ", message_str );
            }
            catch (json::exception& e) {
                LOGD( "Hit exception" );
                res.set_content(e.what(), "text/plain");
                res.status = 400;
                return;
            }

            String return_msg = graph_->sendConfigMessage(processor, String(message_str));
             
            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json");
            });

        svr_->Get("/api/processors/clear", [this](const httplib::Request&, httplib::Response& res) {

            String return_msg;

            if (!CoreServices::getAcquisitionStatus())
            {
                const MessageManagerLock mml;
                graph_->clearSignalChain();
                return_msg = "Signal chain cleared successfully.";
            } else {
                return_msg = "Cannot clear signal chain while acquisition is active!";
            }

            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json");
            });

        svr_->Put("/api/processors/delete", [this](const httplib::Request& req, httplib::Response& res) {

            LOGD( "Received PUT request" );
            
            json request_json;
            try {
                LOGD( "Trying to decode" );
                request_json = json::parse(req.body);
                LOGD( "Parsed" );
            }
            catch (json::exception& e) {
                LOGD( "Hit exception" );
                res.set_content(e.what(), "text/plain");
                res.status = 400;
                return;
            }
            
            int procId;
            if (!request_json.contains("id")) {
                LOGD( "No 'id' element found." );
                res.set_content("Request must contain processor id.", "text/plain");
                res.status = 400;
                return;
            }
            else {
                procId = request_json["id"];
                LOGD( "Found a processor id." );
            }

            auto processor = find_processor(String(procId).toStdString());
            if (processor == nullptr) {
                LOGD( "Could not find processor" );
                res.status = 404;
                return;
            }

            String return_msg;
            
            if (!CoreServices::getAcquisitionStatus())
            {   
                Array<GenericProcessor*> processorNodes;
                String processorName = processor->getDisplayName();

                processorNodes.add(processor);

                const MessageManagerLock mml;
                graph_->deleteNodes(processorNodes);

                return_msg = processorName + " [" +  String(procId) + "] deleted successfully";
            } else {
                return_msg = "Cannot delete processors while acquisition is active.";
            }
             
            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json");
            });

        
        svr_->Put("/api/processors/add", [this](const httplib::Request& req, httplib::Response& res) {

            LOGD( "Received PUT request" );
            
            json request_json;
            try {
                LOGD( "Trying to decode" );
                request_json = json::parse(req.body);
                LOGD( "Parsed" );
            }
            catch (json::exception& e) {
                LOGD( "Hit exception" );
                res.set_content(e.what(), "text/plain");
                res.status = 400;
                return;
            }
            
            std::string procName;
            if (!request_json.contains("name")) {
                LOGD( "No 'name' element found." );
                res.set_content("Request must contain processor name.", "text/plain");
                res.status = 400;
                return;
            }
            else {
                procName = request_json["name"];
                LOGD( "Found processor name: ", procName);
            }

            int sourceNodeId = 0;
            int destNodeId = 0;

            if(request_json.contains("source_id"))
                sourceNodeId = request_json["source_id"];
            else if (request_json.contains("dest_id"))
                destNodeId = request_json["dest_id"];

            LOGD( "Found a source/dest node id." );

            
            auto listOfProc = AccessClass::getProcessorList()->getItemList();
            bool foundProcessor = false;
            for(auto p : listOfProc)
            {
                if(p.equalsIgnoreCase(String(procName)))
                {
                    foundProcessor = true;
                    break;
                }
            }

            if (!foundProcessor) {
                LOGD( "Could not find processor in the Processor List" );
                res.status = 404;
                return;
            }

            String return_msg;
            
            if (!CoreServices::getAcquisitionStatus())
            {
                auto description = AccessClass::getProcessorList()->getItemDescriptionfromList(procName);

                GenericProcessor* sourceProcessor = nullptr;
                GenericProcessor* destProcessor = nullptr;
                
                if (sourceNodeId == 0)
                {
                    destProcessor = graph_->getProcessorWithNodeId(destNodeId);
                    
                    if (destProcessor != nullptr)
                        sourceProcessor = destProcessor->getSourceNode();
                }
                else
                {
                    sourceProcessor = graph_->getProcessorWithNodeId(sourceNodeId);
                    
                    if (sourceProcessor != nullptr)
                        destProcessor = sourceProcessor->getDestNode();
                }

                const MessageManagerLock mml;
                graph_->createProcessor(description,
                    sourceProcessor,
                    destProcessor);

                return_msg = procName + " added successfully";
                
            } else {
                return_msg = "Cannot add processors while acquisition is active.";
            }
             
            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json");
            });

        svr_->Put(R"(/api/processors/([0-9]+)/parameters/([A-Za-z0-9_\.\-]+))",
            [this](const httplib::Request& req, httplib::Response& res) {
                auto processor = find_processor(req.matches[1]);
                if (processor == nullptr) {
                    res.status = 404;
                    return;
                }

                auto parameter = find_parameter(processor, req.matches[2]);
            
                if (parameter == nullptr) {
                    res.status = 404;
                    return;
                } else {
                    LOGD( "Found parameter: ", parameter->getName() );
                }

                json request_json;
                try {
                    request_json = json::parse(req.body);
                }
                catch (json::exception& e) {
                    LOGD( "Failed to parse request body." );
                    res.set_content(e.what(), "text/plain");
                    res.status = 400;
                    return;
                }

                if (!request_json.contains("value")) {
                    LOGD( "No 'value' element found." );
                    res.set_content("Request must contain value.", "text/plain");
                    res.status = 400;
                    return;
                } else {
                    LOGD( "Found a value." );
                }
            
                var val;

                try {
                    auto value = request_json["value"];
                    val = json_to_var(value);
                }
                catch (json::exception& e) {
                    res.set_content(e.what(), "text/plain");
                    res.status = 400;
                    return;
                }
            
               //LOGD( "Got value: " << String(val) );
 
                if (val.isUndefined()) {
                    res.set_content("Request value could not be converted.", "text/plain");
                    res.status = 400;
                    return;
                }

                parameter->setNextValue(val);

                json ret;
                parameter_to_json(parameter, &ret);
                res.set_content(ret.dump(), "application/json");
            });
        
        svr_->Put(R"(/api/processors/([0-9]+)/streams/([0-9]+)/parameters/([A-Za-z0-9_\.\-]+))",
                   [this](const httplib::Request& req, httplib::Response& res) {
                       auto processor = find_processor(req.matches[1]);
                       if (processor == nullptr) {
                           res.status = 404;
                           return;
                       }

                       auto stream = find_stream(processor, req.matches[2]);
                   
                       if (stream == nullptr) {
                           res.status = 404;
                           return;
                       } else {
                           LOGD( "Found stream: ", stream->getName() );
                       }
            
                        auto parameter = find_parameter(processor, stream->getStreamId(), req.matches[3]);
                    
                        if (parameter == nullptr) {
                            res.status = 404;
                            return;
                        } else {
                            LOGD( "Found parameter: ", parameter->getName() );
                        }

                       json request_json;
                       try {
                           request_json = json::parse(req.body);
                       }
                       catch (json::exception& e) {
                           LOGD( "Failed to parse request body." );
                           res.set_content(e.what(), "text/plain");
                           res.status = 400;
                           return;
                       }

                       if (!request_json.contains("value")) {
                           LOGD( "No 'value' element found." );
                           res.set_content("Request must contain value.", "text/plain");
                           res.status = 400;
                           return;
                       } else {
                           LOGD( "Found a value." );
                       }
                   
                       var val;

                       try {
                           auto value = request_json["value"];
                           val = json_to_var(value);
                       }
                       catch (json::exception& e) {
                           res.set_content(e.what(), "text/plain");
                           res.status = 400;
                           return;
                       }
                   
                      //LOGD( "Got value: " << String(val) );
        
                       if (val.isUndefined()) {
                           res.set_content("Request value could not be converted.", "text/plain");
                           res.status = 400;
                           return;
                       }

                       parameter->setNextValue(val);

                       json ret;
                       parameter_to_json(parameter, &ret);
                       res.set_content(ret.dump(), "application/json");
                   });

        svr_->Put("/api/window", [this](const httplib::Request& req, httplib::Response& res) {
            std::string message_str;
            LOGD( "Received PUT WINDOW request" );

            try {
                LOGD( "Trying to decode" );
                json request_json;
                request_json = json::parse(req.body);
                LOGD( "Parsed" );
                message_str = request_json["command"];
                LOGD( "Message string: ", message_str );
            }
            catch (json::exception& e) {
                LOGD( "Hit exception" );
                res.set_content(e.what(), "text/plain");
                res.status = 400;
                return;
            }

            if (String(message_str).equalsIgnoreCase("quit"))
            {
                json ret;
                res.set_content(ret.dump(), "application/json");
                res.status = 400;

                const MessageManagerLock mml;
                JUCEApplication::getInstance()->systemRequestedQuit();
            }
            else {
                LOGD("Unrecognized command");
            }
            

            });

        svr_->Get("/api/undo", [this](const httplib::Request& req, httplib::Response& res) {
            std::string message_str;
            LOGD( "Received undo request" );

            json ret;
            res.set_content(ret.dump(), "application/json");
            res.status = 400;

            const MessageManagerLock mml;
            AccessClass::getEditorViewport()->undo();

            });

        svr_->Get("/api/redo", [this](const httplib::Request& req, httplib::Response& res) {
            std::string message_str;
            LOGD( "Received redo request" );

            json ret;
            res.set_content(ret.dump(), "application/json");
            res.status = 400;

            const MessageManagerLock mml;
            AccessClass::getEditorViewport()->redo();

            });
                   
        LOGC("Beginning HTTP server on port ", PORT);
        svr_->listen("0.0.0.0", PORT);
    }

    void start() {
        if (!svr_) {
            svr_ = std::make_unique<httplib::Server>();
        }
        startThread();
    }

    void stop() {
        if (svr_) {
            LOGC("Shutting down HTTP server");
            svr_->stop();
        }
        stopThread(5000);
    }

private:
    std::unique_ptr<httplib::Server> svr_;
    MainWindow* main_;
    ProcessorGraph* graph_;

    var json_to_var(const json& value) {
        if (value.is_number_integer()) {
            return var(value.get<int>());
        }
        else if (value.is_number_float()) {
            return var(value.get<double>());
        }
        else if (value.is_boolean()) {
            return var(value.get<bool>());
        }
        else if (value.is_string()) {
            return var(value.get<std::string>());
        }
        else if (value.is_array()) {
            juce::Array<var> vars;
            for (auto& element : value) {
                vars.add(element.get<double>());
            }
            return var(vars);
        }
        else {
            return var::undefined();
        }
    }

    inline static String getCurrentDataDir(const ProcessorGraph* graph) {

       /* if (graph->getRecordNode()) {
            auto path = graph->getRecordNode()->getDataDirectory().getFullPathName();
            if (!path.isEmpty()) {
                return path.toStdString();
            }
        }
        return "";*/
        return "";
    }

    inline static void recording_info_to_json(const ProcessorGraph* graph, json* ret)
    {
        
        (*ret)["parent_directory"] = CoreServices::getRecordingParentDirectory().getFullPathName().toStdString();

        (*ret)["base_text"] = CoreServices::getRecordingDirectoryBaseText().toStdString();

        (*ret)["prepend_text"] = CoreServices::getRecordingDirectoryPrependText().toStdString();

        (*ret)["append_text"] = CoreServices::getRecordingDirectoryAppendText().toStdString();

        (*ret)["default_record_engine"] = CoreServices::getDefaultRecordEngineId().toStdString();

        std::vector<json> record_nodes_json;

        for (int nodeId : CoreServices::getAvailableRecordNodeIds()) 
        {
            json record_node_json;
            record_node_to_json(nodeId, &record_node_json);
            record_nodes_json.push_back(record_node_json);
        }

        (*ret)["record_nodes"] = record_nodes_json;

    }

    inline static void record_node_to_json(int nodeId, json* ret)
    {

        (*ret)["node_id"] = nodeId;

        (*ret)["parent_directory"] = CoreServices::RecordNode::getRecordingDirectory(nodeId).getFullPathName().toStdString();

        (*ret)["record_engine"] = CoreServices::RecordNode::getRecordEngineId(nodeId).toStdString();

        (*ret)["experiment_number"] = CoreServices::RecordNode::getExperimentNumber(nodeId);

        (*ret)["recording_number"] = CoreServices::RecordNode::getRecordingNumber(nodeId);
        
        (*ret)["is_synchronized"] = CoreServices::RecordNode::isSynchronized(nodeId);
    }

    inline static void status_to_json(const ProcessorGraph* graph, json* ret) 
    {
        if (CoreServices::getRecordingStatus()) {
            (*ret)["mode"] = "RECORD";
        }
        else if (CoreServices::getAcquisitionStatus()) {
            (*ret)["mode"] = "ACQUIRE";
        }
        else {
            (*ret)["mode"] = "IDLE";
        }
    }

    inline static void parameter_to_json(Parameter* parameter, json* parameter_json)
    {
        (*parameter_json)["name"] = parameter->getName().toStdString();
        (*parameter_json)["type"] = parameter->getParameterTypeString().toStdString();
        (*parameter_json)["value"] = parameter->getValueAsString().toStdString();
    }

    inline static void parameters_to_json(GenericProcessor* processor, std::vector<json>* parameters_json) {
        for (const auto& parameter : processor->getParameters()) {
            json parameter_json;
            parameter_to_json(parameter, &parameter_json);
            parameters_json->push_back(parameter_json);
        }
    }
    
    inline static void parameters_to_json(GenericProcessor* processor, uint16 streamId, std::vector<json>* parameters_json) {
        for (const auto& parameter : processor->getParameters(streamId)) {
            json parameter_json;
            parameter_to_json(parameter, &parameter_json);
            parameters_json->push_back(parameter_json);
        }
    }
    
    inline static void stream_to_json(GenericProcessor* processor, const DataStream* stream, json* stream_json)
    {
        (*stream_json)["name"] = stream->getName().toStdString();
        (*stream_json)["source_id"] = stream->getSourceNodeId();
        (*stream_json)["sample_rate"] = stream->getSampleRate();
        (*stream_json)["channel_count"] = stream->getChannelCount();
        
        std::vector<json> parameters_json;
        parameters_to_json(processor, stream->getStreamId(), &parameters_json);
        (*stream_json)["parameters"] = parameters_json;
    
    }

    inline static void processor_to_json(GenericProcessor* processor, json* processor_json)
    {
        (*processor_json)["id"] = processor->getNodeId();
        (*processor_json)["name"] = processor->getName().toStdString();

        std::vector<json> parameters_json;
        parameters_to_json(processor, &parameters_json);
        (*processor_json)["parameters"] = parameters_json;
        
        std::vector<json> streams_json;
        
        for (auto stream : processor->getDataStreams())
        {
            json stream_json;
            stream_to_json(processor, stream, &stream_json);
            streams_json.push_back(stream_json);
        }
        
        (*processor_json)["streams"] = streams_json;

        if (processor->getSourceNode() == nullptr) {
            (*processor_json)["predecessor"] = json::value_t::null;
        }
        else {
            (*processor_json)["predecessor"] = processor->getSourceNode()->getNodeId();
        }
    }

    inline GenericProcessor* find_processor(const std::string& id_string) {
        int processor_id = juce::String(id_string).getIntValue();
        LOGD("Searching for processor with ID ", processor_id);

        auto processors = graph_->getListOfProcessors();
        for (const auto& processor : processors) {
            if (processor->getNodeId() == processor_id) {
                return processor;
            }
        }

        return nullptr;
    }
    
    inline const DataStream* find_stream(GenericProcessor* p, const std::string& id_string) {
        int stream_index = juce::String(id_string).getIntValue();
        
        if (stream_index < 0)
            return nullptr;

        if (p->getDataStreams().size() > stream_index)
            return p->getDataStreams()[stream_index];

        return nullptr;
    }

    static inline Parameter* find_parameter(GenericProcessor* processor, const std::string& parameter_name)
    {
        Parameter* parameter = processor->getParameter(juce::String(parameter_name));
        
        if (parameter != nullptr)
        {
            return parameter;
        }
        
        return nullptr;
    }
    
    static inline Parameter* find_parameter(GenericProcessor* processor, uint16 streamId, const std::string& parameter_name)
    {
        Parameter* parameter = processor->getDataStream(streamId)->getParameter(parameter_name);
        
        if (parameter != nullptr)
        {
            return parameter;
        }
        
        return nullptr;
    }
};

#endif  // __PROCESSORGRAPHHTTPSERVER_H_124F8B50__
