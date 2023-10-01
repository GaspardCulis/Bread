#pragma once

#include "AdvancedClient.hpp"
#include "botcraft/Game/ManagersClient.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "protocolCraft/Types/Recipes/Ingredient.hpp"
#include "protocolCraft/Types/Recipes/Recipe.hpp"

namespace CraftingUtils {
    /// @brief Gets the result from a generic recipe
    const ProtocolCraft::Slot GetResult(const ProtocolCraft::Recipe &recipe);

    /// @brief Gets the ingredients for a generic recipe
    const std::vector<ProtocolCraft::Ingredient> GetIngredients(ProtocolCraft::Recipe &recipe);

    /// @brief Gets all available recipes to craft a given item given an AdvancedClient recipe list
    std::vector<ProtocolCraft::Recipe> GetAvailableRecipes(AdvancedClient& client, const std::string& item_name);

    /// @brief Returns true if the client has the required ingredients to craft the recipe
    bool CanCraft(ManagersClient& client, ProtocolCraft::Recipe &recipe);
}
