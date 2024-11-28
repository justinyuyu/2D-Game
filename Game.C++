#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <map>
#include <string>
#include <functional>

// Resource Manager (Singleton)
class ResourceManager {
private:
    static ResourceManager* instance;
    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::SoundBuffer> soundBuffers;
    std::map<std::string, sf::Font> fonts;

    ResourceManager() {} // Private constructor

public:
    static ResourceManager& getInstance() {
        if (!instance) instance = new ResourceManager();
        return *instance;
    }

    void loadTexture(const std::string& name, const std::string& filename) {
        sf::Texture texture;
        if (texture.loadFromFile(filename)) {
            textures[name] = texture;
        }
    }

    sf::Texture& getTexture(const std::string& name) {
        return textures[name];
    }

    // Similar methods for sounds and fonts
};

// Animation System
class Animation {
private:
    sf::Sprite& sprite;
    std::vector<sf::IntRect> frames;
    float frameTime;
    float currentTime;
    int currentFrame;
    bool isLooping;

public:
    Animation(sf::Sprite& sprite, float frameTime = 0.1f)
        : sprite(sprite), frameTime(frameTime), currentTime(0), currentFrame(0), isLooping(true) {}

    void addFrame(const sf::IntRect& frame) {
        frames.push_back(frame);
    }

    void update(float deltaTime) {
        if (frames.empty()) return;

        currentTime += deltaTime;
        if (currentTime >= frameTime) {
            currentTime = 0;
            currentFrame = (currentFrame + 1) % frames.size();
            sprite.setTextureRect(frames[currentFrame]);
        }
    }
};

// Physics Component
struct Physics {
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float gravity;
    bool isGrounded;

    Physics() : gravity(980.f), isGrounded(false) {}

    void update(sf::Vector2f& position, float deltaTime) {
        velocity += acceleration * deltaTime;
        if (!isGrounded) {
            velocity.y += gravity * deltaTime;
        }
        position += velocity * deltaTime;
    }
};

class GameObject {
protected:
    sf::Sprite sprite;
    sf::Vector2f position;
    bool active;
    std::unique_ptr<Animation> currentAnimation;
    std::map<std::string, std::unique_ptr<Animation>> animations;

public:
    GameObject() : active(true) {}
    virtual ~GameObject() = default;

    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) {
        if (active) window.draw(sprite);
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    bool isActive() const { return active; }
    void setActive(bool state) { active = state; }

    void setPosition(const sf::Vector2f& pos) {
        position = pos;
        sprite.setPosition(position);
    }

protected:
    void addAnimation(const std::string& name, std::unique_ptr<Animation> animation) {
        animations[name] = std::move(animation);
    }

    void playAnimation(const std::string& name) {
        auto it = animations.find(name);
        if (it != animations.end()) {
            currentAnimation = std::move(it->second);
        }
    }
};

class Player : public GameObject {
private:
    float speed;
    Physics physics;
    bool facingRight;
    sf::Sound jumpSound;

public:
    Player() : speed(200.f), facingRight(true) {
        // Initialize player sprite and animations
        sprite.setTexture(ResourceManager::getInstance().getTexture("player"));
        
        // Setup animations
        auto idleAnim = std::make_unique<Animation>(sprite, 0.2f);
        // Add frames to animation
        addAnimation("idle", std::move(idleAnim));

        auto runAnim = std::make_unique<Animation>(sprite, 0.1f);
        // Add frames to animation
        addAnimation("run", std::move(runAnim));

        playAnimation("idle");
    }

    void update(float deltaTime) override {
        handleInput(deltaTime);
        physics.update(position, deltaTime);
        sprite.setPosition(position);

        if (currentAnimation) {
            currentAnimation->update(deltaTime);
        }
    }

private:
    void handleInput(float deltaTime) {
        bool isMoving = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            physics.velocity.x = speed;
            if (!facingRight) {
                sprite.scale(-1.f, 1.f);
                facingRight = true;
            }
            isMoving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            physics.velocity.x = -speed;
            if (facingRight) {
                sprite.scale(-1.f, 1.f);
                facingRight = false;
            }
            isMoving = true;
        }
        else {
            physics.velocity.x = 0;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && physics.isGrounded) {
            physics.velocity.y = -400.f;
            physics.isGrounded = false;
            // Play jump sound
        }

        playAnimation(isMoving ? "run" : "idle");
    }
};

class Game {
private:
    sf::RenderWindow window;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    sf::Clock clock;
    sf::View camera;
    sf::Music backgroundMusic;

public:
    Game() : window(sf::VideoMode(800, 600), "2D Game"), 
             camera(sf::FloatRect(0, 0, 800, 600)) {
        window.setFramerateLimit(60);
        loadResources();
        initialize();
    }

    void loadResources() {
        // Load textures, sounds, etc.
        ResourceManager::getInstance().loadTexture("player", "assets/player.png");
        // Load other resources...

        backgroundMusic.openFromFile("assets/music.ogg");
        backgroundMusic.setLoop(true);
        backgroundMusic.play();
    }

    void initialize() {
        gameObjects.push_back(std::make_unique<Player>());
        // Add other game objects
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            // Handle other events...
        }
    }

    void update() {
        float deltaTime = clock.restart().asSeconds();
        
        // Update game objects
        for (auto& obj : gameObjects) {
            if (obj->isActive()) {
                obj->update(deltaTime);
            }
        }

        // Update camera to follow player
        if (!gameObjects.empty()) {
            sf::Vector2f playerCenter = gameObjects[0]->getBounds().getPosition();
            camera.setCenter(playerCenter);
            window.setView(camera);
        }

        // Check collisions
        checkCollisions();
    }

    void checkCollisions() {
        // Implement collision detection and response
    }

    void render() {
        window.clear(sf::Color(135, 206, 235)); // Sky blue background
        
        for (auto& obj : gameObjects) {
            if (obj->isActive()) {
                obj->render(window);
            }
        }

        window.display();
    }
};

ResourceManager* ResourceManager::instance = nullptr;

int main() {
    Game game;
    game.run();
    return 0;
}
