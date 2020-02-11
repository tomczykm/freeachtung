#include "menu/StateMenu.hpp"
#include "game/StateGame.hpp"
#include "game/StateSandbox.hpp"

StateMenu::StateMenu(const Application::Interface& app):
    app_(app)
{
    uiFont_.loadFromFile("ui.ttf");

    titleText_.setFont(uiFont_);
    titleText_.setString("freeachtung");
    titleText_.setCharacterSize(32);
    titleText_.setFillColor(sf::Color::Yellow);
    titleText_.setStyle(sf::Text::Italic);
    titleText_.setPosition(30, 30);

    print::info("StateMenu ready");
}

void StateMenu::input(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            app_.quit();
        }
    }
}

void StateMenu::logic() {

}

void StateMenu::render() {
    app_.window.draw(titleText_);
}

void StateMenu::addPlayer() {

}