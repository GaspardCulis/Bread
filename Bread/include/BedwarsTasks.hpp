#pragma once
#include <botcraft/AI/BehaviourTree.hpp>
#include <botcraft/AI/BehaviourClient.hpp>

/// @brief Finds the closest bed
/// @param client The client performing the action
/// @return Success if the bed found, false otherwise
Botcraft::Status FindBed(Botcraft::BehaviourClient& client);