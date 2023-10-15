#pragma once

#include "AdvancedClient.hpp"
#include "botcraft/AI/Status.hpp"

namespace CraftingTasks
{
    Botcraft::Status CanCraft(AdvancedClient &client, const std::string &item_name);

    Botcraft::Status Craft(AdvancedClient &client, const std::string &item_name);
}
