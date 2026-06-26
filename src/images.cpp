#include "g_engine/img_loading/image_loader.hpp"
#include "game.hpp"

void Game::addImage (std::string image) {
    gore::IMG img = gore::imageloader::loadPNG("resources/"+image);
    images.insert({image, std::move(img)});
}