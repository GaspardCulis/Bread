#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"

namespace FarmingTasks {

/// @brief Check condition
/// @param client The client performing the action
/// @param radius The max distance to search blocks
/// @return Returns success if all blocks have been found
Botcraft::Status InitializeBlocks(AdvancedClient& client, const int radius = 128);

/// @brief Go fishing
/// @param client The client performing the action
/// @return Returns success if fish has been aquired
Botcraft::Status Fish(AdvancedClient& client, const std::string& block_pos_blackboard);

/// @brief Break carrots/potatoes/wheat/pumpkin/melon, collect the items, replant
/// @param client The client performing the action
/// @param blocks_pos_blackboard Blackboard address of the workstation block position
/// @param item_name Name of the item we want to collect
/// @return Success if blocks are broken, items gathered and crops replanted, Failure otherwise
Botcraft::Status CollectCropsAndReplant(Botcraft::BehaviourClient& client, const std::string& block_pos_blackboard, const int crops_radius = 8);

std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateTree();
}