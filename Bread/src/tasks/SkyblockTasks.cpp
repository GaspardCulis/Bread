#include "tasks/SkyblockTasks.hpp"
#include "CraftingUtils.hpp"
#include "botcraft/AI/Status.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Utilities/Logger.hpp"
#include "tasks/AdvancedTasks.hpp"
#include "tasks/FarmingTasks.hpp"
#include "botcraft/AI/Tasks/InventoryTasks.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "tasks/SurvivalTasks.hpp"

Status SkyblockTasks::InitializeBlocks(AdvancedClient &client, const int radius)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    std::vector<Botcraft::Position> oak_blocks = client.findBlocks(
        [](const Botcraft::Blockstate *block, const Botcraft::Position position, std::shared_ptr<Botcraft::World> world)
        {
            if (block->GetName() == "minecraft:oak_log")
            {
                if (world->GetBlock(position + Botcraft::Vector3<int>(0, -1, 0))->GetName() == "minecraft:dirt")
                {
                    return true;
                }
            }
            return false;
        },
        radius);

    std::vector<Botcraft::Position> oak_saplings = client.findBlocks("minecraft:oak_sapling");

    std::vector<Botcraft::Position> blocks;
    blocks.insert(blocks.end(), oak_blocks.begin(), oak_blocks.end());
    blocks.insert(blocks.end(), oak_saplings.begin(), oak_saplings.end());

    b.Set("SkyblockTasks.tree_blocks", blocks);

    Botcraft::Position stone_pos = client.findNearestBlock(
        [](const Botcraft::Blockstate *block, const Botcraft::Position position, std::shared_ptr<Botcraft::World> world)
        {
            if (block->GetName() == "minecraft:cobblestone")
            {
                if (world->GetBlock(position + Botcraft::Vector3<int>(0, -1, 0))->GetName() == "minecraft:pumpkin")
                {
                    return true;
                }
            }
            return false;
        },
        radius);

    Botcraft::Position crafting_table_pos = client.findNearestBlock("minecraft:crafting_table", 5, stone_pos);

    Botcraft::Position chest_pos = client.findNearestBlock(
        [](const Botcraft::Blockstate *block, const Botcraft::Position position, std::shared_ptr<Botcraft::World> world)
        {
            if (block->GetName() == "minecraft:chest")
            {
                if (world->GetBlock(position + Botcraft::Vector3<int>(0, -1, 0))->GetName() == "minecraft:pumpkin")
                {
                    return true;
                }
            }
            return false;
        },
        radius);

    b.Set("SkyblockTasks.stone_pos", stone_pos);
    b.Set("SkyblockTasks.crafting_table_pos", crafting_table_pos);
    b.Set("SkyblockTasks.chest_pos", chest_pos);

    b.Set("SkyblockTasks.initialized", true);

    LOG_INFO("[SkyblockTasks] Blocks initialized");

    return Status::Success;
}

Status SkyblockTasks::ChopTrees(AdvancedClient &client)
{
    LOG_INFO("[ChopTrees] Started task");
    Botcraft::Blackboard &b = client.GetBlackboard();

    std::vector<Botcraft::Position> blocks = b.Get<std::vector<Botcraft::Position>>("SkyblockTasks.tree_blocks");
    if (blocks.size() == 0)
    {
        return Status::Failure;
    }

    for (Botcraft::Position block : blocks)
    {
        bool is_oak = false;
        {
            std::shared_ptr<World> world = client.GetWorld();

            if (world->GetBlock(block)->GetName() == "minecraft:oak_log")
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

            Botcraft::Position current_block = block;
            while (world->GetBlock(current_block)->GetName() == "minecraft:oak_log")
            {
                trunk.push_back(current_block);
                current_block += Botcraft::Vector3<int>(0, 1, 0);
            }
        }
        // Ignore leaves for now
        for (int i = 0; i < trunk.size(); i++)
        {
            if (i == 2)
            {
                // Replant
                if (PlaceBlock(client, "minecraft:oak_sapling", block) == Status::Failure)
                {
                    LOG_WARNING("[ChopTrees] Failed to place block");
                    return Status::Failure;
                }
                // Go to the tree stem
                while (GoTo(client, block) == Status::Failure)
                {
                    // Must be leaves
                    Botcraft::Position leave = client.findNearestBlock("minecraft:oak_leaves", 5, block);
                    if (Dig(client, leave) == Status::Failure)
                    {
                        LOG_WARNING("[ChopTrees] Failed to dig leave at " << leave);
                        return Status::Failure;
                    }
                }
            }
            Botcraft::Position trunk_block = trunk[i];
            SetItemInHand(client, "minecraft:stone_axe");
            if (Dig(client, trunk_block) == Status::Failure)
            {
                LOG_WARNING("[ChopTrees] Failed to dig block at " << trunk_block);
                return Status::Failure;
            }
        }

        for (int i = 0; i < 100; i++)
        {
            client.Yield();
        }
    }

    // Collect the sapling and apples
    AdvancedTasks::CollectItems(client, "minecraft:oak_sapling");
    AdvancedTasks::CollectItems(client, "minecraft:apple");

    return Status::Success;
}

