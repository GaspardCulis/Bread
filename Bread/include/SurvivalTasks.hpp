#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"

namespace SurvivalTasks
{

    /// @brief Finds the closest bed
    /// @param client The client performing the action
    /// @return Always success
    Botcraft::Status InitializeBlocks(AdvancedClient &client, const int search_radius = 128);

    std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateTree();

}