#pragma once

#include "hex/actor.hpp"

namespace hex {

class Scene {
public:
    Scene();
    ~Scene();

    void update();
    void render();

    void addActor(Actor* actor);
    void removeActor(Actor* actor);

    void editorUI();
protected:
    std::vector<std::unique_ptr<Actor>> m_actors;

    Actor* m_selectedActor = nullptr;

    void drawActorTree(Actor* actor, int depth = 0);
};

}; // namespace hex