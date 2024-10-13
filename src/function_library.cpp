#include "actor_data.h"
#include "animation_event_sink.h"
#include "config.h"
#include "function_library.h"
#include "logger.h"
#include <filesystem>
#include <string>
#include <vector>

void SendDebugNotification(const char* message, const char* soundToPlay, bool cancelIfAlreadyQueued) {
    if (config::bEnableNotifications) {
        RE::DebugNotification(message, soundToPlay, cancelIfAlreadyQueued);
    }   
}

void PlaySound(int soundID) {
    const auto soundFormID = static_cast<RE::FormID>(soundID);
    if (auto* audio = RE::BSAudioManager::GetSingleton()) {
        audio->Play(soundFormID);
    }
}

bool IsPlayerInFirstPerson() {
    if (const auto* playerCamera = RE::PlayerCamera::GetSingleton()) {
        return playerCamera->IsInFirstPerson();
    }
    return false;
}

void UpdateNode(RE::NiNode* node, SKSE::stl::enumeration<RE::NiUpdateData::Flag, std::uint32_t> flags,
                float updateTime) {
    auto updateData = RE::NiUpdateData();
    updateData.flags = flags;
    updateData.time = updateTime;
    node->Update(updateData);
}

void ScaleNode(RE::Actor* a_actor) {
    // Access the First-Person Node directly using Get3D
    RE::NiNode* firstpersonNode = nullptr;
    if (const auto root3D = a_actor->Get3D(1)) {
        firstpersonNode = root3D->AsNode();
    }

    if (!firstpersonNode) {
        logger::info("Failed to find the first-person node.");
        return;
    }

    // Access the Arm Nodes
    RE::NiAVObject* leftArmAVObj = firstpersonNode->GetObjectByName("NPC L UpperArm [LUar]");
    RE::NiAVObject* rightArmAVObj = firstpersonNode->GetObjectByName("NPC R UpperArm [RUar]");

    RE::NiNode* leftarmNode = leftArmAVObj ? leftArmAVObj->AsNode() : nullptr;
    RE::NiNode* rightarmNode = rightArmAVObj ? rightArmAVObj->AsNode() : nullptr;

    // Check if the arm nodes exist
    if (!leftarmNode || !rightarmNode) {
        logger::info("Failed to find the arm nodes.");
        return;
    }

    logger::info("Successfully found the arm nodes.");

    // Adjust the Scale
    leftarmNode->local.scale = 1.0f;  // 0.001f to hide
    rightarmNode->local.scale = 1.0f;
    UpdateNode(leftarmNode);
    UpdateNode(rightarmNode);
}

