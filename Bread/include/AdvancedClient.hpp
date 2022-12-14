#pragma once

#include <vector>

#include <botcraft/Game/Entities/EntityManager.hpp>
#include <botcraft/AI/TemplatedBehaviourClient.hpp>
#include <botcraft/Game/Entities/LocalPlayer.hpp>
#include <botcraft/Game/World/World.hpp>
#include <botcraft/Utilities/Logger.hpp>
#include <botcraft/AI/Tasks/AllTasks.hpp>
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
     * Returns the client current position in the world
    */
    Vector3<double> getPosition() const;

    /**
     * Gets the block at a given position
    */
    const Block* getBlock(Vector3<int> position) const;

    /**
     * Gets the entity with the given id
    */
    const shared_ptr<Entity> getEntity(int id) const;

    /**
     * Finds all the blocks for which the match_function callback returned true in a certain radius. 
     * The search will stop and return when the number of found blocks equals max_results.
    */
    vector<Vector3<int>> findBlocks(std::function<bool(const Block *block)> match_function, int search_radius = 64, int max_results = -1) const;
    
    /**
     * Finds all the blocks matching the block_name (example: "minecraft:white_stained_glass", the minecraft: is required), in a certain radius. 
     * The search will stop and return when the number of found blocks equals max_results.
     * 
     * Alias for findBlocks(std::function<bool(const Block *block)>, int, int)
    */
    vector<Vector3<int>> findBlocks(const string block_name, int search_radius = 64, int max_results = -1) const;

    /**
     * Finds the nearest block for which the match_function calback returned true in a certain radius.
     * 
     * It gets all the surrounding blocks with findBlocks(std::function<bool(const Block *block)>, int, int), sorts the positions with sortPositionsFromNearest(vector<Vector3<int>>) and returns the first result.
    */
    Vector3<int> findNearestBlock(std::function<bool(const Block *block)> match_function, int search_radius = 64) const;

    /**
     * Finds the nearest block matching the block_name (example: "minecraft:white_stained_glass", the minecraft: is required), in a certain radius. 
     * 
     * It gets all the surrounding blocks with findBlocks(const string, int, int), sorts the positions with sortPositionsFromNearest(vector<Vector3<int>>) and returns the first result.
     * 
     * Alias for findNearestBlock(std::function<bool(const Block *block)>, int)
    */
    Vector3<int> findNearestBlock(const string block_name, int search_radius = 64) const;

    /**
     * Finds all the blocks for which the match_function callback returned true. 
     * The search will stop and return when the number of found blocks equals max_results.
    */
    vector<int> findEntities(std::function<bool(const std::shared_ptr<Entity> entity)> match_function, int max_results = -1) const;

    /**
     * Finds all the entities with the specified EntityType
     * The search will stop and return when the number of found blocks equals max_results.
    */
    vector<int> findEntities(const EntityType type, int max_results = -1) const;

    /**
     * Sorts a given vector of Positions from the nearest to the furthest from the client.
     * 
     * Uses std::sort with a custom comparator.
    */
    void sortPositionsFromClosest(vector<Vector3<int>> &positions) const;
    
    virtual ~AdvancedClient();
};