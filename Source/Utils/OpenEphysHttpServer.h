/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "../Processors/GenericProcessor/GenericProcessor.h"
#include "../Processors/Parameter/Parameter.h"
#include "../Processors/ProcessorGraph/ProcessorGraphActions.h"
#include "../Processors/ProcessorManager/ProcessorManager.h"

#include "httplib.h"
#include "json.hpp"
#include <sstream>

#include "../AccessClass.h"
#include "../MainWindow.h"
#include "../UI/ProcessorList.h"

#include "Utils.h"

using json = nlohmann::json;

#define PORT 37497

/**
 * HTTP server thread for controlling Processor Parameters via an HTTP API. This starts an HTTP server on port 37497
 * (== "EPHYS" on a phone keypad) and allows remote manipulation of parameters of processors currently in the graph.
 *
 * The API is "RESTful", such that the resource URLs are:
 * 
 * - GET /api/config :
 *          returns an XML string with the current configuration of the GUI
 *
 * - GET /api/status : 
 *          returns a JSON string with the GUI's current mode (IDLE, ACQUIRE, RECORD)
 * 
 * - PUT /api/status : 
 *          sets the GUI's mode
 *          e.g.: {"mode" : "ACQUIRE"}
 * 
 * - GET /api/cpu :
 *         returns a JSON string with the average proportion of available CPU being spent inside the audio callbacks
 *
 * - GET /api/audio/devices :
 *        returns a JSON string with the available audio devices
 *
 * - GET /api/audio/device :
 *       returns a JSON string with the current audio device
 *
 * - PUT /api/audio :
 *        sets the audio device
 *        e.g.: {"device_type" : "input", "device_name" : "Microphone (Realtek High Definition Audio)", "sample_rate" : 44100, "buffer_size" : 512}
 *
 * - PUT /api/recording :
 *         sets the default recording options
 *         e.g.: {"parent_directory" : "C:/Users/username/Documents/OpenEphys", "prepend_text" : "Prepend", "base_text" : "Base", "append_text" : "Append", "default_record_engine" : "OpenEphys", "start_new_directory" : "true"}
 *
 * - PUT /api/recording/<processor_id> :
 *        sets the options for a given Record Node
 *        e.g.: {"parent_directory" : "C:/Users/username/Documents/OpenEphys", "record_engine" : "OpenEphys"}
 *
 * - PUT /api/message :
 *          sends a broadcast message to all processors, e.g.: {"text" : "Message content"}
 *          only works while acquisition is active
 *
 * - PUT /api/load :
 *         loads a new signal chain from a file, e.g.: {"path" : "C:/Users/username/Documents/OpenEphys/chain.xml"}
 *
 * - PUT /api/save :
 *          saves the current signal chain to a file, e.g.: {"filepath" : "C:/Users/username/Documents/OpenEphys/chain.xml"}
 *
 * - GET /api/processors/list :
 *          returns a JSON string with the available processors
 *
 * - GET /api/processors :
 *          returns a JSON string with the processors currently in the graph
 *
 * - GET /api/processors/<processor_id> :
 *         returns a JSON string with the processor's parameters
 *
 * - GET /api/processors/<processor_id>/parameters :
 *          returns a JSON string with the processor's parameters
 *
 * - GET /api/processors/<processor_id>/parameters/<parameter_name> :
 *          returns a JSON string with the parameter's value
 *
 * - GET /api/processors/<processor_id>/streams/<stream_index> :
 *          returns a JSON string with the stream's info and parameters
 *
 * - GET /api/processors/<processor_id>/streams/<stream_index>/parameters :
 *          returns a JSON string with the stream's parameters
 *
 * - GET /api/processors/<processor_id>/streams/<stream_index>/parameters/<parameter_name>
 *         returns a JSON string with the stream's parameter value
 *
 * - PUT /api/processors/<processor_id>/config :
 *          sends a configuration message to a processor, e.g.: {"text" : "Message content"}
 *
 * - GET /api/processors/clear :
 *         clears the signal chain
 *
 * - PUT /api/processors/delete :
 *         deletes a processor, e.g.: {"id" : 101}
 *
 * - PUT /api/processors/add :
 *          adds a processor, e.g.: {"name" : "ProcessorName", "source_id" : 100 }
 *
 * - PUT /api/undo :
 *          undoes the last action
 *
 * - PUT /api/redo :
 *          redoes the last action
 *
 * - PUT /api/processors/<processor_id>/parameters/<parameter_name> :
 *          sets a parameter's value, e.g.: {"value" : 0.5}
 *
 * - PUT /api/processors/<processor_id>/streams/<stream_index>/parameters/<parameter_name> :
 *          sets a stream's parameter value, e.g.: {"value" : 0.5}
 *
 * - PUT /api/quit:
 *          quits the application
 */

