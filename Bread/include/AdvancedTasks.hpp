#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include "AdvancedClient.hpp"

namespace AdvancedTasks {

/// @brief Digs a given block and tries to collect the drop. Only pathfinds to the item drop!
/// @param client The client performing the action
/// @param position The block position
/// @param item_name The drop item name
/// @return Success if the bed found, false otherwise
Botcraft::Status DigAndCollect(AdvancedClient& client, const Position position, const std::string &item_name);
    
}