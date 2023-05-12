#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"

namespace AdvancedTasks
{

    /// @brief Pathfinds to an item and wait for pickup
    /// @param id The item's entity id
    /// @returns Success if item has been collected, Failure otherwise (4s collect timeout)
    Botcraft::Status CollectItem(AdvancedClient &client, int id);

    /// @brief Digs a given block and tries to collect the drop. Pathfinds to the block and the item drop. Automatically finds new item drops
    /// @param client The client performing the action
    /// @param position The block position
    /// @return Success if the bed found, false otherwise
    Botcraft::Status DigAndCollect(AdvancedClient &client, const Position position);

}