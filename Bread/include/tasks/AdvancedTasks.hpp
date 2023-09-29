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

    /// @brief Digs a given block and tries to collect the drop. Pathfinds to the block and the item drop. Automatically finds new item drops
    /// @param client The client performing the action
    /// @param position The block position
    /// @return Success if the bed found, Failure otherwise
    Botcraft::Status DigAndCollect(AdvancedClient &client, const Position position);

    /// @brief Ensures there is the required count of items in the inventory, storing/retriving them in/from a container
    /// @param client The client performing the action
    /// @param client The storage position
    /// @param item The item to check
    /// @param min The inclusive minimum count
    /// @param max The inclusive maximum count
    /// @return Success if the item count is correct, Failure otherwise
    Botcraft::Status EnsureItemCount(AdvancedClient &client, const Position storage_pos, const std::string &item_name, const int min, const int max);

}