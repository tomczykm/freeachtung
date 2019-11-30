#include "game/StateGame.hpp"
#include "game/StateSandbox.hpp"

#include <algorithm>

#include "Utils.hpp"

StateGame::StateGame(const Application::Interface& ctx):
    app_(ctx),
    border_(0.05 * app_.settings.getResH(), 0.9 * app_.settings.getResH())
{
    players_.emplace_back("player"/*, SDL_SCANCODE_Q, SDL_SCANCODE_W*/);
    for (auto &p: players_) {
        p.newRoundSetup(100, 600, 100, 600);
    }
    lastAlive_ = players_.end();
}

void StateGame::input(const sf::Event&) {
    // for (auto it = players_.begin() ; it != lastAlive_ ; it++) {
    //     it->input();
    // }
}

void StateGame::input(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        switch(event.key.code) {
        case sf::Keyboard::Space:
            // space (hehe) for shitty debugging
            app_.enterState<StateSandbox>();
            break;
        default: break;
        }
    }
}

void StateGame::logic() {
    bool playerDied = false;

    for (auto player = players_.begin() ; player != lastAlive_ ; player++) {
        // it->move((SDL_GetTicks() - moveTimer_) / 1000.f);
        player->createTrail(trails_);

        for (const auto& t: trails_) {
            if (!player->isGap() && player->checkCollision(t)) {
                player->kill();
                playerDied = true;
            }
        }
    }

    if (playerDied) {
        lastAlive_ = std::partition(players_.begin(), players_.end(),
            [] (auto& p) { return !p.isDead(); });
    }

    // moveTimer_ = SDL_GetTicks();
}

void StateGame::render() {
    // // draw trails
    // for (auto& t: trails_) {
    //     auto rec = t.getRenderRect();
    //     trailTex_.render(rec.x, rec.y, rec.w, rec.h, t.getAngle());
    // }

    for (const auto& shape: border_.getShapes()) {
        app_.window.draw(shape);
    }

    for (const auto& p: players_) {
        app_.window.draw(p.getShape());
    }
}
