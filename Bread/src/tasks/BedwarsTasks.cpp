#include "tasks/BedwarsTasks.hpp"

using namespace std;
using namespace Botcraft;
using namespace ProtocolCraft;

Status BedwarsTasks::InitializeBlocks(AdvancedClient &client, int search_radius)
{
    Blackboard &b = client.GetBlackboard();

    try
    {
        Vector3<int> bed_position = client.findNearestBlock([](const Blockstate *block, const Position _, std::shared_ptr<World> __) -> bool
                                                            {
            const string suffix = "_bed";
            const string& block_name = block->GetName();
            return block_name.size() >= suffix.size() && 0 == block_name.compare(block_name.size()-suffix.size(), suffix.size(), suffix); },
                                                            search_radius);

        b.Set("bed_position", bed_position);
        LOG_INFO("Found bed at " << bed_position);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Error finding bed: " << e.what());
        return Status::Failure;
    }
    b.Set("BedwarsTasks.initialized", true);

    return Status::Success;
}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> BedwarsTasks::CreateTree()
{
    // clang-format off
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "BedwarsTasks.initialized")
                .sequence()
                    .leaf(BedwarsTasks::InitializeBlocks, 100)
                .end()
            .end()
        .end();
    // clang-format on
}