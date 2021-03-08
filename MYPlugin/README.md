Steps involved in implementing new RDK services Plug-In
--------------------------------------------------------

 reference: https://wiki.rdkcentral.com/display/RDK/How+to+Make+a+Thunder+Nano+Service


 In RDK services -plugins workspace:
	(Cloned from https://github.com/rdkcentral/rdkservices)
 or 
	cd build-raspberrypi-rdk-mc/tmp/work/cortexa7t2hf-neon-vfpv4-rdk-linux-gnueabi/rdkservices/3.0+gitAUTOINC+1180f1ee5b-r1/git/

 Create Plugin-Name folder
	Eg: mkdir MYPlugin

Inside PluginName directory:
----------------------------
 1.	<PluginName>.json : This file contains the plugin's information like schema, information and interface json file
	eg: 	MYPlugin.json
	DeviceInfo.json
 2.	CmakeLists.txt:  To compile the Plug-in code and to generate the shared library (“.so”)
	This will handle all the dependencies as well
 3.	Module.h: This header file includes the support for JSON request, response, logging etc.,
 4.	Module.cpp: This file is used to declare the module name for the Plug-in
	This file contains the plugin's information like schema, information and interface json file (defined earlier)
 5.	<PluginName>.config: This file is used to set configurations of the Plug-in 
	Ex:- set (autostart true) - Used to make the Plug-in to start automatically along with wpeframework daemon
 6.	<PluginName>.h :Declare the plugin class in this which should contains all the structures, variables and methods which are needed for plugin implementation. The interface header auto-generated earlier will be used here,\
 7.	<PluginName>.cpp: This class does contains all the definitions for the methods declared in the Plugin.h and those definitions should be defined inside the below namespace.
 8.	Cmake / (directory) :

		MYPlugin/
		├── CMakeLists.txt
		├── MYPlugin.config
		├── MYPlugin.cpp
		├── MYPlugin.h
		├── MYPlugin.json
		├── Module.cpp
		├── Module.h
		├── README.md
		├── cmake
		|   ├── FindDS.cmake
		│   └── FindIARMBus.cmake
		└── doc
			└── MYPlugin.md

<PluginName>.json :
-------------------
 This file contain plugin information.
	
 Syntax:
			{
			  "$schema": "plugin.schema.json",
			  "info": {
				"title": "Plugin Name Plugin",
				"callsign": "PluginName",
				"locator": "libWPEFrameworkPluginName.so",
				"status": "production",
				"description": "The PluginName plugin allows retrieving of various plugin-related information.",
				"version": "1.0"
			  },
			  "interface": {
				"$ref": "{interfacedir}/PluginName.json#"
			  }
			}
			
 eg:
 MYPlugin.json
				{
				 "locator":"libWPEFrameworkMYPlugin.so",
				 "classname":"MYPlugin",
				 "precondition":[
				  "Platform"
				 ],
				 "callsign":"org.rdk.MYPlugin",
				 "autostart":false
				}


<PluginName>.config:
--------------------
 This file initial configuration setting.

 eg: MYPlugin.config
			set (autostart false)
			set (preconditions Platform)
			set (callsign "org.rdk.MYPlugin")
			

<PluginName>.h
---------------
 Declare the plugin class in this which should contain all the structures, variables, and methods which 
 are needed for plugin implementation. 

	namespace WPEFramework {
		namespace Plugin {
		 
			class PluginName : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
			public:
					PluginName()
						: _skipURL(0)
						, _service(nullptr)
						, _subSystem(nullptr)
					{
						RegisterAll();
					}
	 
					virtual ~PluginName()
					{
						UnregisterAll();
					}
			} 


 eg: MYPlugin.h
 for more information refer MYPlugin.h 

<PluginName>.cpp:
----------------- 
 This class does contain all the definitions for the methods declared in the Plugin.h and those definitions 
 should be defined inside the below namespace. 

 The plugin should register using service registration MACRO as declared below:

		namespace WPEFramework {
			namespace Plugin {  
				SERVICE_REGISTRATION(Plugin, 1, 0);
				---------------------------------------
				---------------------------------------
				---------------------------------------
			}
		} 
		
 To initialize and deinitialize or activate or deactivate handler for the plugin services:

        const string MYPlugin::Initialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
            return (string());
        }

        void MYPlugin::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
        }
		
 eg:
		namespace WPEFramework {
			namespace Plugin {  
				SERVICE_REGISTRATION(Plugin, 1, 0);
				
				//registration
				//All the methods declared in Plugin.h should be registered here
				
				//initialize and deinitialize the handlers for the plug-in service
				
				//All the methods declared in Plugin.h should be defined here
				
				  
			}
		} 


