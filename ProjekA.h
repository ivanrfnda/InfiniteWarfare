#ifndef PROJEKA_H
#define PROJEKA_H

#include "Game.h"
#include "Setting.h"
#include "Texture.h"
#include "Sprite.h"
#include "Bullet.h"
#include "Music.h"
#include "Sound.h"
#include "Text.h"
#include <vector>
#include <iostream>

namespace Engine {
    class ProjekA : public Engine::Game {
    public:
        ProjekA(Setting* setting);
        ~ProjekA();

        virtual void Init();
        virtual void Update();
        virtual void Render();

    private:
        Engine::Texture* texture = NULL;
        Engine::Texture* bulletTexture = NULL;
        Engine::Texture* enemyTexture = NULL;
        Engine::Texture* backgroundTexture = NULL;

        Engine::Sprite* sprite = NULL;
        Engine::Sprite* backgroundSprite = NULL;

        Music* bgm = NULL;
        Sound* jumpsfx = NULL;
        Sound* shootsfx = NULL;

        std::vector<Bullet*> inUseBullets;
        std::vector<Bullet*> readyBullets;
        std::vector<Sprite*> enemies;

        float yVelocity = 0.0f;
        float gravity = 0.0f;

        float timeInterval = 0.0f;
        float enemySpawnInterval = 2000.0f;
        float enemySpawnTimer = 0.0f;

        int score = 0;
        bool jump = false;
        bool toRight = true;
        bool isGameOver = false;

        void SpawnBullets();
        void SpawnEnemy();
        void ResetGame();
    };
}

#endif