void CloseMenu() {
    auto* uiManager = RE::UI::GetSingleton();

    if (const auto inventoryMenu = uiManager->GetMenu<RE::InventoryMenu>()) {
        RE::UIMessageQueue::GetSingleton()->AddMessage("InventoryMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
        RE::UIMessageQueue::GetSingleton()->AddMessage("TweenMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }

    if (const auto favoritesMenu = uiManager->GetMenu<RE::FavoritesMenu>()) {
        RE::UIMessageQueue::GetSingleton()->AddMessage("FavoritesMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }

    if (const auto containerMenu = uiManager->GetMenu<RE::ContainerMenu>()) {
        RE::UIMessageQueue::GetSingleton()->AddMessage("ContainerMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
}

std::vector<std::string> GetMagicEffectNames(RE::TESBoundObject* a_object) {
    std::vector<std::string> effectNames;
    const auto* alchemyItem = skyrim_cast<RE::AlchemyItem*>(a_object);
    for (RE::BSTArray<RE::Effect*> effects = alchemyItem->effects; const auto* effect : effects) {
        if (effect) {
            effectNames.emplace_back(effect->baseEffect->GetFullName());
        }
    }
    return effectNames;
}

void SendAnimationEvent(RE::Actor* a_actor, const char* AnimationString) {
    if (const auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor)) {
        animGraphHolder->NotifyAnimationGraph(AnimationString);
        logger::info("Sending animation event: '{}' Actor: '{}'", AnimationString,
                      a_actor->GetName());
    } else {
        logger::error("Failed to get animGraphHolder for Actor: '{}'",
                      a_actor->GetName());
    }
}

int GetAnimationVariableInt(RE::Actor* a_actor, const std::string& variableName) {
    int value = 0;
    if (const auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor)) {
        animGraphHolder->GetGraphVariableInt(variableName.c_str(), value);
    } else {
        logger::error("Failed to get animGraphHolder for Actor: '{}'", a_actor->GetName());
    }
    return value;
}

void SetAnimationVariableInt(RE::Actor* a_actor, const char* variableName, int value) {
    if (const auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor)) {
        if (animGraphHolder->SetGraphVariableInt(variableName, value)) {
            logger::info("Setting animation variable:'{}' Value: '{}' Actor: '{}'", variableName, value, a_actor->GetName());
        } else {
            logger::error("Failed to set animation variable: '{}' Value: '{}' Actor: '{}'", variableName, value, a_actor->GetName());
        }
    } else {
        logger::error("Failed to get animGraphHolder for Actor: '{}'", a_actor->GetName());
    }
}

bool GetAnimationVariableBool(RE::Actor* a_actor, const std::string& variableName) {
    bool value = false;
    if (const auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor)) {
        animGraphHolder->GetGraphVariableBool(variableName.c_str(), value);
    } else {
        logger::error("Failed to get animGraphHolder for Actor: '{}'",
                      a_actor->GetName());
    }
    return value;
}

void SetAnimationVariableBool(RE::Actor* a_actor, const char* variableName, bool value) {
    if (const auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor)) {
        animGraphHolder->SetGraphVariableBool(variableName, value);
        logger::info("Setting animation variable: '{}' Value: '{}' Actor: '{}'",
                      variableName, value ? "true" : "false", a_actor->GetName());
    } else {
        logger::error("Failed to get animGraphHolder for Actor: '{}'",
                      a_actor->GetName());
    }
}

void HideWeapon(RE::Actor* a_actor, bool a_cull) {
    const RE::BSFixedString nodeName("Weapon");
    if (const auto rootNode = a_actor->GetNodeByName(nodeName)) {
        rootNode->CullNode(a_cull);
    }
}

RE::TESObjectWEAP* CheckActorWeaponEquipped(const RE::Actor* a_actor) {
    if (RE::TESForm* equippedObject = a_actor->GetEquippedObject(false); equippedObject && equippedObject->Is(RE::FormType::Weapon)) {
        RE::TESObjectWEAP* weapon = skyrim_cast<RE::TESObjectWEAP*>(equippedObject);
        logger::info("EquippedObject: {}", weapon->GetName());
        return weapon;
    }
    return nullptr;
}

bool CheckKeyword(RE::FormID a_formID, const std::variant<RE::TESBoundObject*, RE::TESRace*>& a_form) {
    if (RE::TESForm* form = RE::BGSKeyword::LookupByID(a_formID)) {
        RE::BGSKeyword* myKeyword = skyrim_cast<RE::BGSKeyword*>(form);
        std::vector<RE::BGSKeyword*> keywords = {myKeyword};

        if (const auto object = std::get_if<RE::TESBoundObject*>(&a_form)) {
            return (*object)->HasKeywordInArray(keywords, false);
        } else if (const auto actor = std::get_if<RE::TESRace*>(&a_form)) {
            return (*actor)->HasKeywordInArray(keywords, false);
        }
    }
    return false;
}

bool GetSwimming(RE::Actor* a_actor) {
    if (a_actor->AsActorState()->IsSwimming()) {
        return true;
    } else {
        return false;
    }
}

bool CheckActorAndObjectKeywords(RE::TESBoundObject* a_object, RE::Actor* a_actor) {
    return CheckKeyword(0x08CDEC, a_object) && CheckKeyword(0x013794, a_actor->GetActorBase()->GetRace());
}

bool IsObjectAPotion(RE::TESBoundObject* a_object) {
    if (!a_object) {
        return false;
    }
    const auto alchemyItem = skyrim_cast<RE::AlchemyItem*>(a_object);
    if (!alchemyItem) {
        return false; 
    }

    //return !alchemyItem->IsFood() && (alchemyItem->IsPoison() || alchemyItem->IsMedicine());
    std::string objectName = alchemyItem->GetName();
    std::ranges::transform(objectName, objectName.begin(),
                           [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return objectName.find("potion") != std::string::npos;
}

std::vector<std::string> ReadEditorIDsFromINISection(const std::string& section) {
    const std::string iniFilePath = config::GetIniFilePath();
    char buffer[4096];
    GetPrivateProfileSectionA(section.c_str(), buffer, sizeof(buffer), iniFilePath.c_str());

    std::vector<std::string> editorIDs;
    char* ptr = buffer;
    while (*ptr != '\0') {
        std::string line(ptr);
        std::string editorID = line.substr(0, line.find(';'));
        std::erase_if(editorID, [](const unsigned char c) { return std::isspace(c); });
        if (!editorID.empty()) {
            std::ranges::transform(editorID, editorID.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            editorIDs.push_back(editorID);
        }
        ptr += line.size() + 1;
    }
    return editorIDs;
}

std::vector<std::string> ReadAllowedPotionsFromINI() {
    return ReadEditorIDsFromINISection("AllowedPotions");
}

std::vector<std::string> ReadBlacklistedPotionsFromINI() {
    return ReadEditorIDsFromINISection("BlacklistedPotions");
}

bool IsObjectBlacklisted(RE::TESBoundObject* a_object) {
    static const auto blacklistedEditorIDs = ReadBlacklistedPotionsFromINI();
    const auto& objectFormID = a_object->GetFormID();
    bool found = false;
    for (const auto& editorID : blacklistedEditorIDs) {
        if (const auto iniObject = RE::TESForm::LookupByEditorID(editorID); iniObject && iniObject->GetFormID() == objectFormID) {
            found = true;
            break;
        }
    }
    if (found){
        logger::info("-----------------Checking black list--------------------------");
        logger::info("Potion: {} found in BlacklistedPotions", a_object->GetName());
    }
    return found;
}

bool IsObjectWhitelisted(RE::TESBoundObject* a_object) {
    if (a_object->GetFormType() != RE::FormType::AlchemyItem) {
        return false;
    }
    static const auto allowedEditorIDs = ReadAllowedPotionsFromINI();
    const auto& objectFormID = a_object->GetFormID();
    bool found = false;
    for (const auto& editorID : allowedEditorIDs) {
        if (const auto iniObject = RE::TESForm::LookupByEditorID(editorID); iniObject && iniObject->GetFormID() == objectFormID) {
            found = true;
            break;
        }
    }
    return found;
}

bool IsActorReadyForAnimation(RE::Actor* a_actor) {
    const int currentDefaultState = GetAnimationVariableInt(a_actor, "currentDefaultState");
    const bool isPaired = GetAnimationVariableBool(a_actor, "bIsSynced");
    const auto actor_custom = ActorCustom(a_actor);
    return (!actorDataMap.contains(actor_custom) || !actorDataMap[actor_custom].bAnimationInProgress.load()) &&
           (currentDefaultState < 10 || currentDefaultState > 20) && !isPaired && !a_actor->IsOnMount() &&
           !a_actor->IsInMidair() && !GetSwimming(a_actor);
}

void NotifyActorDrinkingLastPotion(RE::Actor* currentActor, const RE::TESBoundObject* lastEquippedObject) {
    if (config::bEnableNotifications && currentActor->GetActorBase()->IsPlayer() && lastEquippedObject) {
        std::string message;
        message += currentActor->GetName();
        message += " is currently drinking ";
        message += lastEquippedObject->GetName();
        SendDebugNotification(message.c_str());
    }
}

void LogActorData(RE::Actor* a_actor) {
    if (const auto actor_custom = ActorCustom(a_actor); actorDataMap.contains(actor_custom)) {
        const ActorData& data = actorDataMap[actor_custom];
        logger::info("Ultimate Potion Animation NG - lastActor: {}", data.lastActor->GetName());
        logger::info("Ultimate Potion Animation NG - lastObject: {}", data.lastObject->GetName());
        logger::info("Ultimate Potion Animation NG - bDrinking: {}", data.bDrinking.load());
        logger::info("Ultimate Potion Animation NG - bAnimationInProgress: {}", data.bAnimationInProgress.load());
    }
}

void EquipPotion(RE::Actor* a_actor, RE::ActorEquipManager* a_self, RE::TESBoundObject* a_object, std::uint64_t a_unk) {
    logger::info("-----------------Logging Potion Equip event-------------------");
    logger::info("Equipping potion: {} Actor: '{}'", a_object->GetName(), a_actor->GetName());
    if (const RE::AlchemyItem* a_potion = a_object->As<RE::AlchemyItem>()) {        
        AnimationEventSink* eventSink = GetOrCreateEventSink(a_actor, a_self, a_object, a_unk);
        if (a_actor->GetActorBase()->IsPlayer()) {
            PlaySound(0x03C7BA);
        }
        a_actor->AddAnimationGraphEventSink(eventSink);
        SendAnimationEvent(eventSink->a_actor.get(), "HeadtrackingOff");
        SetAnimationVariableBool(eventSink->a_actor.get(), "tdmHeadtrackingBehavior", false);
        SetAnimationVariableBool(eventSink->a_actor.get(), "bPlayEquipSound", true);

        switch (config::iMovementType) {
            case 0:
                SendAnimationEvent(a_actor, "DrinkPotionStart");
                break;
            case 1:
                SendAnimationEvent(a_actor, "DrinkPotion_Slow");
                break;
            case 2:
                SendAnimationEvent(a_actor, "DrinkPotion_NoMove");
                break;
            default:
                logger::error("Invalid movement type value: {}", config::iMovementType);
                break;
        }
        HideWeapon(a_actor, true);
    } else {
        logger::error("ERROR ++++++++++++++++ Failed to cast TESBoundObject to AlchemyItem for actor '{}'", a_actor->GetName());
    }
}

std::string AlchObjectGetModelPath(RE::TESBoundObject* a_object) {
    if (a_object) {
        if (const RE::AlchemyItem* alchItem = a_object->As<RE::AlchemyItem>()) {
            std::string modelPath = alchItem->GetModel();
            if (const std::size_t found = modelPath.find_last_of("/\\"); found != std::string::npos) {
                modelPath = modelPath.substr(found + 1);
            }
            return "AnimObjects\\" + modelPath;
        }
    }
    return "null";
}

std::string GetGameDataPath() { return "Data\\meshes\\"; }

std::string ConvertToAbsolutePath(const std::string& relativePath) {
    std::string gameDataPath = GetGameDataPath();
    return gameDataPath + relativePath;
}

bool DoesFileExist(const std::string& filePath) { return std::filesystem::exists(filePath); }

void AnimObjectReplaceModelByFormID(RE::FormID a_formID, const char* newModelPath) {
    auto* form = RE::TESObjectANIO::LookupByID(a_formID);
    if (form) {
        RE::TESObjectANIO* animObject = static_cast<RE::TESObjectANIO*>(form);
        RE::TESModel* model = static_cast<RE::TESModel*>(animObject);
        if (model) {
            model->SetModel(newModelPath);
            const char* modelPath = model->GetModel();
            std::string absoluteModelPath = ConvertToAbsolutePath(modelPath);
            if (!DoesFileExist(absoluteModelPath)) {
                logger::info("-----------------Logging AnimObject Path----------------------");
                logger::info("Updated AnimObject model path does not exist: {}", absoluteModelPath);
                model->SetModel("Animobjects\\AnimObjectDrinkPotion.nif");
            }
        }
    }
}