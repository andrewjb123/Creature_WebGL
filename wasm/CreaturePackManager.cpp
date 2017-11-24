#include "CreaturePackManager.h"

CreaturePack::PackManager::PackManager()
    : pack_id_gen(0)
{

}

CreaturePack::PackManager::~PackManager()
{

}

bool CreaturePack::PackManager::addPackLoader(
    const std::string& name_in,
    rawPtr_t data, 
    int length)
{
    if(pack_loaders.count(name_in) > 0)
    {
        std::cout<<"CreaturePack::PackManager::addPackLoader() - Cannot add PackLoader, " + name_in + " already exists!"<<std::endl;
        return false;
    }

    const uint8_t * raw_data = reinterpret_cast<const uint8_t*>(data);
    auto byte_array = std::vector<uint8_t>(raw_data, raw_data + length);
    pack_loaders[name_in] = std::make_shared<CreaturePackLoader>(byte_array);
    std::cout<<"CreaturePack::PackManager::addPackLoader() - Added new PackLoader: "<<name_in<<std::endl;

    auto loader = pack_loaders[name_in];
    std::cout<<name_in<<" has Num Points: "<<loader->getNumPoints()<<" Num Indices: "<<loader->getNumIndices()<<std::endl;
    return true;
}

int CreaturePack::PackManager::addPackPlayer(const std::string& loader_name)
{
    if(pack_loaders.count(loader_name) == 0)
    {
        std::cout<<"CreaturePack::PackManager::addPackPlayer() - ERROR! PackLoader: " + loader_name + " does not exist!"<<std::endl;
        return -1;
    }

    pack_id_gen++;
    pack_players[pack_id_gen] = std::make_shared<CreaturePackPlayer>(*pack_loaders[loader_name]);
    std::cout<<"CreaturePack::PackManager::addPackPlayer() - Added new PackPlayer with id: "<<pack_id_gen<<std::endl;

    return pack_id_gen;
}

bool CreaturePack::PackManager::removePackPlayer(int handle)
{
    if(pack_players.count(handle) == 0)
    {
        std::cout<<"CreaturePack::PackManager::removePackPlayer() - ERROR! Player with handle: " + std::to_string(handle) + " not found!"<<std::endl;
        return false;
    }

    pack_players.erase(handle);
    std::cout<<"CreaturePack::PackManager::addPackPlayer() -  Removed PackPlayer with id: "<<handle<<std::endl;
    return true;
}

bool CreaturePack::PackManager::setPlayerActiveAnimation(int handle, const std::string& name)
{
    if(pack_players.count(handle) == 0)
    {
        std::cout<<"CreaturePack::PackManager::setPlayerActiveAnimation() - ERROR! Player with handle: " + std::to_string(handle) + " not found!"<<std::endl;
        return false;
    }

    auto& player = pack_players[handle];
    player->setActiveAnimation(name);
    return true;
}

bool CreaturePack::PackManager::setPlayerBlendToAnimation(int handle, const std::string& name, float blend_delta)
{
    if(pack_players.count(handle) == 0)
    {
        std::cout<<"CreaturePack::PackManager::setPlayerBlendToAnimation() - ERROR! Player with handle: " + std::to_string(handle) + " not found!"<<std::endl;
        return false;
    }

    auto& player = pack_players[handle];
    player->blendToAnimation(name, blend_delta);

    return true;    
}

bool CreaturePack::PackManager::stepPlayer(int handle, float delta)
{
    if(pack_players.count(handle) == 0)
    {
        std::cout<<"CreaturePack::PackManager::stepPlayer() - ERROR! Player with handle: " + std::to_string(handle) + " not found!"<<std::endl;
        return false;
    }

    auto& player = pack_players[handle];
    player->stepTime(delta);
    player->syncRenderData();

    return true;
}

float CreaturePack::PackManager::getPlayerRunTime(int handle) const
{
    if(pack_players.count(handle) == 0)
    {
        std::cout<<"CreaturePack::PackManager::getPlayerRunTime() - ERROR! Player with handle: " + std::to_string(handle) + " not found!"<<std::endl;
        return 0.0f;
    }

    const auto& player = pack_players.at(handle);
    return player->getRunTime();
}

