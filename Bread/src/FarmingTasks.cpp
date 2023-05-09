#include "FarmingTasks.hpp"

Botcraft::Status FarmingTasks::InitializeBlocks(AdvancedClient& client, const int search_radius) {
    Botcraft::Blackboard& b = client.GetBlackboard();

    Position fishing_workstation_position;
    Position farming_workstation_position;

    try {
        client.findBlocks([&client, &fishing_workstation_position, &farming_workstation_position](const Block *block, const Position position) -> bool {
            if (block->GetBlockstate()->GetName() == "minecraft:barrel" && client.getBlock(position + Position(0, -1, 0))->GetBlockstate()->GetName() == "minecraft:gold_block") {
                fishing_workstation_position = position;
                LOG_INFO("Fishing workstation found at: " << fishing_workstation_position << "!");
            } else if (block->GetBlockstate()->GetName() == "minecraft:composter" && client.getBlock(position + Position(0, -1, 0))->GetBlockstate()->GetName() == "minecraft:gold_block") {
                farming_workstation_position = position;
                LOG_INFO("Farming workstation found at: " << farming_workstation_position << "!");
            }

            return false;
        }, search_radius);
    } catch(std::exception &e) {
        LOG_ERROR("Error while searching workstations: " << e.what());

        return Status::Failure;
    }


    b.Set("FarmingTasks.initialized", true);

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