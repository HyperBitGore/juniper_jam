#include "g_engine/file_loading/font_loader.hpp"
#include "g_engine/g_engine_2d.hpp"
#include "g_engine/rendering/font_renderer.hpp"
#include "g_engine/rendering/image_renderer.hpp"
#include "g_engine/rendering/primitive_renderer.hpp"
#include "game.hpp"
#include <memory>

gore::g_engine_2d eng("The Motor Must Spin", WINDOW_WIDTH, WINDOW_HEIGHT, MAINTAIN_ASPECT_RATIO_COMPONENT | USE_VIEW_MATRICE, gore::LogType::NONE);
static std::unique_ptr<Game> game;
static std::unique_ptr<gore::trianglerenderer> triangle_r;
static std::unique_ptr<gore::trianglerenderer> static_triangle_r;
static std::unique_ptr<gore::imagerenderer> image_r;
static std::unique_ptr<gore::fontrenderer> font_r;

void render () {
    game->loop();
}

int main() {
    std::cout << "Hello Juniper Jam!" << std::endl;
    triangle_r = gore::trianglerenderer::create(WINDOW_WIDTH, WINDOW_HEIGHT);
    static_triangle_r = gore::trianglerenderer::create(WINDOW_WIDTH, WINDOW_HEIGHT);
    image_r = gore::imagerenderer::create(WINDOW_WIDTH, WINDOW_HEIGHT);
    font_r = gore::fontrenderer::create(WINDOW_WIDTH, WINDOW_HEIGHT);
    game = std::make_unique<Game>(image_r, triangle_r, static_triangle_r, font_r);
    eng.addRenderer(triangle_r.get(), true, true, false);
    eng.addRenderer(static_triangle_r.get(), true, false, false);
    eng.addRenderer(image_r.get(), true, true, false);
    eng.addRenderer(font_r.get(), true, false, false);
    eng.setRenderFunction(render);
    game->addFont("resources/OpenSans-Regular.ttf", eng.getDPI());
    game->eng = &eng;
    eng.setFrameLimit(60);
    eng.toggleFrameLimitActive();
    SpatialHashmap::mapBenchmark();
    while (eng.updateWindow() && game->play) {
        double dt = eng.getDelta();
        game->delta = dt;
        eng.updateInputState();
    }
    return 0;
}
