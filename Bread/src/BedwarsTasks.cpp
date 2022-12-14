#include "BedwarsTasks.hpp"


using namespace std;
using namespace Botcraft;
using namespace ProtocolCraft;

Status InitializeBlocks(AdvancedClient& client, int search_radius) {
    Blackboard& b = client.GetBlackboard();

    Vector3<int> bed_position = client.findNearestBlock([](const Block *block) -> bool {
        const string suffix = "_bed";
        const string& block_name = block->GetBlockstate()->GetName();
        return block_name.size() >= suffix.size() && 0 == block_name.compare(block_name.size()-suffix.size(), suffix.size(), suffix);
    }, search_radius);

    if (bed_position == NULL) {
        return Status::Failure;
    }

    b.Set("bed_position", bed_position);
    LOG_INFO("Found bed at " << bed_position);

    b.Set("BedwarsTasks.initialized", true);

    return Status::Success;

}

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> createTree() {
    return Botcraft::Builder<AdvancedClient>()
        .sequence()
            .selector()
                .leaf(CheckBlackboardBoolData, "BedwarsTasks.initialized")
                .sequence()
                    .leaf(InitializeBlocks, 100)
                .end()
            .end()
        .end()
    .build();
}