CMakeLists.txt
--------------
 using the CMake utility this file contains the task needed to be done to make a plug-in.
 this file contains packages, libraries needed to compile, its path, and other plugin-in configuration option



code control Flow:
------------------
 1) Enable or disable the plug-in flag in the recipe file
 2) add this flag into the main CMakeLists.txt file present in the rdkservices
 3) it will invoke CMakeLists file present in the <plugin name>/ (eg: MYPlugin/CMakeLists.txt)
 4) When this file started to execute it finds dependencies, packages. it compiled and generate .so file


To add Plugin in the recipe file :
----------------------------
 Open rdkservices recepi file
	vi meta-rdk-video/recipes-extended/rdkservices/rdkservices_git.bb

 add below line in between other plug-in flags

	"  PACKAGECONFIG[myplugin]      = "-DPLUGIN_MYPLUGIN=OFF,-DPLUGIN_MYPLUGIN=ON, " 
	
To add plugin in rdkservices CMakeLists.txt:
-------------------------------------------
 Open rdkservises directory 
	cd build-raspberrypi-rdk-mc/tmp/work/cortexa7t2hf-neon-vfpv4-rdk-linux-gnueabi/rdkservices/3.0+gitAUTOINC+1180f1ee5b-r1/git/
 open CmakeLists.txt
	vi CmakeLists.txt
	
 add these lines :

		  if(PLUGIN_MYPLUGIN)
			  add_subdirectory(MYPlugin)
		  endif()

 it will invoke your CMakeLists.txt file present in your plugin directory. 

Compilation and Install:
------------------------
 Enable the plugin in the main CMakeLists.txt of rdkservices-plugins

 to compile and install:

	bitbake -c compile -f rdkservices

 once build complete copy .json, .so file into raspberrypi
	to copy .json file:
		Copy the Plugin.json(eg: MYPlugin.json) file to “/etc/WPEFramework/plugins” in raspberrypi
	to copy .so file:
		libWPEFrameworkPlugin.so file present in the build directory.
		build-raspberrypi-rdk-mc/tmp/work/cortexa7t2hf-neon-vfpv4-rdk-linux-gnueabi/rdkservices/3.0+gitAUTOINC+1180f1ee5b-r1/build/MYPlugin
		Copy the plugin library (libWPEFrameworkPlugin.so) to “/usr/lib/wpeframework/plugins” 
	
 so that the controller plugin identify it and list it in the WebUI (controller UI ) 



MYPlugin API :
------------
 Each RDK Service can be validated through JSON RPC Commands through HTTP.  
 It has a request and response in JSON format. 
 Note: the argument is case sensitive
 "callsign":"org.rdk.MYPlugin"

 Activate and deactivate API: 
 Any service can be enabled or disabled also through JSONRPC commands. 

 activate: 

	curl http://localhost:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{ "jsonrpc":"2.0", "id":3, "method":"Controller.1.activate", "params":{"callsign":"org.rdk.MYPlugin"} }'  
	{"jsonrpc":"2.0","id":3,"result":{"success":true}}  
	callsign argument can accept all plugins 

 deactivate:  

	curl http://localhost:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{ "jsonrpc":"2.0", "id":3, "method":"Controller.1.deactivate", "params":{"callsign":"org.rdk.MYPlugin"} }'  
	{"jsonrpc":"2.0","id":3,"result":{"success":true}} 


 Other APIs
 ----------- 

 getPluginStatus :
 
	curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.MYPlugin.1.getMYPluginStatus"}' 
	{"jsonrpc":"2.0","id":3,"result":{"connection status from plugin":["CONNECTED"],"success":true}} 

 getMYPluginList :

	curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.MYPlugin.1.getMYPluginList"}' 
	{"jsonrpc":"2.0","id":3,"result":{"Supported plugin list":["plug-A","plug-B","plug-C","plug-D","plug-E"],"success":true}} 

 getMYPluginInfo : 

	curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.MYPlugin.1.getMYPluginInfo", "params":{"videoDisplay//plugin_name":"plug-A"}}' 

Controller UI :
-----------------
 Controller UI is a web UI that can be launched from a host machine's (machine under the same network where RPi resides) 
 browser. This UI can be loaded with the RPi box's IP address with port number 9998.
	
	URL: http://<Rpi's IP address>:9998

 In the Controller tab, any plugins can be enabled or disabled. On loading the controller UI initially,
 it loads the Controller tab with all possible services with the enable or disable status.