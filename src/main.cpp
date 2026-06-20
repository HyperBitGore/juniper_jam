#include <iostream>
#include <g_engine/g_engine_2d.hpp>

gore::g_engine_2d eng("The Motor Must Spin", 1024, 720, 0, gore::LogType::NONE);

static std::unique_ptr<gore::trianglerenderer> triangle_r;

void render () {
    triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    triangle_r->addCircleFilled({200.0f, 200.0f}, 10.0f);
    triangle_r->drawBuffer();
}

int main() {
    std::cout << "Hello from Juniper Jam!" << std::endl;
    
    triangle_r = gore::trianglerenderer::create(1024, 720);
    eng.addRenderer(triangle_r.get(), false, false, false);
    eng.setRenderFunction(render);
    while (eng.updateWindow()) {
        double dt = eng.getDelta();
        eng.updateInputState();
    }
    return 0;
}
