#include "AdvancedClient.hpp"

using namespace std::chrono;
using namespace Botcraft;

Vector3<double> AdvancedClient::getPosition() const {
    std::lock_guard<std::mutex> lock_localplayer(this->GetEntityManager()->GetLocalPlayer()->GetMutex());
    return this->GetEntityManager()->GetLocalPlayer()->GetPosition();
}

vector<Vector3<int>> AdvancedClient::findBlocks(bool(*match_function)(const Block *block), int search_radius, int max_blocks) const {
    vector<Vector3<int>> out;

    std::shared_ptr<World> world = this->GetWorld();
    std::lock_guard<std::mutex> lock_world(world->GetMutex());

    Position my_pos = getPosition();

    const Vector3<int> min_pos(my_pos.x - search_radius, max(my_pos.y - search_radius, world->GetMinY()), my_pos.z - search_radius);
    const Vector3<int> max_pos(my_pos.x + search_radius, min(my_pos.y + search_radius, world->GetHeight()), my_pos.z + search_radius);
    Vector3<int> current;
    
    int nb_checks = 0;
    auto start = high_resolution_clock::now();
    current.y = min_pos.y;
    while(current.y <= max_pos.y) {
        current.x = min_pos.x;
        while(current.x <= max_pos.x) {
            current.z = min_pos.z;
            while(current.z <= max_pos.z) {
                const Block *block = world->GetBlock(current);
                nb_checks++;
                if ( block != nullptr && match_function(block) ) {
                    out.push_back(Vector3(current));
                    if (out.size() == max_blocks) {
                        goto end;
                    }
                }
                current.z++;
            }
            current.x++;    
        }
        current.y++;
    }
end:
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    LOG_INFO("Checking " << nb_checks << " blocks took " << duration.count() << " milliseconds.");

    return out;
}

void AdvancedClient::sortPositionsFromNearest(vector<Vector3<int>> positions) const {
    Vector3<int> position = this->getPosition();

    auto start = high_resolution_clock::now();
    for (int i = positions.size() - 1; i > 0; i--) {
        bool sorted = true;
        for(int j = 0; j < i; j++) {
            if (positions[j+1].SqrDist(position) < positions[j].SqrDist(position)) {
                Vector3<int> temp = positions[j+1];
                positions[j+1] = positions[j];
                positions[j] = temp;
                sorted = false;
            }
        }
        if (sorted) break;
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    LOG_INFO("Sorting " << positions.size() << " positions " << duration.count() << " microseconds." );
}

Vector3<int> AdvancedClient::findNearestBlock(bool(*match_function)(const Block *block), int search_radius) const {
    const vector<Vector3<int>> blocks = this->findBlocks(match_function, search_radius);
    sortPositionsFromNearest(blocks);
    return blocks[0];
}

AdvancedClient::AdvancedClient(const bool use_renderer_) : TemplatedBehaviourClient<AdvancedClient>(use_renderer_)
{

}

AdvancedClient::~AdvancedClient()
{

}

