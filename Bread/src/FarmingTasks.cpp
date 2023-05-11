#include "FarmingTasks.hpp"
#include "AdvancedTasks.hpp"
#include "botcraft/AI/Tasks/InventoryTasks.hpp"
#include "botcraft/Game/Entities/entities/projectile/FishingHookEntity.hpp"
#include "botcraft/Network/NetworkManager.hpp"

const vector<string> need_fishing_rod_messages({"Yo, hook me up with a fishing rod, cuz I ain't catchin' nothin' with my bare hands.",
                                                "Listen up, homie, I need a fishing rod like a fish needs water.",
                                                "Hey, boss, can you toss me a fishing rod? I'm tryna reel in some big fish, you feel me?",
                                                "Ayo, let me get a rod, I'm tryna get my fish on, ya dig?",
                                                "Yo, I need a rod like a rapper needs a mic, it's my tool for success.",
                                                "I'm like a fish outta water without a fishing rod, you gotta help me out.",
                                                "Can't be a boss without a fishing rod, it's my key to the sea.",
                                                "Hey, can you lend me a fishing rod? I promise to return it in one piece, unless I catch a big one.",
                                                "Gimme a fishing rod and watch me work the waters like a pro.",
                                                "I need a fishing rod like a shark needs teeth, it's essential to my survival."});

const vector<string> thanks_fishing_rod_messages({"Yo, much love and respect for the fishing rod, my man. You're a true G.",
                                                  "Thanks for the hook up with the fishing rod, my dude. You're a real friend.",
                                                  "Hey, big thanks for the fishing rod, my guy. You're a lifesaver.",
                                                  "I appreciate you getting me the fishing rod, homie. You're the real MVP.",
                                                  "Thank you for the fishing rod, boss. You always come through in clutch.",
                                                  "Can't thank you enough for the fishing rod, bro. You're a legend.",
                                                  "You're the man for giving me the fishing rod, my friend. Let's catch some big ones together.",
                                                  "Thanks for the fishing rod, my guy. You're a true gangsta of the sea.",
                                                  "I owe you big time for the fishing rod, homie. You're a true player.",
                                                  "Appreciate the fishing rod, boss. You always know how to keep a G happy."});

Botcraft::Status FarmingTasks::InitializeBlocks(AdvancedClient &client, const int search_radius)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    try
    {
        auto _ = client.findBlocks([&client, &b](const Block *block, const Position position, std::shared_ptr<World> world) -> bool
                                   {
            Position down = position + Position(0, -1, 0);

            if (block->GetBlockstate()->GetName() == "minecraft:barrel" && world->GetBlock(down)->GetBlockstate()->GetName() == "minecraft:gold_block") {
                b.Set("FarmingTasks.fishing_workstation_pos", position);
                LOG_INFO("Fishing workstation found at: " << position << "!");
            } else if (block->GetBlockstate()->GetName() == "minecraft:composter") {
                b.Set("FarmingTasks.farming_workstation_pos", position);
                LOG_INFO("Farming workstation found at: " << position << "!");
            }

            return false; },
                                   search_radius);
    }
    catch (std::range_error &e)
    {
        LOG_ERROR("Error while searching workstations: " << e.what());

        return Status::Failure;
    }

    b.Set("FarmingTasks.initialized", true);
    LOG_INFO("Blocks initialized");

    return Status::Success;
}

