//
// Created by xhy on 2020/10/29.
//

#include "Level.h"
#include "entity/Actor.h"
#include "common/BlockPos.h"

void Level::forEachPlayer(const std::function<void(Actor *)> &todo) {
    auto begin = (uint64_t *) *((uint64_t *) this + 11);
    auto end = (uint64_t *) *((uint64_t *) this + 12);
    while (begin != end) {
        auto *player = (Actor *) (*begin);
        if (player)todo(player);
        ++begin;
    }
}

Actor *Level::getNearestPlayer(BlockPos &pos) {
    Actor *nearest = nullptr;
    float minDistance = 1024;
    this->forEachPlayer([&pos, &minDistance, &nearest](Actor *player) {
        BlockPos p(player->getPos()->x, player->getPos()->y, player->getPos()->z);
        auto d = p.distanceTo(pos);
        if (d < minDistance) {
            minDistance = d;
            nearest = player;
        }
    });
    return nearest;
}