#include "flappy.hpp"

#include <badge/drawing.hpp>

namespace flappy
{

    void FlappyGame::update(int delta_ms) { State::update(delta_ms); }

    void FlappyGame::draw() { drawing::clear(COLOR_BLACK); }

    void FlappyGame::pause() {}

    void FlappyGame::resume() {}


} // namespace flappy
