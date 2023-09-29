#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"

namespace FarmingTasks
{

    /// @brief Check condition
    /// @param client The client performing the action
    /// @param radius The max distance to search blocks
    /// @return Returns success if all blocks have been found
    Botcraft::Status InitializeBlocks(AdvancedClient &client, const int radius = 128);

    /// @brief Go fishing
    /// @param client The client performing the action
    /// @return Returns success if fish has been aquired
    Botcraft::Status Fish(AdvancedClient &client);

    /// @brief Break carrots/potatoes/wheat/pumpkin/melon, collect the items, replant
    /// @param client The client performing the action
    /// @param blocks_pos_blackboard Blackboard address of the workstation block position
    /// @param item_name Name of the item we want to collect
    /// @return Success if blocks are broken, items gathered and crops replanted, Failure otherwise
    Botcraft::Status CollectCropsAndReplant(AdvancedClient &client, const int crops_radius = 8);

    Botcraft::Status MaintainField(AdvancedClient &client);

    /// @brief Composts the vegetable matching item_name, keeping a minimum of keep_count
    /// @param client The client performing the action
    /// @param item_name The minecraft item name of the vegetable
    /// @param keep_count The number of vegetables the bot should keep, not composting them
    /// @return Success if the items could be composted
    Botcraft::Status CompostVegetables(AdvancedClient &client, const std::string item_name, const int keep_count);

    Botcraft::Status SmeltFood(AdvancedClient &client);

    std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateTree();
}