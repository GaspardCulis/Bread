#include <chrono>
#include <iostream>
#include <botcraft/AI/SimpleBehaviourClient.hpp>
#include <botcraft/AI/BehaviourTree.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <memory>
#include <unistd.h>
#include <string>
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Game/World/World.hpp"
#include "botcraft/Utilities/SleepUtilities.hpp"
#include "tasks/AdvancedTasks.hpp"
#include "tasks/FarmingTasks.hpp"
#include "tasks/SkyblockTasks.hpp"
#include "AdvancedClient.hpp"
#include "tasks/SurvivalTasks.hpp"

#define NUM_BOTS 3

using namespace std;

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateMauriceTree();

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateGertrudeTree();

int main(int argc, char *argv[])
{
    std::string bot_names[NUM_BOTS] = {
        "Maurice",
        "Gertrude",
        "Karlos"
    };
    
    std::vector<std::shared_ptr<Botcraft::World>> shared_worlds(NUM_BOTS);
    for (int i = 0; i < NUM_BOTS; i++) {
        shared_worlds[i] = std::make_shared<Botcraft::World>(true);
    }
    
    std::vector<std::shared_ptr<AdvancedClient>> clients(NUM_BOTS);
    for (int i = 0; i < NUM_BOTS; i++) {
        clients[i] = std::make_shared<AdvancedClient>();
        
        clients[i]->SetSharedWorld(shared_worlds[i]);
        clients[i]->SetAutoRespawn(true);
        clients[i]->Connect("127.0.0.1:25565", bot_names[i]);
        clients[i]->StartBehaviour();
        if (i == 0) {
            clients[i]->SetBehaviourTree(CreateMauriceTree());
        } else if (i == 1) {
            clients[i]->SetBehaviourTree(CreateGertrudeTree());
        }

        Botcraft::Utilities::SleepFor(std::chrono::seconds(2));
    }

    std::vector<std::thread> behaviours_threads(NUM_BOTS);
    for (int i = 0; i < NUM_BOTS; ++i)
    {
        behaviours_threads[i] = std::thread(&AdvancedClient::RunBehaviourUntilClosed, clients[i]);
        // Start all the behaviours with a 2 seconds interval
        Botcraft::Utilities::SleepFor(std::chrono::seconds(2));
    }

    // Wait for all the bots to disconnect (meaning the job is done if everything worked properly)
    for (int i = 0; i < NUM_BOTS; ++i)
    {
        behaviours_threads[i].join();
    }

    return 0;
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateMauriceTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "SkyblockTasks.initialized")
                .sequence()
                    .leaf(SkyblockTasks::InitializeBlocks, 64)
                    .leaf(AdvancedTasks::FindNamedChestBlackboard, "cobblestone", "Storage.cobblestone")
                    .leaf(AdvancedTasks::FindNamedChestBlackboard, "wood", "Storage.wood")
                    .leaf(AdvancedTasks::FindNamedChestBlackboard, "farming", "Storage.farming")
                .end()
            .end()
            .sequence()
                .tree(SurvivalTasks::CreateTree())
                .leaf(SkyblockTasks::ChopTrees)
                .repeater(16)
                .leaf(SkyblockTasks::MineCobblestone)
                .sequence("Store items")
                    .leaf(AdvancedTasks::StoreItemsBlackboard, "Storage.cobblestone", "minecraft:cobblestone", 32, 64)
                    .leaf(AdvancedTasks::StoreItemsBlackboard, "Storage.wood", "minecraft:oak_log", 8, 32)
                    .leaf(AdvancedTasks::StoreItemsBlackboard, "Storage.farming", "minecraft:bone_meal", 0, 8)
                .end()
                .leaf(FarmingTasks::CompostVegetables, "minecraft:oak_sapling", 32)
            .end()
        .end();
    // clang-format on
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateGertrudeTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "SkyblockTasks.initialized")
                .sequence()
                    .leaf(SkyblockTasks::InitializeBlocks, 64)
                    .leaf(AdvancedTasks::FindNamedChestBlackboard, "farming", "Storage.farming")
                .end()
            .end()
            .sequence()
                .tree(SurvivalTasks::CreateTree())
                .leaf(SkyblockTasks::Farm)
                .leaf(FarmingTasks::CompostVegetables, "minecraft:wheat_seeds", 32)
                .sequence("Store items")
                    .leaf(AdvancedTasks::StoreItemsBlackboard, "Storage.farming", "minecraft:wheat", 0, 8)
                    .leaf(AdvancedTasks::StoreItemsBlackboard, "Storage.farming", "minecraft:bone_meal", 0, 8)
                .end()
            .end()
        .end();
    // clang-format on
}