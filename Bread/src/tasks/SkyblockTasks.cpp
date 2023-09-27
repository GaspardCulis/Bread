#include "tasks/SkyblockTasks.hpp"
#include "tasks/AdvancedTasks.hpp"
#include "botcraft/AI/Tasks/InventoryTasks.hpp"

Botcraft::Status SkyblockTasks::InitializeBlocks(AdvancedClient &client, const int radius)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    std::vector<Botcraft::Position> oak_blocks = client.findBlocks(
        [](const Botcraft::Block *block, const Botcraft::Position position, std::shared_ptr<Botcraft::World> world)
        {
            if (block->GetBlockstate()->GetName() == "minecraft:oak_log")
            {
                if (world->GetBlock(position + Botcraft::Vector3<int>(0, -1, 0))->GetBlockstate()->GetName() == "minecraft:dirt")
                {
                    return true;
                }
            }
            return false;
        });

    std::vector<Botcraft::Position> oak_saplings = client.findBlocks("minecraft:oak_sapling");

    std::vector<Botcraft::Position> blocks;
    blocks.insert(blocks.end(), oak_blocks.begin(), oak_blocks.end());
    blocks.insert(blocks.end(), oak_saplings.begin(), oak_saplings.end());

    b.Set("SkyblockTasks.tree_blocks", blocks);

    Botcraft::Position stone_pos = client.findNearestBlock(
        [](const Botcraft::Block *block, const Botcraft::Position position, std::shared_ptr<Botcraft::World> world)
        {
            if (block->GetBlockstate()->GetName() == "minecraft:cobblestone")
            {
                if (world->GetBlock(position + Botcraft::Vector3<int>(0, -1, 0))->GetBlockstate()->GetName() == "minecraft:pumpkin")
                {
                    return true;
                }
            }
            return false;
        });

    Botcraft::Position crafting_table_pos = client.findNearestBlock("minecraft:crafting_table", 5, stone_pos);

    b.Set("SkyblockTasks.stone_pos", stone_pos);
    b.Set("SkyblockTasks.crafting_table_pos", crafting_table_pos);

    b.Set("SkyblockTasks.initialized", true);

    LOG_INFO("Found " << blocks.size() << " blocks");

    return Botcraft::Status::Success;
}

Botcraft::Status SkyblockTasks::ChopTrees(AdvancedClient &client)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    std::vector<Botcraft::Position> blocks = b.Get<std::vector<Botcraft::Position>>("SkyblockTasks.tree_blocks");
    if (blocks.size() == 0)
    {
        return Botcraft::Status::Failure;
    }

    for (Botcraft::Position block : blocks)
    {
        bool is_oak = false;
        {
            std::shared_ptr<World> world = client.GetWorld();
            std::lock_guard<std::mutex> lock_world(world->GetMutex());

            if (world->GetBlock(block)->GetBlockstate()->GetName() == "minecraft:oak_log")
            {
                is_oak = true;
            }
        }

        if (!is_oak)
        {
            continue;
        }

        std::vector<Botcraft::Position> trunk;
        {
            std::shared_ptr<World> world = client.GetWorld();
            std::lock_guard<std::mutex> lock_world(world->GetMutex());

            Botcraft::Position current_block = block;
            while (world->GetBlock(current_block)->GetBlockstate()->GetName() == "minecraft:oak_log")
            {
                trunk.push_back(current_block);
                current_block += Botcraft::Vector3<int>(0, 1, 0);
            }
        }
        // Ignore leaves for now
        LOG_INFO("Chopping tree at " << block);
        for (int i = 0; i < trunk.size(); i++)
        {
            if (i == 2)
            {
                // Replant
                if (PlaceBlock(client, "minecraft:oak_sapling", block) == Botcraft::Status::Failure)
                {
                    LOG_WARNING("Failed to place block");
                    return Botcraft::Status::Failure;
                }
                // Go to the tree stem
                while (GoTo(client, block) == Botcraft::Status::Failure)
                {
                    // Must be leaves
                    Botcraft::Position leave = client.findNearestBlock("minecraft:oak_leaves", 5, block);
                    if (Dig(client, leave) == Botcraft::Status::Failure)
                    {
                        LOG_WARNING("Failed to dig leave at " << leave);
                        return Botcraft::Status::Failure;
                    }
                }
            }
            Botcraft::Position trunk_block = trunk[i];
            SetItemInHand(client, "minecraft:stone_axe");
            if (Dig(client, trunk_block) == Botcraft::Status::Failure)
            {
                LOG_WARNING("Failed to dig block at " << trunk_block);
                return Botcraft::Status::Failure;
            }
        }

        for (int i = 0; i < 100; i++)
        {
            client.Yield();
        }
        LOG_INFO("Done chopping tree at " << block);
    }

    return Botcraft::Status::Success;
}

Botcraft::Status SkyblockTasks::MineCobblestone(AdvancedClient &client)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    Botcraft::Position stone_pos = b.Get<Botcraft::Position>("SkyblockTasks.stone_pos");
    Botcraft::Position crafting_table_pos = b.Get<Botcraft::Position>("SkyblockTasks.crafting_table_pos");

    if (GoTo(client, crafting_table_pos + Botcraft::Vector3<int>(0, 1, 0)) == Botcraft::Status::Failure)
    {
        LOG_WARNING("Failed to go to crafting table");
        return Botcraft::Status::Failure;
    }

    // Wait for cobble to spawn
    while (1)
    {
        {
            std::shared_ptr<World> world = client.GetWorld();
            std::lock_guard<std::mutex> lock_world(world->GetMutex());

            if (world->GetBlock(stone_pos)->GetBlockstate()->GetName() == "minecraft:cobblestone")
            {
                break;
            }
        }
        client.Yield();
    }

    SetItemInHand(client, "minecraft:stone_pickaxe");
    if (Dig(client, stone_pos) == Botcraft::Status::Failure)
    {
        LOG_WARNING("Failed to dig stone");
        return Botcraft::Status::Failure;
    }

    return Botcraft::Status::Success;
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> SkyblockTasks::CreateTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "SkyblockTasks.initialized")
                .sequence()
                    .leaf(SkyblockTasks::InitializeBlocks, 128)
                .end()
            .end()
                .sequence()
                .leaf(ChopTrees)
                .repeater(16)
                .leaf(MineCobblestone)
                .selector()
                    .inverter().leaf("check is hungry", Botcraft::IsHungry, 20)
                    .sequence()
                        .leaf("eat", Botcraft::Eat, "minecraft:apple", true)
                    .end()
                .end()
            .end()
        .end();
    // clang-format on
}