#include "CraftingUtils.hpp"
#include "AdvancedClient.hpp"
#include "botcraft/AI/Status.hpp"
#include "botcraft/Game/Inventory/InventoryManager.hpp"
#include "botcraft/Game/Inventory/Item.hpp"
#include "botcraft/Game/Inventory/Window.hpp"
#include "botcraft/Game/ManagersClient.hpp"
#include "botcraft/Game/Vector3.hpp"
#include "botcraft/Utilities/Logger.hpp"
#include "protocolCraft/Types/Identifier.hpp"
#include "protocolCraft/Types/Recipes/Ingredient.hpp"
#include "protocolCraft/Types/Recipes/Recipe.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeData.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataBlasting.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataCampfireCooking.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataNone.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataShaped.hpp"
#include "botcraft/Game/AssetsManager.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataShapeless.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataSmelting.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataSmithingTransform.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataSmithingTrim.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataSmoking.hpp"
#include "protocolCraft/Types/Recipes/RecipeTypeDataStoneCutting.hpp"
#include "protocolCraft/Types/Slot.hpp"
#include <stdexcept>
#include <utility>

const ProtocolCraft::Slot CraftingUtils::GetResult(const ProtocolCraft::Recipe &recipe)
{
    const ProtocolCraft::Identifier &recipe_type = recipe.GetType();

    if (recipe_type.GetFull() == "minecraft:crafting_shapeless")
    {
        return std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataShapeless>(recipe.GetData())->GetResult();
    }
    else if (recipe_type.GetFull() == "minecraft:crafting_shaped")
    {
        return std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataShaped>(recipe.GetData())->GetResult();
    }
    else if (recipe_type.GetFull() == "minecraft:smelting")
    {
        return std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataSmelting>(recipe.GetData())->GetResult();
    }
#if PROTOCOL_VERSION > 452 /* > 1.13.2 */
    else if (recipe_type.GetFull() == "minecraft:blasting")
    {
        return std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataBlasting>(recipe.GetData())->GetResult();
    }
    else if (recipe_type.GetFull() == "minecraft:smoking")
    {
        return std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataSmoking>(recipe.GetData())->GetResult();
    }
    else if (recipe_type.GetFull() == "minecraft:campfire_cooking")
    {
        return std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataCampfireCooking>(recipe.GetData())->GetResult();
    }
    else if (recipe_type.GetFull() == "minecraft:stonecutting")
    {
        return std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataStoneCutting>(recipe.GetData())->GetResult();
    }
#endif
    // TODO Smithing
    else 
    {
        return ProtocolCraft::Slot();
    }

}

const std::vector<ProtocolCraft::Ingredient> CraftingUtils::GetIngredients(const ProtocolCraft::Recipe &recipe)
{
    const ProtocolCraft::Identifier &recipe_type = recipe.GetType();

    if (recipe_type.GetFull() == "minecraft:crafting_shapeless")
    {
        return std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataShapeless>(recipe.GetData())->GetIngredients();
    }
    else if (recipe_type.GetFull() == "minecraft:crafting_shaped")
    {
        return std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataShaped>(recipe.GetData())->GetIngredients();
    }
    else if (recipe_type.GetFull() == "minecraft:smelting")
    {
        return std::vector<ProtocolCraft::Ingredient> {
            std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataSmelting>(recipe.GetData())->GetIngredient()
        };
    }
#if PROTOCOL_VERSION > 452 /* > 1.13.2 */
    else if (recipe_type.GetFull() == "minecraft:blasting")
    {
        return std::vector<ProtocolCraft::Ingredient> {
            std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataBlasting>(recipe.GetData())->GetIngredient()
        };
    }
    else if (recipe_type.GetFull() == "minecraft:smoking")
    {
        return std::vector<ProtocolCraft::Ingredient> {
            std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataSmoking>(recipe.GetData())->GetIngredient()
        };
    }
    else if (recipe_type.GetFull() == "minecraft:campfire_cooking")
    {
        return std::vector<ProtocolCraft::Ingredient> {
            std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataCampfireCooking>(recipe.GetData())->GetIngredient()
        };
    }
    else if (recipe_type.GetFull() == "minecraft:stonecutting")
    {
        return std::vector<ProtocolCraft::Ingredient> {
            std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataStoneCutting>(recipe.GetData())->GetIngredient()
        };
    }
#endif
    //  TODO Smithing
    else 
    {
        return std::vector<ProtocolCraft::Ingredient> ();
    }
}

