#include "game/StateGame.hpp"

#include <algorithm>
#include <functional>

#include "menu/StateMenu.hpp"
#include "app/AssetManager.hpp"
#include "app/Utils.hpp"
#include "app/Log.hpp"
#include "framework/Observer.hpp"

namespace {

constexpr auto scoreListEntryHeight = 30u;

AssetManager::TextureSet gameTextures = {
        AssetManager::Texture::SelfHaste,
        AssetManager::Texture::OpponentHaste,
        AssetManager::Texture::SelfSlow,
        AssetManager::Texture::OpponentSlow,
        AssetManager::Texture::SelfRightAngle,
        AssetManager::Texture::OpponentRightAngle,
        AssetManager::Texture::ClearTrails,
        AssetManager::Texture::ControlSwap,
        AssetManager::Texture::MassPowerups,
        AssetManager::Texture::RandomPickMeUp
};

}  // namespace

StateGame::StateGame(const Application::Interface& ctx, const PlayerInfos& infos):
    app_{ctx},
    engine_{
        app_.assets,
        infos,
        app_.config.get<int>(Setting::TickRate),
        0.05 * app_.config.get<int>(Setting::ResHeight),
        0.9 * app_.config.get<int>(Setting::ResHeight)
    }
{
    app_.window.setMouseCursorVisible(false);

    loadGui(infos);
    attachObservers();

    app_.assets.loadTextures(gameTextures);

    state_ = std::make_unique<RoundBegin>(*this);

    print::info("StateGame ready");
}

StateGame::~StateGame() {
    app_.assets.releaseTextures(gameTextures);
    app_.window.setMouseCursorVisible(true);
}

void StateGame::attachObservers() {
    engine_.attach([this] (const auto& e) {
        if (auto event = dynamic_cast<const PointsAwardedEvent*>(&e); event) {
            updateScoreLabels(event->players);
        }
    });

    engine_.attach([this] (const auto& e) {
        if (auto event = dynamic_cast<const RoundEndEvent*>(&e); event) {
            changeState<RoundEnd>();
        }
    });

    engine_.attach([this] (const auto& e) {
        if (auto event = dynamic_cast<const MatchEndEvent*>(&e); event) {
            changeState<MatchEnd>();
        }
    });
}

void StateGame::input(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        switch(event.key.code) {
        case sf::Keyboard::Space:
            state_->onSpacebar();
            break;
        case sf::Keyboard::Escape:
            state_->onEscape();
            break;
        default: break;
        }
    }
    engine_.input(event);
}

void StateGame::tick(double deltaTime) {
    state_->onTick(deltaTime);
}

void StateGame::render() {
    for (auto d: engine_.getDrawables()) {
        app_.window.draw(*d);
    }
}

void StateGame::loadGui(const PlayerInfos& infos) {
    constexpr auto resName = "ui/game";
    try {
        app_.gui.loadWidgetsFromStream(AssetManager::openResource(resName));
    }
    catch (const tgui::Exception& e) {
        const auto msg = fmt::format("Failed to create GUI from {}. {}", resName, e.what());
        print::error(msg);
        throw std::runtime_error{msg};
    }

    auto goalLabel = app_.getWidget<tgui::Label>("ScoreGoal");
    goalLabel->setText(fmt::format("Goal: {}", (infos.size()-1)*10));

    prepareScoreLabels(infos);
}

void StateGame::prepareScoreLabels(const PlayerInfos& infos) {
    auto scoresPanel = app_.getWidget<tgui::Panel>("Scores");
    auto i = 0u;
    for (const auto&[id, info]: infos) {
        auto scoreListEntryPanel = tgui::Panel::create({"100%", scoreListEntryHeight});
        scoreListEntryPanel->loadWidgetsFromStream(AssetManager::openResource("ui/scoresEntry"));
        scoreListEntryPanel->setRenderer(scoresPanel->getSharedRenderer()->getData());
        scoreListEntryPanel->setPosition(0, (i++)*scoreListEntryHeight);

        auto nameLabel = static_cast<tgui::Label*>(scoreListEntryPanel->get("Name").get());
        nameLabel->getRenderer()->setTextColor(info.color);
        nameLabel->setText(info.name);

        auto scoreLabel = static_cast<tgui::Label*>(scoreListEntryPanel->get("Score").get());
        scoreLabel->getRenderer()->setTextColor(info.color);

        scoresPanel->add(scoreListEntryPanel, fmt::format("{}", id));
    }
}

void StateGame::updateScoreLabels(const Engine::Players& players) {
    std::multimap<PlayerThing::Score, ProfileId, std::greater<PlayerThing::Score>> playersByScore;
    for (const auto& [id, player]: players) {
        playersByScore.emplace(player->getScore(), id);
    }

    auto i = 0u;
    auto scoresPanel = app_.getWidget<tgui::Panel>("Scores");
    for (auto[score, id]: playersByScore) {
        auto playerPanel = static_cast<tgui::Panel*>(scoresPanel->get(fmt::format("{}", id)).get());
        playerPanel->setPosition(0, (i++)*scoreListEntryHeight);
        static_cast<tgui::Label*>(playerPanel->get("Score").get())->setText(fmt::format("{}", score));
    }
}

void StateGame::RoundBegin::onEnterState() {
    gs.engine_.resetRound();
}

void StateGame::RoundBegin::onSpacebar() {
    gs.changeState<Running>();
}

void StateGame::RoundBegin::onEscape() {
    gs.app_.enterState<StateMenu>();
}

void StateGame::Running::onSpacebar() {
    gs.changeState<Pause>();
}

void StateGame::Running::onTick(double deltaTime) {
    gs.engine_.step(deltaTime);
}


void StateGame::Pause::onSpacebar() {
    gs.changeState<Running>();
}

void StateGame::Pause::onEscape() {
    gs.app_.enterState<StateMenu>();
}


void StateGame::RoundEnd::onSpacebar() {
    gs.changeState<RoundBegin>();
}

void StateGame::RoundEnd::onEscape() {
    gs.app_.enterState<StateMenu>();
}


void StateGame::MatchEnd::onEnterState() {
    // todo: show end of match splash screen
}

void StateGame::MatchEnd::onSpacebar() {
    gs.app_.enterState<StateMenu>();
}

void StateGame::MatchEnd::onEscape() {
    gs.app_.enterState<StateMenu>();
}
