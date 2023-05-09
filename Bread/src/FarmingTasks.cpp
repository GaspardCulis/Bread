#include "FarmingTasks.hpp"

Botcraft::Status FarmingTasks::InitializeBlocks(AdvancedClient& client, const int search_radius) {
    Botcraft::Blackboard& b = client.GetBlackboard();

    LOG_INFO("Initializing blocks");

    Position fishing_workstation_position;
    Position farming_workstation_position;

    try {
        client.findBlocks([&client](const Block *block, const Position position, std::shared_ptr<World> world) -> bool {
            Position down = position + Position(0, -1, 0);

            if (block->GetBlockstate()->GetName() == "minecraft:barrel" && world->GetBlock(down)->GetBlockstate()->GetName() == "minecraft:gold_block") {
                LOG_INFO("Fishing workstation found at: " << position << "!");
            } else if (block->GetBlockstate()->GetName() == "minecraft:composter" && world->GetBlock(down)->GetBlockstate()->GetName() == "minecraft:gold_block") {
                LOG_INFO("Farming workstation found at: " << position << "!");
            }

            return false;
        }, search_radius);
    } catch(std::range_error &e) {
        LOG_ERROR("Error while searching workstations: " << e.what());

        return Status::Failure;
    }


    b.Set("FarmingTasks.initialized", true);
    LOG_INFO("Blocks initialized");

    return Status::Success;
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> FarmingTasks::CreateTree() {
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "FarmingTasks.initialized")
                .sequence()
                    .leaf(FarmingTasks::InitializeBlocks, 100)
                .end()
            .end()
        .end();
}