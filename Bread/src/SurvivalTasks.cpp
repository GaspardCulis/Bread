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
        },
        search_radius);
}