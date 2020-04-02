#include "engine/Engine.hpp"

#include <TGUI/TGUI.hpp>

#include "app/Utils.hpp"

namespace {

constexpr auto playerToGameAreaSizeRatio = 253.334f;
constexpr auto pickMeUpToGameAreaSizeRatio = 32.2f;
constexpr auto playerSpeedToGameAreaSizeRatio = 7.6f;

const auto selfHasteDuration = sf::milliseconds(3500);
const auto oppHasteDuration = sf::milliseconds(2500);

const auto selfSlowDuration = sf::milliseconds(5000);
const auto oppSlowDuration = sf::milliseconds(4000);

const auto selfRightAngleMovementDuration = sf::milliseconds(8500);
const auto oppRightAngleMovementDuration = sf::milliseconds(5500);

const auto controlSwapDuration = sf::milliseconds(6500);

}  // namespace

Engine::Engine(IAssetManager& a, const PlayerInfos& infos, int tickrate, int playAreaCorner, int playAreaSide):
    assets_{a},
    timerService_{tickrate},
    playAreaCornerOffset_{playAreaCorner},
    playAreaSideLength_{playAreaSide},
    pickMeUpRadius_{playAreaSideLength_ / pickMeUpToGameAreaSizeRatio},
    scoreVictoryGoal_{(infos.size()-1)*10},
    border_{playAreaCornerOffset_, playAreaSideLength_},
    pickmeupSpawnTimer_{timerService_.makeTimer(sf::milliseconds(0))}
{
    initializePlayers(infos);
    resetPickmeupSpawnTimer();
}

void Engine::input(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        for (auto& [id, player]: players_) {
            if (player->isDead()) continue;
            player->input(event);
        }
    }
}

void Engine::step(double deltaTime) {
    timerService_.advanceAll();

    bool playerDied = false;
    for (auto& [id, player]: players_) {
        if (player->isDead()) continue;
        player->step(deltaTime, trails_);

        if (checkCollisions(id, *player)) {
            playerDied = true;
        }
    }

    if (pickmeupSpawnTimer_->isExpired()) {
        createRandomPickMeUp();
        resetPickmeupSpawnTimer();
    }

    if (playerDied) {
        awardPoints();
    }

    if (massPowerups_ && massPowerups_->isExpired()) {
        massPowerups_.reset();
    }
}

void Engine::resetRound() {
    trails_.clear();
    pickmeups_.clear();
    massPowerups_.reset();
    resetPickmeupSpawnTimer();
    for (auto& [id, player] : players_) {
        player->newRoundSetup(
            xor_rand::next(playAreaCornerOffset_ + 0.15 * playAreaSideLength_,
                playAreaCornerOffset_ + 0.85 * playAreaSideLength_),
            xor_rand::next(playAreaCornerOffset_ + 0.15 * playAreaSideLength_,
                playAreaCornerOffset_ + 0.85 * playAreaSideLength_),
            trails_
        );
    }
}

std::vector<const sf::Drawable*> Engine::getDrawables() {
    std::vector<const sf::Drawable*> drawables;
    for (const auto& p: pickmeups_) {
        drawables.push_back(&p.getSprite());
    }

    for (const auto& t: trails_) {
        drawables.push_back(&t.getShape());
    }

    for (const auto& shape: border_.getShapes()) {
        drawables.push_back(&shape);
    }

    for (const auto& [id, player]: players_) {
        drawables.push_back(&player->getShape());
    }
    return drawables;
}

void Engine::initializePlayers(const PlayerInfos& infos) {
    const auto radius = playAreaSideLength_ / playerToGameAreaSizeRatio;
    const auto velocity = playAreaSideLength_ / playerSpeedToGameAreaSizeRatio;
    for (const auto& [id, info] : infos) {
        players_.emplace(id, std::make_unique<PlayerThing>(info, radius, velocity, timerService_.makeTimer(sf::milliseconds(400))));
    }
}

bool Engine::checkCollisions(ProfileId id, PlayerThing& player) {
    for (const auto& shape: border_.getShapes()) {
        if (player.checkCollision(shape)) {
            player.kill();
            return true;
        }
    }

    // following code is fairly shoddy, better come up with a better idea for collisions later:
    // the reason we skip the newest bunch of trail segments is that
    // we don't want to check if we collide with segments we just created
    // (we obviously do, they're right underneath and we don't want to die immediately)

    auto toSkip = 45u;
    for (const auto& t: trails_) {
        if (t.getShape().getFillColor() == player.getColor() && toSkip > 0) {
            --toSkip;
            continue;
        }
        if (player.checkCollision(t.getShape())) {
            player.kill();
            return true;
        }
    }

    auto pickmeup = pickmeups_.begin();
    while (pickmeup != pickmeups_.end()) {
        if (player.checkCollision(pickmeup->getShape())) {
            pickmeup->onPickUp(id, player);
            pickmeup = pickmeups_.erase(pickmeup);
        } else {
            ++pickmeup;
        }
    }

    return false;
}

void Engine::awardPoints() {
    for (auto& [id, player]: players_) {
        if (player->isDead()) continue;
        player->addPoint();
    }
    notifyAll(PointsAwardedEvent{players_});

    const auto aliveCount = std::count_if(players_.begin(), players_.end(), [] (const auto& kv) {
            return not kv.second->isDead();
        });
    if (aliveCount <= 1) {
        if (victoryGoalAchieved()) {
            notifyAll(MatchEndEvent{});
        } else {
            notifyAll(RoundEndEvent{});
        }
    }
}

