#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"
#include "botcraft/Game/Vector3.hpp"

namespace AdvancedTasks
{

    /// @brief Pathfinds to an item and wait for pickup
    /// @param id The item's entity id
    /// @returns Success if item has been collected, Failure otherwise (4s collect timeout)
    Botcraft::Status CollectItem(AdvancedClient &client, int id);

    /// @brief Collects all of the items for which match_function returns true in a given radius around the client
    /// @param match_function If this function returns true for a given item entity, it will be added to the collect list
    /// @param collect_radius The radius around the player for which items will be collected (defaults to 0, unlimited range)
    Botcraft::Status CollectItems(AdvancedClient & client, std::function<bool(const int entity_id, std::shared_ptr<Botcraft::Entity> entity)> match_function, const unsigned int collect_radius = 0);

    /// @brief Collects all of the items matching item_name in a given radius around the client
    /// @param item_name The collected item name
    /// @param collect_radius The radius around the player for which items will be collected (defaults to 0, unlimited range)
    Botcraft::Status CollectItems(AdvancedClient & client, const std::string &item_name, const unsigned int collect_radius = 0);

    /// @brief Digs a given block and tries to collect the drop. Pathfinds to the block and the item drop. Automatically finds new item drops
    /// @param client The client performing the action
    /// @param position The block position
    /// @return Success if the bed found, Failure otherwise
    Botcraft::Status DigAndCollect(AdvancedClient &client, const Position position);

    /// @brief Ensures there is the required count of items in the inventory, storing/retriving them in/from a container
    /// @param client The client performing the action
    /// @param storage_pos The storage position
    /// @param item The item to check
    /// @param min The inclusive minimum count
    /// @param max The inclusive maximum count
    /// @return Success if the item count is correct, Failure otherwise
    Botcraft::Status EnsureItemCount(AdvancedClient &client, const Position storage_pos, const std::string &item_name, const int min, const int max);

    /// @brief Finds the nearest block and stores its position to a blackboard entry
    /// @param client The client performing the action
    /// @param block_name The searched block name
    /// @param blackboard_index The blackboard index where the position will be stored
    /// @return Success if the block has been found, Failure otherwise
    Botcraft::Status FindNearestBlockBlackboard(AdvancedClient &client, const std::string block_name, const std::string blackboard_index);
}