class OpenEphysHttpServer : juce::Thread
{
public:
    explicit OpenEphysHttpServer (ProcessorGraph* graph) : graph_ (graph),
                                                           juce::Thread ("HttpServer") {}

    void run() override
    {
        svr_->Get ("/api/config", [this] (const httplib::Request&, httplib::Response& res)
                   {
            std::unique_ptr<XmlElement> xmlElement = std::make_unique<XmlElement> ("SETTINGS");
            graph_->saveToXml (xmlElement.get());

            json ret;
            ret["info"] = xmlElement.get()->toString().toStdString();
            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/status", [this] (const httplib::Request&, httplib::Response& res)
                   {
            json ret;
            status_to_json(graph_, &ret);
            res.set_content(ret.dump(), "application/json"); });

        svr_->Put ("/api/status", [this] (const httplib::Request& req, httplib::Response& res)
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
                std::promise<void> signalRecordingStarted;
                std::future<void> signalRecordingStartedFuture = signalRecordingStarted.get_future();

                MessageManager::callAsync([this, &signalRecordingStarted] {
                    CoreServices::setRecordingStatus(true);
                    signalRecordingStarted.set_value(); // Signal that recording has started
                });

                // Wait for recording to start
                signalRecordingStartedFuture.wait();
            }
            else if (desired_mode == "ACQUIRE") {
                std::promise<void> signalAcquisitionStarted;
                std::future<void> signalAcquisitionStartedFuture = signalAcquisitionStarted.get_future();

                MessageManager::callAsync([this, &signalAcquisitionStarted] {
                    CoreServices::setRecordingStatus(false);
                    CoreServices::setAcquisitionStatus(true);
                    signalAcquisitionStarted.set_value(); // Signal that acquisition has started
                });

                // Wait for acquisition to start
                signalAcquisitionStartedFuture.wait();
            }
            else if (desired_mode == "IDLE") {
                std::promise<void> signalAcquisitionStopped;
                std::future<void> signalAcquisitionStoppedFuture = signalAcquisitionStopped.get_future();

                MessageManager::callAsync([this, &signalAcquisitionStopped] {
                    CoreServices::setRecordingStatus(false);
                    CoreServices::setAcquisitionStatus(false);
                    signalAcquisitionStopped.set_value(); // Signal that acquisition has stopped
                });

                // Wait for acquisition to stop
                signalAcquisitionStoppedFuture.wait();
            }

            json ret;
            status_to_json(graph_, &ret);
            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/cpu", [this] (const httplib::Request&, httplib::Response& res)
                   {
            json ret;
            ret["usage"] = AccessClass::getAudioComponent()->deviceManager.getCpuUsage();
            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/latency", [this] (const httplib::Request&, httplib::Response& res)
                   {
            Array<GenericProcessor*> processors = graph_->getListOfProcessors();

            std::vector<json> processor_latencies_json;
            for (const auto& processor : processors) {
                json processor_latency_json;
                processor_latency_to_json(processor, &processor_latency_json);
                processor_latencies_json.push_back(processor_latency_json);
            }
            json ret;
            ret["processors"] = processor_latencies_json;

            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/audio/devices", [this] (const httplib::Request&, httplib::Response& res)
                   {
            json ret;
            audio_devices_to_json(&ret);
            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/audio/device", [this] (const httplib::Request&, httplib::Response& res)
                   {
            json ret;
            audio_device_info_to_json(&ret);
            res.set_content(ret.dump(), "application/json"); });

        svr_->Put ("/api/audio", [this] (const httplib::Request& req, httplib::Response& res)
                   {
                json request_json;

                LOGD("Received PUT request at /api/audio with content: ", req.body);

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
                    std::string device_type = request_json["device_type"];
                    LOGD("Found 'device_type': ", device_type);
                    const MessageManagerLock mml;
                    AccessClass::getAudioComponent()->setDeviceType(String(device_type));
                }
                catch (json::exception& e) {
                    LOGD("'device_type' not specified'");
                }

                try {
                    std::string device_name = request_json["device_name"];
                    LOGD("Found 'device_name': ", device_name);
                    const MessageManagerLock mml;
                    AccessClass::getAudioComponent()->setDeviceName(String(device_name));
                }
                catch (json::exception& e) {
                    LOGD("'device_name' not specified'");
                }

                try {
                    int sample_rate = request_json["sample_rate"];
                    LOGD("Found 'sample_rate': ", sample_rate);
                    const MessageManagerLock mml;
                    AccessClass::getAudioComponent()->setSampleRate(sample_rate);
                }
                catch (json::exception& e) {
                    LOGD("'sample_rate' not specified'");
                }

                try {
                    int buffer_size = request_json["buffer_size"];
                    LOGD("Found 'buffer_size': ", buffer_size);
                    const MessageManagerLock mml;
                    AccessClass::getAudioComponent()->setBufferSize(buffer_size);
                    graph_->updateBufferSize();
                }
                catch (json::exception& e) {
                    LOGD("'buffer_size' not specified'");
                }

                json ret;
                audio_device_info_to_json(&ret);
                res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/recording", [this] (const httplib::Request&, httplib::Response& res)
                   {
            json ret;
            recording_info_to_json(graph_, &ret);
            res.set_content(ret.dump(), "application/json"); });

        svr_->Put ("/api/recording", [this] (const httplib::Request& req, httplib::Response& res)
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
                res.set_content(ret.dump(), "application/json"); });

        svr_->Put ("/api/recording/([0-9]+)", [this] (const httplib::Request& req, httplib::Response& res)
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
                res.set_content(ret.dump(), "application/json"); });

        svr_->Put ("/api/message", [this] (const httplib::Request& req, httplib::Response& res)
                   {
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
            res.set_content(ret.dump(), "application/json"); });

        svr_->Put ("/api/load", [this] (const httplib::Request& req, httplib::Response& res)
                   {
            
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

            std::promise<void> signalChainLoaded;
            std::future<void> signalChainLoadedFuture = signalChainLoaded.get_future();

            MessageManager::callAsync([message_str, &signalChainLoaded] {
                CoreServices::loadSignalChain(message_str);
                signalChainLoaded.set_value(); // Signal that loadSignalChain is finished
            });

            // Wait for loadSignalChain to finish
            signalChainLoadedFuture.wait();

            json ret;
            status_to_json(graph_, &ret);
            res.set_content(ret.dump(), "application/json"); });

        svr_->Put ("/api/save", [this] (const httplib::Request& req, httplib::Response& res)
                   {
            std::string message_str;
            LOGD("Received PUT request");

            try {
                LOGD("Trying to decode");
                json request_json;
                request_json = json::parse(req.body);
                LOGD("Parsed");
                message_str = request_json["filepath"];
                LOGD("Message string: ", message_str);
            }
            catch (json::exception& e) {
                LOGD("Hit exception");
                res.set_content(e.what(), "text/plain");
                res.status = 400;
                return;
            }

            File writePath = File (message_str);

            if ( writePath.existsAsFile())
            {
                LOGE("[HTTPServer] Failed to save configuration, file already exists at path: ", writePath.getFullPathName());
                json ret;
                ret["info"] = "File already exists at path: " + writePath.getFullPathName().toStdString();
                res.set_content(ret.dump(), "application/json");
                return;
            }

            std::promise<void> signalChainSaved;
            std::future<void> signalChainSavedFuture = signalChainSaved.get_future();

            String xml;

            MessageManager::callAsync([this, message_str, writePath, &signalChainSaved, &xml] {

                std::unique_ptr<XmlElement> xmlElement = std::make_unique<XmlElement> ("SETTINGS");

                graph_->saveToXml (xmlElement.get());

                String message;

                if (! xmlElement->writeTo ( writePath ))
                    message = "Couldn't write to file ";
                else
                    message = "Saved configuration as ";

                message += writePath.getFileName();

                xml = xmlElement->toString();
                signalChainSaved.set_value(); // Signal that saveSignalChain is finished
            });

            // Wait for saveSignalChain to finish
            signalChainSavedFuture.wait();

            json ret;
            ret["info"] = xml.toStdString();
            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/processors/list", [this] (const httplib::Request&, httplib::Response& res)
                   {

            auto listOfProc = ProcessorManager::getAvailableProcessors();

            std::vector<json> processors_json;
            for(const auto& p : listOfProc) {
                json processor_json;
                processor_json["name"] = p.toStdString();
                processors_json.push_back(processor_json);
            }
            json ret;
            ret["processors"] = processors_json;

            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/processors", [this] (const httplib::Request&, httplib::Response& res)
                   {
            Array<GenericProcessor*> processors = graph_->getListOfProcessors();

            std::vector<json> processors_json;
            for (const auto& processor : processors) {
                json processor_json;
                processor_to_json(processor, &processor_json);
                processors_json.push_back(processor_json);
            }
            json ret;
            ret["processors"] = processors_json;

            res.set_content(ret.dump(), "application/json"); });

        svr_->Get (R"(/api/processors/([0-9]+))", [this] (const httplib::Request& req, httplib::Response& res)
                   {
            auto processor = find_processor(req.matches[1]);
            if (processor == nullptr) {
                res.status = 404;
                return;
            }
            json processor_json;
            processor_to_json(processor, &processor_json);
            res.set_content(processor_json.dump(), "application/json"); });

        svr_->Get (R"(/api/processors/([0-9]+)/parameters)", [this] (const httplib::Request& req, httplib::Response& res)
                   {
            auto processor = find_processor(req.matches[1]);
            if (processor == nullptr) {
                res.status = 404;
                return;
            }
            std::vector<json> parameters_json;
            parameters_to_json(processor, &parameters_json);
            json ret;
            ret["parameters"] = parameters_json;
            res.set_content(ret.dump(), "application/json"); });

        svr_->Get (R"(/api/processors/([0-9]+)/parameters/([A-Za-z0-9_\.\-]+))",
                   [this] (const httplib::Request& req, httplib::Response& res)
                   {
                       auto processor = find_processor (req.matches[1]);
                       if (processor == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       auto parameter = find_parameter (processor, req.matches[2]);
                       if (parameter == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       json ret;
                       parameter_to_json (parameter, &ret);
                       res.set_content (ret.dump(), "application/json");
                   });

        svr_->Get (R"(/api/processors/([0-9]+)/streams/([0-9]+))",
                   [this] (const httplib::Request& req, httplib::Response& res)
                   {
                       auto processor = find_processor (req.matches[1]);
                       if (processor == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       auto stream = find_stream (processor, req.matches[2]);
                       if (stream == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       json ret;
                       stream_to_json (processor, stream, &ret);
                       res.set_content (ret.dump(), "application/json");
                   });

        svr_->Get (R"(/api/processors/([0-9]+)/streams/([0-9]+)/parameters)",
                   [this] (const httplib::Request& req, httplib::Response& res)
                   {
                       auto processor = find_processor (req.matches[1]);
                       if (processor == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       auto stream = find_stream (processor, req.matches[2]);
                       if (stream == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       json ret;

                       std::vector<json> parameters_json;
                       parameters_to_json (processor, stream->getStreamId(), &parameters_json);
                       ret["parameters"] = parameters_json;
                       res.set_content (ret.dump(), "application/json");
                   });

        svr_->Get (R"(/api/processors/([0-9]+)/streams/([0-9]+)/parameters/([A-Za-z0-9_\.\-]+))",
                   [this] (const httplib::Request& req, httplib::Response& res)
                   {
                       auto processor = find_processor (req.matches[1]);
                       if (processor == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       auto stream = find_stream (processor, req.matches[2]);
                       if (stream == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       auto parameter = find_parameter (processor, stream->getStreamId(), req.matches[3]);
                       if (parameter == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       json ret;
                       parameter_to_json (parameter, &ret);
                       res.set_content (ret.dump(), "application/json");
                   });

        svr_->Put ("/api/processors/([0-9]+)/config", [this] (const httplib::Request& req, httplib::Response& res)
                   {
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

            std::promise<void> processorConfigured;
            std::future<void> processorConfiguredFuture = processorConfigured.get_future();

            String return_msg;
            MessageManager::callAsync([this, processor, message_str, &return_msg, &processorConfigured] {
                return_msg = graph_->sendConfigMessage(processor, String(message_str));
                processorConfigured.set_value(); // Signal that processor has received configuration message
            });

            // Wait for processor to parse config message
            processorConfiguredFuture.wait();
             
            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/processors/clear", [this] (const httplib::Request&, httplib::Response& res)
                   {
            String return_msg;

            if (!CoreServices::getAcquisitionStatus())
            {
                std::promise<void> signalChainCleared;
                std::future<void> signalChainClearedFuture = signalChainCleared.get_future();

                try {
                    MessageManager::callAsync([this, &signalChainCleared] {
                        try {
                            graph_->clearSignalChain();
                            signalChainCleared.set_value();
                        } catch (const std::exception& e) {
                            LOGE("Error clearing signal chain: ", e.what());
                            signalChainCleared.set_value();
                        }
                    });

                    // Wait for loadSignalChain to finish
                    signalChainClearedFuture.wait();
                    return_msg = "Signal chain cleared successfully.";
                } catch (const std::exception& e) {
                    LOGE("Error in clear signal chain async call: ", e.what());
                    return_msg = "Error clearing signal chain: " + String(e.what());
                }
            } else {
                return_msg = "Cannot clear signal chain while acquisition is active!";
            }

            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json"); });

        svr_->Put ("/api/processors/delete", [this] (const httplib::Request& req, httplib::Response& res)
                   {

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
                String processorName = processor->getDisplayName();

                std::promise<void> processorDeleted;
                std::future<void> processorDeletedFuture = processorDeleted.get_future();

                MessageManager::callAsync([this, processor, &processorDeleted] {
                    DeleteProcessor* action = new DeleteProcessor(processor);
                    graph_->getUndoManager()->beginNewTransaction("Disabled during acquisition");
                    graph_->getUndoManager()->perform(action);
                    processorDeleted.set_value(); // Signal that processor has been deleted
                });

                // Wait for processor to be deleted
                processorDeletedFuture.wait();

                return_msg = processorName + " [" +  String(procId) + "] deleted successfully";
            } else {
                return_msg = "Cannot delete processors while acquisition is active.";
            }
             
            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json"); });

        svr_->Put ("/api/processors/add", [this] (const httplib::Request& req, httplib::Response& res)
                   {

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

            
            auto listOfProc = ProcessorManager::getAvailableProcessors();
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
                auto description = ProcessorManager::getPluginDescription(procName);

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

                std::promise <void> processorAdded;
                std::future <void> processorAddedFuture = processorAdded.get_future();

                MessageManager::callAsync([this, description, sourceProcessor, destProcessor, &processorAdded] {
                    AddProcessor* action = new AddProcessor(description, sourceProcessor, destProcessor, false);
                    graph_->getUndoManager()->beginNewTransaction("Disabled during acquisition");
                    graph_->getUndoManager()->perform(action);
                    processorAdded.set_value(); // Signal that processor has been added
                });

                // Wait for processor to be added
                processorAddedFuture.wait();

                return_msg = procName + " added successfully";
                
            } else {
                return_msg = "Cannot add processors while acquisition is active.";
            }
             
            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/undo", [this] (const httplib::Request&, httplib::Response& res)
                   {

            String return_msg;

            auto* um = graph_->getUndoManager();

            bool undoDisabled = CoreServices::getAcquisitionStatus() 
                && um->getUndoDescription().contains ("Disabled during acquisition");

            if (! undoDisabled && um->canUndo())
            {
                std::promise<void> undoCompleted;
                std::future<void> undoCompletedFuture = undoCompleted.get_future();

                MessageManager::callAsync([um, &undoCompleted] {
                    um->undo();
                    undoCompleted.set_value(); // Signal that undo is finished
                });

                // Wait for undo to finish
                undoCompletedFuture.wait();

                return_msg = "Undo operation completed successfully.";
            } else {
                return_msg = "Undo operation was NOT successful!";
            }

            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json"); });

        svr_->Get ("/api/redo", [this] (const httplib::Request&, httplib::Response& res)
                   {

            String return_msg;

            auto* um = graph_->getUndoManager();

            bool redoDisabled = CoreServices::getAcquisitionStatus() 
                && um->getRedoDescription().contains ("Disabled during acquisition");

            if (!redoDisabled && um->canRedo())
            {
                std::promise<void> redoCompleted;
                std::future<void> redoCompletedFuture = redoCompleted.get_future();

                MessageManager::callAsync([um, &redoCompleted] {
                    um->redo();
                    redoCompleted.set_value(); // Signal that redo is finished
                });

                // Wait for redo to finish
                redoCompletedFuture.wait();

                return_msg = "Redo operation completed successfully.";
            } else {
                return_msg = "Redo operation was NOT successful!";
            }

            json ret;
            ret["info"] = return_msg.toStdString();
            res.set_content(ret.dump(), "application/json"); });

        svr_->Put (R"(/api/processors/([0-9]+)/parameters/([A-Za-z0-9_\.\-]+))",
                   [this] (const httplib::Request& req, httplib::Response& res)
                   {
                       auto processor = find_processor (req.matches[1]);
                       if (processor == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       auto parameter = find_parameter (processor, req.matches[2]);

                       if (parameter == nullptr)
                       {
                           res.status = 404;
                           return;
                       }
                       else
                       {
                           LOGD ("Found parameter: ", parameter->getName());
                       }

                       if (parameter->shouldDeactivateDuringAcquisition()
                           && CoreServices::getAcquisitionStatus())
                       {
                           res.set_content ("Cannot change this parameter while acquisition is active.", "text/plain");
                           res.status = 400;
                           return;
                       }

                       json request_json;
                       try
                       {
                           request_json = json::parse (req.body);
                       }
                       catch (json::exception& e)
                       {
                           LOGD ("Failed to parse request body.");
                           res.set_content (e.what(), "text/plain");
                           res.status = 400;
                           return;
                       }

                       if (! request_json.contains ("value"))
                       {
                           LOGD ("No 'value' element found.");
                           res.set_content ("Request must contain value.", "text/plain");
                           res.status = 400;
                           return;
                       }
                       else
                       {
                           LOGD ("Found a value.");
                       }

                       var val;

                       try
                       {
                           auto value = request_json["value"];
                           val = json_to_var (value);
                       }
                       catch (json::exception& e)
                       {
                           res.set_content (e.what(), "text/plain");
                           res.status = 400;
                           return;
                       }

                       //LOGD( "Got value: " << String(val) );

                       if (val.isUndefined())
                       {
                           res.set_content ("Request value could not be converted.", "text/plain");
                           res.status = 400;
                           return;
                       }

                       std::promise<void> parameterChanged;
                       std::future<void> parameterChangedFuture = parameterChanged.get_future();

                       MessageManager::callAsync ([parameter, val, &parameterChanged]
                                                  {
                                                      parameter->setNextValue (val);
                                                      parameterChanged.set_value(); // Signal that parameter has been changed
                                                  });

                       // Wait for parameter to be changed
                       parameterChangedFuture.wait();

                       json ret;
                       parameter_to_json (parameter, &ret);
                       res.set_content (ret.dump(), "application/json");
                   });

        svr_->Put (R"(/api/processors/([0-9]+)/streams/([0-9]+)/parameters/([A-Za-z0-9_\.\-]+))",
                   [this] (const httplib::Request& req, httplib::Response& res)
                   {
                       auto processor = find_processor (req.matches[1]);
                       if (processor == nullptr)
                       {
                           res.status = 404;
                           return;
                       }

                       auto stream = find_stream (processor, req.matches[2]);

                       if (stream == nullptr)
                       {
                           res.status = 404;
                           return;
                       }
                       else
                       {
                           LOGD ("Found stream: ", stream->getName());
                       }

                       auto parameter = find_parameter (processor, stream->getStreamId(), req.matches[3]);

                       if (parameter == nullptr)
                       {
                           res.status = 404;
                           return;
                       }
                       else
                       {
                           LOGD ("Found parameter: ", parameter->getName());
                       }

                       if (parameter->shouldDeactivateDuringAcquisition()
                           && CoreServices::getAcquisitionStatus())
                       {
                           res.set_content ("Cannot change this parameter while acquisition is active.", "text/plain");
                           res.status = 400;
                           return;
                       }

                       json request_json;
                       try
                       {
                           request_json = json::parse (req.body);
                       }
                       catch (json::exception& e)
                       {
                           LOGD ("Failed to parse request body.");
                           res.set_content (e.what(), "text/plain");
                           res.status = 400;
                           return;
                       }

                       if (! request_json.contains ("value"))
                       {
                           LOGD ("No 'value' element found.");
                           res.set_content ("Request must contain value.", "text/plain");
                           res.status = 400;
                           return;
                       }
                       else
                       {
                           LOGD ("Found a value.");
                       }

                       var val;

                       try
                       {
                           auto value = request_json["value"];
                           val = json_to_var (value);
                       }
                       catch (json::exception& e)
                       {
                           res.set_content (e.what(), "text/plain");
                           res.status = 400;
                           return;
                       }

                       //LOGD( "Got value: " << String(val) );

                       if (val.isUndefined())
                       {
                           res.set_content ("Request value could not be converted.", "text/plain");
                           res.status = 400;
                           return;
                       }

                       std::promise<void> parameterChanged;
                       std::future<void> parameterChangedFuture = parameterChanged.get_future();

                       MessageManager::callAsync ([parameter, val, &parameterChanged]
                                                  {
                                                      parameter->setNextValue (val);
                                                      parameterChanged.set_value(); // Signal that parameter has been changed
                                                  });

                       // Wait for parameter to be changed
                       parameterChangedFuture.wait();

                       json ret;
                       parameter_to_json (parameter, &ret);
                       res.set_content (ret.dump(), "application/json");
                   });

        svr_->Put ("/api/quit", [this] (const httplib::Request& req, httplib::Response& res)
                   {
                       MessageManager::callAsync ([this]
                                                  { JUCEApplication::getInstance()->systemRequestedQuit(); });

                       json ret;
                       ret["info"] = "Quitting application";
                       res.set_content (ret.dump(), "application/json");
                   });

        LOGC ("Beginning HTTP server on port ", PORT);
        svr_->listen ("0.0.0.0", PORT);
    }

    void start()
    {
        if (! svr_)
        {
            svr_ = std::make_unique<httplib::Server>();
        }
        startThread();
    }

    void stop()
    {
        if (svr_)
        {
            LOGC ("Shutting down HTTP server");
            svr_->stop();
        }
        stopThread (5000);
    }

private:
    std::unique_ptr<httplib::Server> svr_;
    MainWindow* main_;
    ProcessorGraph* graph_;

    var json_to_var (const json& value)
    {
        if (value.is_number_integer())
        {
            return var (value.get<int>());
        }
        else if (value.is_number_float())
        {
            return var (value.get<double>());
        }
        else if (value.is_boolean())
        {
            return var (value.get<bool>());
        }
        else if (value.is_string())
        {
            return var (value.get<std::string>());
        }
        else if (value.is_array())
        {
            juce::Array<var> vars;
            for (auto& element : value)
            {
                vars.add (element.get<double>());
            }
            return var (vars);
        }
        else
        {
            return var::undefined();
        }
    }

    inline static void audio_devices_to_json (json* ret)
    {
        json devices_json;

        const OwnedArray<AudioIODeviceType>& types = AccessClass::getAudioComponent()->deviceManager.getAvailableDeviceTypes();

        for (int i = 0; i < types.size(); i++)
        {
            std::string type_name = types[i]->getTypeName().toStdString();
            std::vector<std::string> device_names;

            for (int j = 0; j < types[i]->getDeviceNames().size(); j++)
            {
                device_names.push_back (types[i]->getDeviceNames()[j].toStdString());
            }

            devices_json[type_name] = device_names;
        }

        (*ret)["devices"] = devices_json;
    }

    inline static void audio_device_info_to_json (json* ret)
    {
        (*ret)["device_type"] = AccessClass::getAudioComponent()->getDeviceType().toStdString();

        (*ret)["device_name"] = AccessClass::getAudioComponent()->getDeviceName().toStdString();

        (*ret)["sample_rate"] = AccessClass::getAudioComponent()->getSampleRate();

        (*ret)["buffer_size"] = AccessClass::getAudioComponent()->getBufferSize();

        json sample_rates_json;
        sample_rates_to_json (AccessClass::getAudioComponent()->getAvailableSampleRates(), &sample_rates_json);
        (*ret)["available_sample_rates"] = sample_rates_json;

        json buffer_sizes_json;
        buffer_sizes_to_json (AccessClass::getAudioComponent()->getAvailableBufferSizes(), &buffer_sizes_json);
        (*ret)["available_buffer_sizes"] = buffer_sizes_json;
    }

    inline static void sample_rates_to_json (const Array<double> sample_rates, json* ret)
    {
        for (int i = 0; i < sample_rates.size(); i++)
        {
            (*ret)[i] = sample_rates[i];
        }
    }

    inline static void buffer_sizes_to_json (const Array<int> buffer_sizes, json* ret)
    {
        for (int i = 0; i < buffer_sizes.size(); i++)
        {
            (*ret)[i] = buffer_sizes[i];
        }
    }

    inline static void recording_info_to_json (const ProcessorGraph* graph, json* ret)
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
            record_node_to_json (nodeId, &record_node_json);
            record_nodes_json.push_back (record_node_json);
        }

        (*ret)["record_nodes"] = record_nodes_json;
    }

    inline static void record_node_to_json (int nodeId, json* ret)
    {
        (*ret)["node_id"] = nodeId;

        (*ret)["parent_directory"] = CoreServices::RecordNode::getRecordingDirectory (nodeId).getFullPathName().toStdString();

        (*ret)["record_engine"] = CoreServices::RecordNode::getRecordEngineId (nodeId).toStdString();

        (*ret)["experiment_number"] = CoreServices::RecordNode::getExperimentNumber (nodeId);

        (*ret)["recording_number"] = CoreServices::RecordNode::getRecordingNumber (nodeId);

        (*ret)["is_synchronized"] = CoreServices::RecordNode::isSynchronized (nodeId);
    }

    inline static void status_to_json (const ProcessorGraph* graph, json* ret)
    {
        if (CoreServices::getRecordingStatus())
        {
            (*ret)["mode"] = "RECORD";
        }
        else if (CoreServices::getAcquisitionStatus())
        {
            (*ret)["mode"] = "ACQUIRE";
        }
        else
        {
            (*ret)["mode"] = "IDLE";
        }
    }

    inline static void parameter_to_json (Parameter* parameter, json* parameter_json)
    {
        (*parameter_json)["name"] = parameter->getName().toStdString();
        (*parameter_json)["type"] = parameter->getParameterTypeString().toStdString();
        (*parameter_json)["value"] = parameter->getValue().toString().toStdString();
    }

    inline static void parameters_to_json (GenericProcessor* processor, std::vector<json>* parameters_json)
    {
        for (const auto& parameter : processor->getParameters())
        {
            json parameter_json;
            parameter_to_json (parameter, &parameter_json);
            parameters_json->push_back (parameter_json);
        }
    }

    inline static void parameters_to_json (GenericProcessor* processor, uint16 streamId, std::vector<json>* parameters_json)
    {
        for (const auto& parameter : processor->getParameters (streamId))
        {
            json parameter_json;
            parameter_to_json (parameter, &parameter_json);
            parameters_json->push_back (parameter_json);
        }
    }

    inline static void stream_to_json (GenericProcessor* processor, const DataStream* stream, json* stream_json)
    {
        (*stream_json)["name"] = stream->getName().toStdString();
        (*stream_json)["source_id"] = stream->getSourceNodeId();
        (*stream_json)["sample_rate"] = stream->getSampleRate();
        (*stream_json)["channel_count"] = stream->getChannelCount();

        std::vector<json> parameters_json;
        parameters_to_json (processor, stream->getStreamId(), &parameters_json);
        (*stream_json)["parameters"] = parameters_json;
    }

    inline static void processor_to_json (GenericProcessor* processor, json* processor_json)
    {
        (*processor_json)["id"] = processor->getNodeId();
        (*processor_json)["name"] = processor->getName().toStdString();

        std::vector<json> parameters_json;
        parameters_to_json (processor, &parameters_json);
        (*processor_json)["parameters"] = parameters_json;

        std::vector<json> streams_json;

        for (auto stream : processor->getDataStreams())
        {
            json stream_json;
            stream_to_json (processor, stream, &stream_json);
            streams_json.push_back (stream_json);
        }

        (*processor_json)["streams"] = streams_json;

        if (processor->getSourceNode() == nullptr)
        {
            (*processor_json)["predecessor"] = json::value_t::null;
        }
        else
        {
            (*processor_json)["predecessor"] = processor->getSourceNode()->getNodeId();
        }
    }

    inline static void processor_latency_to_json (GenericProcessor* processor, json* processor_json)
    {
        (*processor_json)["id"] = processor->getNodeId();
        (*processor_json)["name"] = processor->getName().toStdString();

        for (const auto& stream : processor->getDataStreams())
        {
            json stream_json;
            stream_latency_to_json (processor, stream, &stream_json);
            (*processor_json)["streams"].push_back (stream_json);
        }
    }

    inline static void stream_latency_to_json (const GenericProcessor* processor, const DataStream* stream, json* stream_json)
    {
        (*stream_json)["name"] = stream->getName().toStdString();
        (*stream_json)["latency"] = processor->getLatency (stream->getStreamId());
    }

    inline GenericProcessor* find_processor (const std::string& id_string)
    {
        int processor_id = juce::String (id_string).getIntValue();
        LOGD ("Searching for processor with ID ", processor_id);

        auto processors = graph_->getListOfProcessors();
        for (const auto& processor : processors)
        {
            if (processor->getNodeId() == processor_id)
            {
                return processor;
            }
        }

        return nullptr;
    }

    inline const DataStream* find_stream (GenericProcessor* p, const std::string& id_string)
    {
        int stream_index = juce::String (id_string).getIntValue();

        if (stream_index < 0)
            return nullptr;

        if (p->getDataStreams().size() > stream_index)
            return p->getDataStreams()[stream_index];

        return nullptr;
    }

    static inline Parameter* find_parameter (GenericProcessor* processor, const std::string& parameter_name)
    {
        Parameter* parameter = processor->getParameter (juce::String (parameter_name));

        if (parameter != nullptr)
        {
            return parameter;
        }

        return nullptr;
    }

    static inline Parameter* find_parameter (GenericProcessor* processor, uint16 streamId, const std::string& parameter_name)
    {
        Parameter* parameter = processor->getDataStream (streamId)->getParameter (parameter_name);

        if (parameter != nullptr)
        {
            return parameter;
        }

        return nullptr;
    }
};

#endif // __PROCESSORGRAPHHTTPSERVER_H_124F8B50__
