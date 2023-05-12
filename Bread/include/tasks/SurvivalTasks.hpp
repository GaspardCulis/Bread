#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"

namespace SurvivalTasks
{

    /// @brief Finds the closest bed
    /// @param client The client performing the action
    /// @return Always success
    Botcraft::Status InitializeBlocks(AdvancedClient &client, const int search_radius = 128);

    /// @brief Searches the client's inventory for the most nutritious food, saves the item name in Blackboard value "SurvivalTasks.best_food_in_inventory"
    /// @param client The client performing the action
    /// @return Failure if no edible food in the inventory
    Botcraft::Status FindBestFoodInInventory(AdvancedClient &client);

    std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateTree();

}