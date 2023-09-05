#include "PCH.h"
#include "config.h"
#include "equip_object_override.h"
#include "function_library.h"
#include "logger.h"

void PluginReady(SKSE::MessagingInterface::Message* a_msg) {
    switch (a_msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            EquipObjectOverRide::Install();
            SKSE::log::info("Ultimate Potion Animation NG - Data loaded");
            break;
        default:
            break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SetupLog();
    auto settings = config::ReadSettings();
    config::iMovementType = settings["iMovementType"];
    config::bEnableNotifications = settings["bEnableNotifications"] != 0;
    SKSE::GetMessagingInterface()->RegisterListener(PluginReady);
    return true;
}