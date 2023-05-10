#include "FarmingTasks.hpp"
#include "botcraft/AI/Tasks/InventoryTasks.hpp"
#include "botcraft/Game/Entities/entities/projectile/FishingHookEntity.hpp"
#include "botcraft/Network/NetworkManager.hpp"

Botcraft::Status FarmingTasks::InitializeBlocks(AdvancedClient &client, const int search_radius)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    LOG_INFO("Initializing blocks");

    try
    {
        auto _ = client.findBlocks([&client, &b](const Block *block, const Position position, std::shared_ptr<World> world) -> bool
                                   {
            Position down = position + Position(0, -1, 0);

            if (block->GetBlockstate()->GetName() == "minecraft:barrel" && world->GetBlock(down)->GetBlockstate()->GetName() == "minecraft:gold_block") {
                b.Set("FarmingTasks.fishing_workstation_pos", position);
                LOG_INFO("Fishing workstation found at: " << position << "!");
            } else if (block->GetBlockstate()->GetName() == "minecraft:composter" && world->GetBlock(down)->GetBlockstate()->GetName() == "minecraft:gold_block") {
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
    LOG_INFO("[Fish] Starting fishing task");
    Blackboard &b = client.GetBlackboard();

    const Position &workstation_pos = b.Get<Position>("FarmingTasks.fishing_workstation_pos", Position(0));
    if (workstation_pos == Position(0))
    {
        LOG_WARNING("Called Fish task with an un-initialized blackboard workstation pos.");
        return Status::Failure;
    }

    LOG_INFO("[Fish] Pathfinding to workstation");
    if (GoTo(client, workstation_pos, 2, 2) == Status::Failure)
    {
        LOG_WARNING("Couldn't pathfind to fishing workstation");
        return Status::Failure;
    }

    LOG_INFO("[Fish] Calculating average water position");
    Vector3<double> average_water_position;
    int nb_water = 0;
    vector<Position> _ = client.findBlocks([&average_water_position, &nb_water](const Block *block, const Position position, std::shared_ptr<World> _) -> bool
                                           {
                                              if (block->GetBlockstate()->GetName() != "minecraft:water")
                                                  return false;

                                              average_water_position = (average_water_position * nb_water + position) / (++nb_water);
                                              return true; },
                                           16);

    LOG_INFO("[Fish] Average water position is " << average_water_position);
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

    LOG_INFO("[Fish] Selecting fishing_rod");
    // Select fishing_rod
    if (SetItemInHand(client, "minecraft:fishing_rod") == Status::Failure)
    {
        LOG_WARNING("Couldn't equip fishing_rod");
        cout.flush();
        return Status::Failure;
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
            LOG_WARNING("Error waiting for new fishing hook (Timeout).");
            return Status::Failure;
        }

        client.Yield();
        fishing_hooks = client.findEntities(EntityType::FishingHook);
    } while (fishing_hooks.size() == old_fishing_hooks.size());

    if (old_fishing_hooks.size() > fishing_hooks.size())
    {
        LOG_WARNING("More old fishing hooks than new, can't find mine!");
        return Status::Failure;
    }

    for (auto i : old_fishing_hooks)
    {
        fishing_hooks.erase(i);
    }

    if (fishing_hooks.size() != 1)
    {
        LOG_WARNING("Can't find my fishing hook!");
        return Status::Failure;
    }
    int fishing_hook_eid = *(fishing_hooks.begin());
    // Wait for bite (mdr)

    while (1)
    {
        client.Yield();

        std::shared_ptr<FishingHookEntity> e = std::dynamic_pointer_cast<FishingHookEntity>(client.getEntity(fishing_hook_eid));
        if (e->GetDataBiting())
        {
            break;
        }
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

Botcraft::Status FarmingTasks::CollectCropsAndReplant(Botcraft::BehaviourClient &client, const std::string &block_pos_blackboard, const int crops_radius)
{
    return Botcraft::Status();
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
        .leaf(Fish)
        .end();
}