Status SkyblockTasks::MineCobblestone(AdvancedClient &client)
{
    LOG_INFO("[MineCobblestone] Started task");
    
    Botcraft::Blackboard &b = client.GetBlackboard();

    Botcraft::Position stone_pos = b.Get<Botcraft::Position>("SkyblockTasks.stone_pos", Position(0));
    Botcraft::Position crafting_table_pos = b.Get<Botcraft::Position>("SkyblockTasks.crafting_table_pos", Position(0));
    if (stone_pos == Position(0) || crafting_table_pos == Position(0))
    {
        LOG_WARNING("[MineCobblestone] Called task with un-initialized blackboard positions");
        return Status::Failure;
    }

    if (GoTo(client, crafting_table_pos + Botcraft::Vector3<int>(0, 1, 0)) == Status::Failure)
    {
        LOG_WARNING("[MineCobblestone] Failed to go to crafting table");
        return Status::Failure;
    }

    // Wait for cobble to spawn
    while (1)
    {
        {
            std::shared_ptr<World> world = client.GetWorld();

            if (world->GetBlock(stone_pos)->GetName() == "minecraft:cobblestone")
            {
                break;
            }
        }
        client.Yield();
    }

    SetItemInHand(client, "minecraft:stone_pickaxe");
    if (Dig(client, stone_pos) == Status::Failure)
    {
        LOG_WARNING("[MineCobblestone] Failed to dig stone");
        return Status::Failure;
    }

    return Status::Success;
}

Status SkyblockTasks::StoreItems(AdvancedClient &client)
{
    LOG_INFO("[StoreItems] Started task");
    
    Blackboard &b = client.GetBlackboard();
    AdvancedTasks::EnsureItemCount(client, b.Get<Position>("SkyblockTasks.chest_pos"), "minecraft:cobblestone", 32, 64);
    AdvancedTasks::EnsureItemCount(client, b.Get<Position>("SkyblockTasks.chest_pos"), "minecraft:oak_log", 32, 64);
    AdvancedTasks::EnsureItemCount(client, b.Get<Position>("SkyblockTasks.chest_pos"), "minecraft:bone_meal", 0, 8);
    AdvancedTasks::EnsureItemCount(client, b.Get<Position>("SkyblockTasks.chest_pos"), "minecraft:wheat", 0, 3);
/*
    LogInventoryContent(client);

    auto recipes = CraftingUtils::GetAvailableRecipes(client, "minecraft:stone_pickaxe");
    LOG_INFO("There are " << recipes.size() << " recipes for pick");
    auto recipe = recipes[0];
    if (CraftingUtils::CanCraft(client, recipe))
    {
        LOG_INFO("Hey, I can craft pickaxe !");
    }
    else
    {
        LOG_INFO("Bruh can't craft pixk");
    }
*/
    return Status::Success;
}

Botcraft::Status SkyblockTasks::Farm(AdvancedClient &client)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    for (Position workstation : client.findBlocks("minecraft:composter"))
    {
        b.Set<Position>("FarmingTasks.farming_workstation_pos", workstation);

        FarmingTasks::CollectCropsAndReplant(client, 3);
        FarmingTasks::MaintainField(client);
    }
    FarmingTasks::CompostVegetables(client, "minecraft:oak_sapling", 32);
    FarmingTasks::CompostVegetables(client, "minecraft:wheat_seeds", 32);

    return Status::Success;
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> SkyblockTasks::CreateTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "SkyblockTasks.initialized")
                .sequence()
                    .leaf(SkyblockTasks::InitializeBlocks, 64)
                .end()
            .end()
            .sequence()
                .tree(SurvivalTasks::CreateTree())
                .leaf(ChopTrees)
                .repeater(16)
                .leaf(MineCobblestone)
                .leaf(StoreItems)
            .end()
            .leaf(SkyblockTasks::Farm)
        .end();
    // clang-format on
}