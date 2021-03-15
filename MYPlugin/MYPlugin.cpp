/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

//I have put several "TODO(MROLLINS)" in the code below to mark areas of concern I encountered
//  when refactoring the servicemanager's version of displaysettings into this new thunder plugin format



#include "MYPlugin.h"
//#include "tracing/Logging.h"


#include <algorithm>
#include "dsMgr.h"
#include "libIBusDaemon.h"
#include "host.hpp"
#include "exception.hpp"
#include "videoOutputPort.hpp"
#include "videoOutputPortType.hpp"
#include "videoOutputPortConfig.hpp"
#include "videoResolution.hpp"
#include "audioOutputPort.hpp"
#include "audioOutputPortType.hpp"
#include "audioOutputPortConfig.hpp"
#include "manager.hpp"
#include "dsUtl.h"
#include "dsError.h"
#include "list.hpp"
#include "libIBus.h"
#include "dsDisplay.h"
#include "rdk/iarmmgrs-hal/pwrMgr.h"

#include "tracing/Logging.h"
#include <syscall.h>
#include "utils.h"

#define HDMI_HOT_PLUG_EVENT_CONNECTED 0



using namespace std;


namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(MYPlugin, 1, 0);

        MYPlugin* MYPlugin::_instance = nullptr;

        MYPlugin::MYPlugin()
            : AbstractPlugin()
        {
            LOGINFO("ctor");
            MYPlugin::_instance = this;

            registerMethod("getMYPluginStatus", &MYPlugin::getMYPluginStatus, this);
            registerMethod("getMYPluginList", &MYPlugin::getMYPluginList, this);
            registerMethod("getMYPluginInfo", &MYPlugin::getMYPluginInfo, this);
            registerMethod("getConnectedVideoDisplays", &MYPlugin::getConnectedVideoDisplays, this);


        }

        MYPlugin::~MYPlugin()
        {
            LOGINFO("dtor");
            MYPlugin::_instance = nullptr;
        }

        const string MYPlugin::Initialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();

	    InitializeIARM();

            return (string());
        }

        void MYPlugin::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
	    DeinitializeIARM();

        }

        void MYPlugin::InitializeIARM()
        {
            LOGINFO();

            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
            }

            try
            {
                //TODO(MROLLINS) this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
                device::Manager::Initialize();
                LOGINFO("device::Manager::Initialize success");
            }
            catch(...)
            {
                LOGINFO("device::Manager::Initialize failed");
            }
        }


        void MYPlugin::DeinitializeIARM()
        {
            LOGINFO();

            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG) );
            }

            try
            {
                //TODO(MROLLINS) this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
                device::Manager::DeInitialize();
                LOGINFO("device::Manager::DeInitialize success");
            }
            catch(...)
            {
                LOGINFO("device::Manager::DeInitialize failed");
            }
        }


