#include "ProjekA.h"

Engine::ProjekA::ProjekA(Setting* setting) : Engine::Game(setting) {
    setting->windowTitle = "Infinite Warfare";
    enemySpawnInterval = 2000.0f;
    enemySpawnTimer = 0.0f;
    score = 0;
}

Engine::ProjekA::~ProjekA() {
    for (auto enemy : enemies) {
        delete enemy;
    }
    for (auto bullet : readyBullets) {
        delete bullet;
    }
    for (auto bullet : inUseBullets) {
        delete bullet;
    }
    delete bgm;
    delete jumpsfx;
    delete shootsfx;
    delete texture;
    delete bulletTexture;
    delete enemyTexture;
    delete backgroundTexture;
    delete sprite;
    delete backgroundSprite;
}

void Engine::ProjekA::Init() {
    // Player
    texture = new Texture("IdleFinal.png");
    sprite = new Sprite(texture, defaultSpriteShader, defaultQuad);
    sprite->SetNumXFrames(5)
        ->SetNumYFrames(1)
        ->AddAnimation("attack", 0, 3)
        ->AddAnimation("idle", 0, 4)
        ->AddAnimation("run", 12, 23)
        ->PlayAnim("idle")
        ->SetScale(3)
        ->SetAnimationDuration(90);

    float startX = setting->screenWidth / 2.0f - sprite->GetScaleWidth() / 2.0f;
    float startY = 0.0f;
    sprite->SetPosition(startX, startY);

    // Bullets
    bulletTexture = new Texture("Bullet_9.png");
    for (int i = 0; i < 50; i++) {
        Sprite* bulletSprite = new Sprite(bulletTexture, defaultSpriteShader, defaultQuad);
        bulletSprite->SetNumXFrames(1)->SetNumYFrames(1)->SetScale(2);
        readyBullets.push_back(new Bullet(bulletSprite));
    }

    // Background
    backgroundTexture = new Texture("cyber.png");
    backgroundSprite = new Sprite(backgroundTexture, defaultSpriteShader, defaultQuad);
    backgroundSprite->SetScale(0.67);

    // Enemy texture
    enemyTexture = new Texture("MonsterFinal.png");

    // Input mapping
    inputManager->AddInputMapping("Run Right", SDLK_RIGHT)
        ->AddInputMapping("Run Left", SDLK_LEFT)
        ->AddInputMapping("Jump", SDLK_UP)
        ->AddInputMapping("Attack", SDLK_SPACE)
        ->AddInputMapping("Quit", SDLK_ESCAPE);

    bgm = (new Music("bgmgame.ogg"))->SetVolume(30)->Play(true);
    jumpsfx = (new Sound("jump.wav"))->SetVolume(50);
    shootsfx = (new Sound("shoot.wav"))->SetVolume(30);
}

void Engine::ProjekA::Update() {
    if (isGameOver) {
        // Handle Game Over input
        if (inputManager->IsKeyPressed("R")) {
            ResetGame();
        }
        if (inputManager->IsKeyPressed("ESCAPE")) {
            state = State::EXIT;
        }
        return;
    }

    timeInterval += GetGameTime();
    enemySpawnTimer += GetGameTime();

    float collisionMarginX = 35.0f;
    float collisionMarginY = 5.0f;

    // Quit game
    if (inputManager->IsKeyReleased("Quit")) {
        state = State::EXIT;
        return;
    }

    // Default animation
    sprite->PlayAnim("idle");
    float groundLevel = 0.0f;

    // Player movement and jump
    vec2 oldPos = sprite->GetPosition();
    float x = oldPos.x, y = oldPos.y;

    if (inputManager->IsKeyPressed("Run Right")) {
        x += 0.2f * GetGameTime();
        sprite->PlayAnim("run")->SetFlipHorizontal(false);
        toRight = true;
    }
    if (inputManager->IsKeyPressed("Run Left")) {
        x -= 0.2f * GetGameTime();
        sprite->PlayAnim("run")->SetFlipHorizontal(true);
        toRight = false;
    }

    if (inputManager->IsKeyPressed("Jump") && !jump) {
        yVelocity = 1.5f;
        gravity = 0.006f;
        jump = true;
        jumpsfx->Play(false);
    }

    if (jump) {
        yVelocity -= gravity * GetGameTime();
        y += yVelocity * GetGameTime();
        if (y <= groundLevel) {
            y = groundLevel;
            jump = false;
            yVelocity = 0.0f;
        }
    }

    sprite->SetPosition(x, y)->Update(GetGameTime());

    // Handle attack
    if (inputManager->IsKeyPressed("Attack")) {
        sprite->PlayAnim("attack");
        SpawnBullets();
        shootsfx->Play(false);
    }

    // Update bullets
    for (auto it = inUseBullets.begin(); it != inUseBullets.end();) {
        Bullet* bullet = *it;
        if (bullet->GetPosition().x < -bullet->sprite->GetScaleWidth() ||
            bullet->GetPosition().x > setting->screenWidth) {
            readyBullets.push_back(bullet);
            it = inUseBullets.erase(it);
        }
        else {
            bullet->Update(GetGameTime());
            ++it;
        }
    }

    // Spawn enemies
    if (enemySpawnTimer >= enemySpawnInterval) {
        SpawnEnemy();
        enemySpawnTimer = 0.0f;
    }

    // Update enemies
    for (auto it = enemies.begin(); it != enemies.end();) {
        Sprite* enemy = *it;
        vec2 enemyPos = enemy->GetPosition();
        float moveSpeed = 0.1f;
        enemyPos.x += (sprite->GetPosition().x > enemyPos.x ? moveSpeed : -moveSpeed) * GetGameTime();
        enemyPos.y = groundLevel;

        bool hit = false;
        for (auto bulletIt = inUseBullets.begin(); bulletIt != inUseBullets.end();) {
            Bullet* bullet = *bulletIt;
            vec2 bulletPos = bullet->GetPosition();

            if (bulletPos.x < enemyPos.x + enemy->GetScaleWidth() &&
                bulletPos.x + bullet->sprite->GetScaleWidth() > enemyPos.x &&
                bulletPos.y < enemyPos.y + enemy->GetScaleHeight() &&
                bulletPos.y + bullet->sprite->GetScaleHeight() > enemyPos.y) {

                hit = true;
                score += 10;
                readyBullets.push_back(bullet);
                bulletIt = inUseBullets.erase(bulletIt);
            }
            else {
                ++bulletIt;
            }
        }

        vec2 playerPos = sprite->GetPosition();
        if (playerPos.x + collisionMarginX < enemyPos.x + enemy->GetScaleWidth() - collisionMarginX &&
            playerPos.x + sprite->GetScaleWidth() - collisionMarginX > enemyPos.x + collisionMarginX &&
            playerPos.y + collisionMarginY < enemyPos.y + enemy->GetScaleHeight() - collisionMarginY &&
            playerPos.y + sprite->GetScaleHeight() - collisionMarginY > enemyPos.y + collisionMarginY) {

            enemy->PlayAnim("attack");
            isGameOver = true;
            return;
        }

        if (hit) {
            it = enemies.erase(it);
            delete enemy;
        }
        else {
            enemy->SetPosition(enemyPos.x, enemyPos.y)->Update(GetGameTime());
            ++it;
        }
    }
}

