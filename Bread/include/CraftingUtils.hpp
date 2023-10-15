#pragma once

#include "AdvancedClient.hpp"
#include "botcraft/Game/ManagersClient.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "protocolCraft/Types/Recipes/Ingredient.hpp"
#include "protocolCraft/Types/Recipes/Recipe.hpp"
#include "protocolCraft/Types/Slot.hpp"

namespace CraftingUtils {
    /// @brief Gets the result from a generic recipe
    const ProtocolCraft::Slot GetResult(const ProtocolCraft::Recipe &recipe);

    /// @brief Gets the ingredients for a generic recipe
    const std::vector<ProtocolCraft::Ingredient> GetIngredients(const ProtocolCraft::Recipe &recipe);

    /// @brief Gets the workstation block name for a generic recipe
    const std::string GetWorkstation(const ProtocolCraft::Recipe &recipe);

    /// @brief Checks if the recipe needs a workstation or else if it can be crafted in the inventory.
    const bool NeedsWorkstation(const ProtocolCraft::Recipe &recipe);

    /// @brief Gets all available recipes to craft a given item given an AdvancedClient recipe list
    std::vector<ProtocolCraft::Recipe> GetAvailableRecipes(AdvancedClient& client, const std::string& item_name);

    /// @brief Takes a recipe, and returns the 3x3 crafting matrix taking into a count the items in the client's inventory
    const std::array<std::array<int, 3>, 3> CreateCraftingMatrix(ManagersClient &client, ProtocolCraft::Recipe &recipe);

    /// @brief Returns true if the client has the required ingredients to craft the recipe
    bool CanCraft(ManagersClient& client, ProtocolCraft::Recipe &recipe);
}
