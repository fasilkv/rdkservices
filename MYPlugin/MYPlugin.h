
#pragma once

#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"
#include "libIBus.h"
#include "irMgr.h"

namespace WPEFramework {

    namespace Plugin {

		// This is a server for a JSONRPC communication channel.
		// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
		// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
		// This realization of this interface implements, by default, the following methods on this plugin
		// - exists
		// - register
		// - unregister
		// Any other methood to be handled by this plugin  can be added can be added by using the
		// templated methods Register on the PluginHost::JSONRPC class.
		// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
		// this class exposes a public method called, Notify(), using this methods, all subscribed clients
		// will receive a JSONRPC message as a notification, in case this method is called.
        class MYPlugin : public AbstractPlugin {
        private:
            typedef Core::JSON::String JString;
            typedef Core::JSON::ArrayType<JString> JStringArray;
            typedef Core::JSON::Boolean JBool;

            // We do not allow this plugin to be copied !!
//            DisplaySettings(const DisplaySettings&) = delete;
//            DisplaySettings& operator=(const DisplaySettings&) = delete;

            //Begin methods
//////////////////////////////////////////
            uint32_t getMYPluginStatus(const JsonObject& parameters, JsonObject& response);
            uint32_t getMYPluginList(const JsonObject& parameters, JsonObject& response);
	    uint32_t getMYPluginInfo(const JsonObject& parameters, JsonObject& response);
//////////////////////////////////////////

        public:
            MYPlugin();
            virtual ~MYPlugin();
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
        public:
            static MYPlugin* _instance;

        };
	} // namespace Plugin
} // namespace WPEFramework
