#include "tasks/CraftingTasks.hpp"
#include "AdvancedClient.hpp"
#include "CraftingUtils.hpp"
#include "botcraft/AI/Status.hpp"
#include "botcraft/AI/Tasks/InventoryTasks.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/AI/Tasks/PathfindingTask.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Utilities/Logger.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataShaped.hpp"
#include <chrono>
#include <mutex>
#include <stdexcept>

Botcraft::Status CraftingTasks::CanCraft(AdvancedClient &client, const std::string &item_name) {
    for (auto recipe : CraftingUtils::GetAvailableRecipes(client, item_name))
    {
        if (CraftingUtils::CanCraft(client, recipe)) {
            LOG_INFO("[CanCraft] I can craft " << item_name);
            return Status::Success;
        }
    }
    LOG_INFO("[CanCraft] I can't craft " << item_name);
    return Status::Failure;
}

Botcraft::Status CraftingTasks::Craft(AdvancedClient &client, const std::string &item_name) {
    LOG_INFO("[Craft] Started task for item " << item_name);
    
    auto recipes = CraftingUtils::GetAvailableRecipes(client, item_name);
    if (recipes.size() == 0) {
        LOG_WARNING("[Craft] No available recipe for " << item_name);
        return Status::Failure;
    }

    auto it = recipes.begin();
    while (it != recipes.end() && !CraftingUtils::CanCraft(client, *it)) {
        it++;
    }

    if (it == recipes.end()) {
        LOG_WARNING("[Craft] Required ingredients not in inventory");
        return Status::Failure;
    }

    auto recipe = *it;

    bool needs_workstation = CraftingUtils::NeedsWorkstation(recipe);

    if (needs_workstation) {
        const std::string workstation = CraftingUtils::GetWorkstation(recipe);

        Position workstation_pos;
        try {
            workstation_pos = client.findNearestBlock(workstation);  
        } catch (std::range_error) {
            LOG_WARNING("[Craft] Couldn't find workstation " << workstation);
            return Status::Failure;
        }

        if (GoTo(client, workstation_pos, 3, 2) == Status::Failure) {
            LOG_WARNING("[Craft] Couldn't pathfind to " << workstation << " at " << workstation_pos);
            return Status::Failure;
        }

    
        if (recipe.GetType().GetFull() == "minecraft:campfire_cooking") {
            // TODO
            LOG_WARNING("[Craft] Campfire cooking not supported");
            return Status::Failure;
        } else if (OpenContainer(client, workstation_pos) == Status::Failure) {
            LOG_WARNING("[Craft] Couldn't open " << workstation << " at " << workstation_pos);
            return Status::Failure;
        }

    }

    // Can throw range_error but shouldn't due to prior checks
    auto crafting_matrix = CraftingUtils::CreateCraftingMatrix(client, recipe);

    
    if (Craft(client, crafting_matrix, !needs_workstation) == Status::Success) {
        LOG_INFO("[Craft] Crafted " << item_name);
        return Status::Success;
    } else {
        LOG_WARNING("[Craft] Failed to craft item " << item_name);
        return Status::Failure;
    }
}
