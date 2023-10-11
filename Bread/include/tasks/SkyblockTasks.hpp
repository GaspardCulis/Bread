#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"

namespace SkyblockTasks
{

    /// @brief Finds the closest bed
    /// @param client The client performing the action
    /// @return Success if the bed found, false otherwise
    Botcraft::Status InitializeBlocks(AdvancedClient &client, const int radius = 128);

    Botcraft::Status ChopTrees(AdvancedClient &client);

    Botcraft::Status MineCobblestone(AdvancedClient &client);

    Botcraft::Status Farm(AdvancedClient &client);
    
    std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateTree();

}