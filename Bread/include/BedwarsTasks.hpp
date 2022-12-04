#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"

/// @brief Finds the closest bed
/// @param client The client performing the action
/// @return Success if the bed found, false otherwise
Botcraft::Status InitializeBlocks(AdvancedClient& client, int search_radius);

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> createTree();