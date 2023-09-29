#pragma once

#include "protocolCraft/Messages/Play/Clientbound/ClientboundUpdateRecipesPacket.hpp"
#include "protocolCraft/Types/Recipes/Recipe.hpp"
#include <vector>

#include <botcraft/Game/Entities/EntityManager.hpp>
#include <botcraft/AI/TemplatedBehaviourClient.hpp>
#include <botcraft/Game/Entities/LocalPlayer.hpp>
#include <botcraft/Game/World/World.hpp>
#include <botcraft/Utilities/Logger.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
#include <botcraft/Game/Inventory/Item.hpp>
#include <time.h>
#include <bits/stdc++.h>
#include <chrono>

using namespace Botcraft;
using namespace std;

class AdvancedClient : public TemplatedBehaviourClient<AdvancedClient>
{
public:
    AdvancedClient(const bool use_renderer_ = false);

    /**
     * @brief Returns the client current position in the world
     */
    Vector3<double> getPosition() const;

    /**
     * @brief Gets the block at a given position
     */
    const Block *getBlock(Vector3<int> position) const;

    /**
     * @brief Gets the entity with the given id
     */
    const shared_ptr<Entity> getEntity(int id) const;

    /**
     * @brief Finds all the blocks for which the match_function callback returned true in a certain radius.
     * @param match_function If this function given the current Block returns true, the Block will be added to the result. Callback parameters are explained in the Model Parameters
     * @tparam block: Pointer to the current Block being evaluated.
     * @tparam position: The position of the current Block.
     * @tparam world: Shared pointer to the World object being searched. Use it as the world Mutex is otherwise locked.
     * @param search_radius The maximum distance for the search, it's a cube not a sphere.
     * @param max_results The maximum results for the search, the function returns when reached.
     * @param origin The search radius center, defaults to the player position
     */
    vector<Vector3<int>> findBlocks(std::function<bool(const Block *block, const Position position, std::shared_ptr<World> world)> match_function, const int search_radius = 64, const int max_results = -1, const std::optional<Position> origin = std::nullopt) const;

    /**
     * Finds all the blocks matching the block_name in a certain radius.
     * The search will stop and return when the number of found blocks equals max_results.
     * @param block_name The searched block name (Ex: "minecraft:pumpkin")
     * @param search_radius The maximum distance for the search, it's a cube not a sphere.
     * @param max_results The maximum results for the search, the function returns when reached.
     * @param origin The search radius center, defaults to the player position
     * Alias for findBlocks(std::function<bool(const Block *block, const Position position)>, int, int)
     */
    vector<Vector3<int>> findBlocks(const string block_name, const int search_radius = 64, const int max_results = -1, const std::optional<Position> origin = std::nullopt) const;

    /**
     * Finds the nearest block for which the match_function calback returned true in a certain radius.
     *
     * It gets all the surrounding blocks with findBlocks(std::function<bool(const Block *block, const Position position)>, int, int), sorts the positions with sortPositionsFromNearest(vector<Vector3<int>>) and returns the first result, throws std::range_error if not found.
     */
    Vector3<int> findNearestBlock(std::function<bool(const Block *block, const Position position, std::shared_ptr<World> world)> match_function, const int search_radius = 64, const std::optional<Position> origin = std::nullopt) const;

    /**
     * Finds the nearest block matching the block_name (example: "minecraft:white_stained_glass", the minecraft: is required), in a certain radius.
     *
     * It gets all the surrounding blocks with findBlocks(const string, int, int), sorts the positions with sortPositionsFromNearest(vector<Vector3<int>>) and returns the first result, throws std::range_error if not found.
     *
     * Alias for findNearestBlock(std::function<bool(const Block *block, const Position position)>, int)
     */
    Vector3<int> findNearestBlock(const string block_name, const int search_radius = 64, const std::optional<Position> origin = std::nullopt) const;

    /**
     * Finds all the blocks for which the match_function callback returned true.
     * The search will stop and return when the number of found blocks equals max_results.
     */
    std::set<int> findEntities(std::function<bool(const std::shared_ptr<Entity> entity)> match_function, int max_results = -1) const;

    /**
     * Finds all the entities with the specified EntityType
     * The search will stop and return when the number of found blocks equals max_results.
     */
    std::set<int> findEntities(const EntityType type, int max_results = -1) const;

    /**
     * @brief Finds the item slot in the client's inventory for which the match_function callback returned true.
     * @param match_function This function given the current slot returns true if this is the required item.
     * @returns The slod id of the target item, -1 if not found
     */
    short getItemSlotInInventory(std::function<bool(short slodId, ProtocolCraft::Slot current_slot, Botcraft::Item *item)> match_function);

    /**
     * @brief Finds the item slot in the client's inventory matching the block_name.
     * @param item_name The target item
     * @return The target item slotId in the client's inventory, -1 if not found.
     * Alias for getItemSlotInInventory(std::function<bool(short slodId, ProtocolCraft::Slot current_slot, Botcraft::Item)> match_function)
     */
    short getItemSlotInInventory(const std::string item_name);

    /**
     * @brief Gets the total number of items in the inventory for which the match_function callback retured true.
     * @param match_function This function given the current slot returns true if this is the counted item.
     * @return The total number of items matching match_function
    */
    int getItemCountInInventory(std::function<bool(short slodId, ProtocolCraft::Slot current_slot, Botcraft::Item *item)> match_function);

    /**
     * @brief Gets the total number of items in the inventory matching the block name
     * @param item_name The counted item name
     * @return The item count
    */
    int getItemCountInInventory(const std::string item_name);

     /**
     * @brief Sends a chat message only one time no matter how many times this method gets called
     * @param message The message to say
     * @param message_identifier The string literal that previous existence will be checked, defaults to message to say
     * @return true if message has been said.
     */
    bool sendOTM(const std::string message, const std::optional<std::string> message_identifier = std::nullopt);

    void resetOTM(const std::string message_identifier);

    /**
     * Sorts a given vector of Positions from the nearest to the furthest from the origin.
     *
     * Uses std::sort with a custom comparator.
     */
    void sortPositionsFromClosest(vector<Vector3<int>> &positions, const Vector3<double> origin) const;

    virtual ~AdvancedClient();

 protected:
    virtual void Handle(ProtocolCraft::ClientboundUpdateRecipesPacket &msg) override;

 protected:
    std::vector<ProtocolCraft::Recipe> available_recipes;
};