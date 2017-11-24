#pragma once

#include "CreaturePackModule.hpp"
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

namespace CreaturePack
{
    typedef uint32_t rawPtr_t;

    struct PlayerBounds
    {
        float x1, y1, x2, y2;
    };

    class PackManager
    {
    public:
        PackManager();

        virtual ~PackManager();

        // Adds a new CreaturePack Loader given a typed byte array
        bool addPackLoader(const std::string& name_in, rawPtr_t data, int length);
        
        // Adds a new CreaturePackPlayer and returns a unique handle/id given a loader name
        int addPackPlayer(const std::string& loader_name);

        // Removes an existing CreaturePackPlayer given its handle/id
        bool removePackPlayer(int handle);

        // Sets an active animation without blending given a player handle/id
        bool setPlayerActiveAnimation(int handle, const std::string& name);

        // Blends into an animation given a player handle/id, 0.0 < blend_delta <= 1.0
        bool setPlayerBlendToAnimation(int handle, const std::string& name, float blend_delta);

        // Steps the player object with a given time delta given its handle/id
        bool stepPlayer(int handle, float delta);

        // Returns the current run time of a player object given its handle/id
        float getPlayerRunTime(int handle) const;

        // Sets whether the player animation is looping given its handle/id
        bool setPlayerLoop(int handle, bool should_loop);

        // Returns the points of the player given its handle/id
        emscripten::val getPlayerPoints(int handle);

        // Returns the colors of the player given its handle/id
        emscripten::val getPlayerColors(int handle);

        // Returns the Uvs of the player given its handle/id
        emscripten::val getPlayerUVs(int handle);

        // Returns the Indices of the player given its handle/id
        emscripten::val getPlayerIndices(int handle);

        // Returns the bounds of a player given its handle/id
        PlayerBounds getPlayerBounds(int handle);
        
    protected:
        std::unordered_map<std::string, std::shared_ptr<CreaturePackLoader>> pack_loaders;
        std::unordered_map<int, std::shared_ptr<CreaturePackPlayer>> pack_players;
        int pack_id_gen;
    };
}