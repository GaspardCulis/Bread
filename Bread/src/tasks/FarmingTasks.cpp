#include "tasks/FarmingTasks.hpp"
#include "botcraft/AI/Status.hpp"
#include "botcraft/AI/Tasks/BaseTasks.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Utilities/Logger.hpp"
#include "tasks/AdvancedTasks.hpp"
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

const vector<string> need_hoe_messages({"Yo, hook me up with a farming hoe, cuz I ain't planting nothin' with my bare hands.",
                                        "Listen up, homie, I need a farming hoe like a farmer needs his crops.",
                                        "Hey, boss, can you toss me a farming hoe? I'm tryna till up some serious soil, you feel me?",
                                        "Ayo, let me get a hoe, I'm tryna cultivate some fresh produce, ya dig?",
                                        "Yo, I need a hoe like a rapper needs a beat, it's my tool for success on the farm.",
                                        "I'm like a farmer without a hoe without a farming hoe, you gotta help me out.",
                                        "Can't harvest without a farming hoe, it's my key to the earth.",
                                        "Hey, can you lend me a farming hoe? I promise to return it in one piece, unless I hit a rock.",
                                        "Gimme a farming hoe and watch me work the fields like a pro.",
                                        "I need a farming hoe like a chef needs a knife, it's essential to my work on the farm."});

const vector<string> thanks_hoe_messages({"Yo, much love and respect for the farming hoe, my man. You're a true G of agriculture.",
                                          "Thanks for the hook up with the farming hoe, my dude. You're a real friend to farmers everywhere.",
                                          "Hey, big thanks for the farming hoe, my guy. You're a lifesaver in the field.",
                                          "I appreciate you getting me the farming hoe, homie. You're the real MVP of farming tools.",
                                          "Thank you for the farming hoe, boss. You always come through in clutch when it comes to farming gear.",
                                          "Can't thank you enough for the farming hoe, bro. You're a legend in the world of farming equipment.",
                                          "You're the man for giving me the farming hoe, my friend. Let's grow some amazing crops together.",
                                          "Thanks for the farming hoe, my guy. You're a true gangsta of agriculture.",
                                          "I owe you big time for the farming hoe, homie. You're a true player in the world of farming tools.",
                                          "Appreciate the farming hoe, boss. You always know how to keep a farmer happy and productive."});

const map<string, string> crops({{"minecraft:carrots", "minecraft:carrot"},
                                 {"minecraft:potatoes", "minecraft:potato"},
                                 {"minecraft:wheat", "minecraft:wheat_seeds"}});

