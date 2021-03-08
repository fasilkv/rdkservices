//  when refactoring the servicemanager's version of displaysettings into this new thunder plugin format



#include "MYPlugin.h"
#include "tracing/Logging.h"


using namespace std;



namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(MYPlugin, 1, 0);            //sevice registration

        MYPlugin* MYPlugin::_instance = nullptr;

        MYPlugin::MYPlugin()
            : AbstractPlugin()
        {
            LOGINFO("ctor");
            MYPlugin::_instance = this;
	    
	    //register methodes
            registerMethod("getMYPluginStatus", &MYPlugin::getMYPluginStatus, this);
            registerMethod("getMYPluginList", &MYPlugin::getMYPluginList, this);
            registerMethod("getMYPluginInfo", &MYPlugin::getMYPluginInfo, this);
        }

        MYPlugin::~MYPlugin()
        {
            LOGINFO("dtor");
            MYPlugin::_instance = nullptr;
        }
	
	
	//initialize and deinitialize plugin
        const string MYPlugin::Initialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
            return (string());
        }

        void MYPlugin::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
        }


        void setResponseArray(JsonObject& response, const char* key, const vector<string>& items)
        {
            JsonArray arr;
            for(auto& i : items) arr.Add(JsonValue(i));

            response[key] = arr;

            string json;
            response.ToString(json);
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
        {   ///sample servicemanager response: {"jsonrpc":"2.0","id":3,"result":{"Supported plugin list":["plug-A","plug-B","plug-C","plug-D","plug-E"],"success":true}}
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
	{   //sample servicemanager response:{"success":true,"plugin info ":["xyz-plugin","ID:430HT5"]}
 	LOGINFOMETHOD();
  	string videoDisplay = parameters.HasLabel("plugin_name") ? parameters["plugin_name"].String() : "plug-A";
  	vector<string> plugin_info;
		plugin_info.push_back ("xyz-plugin");
		plugin_info.push_back ("ID:430HT5");
            setResponseArray(response, "plugin info", plugin_info);
	    returnResponse(true);
         }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    } // namespace Plugin
} // namespace WPEFramework
