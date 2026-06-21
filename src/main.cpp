#include "g_engine/file_loading/font_loader.hpp"
#include "g_engine/g_engine_2d.hpp"
#include "g_engine/rendering/font_renderer.hpp"
#include "g_engine/rendering/image_renderer.hpp"
#include "game.hpp"
#include <memory>

gore::g_engine_2d eng("The Motor Must Spin", 1024, 768, MAINTAIN_ASPECT_RATIO_COMPONENT, gore::LogType::NONE);
static std::unique_ptr<Game> game;
static std::unique_ptr<gore::trianglerenderer> triangle_r;
static std::unique_ptr<gore::imagerenderer> image_r;
static std::unique_ptr<gore::fontrenderer> font_r;

void render () {
    game->loop();
}

int main() {
    std::cout << "Hello from Juniper Jam!" << std::endl;
    triangle_r = gore::trianglerenderer::create(1024, 768);
    image_r = gore::imagerenderer::create(1024, 768);
    font_r = gore::fontrenderer::create(1024, 768);
    game = std::make_unique<Game>(image_r, triangle_r, font_r);
    eng.addRenderer(triangle_r.get(), true, false, false);
    eng.addRenderer(image_r.get(), true, false, false);
    eng.addRenderer(font_r.get(), true, false, false);
    eng.setRenderFunction(render);
    game->addFont("resources/OpenSans-Regular.ttf", eng.getDPI());
    game->eng = &eng;
    while (eng.updateWindow()) {
        double dt = eng.getDelta();
        game->delta = dt;
        eng.updateInputState();
    }
    return 0;
}
