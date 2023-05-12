#include "SurvivalTasks.hpp"

Botcraft::Status SurvivalTasks::InitializeBlocks(AdvancedClient &client, const int search_radius)
{
    Botcraft::Blackboard &b = client.GetBlackboard();

    auto _ = client.findBlocks(
        [&b](const Block *block, const Position position, std::shared_ptr<World> world) -> bool
        {
            Position down = position + Position(0, -1, 0);
            const std::string &block_name = block->GetBlockstate()->GetName();
            const std::string bed_suffix = "_bed";

            if (
                block_name.size() >= bed_suffix.size() &&
                0 == block_name.compare(block_name.size() - bed_suffix.size(), bed_suffix.size(), bed_suffix) &&
                world->GetBlock(down)->GetBlockstate()->GetName() == "minecraft:gold_block")
            {
                b.Set("SurvivalTasks.bed_pos", position);
                LOG_INFO(block_name << "found at: " << position << "!");
            }

            return false;
        },
        search_radius);

    b.Set("SurvivalTasks.initialized", true);
    LOG_INFO("Blocks initialized");

    return Status::Success;
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> SurvivalTasks::CreateTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "SurvivalTasks.initialized")
                .sequence()
                    .leaf(SurvivalTasks::InitializeBlocks, 128)
                .end()
            .end()
        .end();
    // clang-format on
}