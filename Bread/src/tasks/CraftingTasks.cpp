#include "tasks/CraftingTasks.hpp"
#include "AdvancedClient.hpp"
#include "CraftingUtils.hpp"
#include "botcraft/AI/Status.hpp"

Botcraft::Status CanCraft(AdvancedClient &client, const std::string &item_name)
{
    for (auto recipe : CraftingUtils::GetAvailableRecipes(client, item_name))
    {
        if (CraftingUtils::CanCraft(client, recipe))
            return Status::Success;
    }
    return Status::Failure;
}