//////////////////////////////////////////////////////////////////////////////////////////////////////
        void MYPlugin::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            LOGINFO();
            switch (eventId)
            {
            case IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG :
                //TODO(MROLLINS) note that there are several services listening for the notifyHdmiHotPlugEvent ServiceManagerNotifier broadcast
                //So if DisplaySettings becomes the owner/originator of this, then those future thunder plugins need to listen to our event
                //But of course, nothing is stopping any thunder plugin for listening to iarm event directly -- this is getting murky
                {
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    int hdmi_hotplug_event = eventData->data.hdmi_hpd.event;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d ", hdmi_hotplug_event);
                    if(MYPlugin::_instance)
                        MYPlugin::_instance->connectedVideoDisplaysUpdated(hdmi_hotplug_event);
                }
                break;
            default:
                //do nothing
                break;
            }
        }



        void setResponseArray(JsonObject& response, const char* key, const vector<string>& items)
        {
            JsonArray arr;
            for(auto& i : items) arr.Add(JsonValue(i));

            response[key] = arr;

            string json;
            response.ToString(json);
        }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
       

        uint32_t MYPlugin::getConnectedVideoDisplays(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response: {"connectedVideoDisplays":["HDMI0"],"success":true}
            //this                          : {"connectedVideoDisplays":["HDMI0"]}
            LOGINFOMETHOD();

            vector<string> connectedVideoDisplays;
            getConnectedVideoDisplaysHelper(connectedVideoDisplays);
            setResponseArray(response, "connectedVideoDisplays", connectedVideoDisplays);
            returnResponse(true);
        }


        void MYPlugin::getConnectedVideoDisplaysHelper(vector<string>& connectedDisplays)
        {
            LOGINFO();
            try
            {
                device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                for (size_t i = 0; i < vPorts.size(); i++)
                {
                    device::VideoOutputPort &vPort = vPorts.at(i);
                    if (vPort.isDisplayConnected())
                    {
                        string displayName = vPort.getName();
                        if (strncasecmp(displayName.c_str(), "hdmi", 4)==0)
                        {
                            connectedDisplays.clear();
                            connectedDisplays.emplace_back(displayName);
                            break;
                        }
                        else
                        {
                            vectorSet(connectedDisplays, displayName);
                        }
                    }
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }
        }




	 //Begin methods


	uint32_t MYPlugin::getMYPluginStatus(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response: {"connection status from plugin":["CONNECTED"],"success":true}i
            //this                          : {connection status from plugin":["CONNECTED"]}
            LOGINFOMETHOD();

            vector<string> plug_status;
	    plug_status.push_back("CONNECTED");
            setResponseArray(response, "connection status from plugin", plug_status);
            returnResponse(true);
        }
 
 
 	uint32_t MYPlugin::getMYPluginList(const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:{"success":true,"supportedSettopResolutions":["720p","1080i","1080p60"]}
                LOGINFOMETHOD();
                vector<string> List_plugin;
		List_plugin.push_back ("plug-A");
		List_plugin.push_back ("plug-B");
		List_plugin.push_back ("plug-C");
		List_plugin.push_back ("plug-D");
		List_plugin.push_back ("plug-E");
		
		setResponseArray(response, "supportedSettopResolutions", List_plugin);
		returnResponse(true);
	}

	uint32_t MYPlugin::getMYPluginInfo(const JsonObject& parameters, JsonObject& response)
	{   //sample servicemanager response:{"success":true,"supportedTvResolutions":["480i","480p","576i","720p","1080i","10     80p"]}
 	LOGINFOMETHOD();
  	string videoDisplay = parameters.HasLabel("plugin_name") ? parameters["plugin_name"].String() : "plug-A";
  	vector<string> plugin_info;
		plugin_info.push_back ("xyz-plugin");
		plugin_info.push_back ("no:430HT5");
            setResponseArray(response, "supportedTvResolutions", plugin_info);
	    returnResponse(true);
         }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////


        void MYPlugin::connectedVideoDisplaysUpdated(int hdmiHotPlugEvent)
        {
            LOGINFO();
            static int previousStatus = HDMI_HOT_PLUG_EVENT_CONNECTED;
            static int firstTime = 1;

            if (firstTime || previousStatus != hdmiHotPlugEvent)
            {
                firstTime = 0;
                JsonArray connectedDisplays;
                if (HDMI_HOT_PLUG_EVENT_CONNECTED == hdmiHotPlugEvent)
                {
                    connectedDisplays.Add("HDMI0");
                }
                else
                {
                    /* notify Empty list on HDMI-output-disconnect hotplug */
                }

                JsonObject params;
                params["connectedVideoDisplays"] = connectedDisplays;
                sendNotify("connectedVideoDisplaysUpdated", params);
            }
            previousStatus = hdmiHotPlugEvent;
        }

/////////////////////////////////////////////////////////////////////////////////////////////


    } // namespace Plugin
} // namespace WPEFramework
