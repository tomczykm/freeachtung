#pragma once

#include <memory>
#include <functional>

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>

#include "app/Log.hpp"
#include "app/AssetManager.hpp"
#include "app/ConfigManager.hpp"
#include "app/ProfileManager.hpp"
#include "app/IGameState.hpp"
#include "app/Utils.hpp"
#include "sql/Connection.hpp"

class Application {
public:
    Application();

    template <typename InitState, typename... Args>
    static inline std::unique_ptr<Application> makeApplication(Args&&...);

    int run();

    class Interface;

private:
    sf::RenderWindow window_;
    tgui::Gui gui_;

    AssetManager assets_;
    ConfigManager config_;
    sql::Connection database_;
    ProfileManager profiles_;

    std::unique_ptr<IGameState> gameState_;
    std::function<void()> changeState_;
    bool quit_ = false;

    sf::Clock deltaTimeClock_;
    sf::Time stepTime_;
};

class Application::Interface
{
public:
    AssetManager& assets;
    ConfigManager& config;
    ProfileManager& profiles;

    sf::RenderWindow& window;
    tgui::Gui& gui;

    void quit() {
        print::info("quit");
        app_.quit_ = true;
    }

    template <class State, typename... Ts>
    void enterState(Ts... args) {
        print::info("Changing state to {}", stripLeadingDigits(typeid(State).name()));
        app_.changeState_ = [this, args...] () {
            app_.gameState_ = std::make_unique<State>(*this, args...);
        };
    }

    template <typename Widget>
    auto getWidget(std::string_view name) {
        return std::static_pointer_cast<Widget>(gui.get(name.data()));
    }

private:
    friend class Application;
    explicit Interface(Application& app):
        assets{app.assets_},
        config{app.config_},
        profiles(app.profiles_),
        window{app.window_},
        gui{app.gui_},
        app_{app}
    {}

    Application& app_;
};


template <typename InitState, typename... Args>
inline std::unique_ptr<Application> Application::makeApplication(Args&&... args) {
    auto app = std::make_unique<Application>();
    app->gameState_ = std::make_unique<InitState>(Interface{*app}, std::forward<Args>(args)...);
    return app;
}