const std::string CraftingUtils::GetWorkstation(const ProtocolCraft::Recipe &recipe) {
    const ProtocolCraft::Identifier &recipe_type = recipe.GetType();

    if (recipe_type.GetFull() == "minecraft:crafting_shapeless")
    {
        return "minecraft:crafting_table";
    }
    else if (recipe_type.GetFull() == "minecraft:crafting_shaped")
    {
        return "minecraft:crafting_table";
    }
    else if (recipe_type.GetFull() == "minecraft:smelting")
    {
        return "minecraft:furnace";
    }
#if PROTOCOL_VERSION > 452 /* > 1.13.2 */
    else if (recipe_type.GetFull() == "minecraft:blasting")
    {
        return "minecraft:blast_furnace";
    }
    else if (recipe_type.GetFull() == "minecraft:smoking")
    {
        return "minecraft:smoker";
    }
    else if (recipe_type.GetFull() == "minecraft:campfire_cooking")
    {
        return "minecraft:campfire";
    }
    else if (recipe_type.GetFull() == "minecraft:stonecutting")
    {
        return "minecraft:stonecutter";
    }
#endif
    // TODO Smithing
    else 
    {
        return "minecraft:crafting_table";
    }

}

const bool CraftingUtils::NeedsWorkstation(const ProtocolCraft::Recipe &recipe) {
    const ProtocolCraft::Identifier &recipe_type = recipe.GetType();

    if (recipe_type.GetFull() == "minecraft:crafting_shaped")
    {
        auto shaped_recipe = std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataShaped>(recipe.GetData());
        return shaped_recipe->GetWidth() > 2 && shaped_recipe->GetHeight() > 2;
    }
    else if (recipe_type.GetFull() == "minecraft:crafting_shapeless")
    {
        auto shapeless_recipe = std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataShapeless>(recipe.GetData());
        return shapeless_recipe->GetIngredients().size() > 4;
    }
    else 
    {
        return true;
    }
}

std::vector<ProtocolCraft::Recipe> CraftingUtils::GetAvailableRecipes(AdvancedClient& client, const std::string& item_name)
{
    std::vector<ProtocolCraft::Recipe> recipes;
        
    const auto item_id = AssetsManager::getInstance().GetItemID(item_name);
    for (ProtocolCraft::Recipe r : client.getRecipes())
    {
        if (GetResult(r).GetItemID() == item_id)
        {
            recipes.push_back(r);
        }
    }

    return recipes;
}

const std::array<std::array<int, 3>, 3> CraftingUtils::CreateCraftingMatrix(ManagersClient &client, ProtocolCraft::Recipe &recipe)
{
    std::array<std::array<int, 3>, 3> out {{
        {-1, -1, -1},
        {-1, -1, -1},
        {-1, -1, -1},
    }};
    int w = 3;
    int h = 3;

    if (!NeedsWorkstation(recipe)) {
        // Can be crafted in player's inventory
        w = 2;
        h = 2;
    }

    auto ingredients = GetIngredients(recipe);

    // Clone player inventory

    // map<itemId, count>
    std::map<short, int> abstract_inv;
    {
        std::shared_ptr<InventoryManager> inventory_manager = client.GetInventoryManager();
        std::lock_guard<std::mutex> inventory_lock(inventory_manager->GetMutex());

        const std::map<short, ProtocolCraft::Slot> &slots = inventory_manager->GetPlayerInventory()->GetSlots();
        for (auto it = slots.begin(); it != slots.end(); ++it)
        {
            if (abstract_inv.count(it->first))
                abstract_inv[it->second.GetItemID()] += (int) it->second.GetItemCount();
            else
                abstract_inv[it->second.GetItemID()] = (int) it->second.GetItemCount();
        }
    }

    int x = 0;
    int y = 0;
    for (int i = 0; i < ingredients.size(); i++)
    {
        x = i % w;
        y = i / h;

        auto ing = ingredients[i];

        if (ing.GetItems().size() == 0) continue;
        short found_item_id = -1;
        char found_item_count = 0;
        for (auto slot : ing.GetItems())
        {
            if (abstract_inv.count(slot.GetItemID()) !=0 && abstract_inv[slot.GetItemID()] >= slot.GetItemCount())
            {
                found_item_id = slot.GetItemID();
                found_item_count = slot.GetItemCount();
                break;
            }
        }
        // If we didn't find an item
        if (found_item_id < 0)
        {
            throw std::out_of_range("Counld't find an item in inventory satisfaying ingredients for recipe");
        } 
        abstract_inv[found_item_id] -= found_item_count;

        out[y][x] = found_item_id;
      
    }

    return out;
}

bool CraftingUtils::CanCraft(ManagersClient &client, ProtocolCraft::Recipe &recipe)
{
    try
    {
        auto _ = CreateCraftingMatrix(client, recipe);
        return true;
    }
    catch (range_error)
    {
        return false;
    }
}