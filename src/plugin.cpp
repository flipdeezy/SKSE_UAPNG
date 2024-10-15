#include "PCH.h"
#include "config.h"
#include "equip_object_override.h"
#include "logger.h"
#include "animation_event_sink.h"


void PluginReady(SKSE::MessagingInterface::Message* a_msg) {
    switch (a_msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            EquipObjectOverRide::Install();
            logger::info("Ultimate Potion Animation NG - Plugin Ready");            
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
            {
                pause.store(true);

                auto settings = config::ReadSettings();
                config::iMovementType = settings["iMovementType"];
                config::bEnableNotifications = settings["bEnableNotifications"] != 0;
                config::bEnableSpamPotions = settings["bEnableSpamPotions"] != 0;

                for (auto& eventSink : actorEventSink | std::views::values) {
                    CleanupAnimationEvent(eventSink.get(), true);
                }

                actorEventSink.clear();
                
                logger::info("Ultimate Potion Animation NG - Post Load Game");
                logger::info("Settings loaded: iMovementType: {}, bEnableNotifications: {}, bEnableSpamPotions: {}", config::iMovementType, config::bEnableNotifications, config::bEnableSpamPotions);

                pause.store(false);
            }
            break;
        default:
            break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SetupLog();
    SKSE::GetMessagingInterface()->RegisterListener(PluginReady);
    return true;
}