Botcraft::Status FarmingTasks::InitializeBlocks(AdvancedClient &client, const int search_radius)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    auto _ = client.findBlocks(
        [&b](const Block *block, const Position position, std::shared_ptr<World> world) -> bool
        {
            Position down = position + Position(0, -1, 0);

            if (block->GetBlockstate()->GetName() == "minecraft:barrel" && world->GetBlock(down)->GetBlockstate()->GetName() == "minecraft:gold_block")
            {
                b.Set("FarmingTasks.fishing_workstation_pos", position);
                LOG_INFO("Fishing workstation found at: " << position << "!");
            }
            else if (block->GetBlockstate()->GetName() == "minecraft:composter")
            {
                b.Set("FarmingTasks.farming_workstation_pos", position);
                LOG_INFO("Farming workstation found at: " << position << "!");
            }

            return false;
        },
        search_radius);

    b.Set("FarmingTasks.initialized", true);
    LOG_INFO("[FarmingTasks] Blocks initialized");

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

    if (GoTo(client, workstation_pos, 4, 4) == Status::Failure)
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
        LOG_WARNING("[Fish] Failed to look at water");
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

    if (GoTo(client, workstation_pos, 4, 4) == Status::Failure)
    {
        LOG_WARNING("[Farming] Couldn't pathfind to farming workstation");
        return Status::Failure;
    }

    vector<std::pair<string, Position>> grown_crops;
    vector<Position> _ = client.findBlocks([&grown_crops](const Block *block, const Position position, std::shared_ptr<World> _) -> bool
                                           {
                                                std::string block_name = block->GetBlockstate()->GetName();
                                                if (
                                                    crops.count(block_name) && block->GetBlockstate()->GetVariableValue("age") == "7") 
                                                {
                                                    std::pair<string, Position> crop_pair;
                                                    crop_pair.first = crops.at(block_name);
                                                    crop_pair.second = position;
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
        // Replant
        int i = 0;
        while (i < 5 && PlaceBlock(client, crop.first, crop.second) == Status::Failure)
        {
            i++;
            client.Yield();
        }
        if (i == 5)
        {
            LOG_WARNING("[Farming] Couldn't replant " << crop.first);
        }
    }

    return Status::Success;
}

Botcraft::Status FarmingTasks::MaintainField(AdvancedClient &client)
{
    Blackboard &b = client.GetBlackboard();

    // Get fishing workstation position
    const Position &workstation_pos = b.Get<Position>("FarmingTasks.farming_workstation_pos", Position(0));
    if (workstation_pos == Position(0))
    {
        LOG_WARNING("[MaintainField] Called farming task with an un-initialized blackboard workstation pos.");
        return Status::Failure;
    }

    // Equip hoe (the farming tool)
    std::string hoe_item_name;
    int hoe_slot = client.getItemSlotInInventory(
        [&hoe_item_name](short slodId, ProtocolCraft::Slot current_slot, Botcraft::Item *item) -> bool
        {
            if (item->GetToolType() == ToolType::Hoe)
            {
                hoe_item_name = item->GetName();
                return true;
            }
            return false;
        });
    if (hoe_slot < 0)
    {
        LOG_WARNING("[MaintainField] No hoe");
        if (client.sendOTM(need_hoe_messages[rand() % need_hoe_messages.size()], "need_hoe"))
        {
            b.Set<bool>("FarmingTasks::MaintainField.asked_hoe", true);
        }
        return Status::Failure;
    }
    else if (b.Get<bool>("FarmingTasks::MaintainField.asked_hoe", false))
    {
        client.SendChatMessage(thanks_hoe_messages[rand() % thanks_hoe_messages.size()]);
        client.resetOTM("need_hoe");
        b.Erase("FarmingTasks::MaintainField.asked_hoe");
    }
    if (SetItemInHand(client, hoe_item_name) == Status::Failure)
    {
        LOG_WARNING("Failed to equip " << hoe_item_name);
        return Status::Failure;
    }
    // Scan for blocks that need a little shaving
    std::vector<Position> water_blocks = client.findBlocks("minecraft:water", 7, 10, workstation_pos);
    std::set<Position> candidate_blocks;
    std::set<Position> empty_farmland_blocks;
    for (auto water : water_blocks)
    {
        std::shared_ptr<World> world = client.GetWorld();

        for (int y = water.y - 2; y <= water.y + 2; y++)
        {
            for (int x = water.x - 4; x <= water.x + 4; x++)
            {
                for (int z = water.z - 4; z <= water.z + 4; z++)
                {
                    std::lock_guard<std::mutex> lock_world(world->GetMutex());
                    const Position current = Position(x, y, z);
                    const Block *block = world->GetBlock(current);
                    if (block == nullptr)
                    {
                        continue;
                    }
                    const std::string block_name = block->GetBlockstate()->GetName();
                    if (block_name == "minecraft:dirt" || block_name == "minecraft:grass_block")
                    {
                        const Block *upper_block = world->GetBlock(Position(x, y + 1, z));
                        if (upper_block != nullptr && upper_block->GetBlockstate()->IsAir())
                        {
                            candidate_blocks.insert(current);
                            empty_farmland_blocks.insert(current);
                        }
                    }
                    else if (block_name == "minecraft:farmland")
                    {
                        const Block *upper_block = world->GetBlock(Position(x, y + 1, z));
                        if (upper_block == nullptr || upper_block->GetBlockstate()->IsAir())
                        {
                            empty_farmland_blocks.insert(current);
                        }
                    }
                }
            }
        }
    }
    // Then give them a little shave
    if (candidate_blocks.size() > 0)
    {
        LOG_INFO("[MaintainField] " << candidate_blocks.size() << " blocks need a little shaving");
    }
    for (auto block : candidate_blocks)
    {
        if (InteractWithBlock(client, block, Direction::Up, true) == Status::Failure)
        {
            LOG_WARNING("Failed to shave block at " << block);
        }
        for (int i = 0; i < 10; ++i)
        {
            client.Yield();
        }
    }
    // Plant on top of empty farmaland blocks
    // Choose waht to plant based on what the bot has the less
    map<std::string, int> available_crops;
    client.getItemSlotInInventory([&available_crops](short slodId, ProtocolCraft::Slot current_slot, Botcraft::Item *item) -> bool
                                  {
            std::string item_name = item->GetName();
            if (std::find_if(crops.begin(), crops.end(), [item_name](const auto& mo) {return mo.second == item_name; }) != crops.end())
            {
                if (available_crops.count(item_name)) {
                    available_crops[item_name] += current_slot.GetItemCount();
                } else {
                    available_crops[item_name] = current_slot.GetItemCount();
                }
            }
            return false; });
    std::vector<std::pair<std::string, int>> available_crops_sorted;
    for (auto &it : available_crops)
    {
        available_crops_sorted.push_back(it);
    }
    sort(available_crops_sorted.begin(), available_crops_sorted.end(), [](pair<std::string, int> &a, pair<std::string, int> &b) -> bool
         { return a.second < b.second; });

    for (auto block : empty_farmland_blocks)
    {
        if (available_crops_sorted.size() == 0)
        {
            LOG_WARNING("[MaintainField] Out of crops");
            break;
        }
        if (SetItemInHand(client, available_crops_sorted[0].first, Hand::Right) == Status::Failure)
        {
            LOG_WARNING("Failed to equip item " << available_crops_sorted[0].first);
        }
        Position crop_pos = block + Position(0, 1, 0);
        if (PlaceBlock(client, available_crops_sorted[0].first, crop_pos) == Status::Failure)
        {
            LOG_WARNING("[MaintainField] Failed to plant " << available_crops_sorted[0].first << " at " << crop_pos);
        }
        else
        {
            available_crops_sorted[0] = std::pair<std::string, int>(available_crops_sorted[0].first, available_crops_sorted[0].second - 1);
        }
        // Check if no more crops
        if (available_crops_sorted[0].second == 0)
        {
            available_crops_sorted.erase(available_crops_sorted.begin());
        }
        for (int i = 0; i < 10; i++)
        {
            client.Yield();
        }
    }

    return Status::Success;
}

Botcraft::Status FarmingTasks::CompostVegetables(AdvancedClient &client, const std::string item_name, const int keep_count)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    const Position &workstation_pos = b.Get<Position>("FarmingTasks.farming_workstation_pos", Position(0));
    if (workstation_pos == Position(0))
    {
        LOG_WARNING("[CompostVegetables] Called Fish task with an un-initialized blackboard workstation pos.");
        return Status::Failure;
    }

    while (client.getItemCountInInventory(item_name) > keep_count) {
        SetItemInHand(client, item_name);
        if (InteractWithBlock(client, workstation_pos) == Status::Failure) {
            LOG_WARNING("[CompostVegetables] Failed to interact with block at " << workstation_pos);
            return Status::Failure;
        }
        client.Yield();
    }

    return Status::Success;
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> FarmingTasks::CreateTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "FarmingTasks.initialized")
                .sequence()
                    .leaf(FarmingTasks::InitializeBlocks, 128)
                .end()
            .end()
            .sequence()
                .leaf("Collect grown crops", CollectCropsAndReplant, 8)
                .leaf("Maintain filed", MaintainField)
                .repeater(4)
                .leaf(Fish)
            .end()
        .end();
    // clang-format on
}