void Engine::ProjekA::Render() {
    backgroundSprite->Draw();
    sprite->Draw();

    for (Bullet* bullet : inUseBullets) {
        bullet->Draw();
    }

    for (Sprite* enemy : enemies) {
        enemy->Draw();
    }

    Text scoreText("lucon.ttf", 24, defaultTextShader);
    scoreText.SetText("Score: " + std::to_string(score));
    scoreText.SetColor(255, 255, 255);
    scoreText.SetPosition(10, setting->screenHeight - 30);
    scoreText.Draw();

    if (isGameOver) {
        Text gameOverText("lucon.ttf", 48, defaultTextShader);
        gameOverText.SetText("Game Over");
        gameOverText.SetColor(255, 0, 0);
        gameOverText.SetPosition(setting->screenWidth / 2 - 100, setting->screenHeight / 2);
        gameOverText.Draw();
    }
}

void Engine::ProjekA::SpawnBullets() {
    if (timeInterval >= 150) {
        if (readyBullets.empty()) return;
        Bullet* bullet = readyBullets.back();
        readyBullets.pop_back();

        float bulletYOffset = 50.0f;
        if (toRight) {
            bullet->SetPosition(sprite->GetPosition().x + sprite->GetScaleWidth(),
                sprite->GetPosition().y + bulletYOffset);
            bullet->xVelocity = 0.6f;
        }
        else {
            bullet->SetPosition(sprite->GetPosition().x - bullet->sprite->GetScaleWidth(),
                sprite->GetPosition().y + bulletYOffset);
            bullet->xVelocity = -0.6f;
        }

        inUseBullets.push_back(bullet);
        timeInterval = 0;
    }
}

void Engine::ProjekA::SpawnEnemy() {
    Sprite* enemy = new Sprite(enemyTexture, defaultSpriteShader, defaultQuad);
    enemy->SetNumXFrames(4)
        ->SetNumYFrames(11)
        ->AddAnimation("walk", 0, 7)
        ->AddAnimation("attack", 8, 16)
        ->PlayAnim("walk")
        ->SetScale(3)
        ->SetAnimationDuration(100);

    float x;
    bool spawnFromRight = rand() % 2 == 0;

    if (spawnFromRight) {
        x = setting->screenWidth;
        enemy->SetFlipHorizontal(false);
    }
    else {
        x = -enemy->GetScaleWidth();
        enemy->SetFlipHorizontal(true);
    }

    float groundLevel = 0.0f;
    enemy->SetPosition(x, groundLevel);

    enemies.push_back(enemy);
}

void Engine::ProjekA::ResetGame() {
    isGameOver = false;
    score = 0;
    jump = false;
    yVelocity = 0.0f;

    float startX = setting->screenWidth / 2.0f - sprite->GetScaleWidth() / 2.0f;
    sprite->SetPosition(startX, 0.0f);

    for (auto enemy : enemies) {
        delete enemy;
    }
    enemies.clear();

    for (auto bullet : inUseBullets) {
        readyBullets.push_back(bullet);
    }
    inUseBullets.clear();
}
