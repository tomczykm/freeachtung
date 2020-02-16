#include <algorithm>

#include "menu/StateMenu.hpp"
#include "game/StateGame.hpp"
#include "game/StateSandbox.hpp"
#include "Utils.hpp"

namespace
{

constexpr auto PLAYER_LIST_ENTRY_HEIGHT = 60u;

}

StateMenu::StateMenu(const Application::Interface& app):
    app_{app}
{
    loadGui();
    addPlayer();
    print::info("StateMenu ready");
}

void StateMenu::input(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            app_.quit();
        }
    }
}

void StateMenu::addPlayer() {
    print::info(__func__);
    auto playerListPanel = app_.getWidget<tgui::Panel>("PlayerListInnerPanel");

    auto newEntryPanel = tgui::Panel::create({"100%", "60"});
    newEntryPanel->setRenderer(playerListPanel->getSharedRenderer()->getData());
    newEntryPanel->setPosition("0", std::to_string(getCurrentNumPlayers()*PLAYER_LIST_ENTRY_HEIGHT));

    const auto newId = PlayerId{playerInfos_.size() == 0 ? 0 : playerInfos_.rbegin()->first + 1};

    const auto widgetNamePrefix = "Player" + std::to_string(newId);

    const auto color = sf::Color{xor_rand::next(0,255), xor_rand::next(0,255), xor_rand::next(0,255)};
    const auto info = PlayerInfo{widgetNamePrefix + "Name", sf::Keyboard::Q, sf::Keyboard::W, color};

    auto nameLabel = tgui::Label::create(info.name);
    nameLabel->setTextSize(17);
    nameLabel->setPosition("10", "50%-height/2");
    nameLabel->getRenderer()->setTextColor(info.color);
    newEntryPanel->add(nameLabel, widgetNamePrefix + "Label");

    auto removeButton = tgui::Button::create("Rem");
    removeButton->setPosition("100% - 10 - width", "15%");
    removeButton->setSize("30", "70%");
    removeButton->connect("pressed", [=] () {
        removePlayer(newId, newEntryPanel);
    });
    newEntryPanel->add(removeButton, widgetNamePrefix + "Remove");

    auto keysLabel = tgui::Label::create("Q   W");
    keysLabel->setPosition(widgetNamePrefix + "Remove.left - 6% - width", "50%-height/2");
    keysLabel->setTextSize(17);
    keysLabel->setSize(60, "100%");
    keysLabel->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
    keysLabel->getRenderer()->setTextColor(info.color);
    keysLabel->connect("clicked", [newId] () {
        print::info("clicked keys of {}", newId);
    });
    newEntryPanel->add(keysLabel, widgetNamePrefix + "Keys");

    playerListPanel->add(newEntryPanel, widgetNamePrefix + "Panel");
    playerInfos_.emplace(newId, info);
}

std::vector<PlayerInfo> StateMenu::preparePlayerInfos() {
    std::vector<PlayerInfo> infos;
    std::transform(playerInfos_.begin(), playerInfos_.end(), std::back_inserter(infos), [] (const auto& kvPair) {
        return kvPair.second;
    });
    return infos;
}

void StateMenu::loadGui() {
    constexpr auto filename = "../menu.gui";
    try {
        app_.gui.loadWidgetsFromFile(filename);
    }
    catch (const tgui::Exception& e) {
        print::error("Failed to load GUI layout from {}. {}", filename, e.what());
        exit(-1);
    }

    app_.gui.get("AddPlayer")->connect("pressed", [this] () {
        addPlayer();
    });

    app_.gui.get("StartGame")->connect("pressed", [this] () {
        app_.enterState<StateGame>(preparePlayerInfos());
    });

    app_.gui.get("QuitGame")->connect("pressed", [this] () {
        app_.quit();
    });
}

std::size_t StateMenu::getCurrentNumPlayers() {
    return std::static_pointer_cast<tgui::Panel>(app_.gui.get("PlayerListInnerPanel"))->
        getWidgets().size();
}

void StateMenu::removePlayer(PlayerId id, tgui::Panel::Ptr panel) {
    print::info("Remove player {}", id);
    app_.getWidget<tgui::Panel>("PlayerListInnerPanel")->remove(panel);
    recalculatePlayerListPositions();

    playerInfos_.erase(id);
}

void StateMenu::recalculatePlayerListPositions() {
    auto playerListPanel = std::static_pointer_cast<tgui::Panel>(app_.gui.get("PlayerListInnerPanel"));
    auto playerPanels = playerListPanel->getWidgets();

    std::size_t i = 0;
    for (auto& panel : playerPanels) {
        panel->setPosition({0, PLAYER_LIST_ENTRY_HEIGHT*(i++)});
    }
}