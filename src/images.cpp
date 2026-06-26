#include "g_engine/img_loading/image_loader.hpp"
#include "game.hpp"

void Game::addImage (std::string image) {
    gore::IMG img = gore::imageloader::loadPNG("resources/"+image);
    images.insert({image, std::move(img)});
}

void Game::addSound (std::string sound) {
    gore::audio snd = ap->loadWavFile("resources/"+sound);
    audios.insert({sound, snd});
}