bool Engine::victoryGoalAchieved() {
    std::vector<PlayerThing::Score> scores;
    for (const auto&[id, player]: players_) {
        scores.push_back(player->getScore());
    }
    std::sort(scores.begin(), scores.end(), std::greater<PlayerThing::Score>{});

    return scores.front() >= scoreVictoryGoal_
        && scores.front() - *(scores.begin()+1) >= 2; // first place needs to be at least two points ahead
}

void Engine::createRandomPickMeUp() {
    const auto type = static_cast<PickUpType>(xor_rand::next(1, static_cast<int>(PickUpType::Count)-1));
    auto [onPickMeUp, texture] = makePickMeUpEffectAndTexture(type);
    if (xor_rand::next(1, static_cast<int>(PickUpType::Count)) == 1) texture = TextureType::RandomPickMeUp;
    pickmeups_.emplace_back(
        xor_rand::next(playAreaCornerOffset_ + pickMeUpRadius_, playAreaCornerOffset_ + playAreaSideLength_ - pickMeUpRadius_),
        xor_rand::next(playAreaCornerOffset_ + pickMeUpRadius_, playAreaCornerOffset_ + playAreaSideLength_ - pickMeUpRadius_),
        pickMeUpRadius_,
        assets_.getTexture(texture),
        onPickMeUp
    );
}

std::pair<PickMeUp::OnPickUp, TextureType> Engine::makePickMeUpEffectAndTexture(PickUpType type) {
    switch (type) {
    case PickUpType::SelfHaste:
        return std::make_pair(makeSelfEffect([this] (auto& player) {
            player.addEffectStack(PlayerEffect::Haste, timerService_.makeTimer(selfHasteDuration));
        }), TextureType::SelfHaste);
    case PickUpType::OpponentHaste:
        return std::make_pair(makeOpponentEffect([this] (auto& player) {
            player.addEffectStack(PlayerEffect::Haste, timerService_.makeTimer(oppHasteDuration));
        }), TextureType::OpponentHaste);
    case PickUpType::SelfSlow:
        return std::make_pair(makeSelfEffect([this] (auto& player) {
            player.addEffectStack(PlayerEffect::Slow, timerService_.makeTimer(selfSlowDuration));
        }), TextureType::SelfSlow);
    case PickUpType::OpponentSlow:
        return std::make_pair(makeOpponentEffect([this] (auto& player) {
            player.addEffectStack(PlayerEffect::Slow, timerService_.makeTimer(oppSlowDuration));
        }), TextureType::OpponentSlow);
    case PickUpType::ClearTrails:
        return std::make_pair(makeSelfEffect([this] (auto&) {
            trails_.clear();
        }), TextureType::ClearTrails);
    case PickUpType::SelfRightAngle:
        return std::make_pair(makeSelfEffect([this] (auto& player) {
            player.addEffectStack(PlayerEffect::RightAngled, timerService_.makeTimer(selfRightAngleMovementDuration));
        }), TextureType::SelfRightAngle);
    case PickUpType::OpponentRightAngle:
        return std::make_pair(makeOpponentEffect([this] (auto& player) {
            player.addEffectStack(PlayerEffect::RightAngled, timerService_.makeTimer(oppRightAngleMovementDuration));
        }), TextureType::OpponentRightAngle);
    case PickUpType::ControlSwap:
        return std::make_pair(makeOpponentEffect([this] (auto& player) {
            player.addEffectStack(PlayerEffect::SwapControl, timerService_.makeTimer(controlSwapDuration));
        }), TextureType::ControlSwap);
    case PickUpType::MassPowerups:
        return std::make_pair(makeSelfEffect([this] (auto&) {
            addMassPowerups();
            resetPickmeupSpawnTimer();
        }), TextureType::MassPowerups);
    default:
        const auto msg = fmt::format("bad pickup type {}", type);
        print::error(msg);
        throw std::runtime_error{msg};
    }
}

template <typename PlayerUnaryOp>
PickMeUp::OnPickUp Engine::makeSelfEffect(PlayerUnaryOp effect) {
    return [this, effect] (ProfileId, PlayerThing& pickedBy) {
        effect(pickedBy);
    };
}

template <typename PlayerUnaryOp>
PickMeUp::OnPickUp Engine::makeOpponentEffect(PlayerUnaryOp effect) {
    return [this, effect] (ProfileId pickedById, PlayerThing&) {
        for (auto& [id, player]: players_) {
            if (id == pickedById) continue;
            effect(*player);
        }
    };
}

void Engine::addMassPowerups() {
    if (massPowerups_) {
        massPowerups_->extend(timerService_.timeToTicks(sf::milliseconds(7000)));
    } else {
        massPowerups_ = timerService_.makeTimer(sf::milliseconds(10000));
    }
}

void Engine::resetPickmeupSpawnTimer() {
    const auto timeUntilNextPickmeupSpawn =
        massPowerups_ ?
        sf::milliseconds(xor_rand::next(800, 2200)) :
        sf::milliseconds(xor_rand::next(4500, 8000));
    pickmeupSpawnTimer_->reset(timerService_.timeToTicks(timeUntilNextPickmeupSpawn));
}
