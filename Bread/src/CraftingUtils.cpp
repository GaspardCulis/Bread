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

    if (recipe_type.GetFull() == "minecraft:crafting_shapeless")
    {
        auto shaped_recipe = std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataShaped>(recipe.GetData());
        return shaped_recipe->GetWidth() > 2 && shaped_recipe->GetHeight() > 2;
    }
    else if (recipe_type.GetFull() == "minecraft:crafting_shaped")
    {
        auto shapeless_recipe = std::dynamic_pointer_cast<ProtocolCraft::RecipeTypeDataShapeless>(recipe.GetData());
        // Need to iterate because recipe contains empty ingredients for some reason
        int count = 0;
        for (auto ingredient : shapeless_recipe->GetIngredients()) {
            if (ingredient.GetItems().size() > 0) {
                count++;
            }
        }
        return count > 4;
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

std::map<short, int> CraftingUtils::GetIngredientsFromInventory(ManagersClient &client, ProtocolCraft::Recipe &recipe)
{
    // map<ItemId, count>
    std::map<short, int> out;
    
    auto ingredients = GetIngredients(recipe);

    // Clone player inventory
    std::shared_ptr<InventoryManager> inventory_manager = client.GetInventoryManager();
    std::lock_guard<std::mutex> inventory_lock(inventory_manager->GetMutex());

    const std::map<short, ProtocolCraft::Slot> &slots = inventory_manager->GetPlayerInventory()->GetSlots();
    // map<itemId, count>
    std::map<short, int> abstract_inv;
    for (auto it = slots.begin(); it != slots.end(); ++it)
    {
        if (abstract_inv.count(it->first))
            abstract_inv[it->second.GetItemID()] += (int) it->second.GetItemCount();
        else
            abstract_inv[it->second.GetItemID()] = (int) it->second.GetItemCount();
    }

    for (auto i : ingredients)
    {
        if (i.GetItems().size() == 0) continue;
        short found_item_id = -1;
        char found_item_count = 0;
        for (auto p : i.GetItems())
        {
            if (abstract_inv.count(p.GetItemID()) !=0 && abstract_inv[p.GetItemID()] >= p.GetItemCount())
            {
                found_item_id = p.GetItemID();
                found_item_count = p.GetItemCount();
                break;
            }
        }
        // If we didn't find an item
        if (found_item_id < 0)
        {
            throw std::out_of_range("Counld't find an item in inventory satisfaying ingredients for recipe");
        } 
        abstract_inv[found_item_id] -= found_item_count;
        if (out.count(found_item_id) == 0) out[found_item_id] = 0;
        out[found_item_id] += found_item_count;
    }

    return out;
}

bool CraftingUtils::CanCraft(ManagersClient &client, ProtocolCraft::Recipe &recipe)
{
    try
    {
        auto _ = GetIngredientsFromInventory(client, recipe);
        return true;
    }
    catch (range_error)
    {
        return false;
    }
}