//
// Created by xhy on 2020/8/25.
//

#include "Actor.h"
#include "tools/MathTool.h"

using namespace SymHook;

namespace actor {
    std::map<std::string, std::vector<Vec3>> mobCounterList;//NOLINT



    Vec3 *getPos(void *actor) {
        return SYM_CALL(
                Vec3*(*)(void * ),
                MSSYM_B1QA6getPosB1AA5ActorB2AAE12UEBAAEBVVec3B2AAA2XZ,
                actor
        );
    }

    Vec3 *Actor::getPos() {
        return SYM_CALL(
                Vec3*(*)(void * ),
                MSSYM_B1QA6getPosB1AA5ActorB2AAE12UEBAAEBVVec3B2AAA2XZ,
                this
        );
    }

    void Actor::getViewActor(Vec3 *vec3, float val) {
        SYM_CALL(
                Vec3*(*)(Actor * , Vec3 *, float),
                MSSYM_B1QE13getViewVectorB1AA5ActorB2AAA4QEBAB1QA6AVVec3B2AAA1MB1AA1Z,
                this, vec3, val
        );
    }


    std::string ActorDefinitionIdentifier::getName() {
        auto str = reinterpret_cast<std::string *>((char *) this + 32);
        return std::string(*str);
    }

    std::vector<std::string> getActorText(void *actor) {
        std::vector<std::string> info;
        if (!actor)return info;
        SYM_CALL(void(*)(void * , std::vector<std::string> &),
                 MSSYM_MD5_f04fad6bac034f1e861181d3580320f2,
                 actor, info);
        return info;
    }

    void sendMobInfo() {
        int mobNum = getMobCount(globalSpawner);
        char str[64];
        gamePrintf("total mob count: %d", mobNum);
    }

    void startSpawnCounter() {
        if (mobSpawnCounterStart) {
            error("it's already start some ticks before");
        } else {
            mobCounterList.clear();
            mobTickCounter = 0;
            mobSpawnCounterStart = true;
            gamePrintf("counter start, type ./spawner end to end the counter");
        }
    }

    void endSpawnerCounter() {
        if (!mobSpawnCounterStart) {
            error("this command can only be run after the counter start");
        } else {
            mobSpawnCounterStart = false;
            gamePrintf("counter end");
        }
    }

    void spawnAnalysis(std::string &type) {
        if (mobTickCounter == 0) {
            warning("no data");
            return;
        }
        if (type == "null") {
            std::map<std::string, size_t> mobList;
            std::map<int, int> heightMap;
            std::map<int, int> disMap;
            auto playerPos = getPos(globalPlayer);
            size_t totalNum = 0;
            for (const auto &item:mobCounterList) {
                mobList[item.first] = item.second.size();
                totalNum += item.second.size();

                for (auto vec : item.second) {
                    heightMap[(int) vec.y]++;
                    disMap[(int) math::distance(vec, *playerPos)]++;
                }
            }
            gamePrintf("total §2%d §r tick", mobTickCounter);
            for (auto &i: mobList)
                gamePrintf("%s : §2%d       §r(§2%.2f§r/h)\n", i.first.c_str(), i.second,
                           (float) i.second * 72000.0 / (float) mobTickCounter);
            gamePrintf("-------distance map---------");
            for (auto &i: heightMap)
                gamePrintf("[%d-%d]: §2%d", i.first, i.first + 1, i.second);
            gamePrintf("--------height map--------");
            for (auto &i: heightMap)
                gamePrintf("%d: §2%d", i.first, i.second);

        } else {

            auto iter = mobCounterList.find(type);
            if (iter == mobCounterList.end()) {
                gamePrintf("no mob type named %s", type.c_str());
            } else {
                gamePrintf("function developing\n");
            }
        }
    }

    std::string getActorName(void *actor) {
//#warning this func is hardcoded, it may need rewrite when update the game
        return actor ? getActorText(actor)[0].substr(13) : "null";
    }


}

THook(
        void,
        MSSYM_B1QA6attackB1AA6PlayerB2AAA4UEAAB1UE10NAEAVActorB3AAAA1Z,
        void *p1,
        void * p2
) {
    if (p1 && p2) {
        // if (getActorName(p2) == "minecraft:villager_v2<>") {
        original(p1, p2);
        if (p2) {
            std::string infoText;
            SYM_CALL(void(*)(void * mob, std::string &),
                     MSSYM_B1QE14buildDebugInfoB1AA3MobB2AAA9UEBAXAEAVB2QDA5basicB1UA6stringB1AA2DUB2QDA4charB1UA6traitsB1AA1DB1AA3stdB2AAA1VB2QDA9allocatorB1AA1DB1AA12B2AAA3stdB3AAAA1Z,
                     p2, infoText
            );

            SYM_CALL(
                    void(*)(void * actor,
                    const std::string &str),
                    MSSYM_B1QE10setNameTagB1AA5ActorB2AAA9UEAAXAEBVB2QDA5basicB1UA6stringB1AA2DUB2QDA4charB1UA6traitsB1AA1DB1AA3stdB2AAA1VB2QDA9allocatorB1AA1DB1AA12B2AAA3stdB3AAAA1Z,
                    p2,
                    infoText
            );
        }
    }
}

THook(
        void,
        MSSYM_B1QA8spawnMobB1AA7SpawnerB2AAE11QEAAPEAVMobB2AAE15AEAVBlockSourceB2AAE29AEBUActorDefinitionIdentifierB2AAA9PEAVActorB2AAA8AEBVVec3B3AAUA3N44B1AA1Z,
        void *sp,
        void *bs,
        actor::ActorDefinitionIdentifier *actorId,
        void *actor,
        Vec3 *pos,
        bool a6,
        bool a7,
        bool a8
) {
    //printf("(%f, %f, %f)\n", pos->x, pos->y, pos->z);
    if (mobSpawnCounterStart && pos) {
        auto mobName = actorId->getName();
        Vec3 vec(pos->x, pos->y, pos->z);
        actor::mobCounterList[mobName].emplace_back(vec);
    }
    original(sp, bs, actorId, actor, pos, a6, a7, a8);
}


THook(
        void,
        MSSYM_MD5_4ad35b002b3931f6af40cb4fe59053ef,
        void *self,
        void *actorID,
        void *actor,
        Vec3 *pos,
        int flag
) {
    // printf("   constructor (%f, %f, %f)\n", pos->x, pos->y, pos->z);
    original(self, actorID, actor, pos, flag);
}

THook(
        void,
        MSSYM_B1QA6setPosB1AA5ActorB2AAE13UEAAXAEBVVec3B3AAAA1Z,
        void *self,
        Vec3 * pos
) {
//    if(self != globalPlayer){
//        printf("set pos: (%f, %f, %f)\n", pos->x, pos->y, pos->z);
//    }
    original(self, pos);
}