bool CreaturePack::PackManager::setPlayerLoop(int handle, bool should_loop)
{
    if(pack_players.count(handle) == 0)
    {
        std::cout<<"CreaturePack::PackManager::setPlayerLoop() - ERROR! Player with handle: " + std::to_string(handle) + " not found!"<<std::endl;
        return false;
    }

    const auto& player = pack_players.at(handle);
    player->isLooping = should_loop;
    return true;
}

emscripten::val CreaturePack::PackManager::getPlayerPoints(int handle)
{
    const auto& player = pack_players.at(handle);
    return emscripten::val(emscripten::typed_memory_view(
        player->getRenderPointsLength(),
        player->render_2d_points.get()
    ));
}

emscripten::val CreaturePack::PackManager::getPlayerColors(int handle)
{
    const auto& player = pack_players.at(handle);
    return emscripten::val(emscripten::typed_memory_view(
        player->getRenderColorsLength(),
        player->render_colors.get()
    ));
}

emscripten::val CreaturePack::PackManager::getPlayerUVs(int handle)
{
    const auto& player = pack_players.at(handle);
    return emscripten::val(emscripten::typed_memory_view(
        player->getRenderUVsLength(),
        player->render_uvs.get()
    ));
}

emscripten::val CreaturePack::PackManager::getPlayerIndices(int handle)
{
    const auto& player = pack_players.at(handle);
    return emscripten::val(emscripten::typed_memory_view(
        player->data.getNumIndices(),
        player->data.indices.get()
    ));
}

CreaturePack::PlayerBounds CreaturePack::PackManager::getPlayerBounds(int handle)
{
    const auto& player = pack_players.at(handle);
    PlayerBounds bounds;
    bounds.x1 = player->render_2d_points.get()[0];
    bounds.y1 = player->render_2d_points.get()[1];
    bounds.x2 = bounds.x1;
    bounds.y2 = bounds.y1;

    for(int i = 0; i < player->renders_base_size; i++)
    {
        bounds.x1 = std::min(bounds.x1, player->render_2d_points.get()[i * 2]);
        bounds.x2 = std::max(bounds.x2, player->render_2d_points.get()[i * 2]);

        bounds.y1 = std::min(bounds.y1, player->render_2d_points.get()[i * 2 + 1]);
        bounds.y2 = std::max(bounds.y2, player->render_2d_points.get()[i * 2 + 1]);
    }

    return bounds;
}

// Binding code
EMSCRIPTEN_BINDINGS(creaturepack_manager_module) {
    emscripten::value_array<CreaturePack::PlayerBounds>("PlayerBounds")
        .element(&CreaturePack::PlayerBounds::x1)
        .element(&CreaturePack::PlayerBounds::y1)
        .element(&CreaturePack::PlayerBounds::x2)
        .element(&CreaturePack::PlayerBounds::y2)
        ;

    emscripten::class_<CreaturePack::PackManager>("PackManager")
        .constructor()
        .function("addPackLoader", &CreaturePack::PackManager::addPackLoader)
        .function("addPackPlayer", &CreaturePack::PackManager::addPackPlayer)
        .function("removePackPlayer", &CreaturePack::PackManager::removePackPlayer)
        .function("setPlayerActiveAnimation", &CreaturePack::PackManager::setPlayerActiveAnimation)
        .function("setPlayerBlendToAnimation", &CreaturePack::PackManager::setPlayerBlendToAnimation)
        .function("stepPlayer", &CreaturePack::PackManager::stepPlayer)
        .function("getPlayerRunTime", &CreaturePack::PackManager::getPlayerRunTime)
        .function("setPlayerLoop", &CreaturePack::PackManager::setPlayerLoop)
        .function("getPlayerPoints", &CreaturePack::PackManager::getPlayerPoints)
        .function("getPlayerColors", &CreaturePack::PackManager::getPlayerColors)
        .function("getPlayerUVs", &CreaturePack::PackManager::getPlayerUVs)
        .function("getPlayerIndices", &CreaturePack::PackManager::getPlayerIndices)
        .function("getPlayerBounds", &CreaturePack::PackManager::getPlayerBounds)
        ;
    }