#pragma once

#include <vector>

#include <botcraft/Game/Entities/EntityManager.hpp>
#include <botcraft/AI/TemplatedBehaviourClient.hpp>
#include <botcraft/Game/Entities/LocalPlayer.hpp>
#include <botcraft/Game/World/World.hpp>
#include <botcraft/Utilities/Logger.hpp>
#include <time.h>
#include <bits/stdc++.h>
#include <chrono>

using namespace Botcraft;
using namespace std;

class AdvancedClient : public TemplatedBehaviourClient<AdvancedClient>
{
public:
    AdvancedClient(const bool use_renderer_ = false);

    Vector3<double> getPosition() const;

    vector<Vector3<int>> findBlocks(bool(*match_function)(const Block *block), int search_radius = 64, int max_blocks = -1) const;

    void sortPositionsFromNearest(vector<Vector3<int>> positions) const;

    Vector3<int> findNearestBlock(bool(*match_function)(const Block *block), int search_radius = 64) const;
    
    virtual ~AdvancedClient();
};