Botcraft::Status FarmingTasks::Fish(AdvancedClient &client)
{
    Blackboard &b = client.GetBlackboard();
    std::shared_ptr<EntityManager> entity_manager = client.GetEntityManager();

    // Get fishing workstation position
    const Position &workstation_pos = b.Get<Position>("FarmingTasks.fishing_workstation_pos", Position(0));
    if (workstation_pos == Position(0))
    {
        LOG_WARNING("[Fish] Called Fish task with an un-initialized blackboard workstation pos.");
        return Status::Failure;
    }

    if (GoTo(client, workstation_pos, 2, 2) == Status::Failure)
    {
        LOG_WARNING("[Fish] Couldn't pathfind to fishing workstation");
        return Status::Failure;
    }

    // Calculate average water position to throw hook at
    Vector3<double> average_water_position;
    int nb_water = 0;
    vector<Position> _ = client.findBlocks([&average_water_position, &nb_water](const Block *block, const Position position, std::shared_ptr<World> _) -> bool
                                           {
                                              if (block->GetBlockstate()->GetName() != "minecraft:water")
                                                  return false;

                                              average_water_position = (average_water_position * nb_water + position) / (++nb_water);
                                              return true; },
                                           16);

    average_water_position.y = client.getPosition().y + 10;

    if (LookAt(client, average_water_position) == Status::Failure)
    {
        LOG_WARNING("Failed to look at water");
        return Status::Failure;
    }

    for (int i = 0; i < 50; ++i)
    {
        client.Yield();
    }

    // Select fishing_rod
    if (SetItemInHand(client, "minecraft:fishing_rod") == Status::Failure)
    {
        LOG_WARNING("[Fish] Couldn't equip fishing_rod");
        if (client.sendOTM(need_fishing_rod_messages[rand() % need_fishing_rod_messages.size()], "need_fishing_rod"))
        {
            b.Set<bool>("FarmingTasks::Fish.asked_fishing_rod", true);
        }
        return Status::Failure;
    }
    if (b.Get<bool>("FarmingTasks::Fish.asked_fishing_rod", false))
    {
        client.SendChatMessage(thanks_fishing_rod_messages[rand() % thanks_fishing_rod_messages.size()]);
        client.resetOTM("need_fishing_rod");
        b.Erase("FarmingTasks::Fish.asked_fishing_rod");
    }

    std::set<int> old_fishing_hooks = client.findEntities(EntityType::FishingHook);
    std::set<int> fishing_hooks;
    // Start fishing
    std::shared_ptr<NetworkManager> network_manager = client.GetNetworkManager();
    std::shared_ptr<ProtocolCraft::ServerboundUseItemPacket> use_item_msg = std::make_shared<ProtocolCraft::ServerboundUseItemPacket>();

    use_item_msg->SetHand(static_cast<int>(Hand::Right));
#if PROTOCOL_VERSION > 758
    {
        std::shared_ptr<World> world = client.GetWorld();
        std::lock_guard<std::mutex> world_guard(world->GetMutex());
        use_item_msg->SetSequence(world->GetNextWorldInteractionSequenceId());
    }
#endif
    network_manager->Send(use_item_msg);

    // Wait for new fishing_hook
    auto start = std::chrono::steady_clock::now();
    do
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= 5000)
        {
            LOG_WARNING("[Fish] Error waiting for new fishing hook (Timeout).");
            return Status::Failure;
        }

        client.Yield();
        fishing_hooks = client.findEntities(EntityType::FishingHook);
    } while (fishing_hooks.size() == old_fishing_hooks.size());

    if (old_fishing_hooks.size() > fishing_hooks.size())
    {
        LOG_WARNING("[Fish] More old fishing hooks than new, can't find mine!");
        return Status::Failure;
    }

    for (auto i : old_fishing_hooks)
    {
        fishing_hooks.erase(i);
    }

    if (fishing_hooks.size() != 1)
    {
        LOG_WARNING("[Fish] Can't find my fishing hook!");
        return Status::Failure;
    }
    int fishing_hook_eid = *(fishing_hooks.begin());

    // Wait for bite (mdr)
    while (1)
    {
        {
            std::lock_guard<std::mutex> lock(entity_manager->GetMutex());
            std::shared_ptr<Entity> e = entity_manager->GetEntity(fishing_hook_eid);
            if (e == nullptr)
            {
                LOG_WARNING("[Fish] Fishing hook despawned");
                return Status::Failure;
            }
            if (std::dynamic_pointer_cast<FishingHookEntity>(e)->GetDataBiting())
            {
                break;
            }
        }

        client.Yield();
    }

    // Hook back
    network_manager->Send(use_item_msg);

    // Wait for drop
    for (int i = 0; i < 100; ++i)
    {
        client.Yield();
    }

    return Status::Success;
}

Botcraft::Status FarmingTasks::CollectCropsAndReplant(AdvancedClient &client, const int crops_radius)
{
    Blackboard &b = client.GetBlackboard();

    // Get fishing workstation position
    const Position &workstation_pos = b.Get<Position>("FarmingTasks.farming_workstation_pos", Position(0));
    if (workstation_pos == Position(0))
    {
        LOG_WARNING("[Farming] Called farming task with an un-initialized blackboard workstation pos.");
        return Status::Failure;
    }

    if (GoTo(client, workstation_pos, 2, 2) == Status::Failure)
    {
        LOG_WARNING("[Farming] Couldn't pathfind to fishing workstation");
        return Status::Failure;
    }

    vector<std::pair<string, Position>> grown_crops;
    vector<Position> _ = client.findBlocks([&grown_crops](const Block *block, const Position position, std::shared_ptr<World> _) -> bool
                                           {
                                                std::string block_name = block->GetBlockstate()->GetName();
                                                if (
                                                    (
                                                        block_name == "minecraft:carrots" || 
                                                        block_name == "minecraft:potatoes" ||
                                                        block_name == "minecraft:wheat"
                                                    ) && block->GetBlockstate()->GetVariableValue("age") == "7") 
                                                {
                                                    std::pair<string, Position> crop_pair;
                                                    crop_pair.second = position;
                                                    if (block_name == "minecraft:wheat") {
                                                        crop_pair.first = "minecraft:wheat_seeds";
                                                    } else if (block_name == "minecraft:carrots") {
                                                        crop_pair.first = "minecraft:carrot";
                                                    } else if (block_name == "minecraft:potatoes") {
                                                        crop_pair.first = "minecraft:potato";
                                                    } else {
                                                        LOG_ERROR("Illegal block_name");
                                                        exit(1);
                                                    }
                                                    grown_crops.push_back(crop_pair);
                                                }
                                                return false; },
                                           16);

    for (auto crop : grown_crops)
    {
        // Collect
        if (AdvancedTasks::DigAndCollect(client, crop.second) == Status::Failure)
        {
            LOG_WARNING("[Farming] Couldn't collect crop drops for " << crop.first);
        }
        // Wait for a bit
        for (int i = 0; i < 20; ++i)
        {
            client.Yield();
        }
        // Replant
        if (PlaceBlock(client, crop.first, crop.second) == Status::Failure)
        {
            LOG_WARNING("[Farming] Couldn't replant " << crop.first);
        }
    }

    return Status::Success;
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> FarmingTasks::CreateTree()
{
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
        .selector()
        .leaf(CheckBlackboardBoolData, "FarmingTasks.initialized")
        .sequence()
        .leaf(FarmingTasks::InitializeBlocks, 100)
        .end()
        .end()
        .leaf(CollectCropsAndReplant, 8)
        .end();
}