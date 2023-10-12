#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"
#include "botcraft/AI/Status.hpp"

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

    /// @brief Finds a unique bed based on the bot id (stored in the "id" blackboard key)
    /// @param client The client performing the action
    /// @param blackboard_key The blackboard key where the bed position will be stored
    /// @return Success if a bed was found, Failure otherwise.
    Botcraft::Status FindUniqueBedBlackboard(AdvancedClient &client, const std::string blackboard_key);

    std::shared_ptr<Botcraft::BehaviourTree<AdvancedClient>> CreateTree();

}