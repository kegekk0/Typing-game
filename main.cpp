#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>

enum GameState { MENU, PLAYING, GAME_OVER, PAUSED, SCOREBOARD, SETTINGS };
enum FontType { BIT_FONT, ARIAL };

std::vector<int> scores;

void loadScores() {
    std::ifstream file("assets//scores.txt");
    if (!file) {
        return;
    }

    int score;
    while (file >> score) {
        scores.push_back(score);
    }
}

void saveScores() {
    std::ofstream file("assets//scores.txt");
    for (int score : scores) {
        file << score << '\n';
    }
}

std::vector<std::string> loadWords(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    std::string word;
    while (file >> word) {
        words.push_back(word);
    }
    return words;
}

std::string getRandomWord(const std::vector<std::string>& words) {
    int index = rand() % words.size();
    return words[index];
}

class FallingWord {
public:
    FallingWord(std::string  word, float x, float y, const sf::Font &font, float speed, int fontSize)
            : word(std::move(word)), position(x, y), speed(speed), fontSize(fontSize) {
        text.setFont(font);
        text.setCharacterSize(fontSize);
        text.setPosition(position);
        text.setFillColor(sf::Color::White);

        matchedText.setFont(font);
        matchedText.setCharacterSize(fontSize);
        matchedText.setPosition(position);
        matchedText.setFillColor(sf::Color::Green);
    }

    void update(float deltaTime) {
        position.y += speed * deltaTime;
        text.setPosition(position);
        matchedText.setPosition(position);
    }

    bool checkCollision(float windowHeight) const {
        return position.y >= windowHeight - 100;
    }

    sf::Text getText() const {
        return text;
    }

    const sf::Text& getMatchedText() {
        return matchedText;
    }

    const std::string& getWord() const {
        return word;
    }

    float getSpeed() const {
        return speed;
    }

    void highlight(const std::string& input) {
        size_t matchLength = 0;
        for (size_t i = 0; i < std::min(input.size(), word.size()); ++i) {
            if (input[i] == word[i]) {
                ++matchLength;
            } else {
                break;
            }
        }

        std::string matchedPart = word.substr(0, matchLength);
        std::string remainingPart = word.substr(matchLength);

        matchedText.setString(matchedPart);
        text.setString(remainingPart);

        float matchedWidth = matchedText.getGlobalBounds().width;
        text.setPosition(position.x + matchedWidth, position.y);
    }

    void changeFont(const sf::Font& newFont) {
        text.setFont(newFont);
        matchedText.setFont(newFont);
    }

    void changeFontSize(int newFontSize) {
        text.setCharacterSize(newFontSize);
        matchedText.setCharacterSize(newFontSize);
    }

private:
    std::string word;
    sf::Vector2f position;
    sf::Text text;
    sf::Text matchedText;
    float speed;
    int fontSize;
};

void saveGameState(int score, int lives, const std::vector<FallingWord>& fallingWords, const sf::Font& currentFont, int currentFontSize) {
    std::ofstream file("assets//save.txt");
    if (!file) {
        fmt::print("Failed to open save file.\n");
        return;
    }

    file << score << '\n';
    file << lives << '\n';
    file << currentFontSize << '\n';

    // Save font file name
    std::string fontFileName = (currentFont.getInfo().family == "Arial") ? "arial.ttf" : "8BitFont.ttf";
    file << fontFileName << '\n';

    for (const auto& word : fallingWords) {
        file << word.getWord() << ' ' <<
             word.getText().getPosition().x << ' ' <<
             word.getText().getPosition().y << ' ' <<
             word.getSpeed() << '\n';
    }
}

void loadSave(int &score, int &lives, std::vector<FallingWord>& fallingWords, sf::Font& currentFont, int& currentFontSize) {
    std::ifstream file("assets//save.txt");
    if (!file) {
        fmt::print("Failed to open save file.\n");
        return;
    }

    file >> score;
    file >> lives;
    file >> currentFontSize;

    std::string fontFileName;
    file >> fontFileName;

    if (!currentFont.loadFromFile("assets//" + fontFileName)) {
        fmt::print("Failed to load font {}\n", fontFileName);
    }

    fallingWords.clear();

    std::string word;
    float x, y, speed;
    while (file >> word >> x >> y >> speed) {
        fallingWords.emplace_back(word, x, y, currentFont, speed, currentFontSize);
    }
}

auto main() -> int {
    loadScores();
    std::vector<std::string> wordList = loadWords("assets/words.txt");
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktopMode, "Dragon Typer", sf::Style::Close | sf::Style::Resize);
    window.setFramerateLimit(60);

    int counter = 0;

    sf::Font arial;
    if (!arial.loadFromFile("assets//arial.ttf")) {
        fmt::print("Failed to load arial.ttf\n");
        return -1;
    }

    sf::Font bitFont;
    if (!bitFont.loadFromFile("assets//8BitFont.ttf")) {
        fmt::print("Failed to load 8BitFont.ttf\n");
        return -1;
    }

    sf::Font currentFont = bitFont;
    FontType currentFontType = BIT_FONT;
    int currentFontSize = 24;

    sf::View view(sf::FloatRect(0, 0,
                                static_cast<float>(desktopMode.width),
                                static_cast<float>(desktopMode.height)));

    //title
    auto title = sf::Text("Dragon Typer", bitFont, 50);
    title.setPosition(262, 250);
    title.setFillColor(sf::Color::Green);

    // Start button
    auto textStart = sf::Text("START", bitFont, 34);
    textStart.setPosition({262, 360});
    textStart.setFillColor(sf::Color::Green);

    auto startButton = sf::RectangleShape(sf::Vector2f(183, 50));
    startButton.setPosition({253, 350});
    startButton.setFillColor(sf::Color::Transparent);

    // Load button
    auto textLoad = sf::Text("LOAD", bitFont, 34);
    textLoad.setPosition({262, 410});
    textLoad.setFillColor(sf::Color::Green);

    auto loadButton = sf::RectangleShape(sf::Vector2f(150, 50));
    loadButton.setPosition({253, 400});
    loadButton.setFillColor(sf::Color::Transparent);

    // Scoreboard button
    auto textScoreboard = sf::Text("SCOREBOARD", bitFont, 34);
    textScoreboard.setPosition({262, 460});
    textScoreboard.setFillColor(sf::Color::Green);

    auto scoreButton = sf::RectangleShape(sf::Vector2f(350, 50));
    scoreButton.setPosition({253, 450});
    scoreButton.setFillColor(sf::Color::Transparent);

    // Settings button
    auto textSettings = sf::Text("SETTINGS", bitFont, 34);
    textSettings.setPosition({262, 510});
    textSettings.setFillColor(sf::Color::Green);

    auto settingsButton = sf::RectangleShape(sf::Vector2f(280, 50));
    settingsButton.setPosition({253, 500});
    settingsButton.setFillColor(sf::Color::Transparent);

    // Change font button
    auto textFont = sf::Text("CHANGE FONT", bitFont, 34);
    textFont.setPosition({262, 460});
    textFont.setFillColor(sf::Color::Green);

    auto fontButton = sf::RectangleShape(sf::Vector2f(385, 50));
    fontButton.setPosition({253, 450});
    fontButton.setFillColor(sf::Color::Transparent);

    // Font change right button
    auto textFontRight = sf::Text(">", bitFont, 34);
    textFontRight.setPosition({1052, 460});
    textFontRight.setFillColor(sf::Color::Green);

    auto fontChangeRightButton = sf::RectangleShape(sf::Vector2f(50, 50));
    fontChangeRightButton.setPosition({1043, 450});
    fontChangeRightButton.setFillColor(sf::Color::Transparent);

    // Font change left button
    auto textFontLeft = sf::Text("<", bitFont, 34);
    textFontLeft.setPosition({762, 460});
    textFontLeft.setFillColor(sf::Color::Green);

    auto fontChangeLeftButton = sf::RectangleShape(sf::Vector2f(50, 50));
    fontChangeLeftButton.setPosition({753, 450});
    fontChangeLeftButton.setFillColor(sf::Color::Transparent);

    // Font text button
    auto textFontSize = sf::Text("FONT SIZE", bitFont, 34);
    textFontSize.setPosition({262, 510});
    textFontSize.setFillColor(sf::Color::Green);

    // Font size increase button
    auto textFontIncrease = sf::Text(">", bitFont, 34);
    textFontIncrease.setPosition({942, 510});
    textFontIncrease.setFillColor(sf::Color::Green);

    auto fontIncreaseButton = sf::RectangleShape(sf::Vector2f(50, 50));
    fontIncreaseButton.setPosition({933, 500});
    fontIncreaseButton.setFillColor(sf::Color::Transparent);

    // Font size decrease button
    auto textFontDecrease = sf::Text("<", bitFont, 34);
    textFontDecrease.setPosition({762, 510});
    textFontDecrease.setFillColor(sf::Color::Green);

    auto fontDecreaseButton = sf::RectangleShape(sf::Vector2f(50, 50));
    fontDecreaseButton.setPosition({753, 500});
    fontDecreaseButton.setFillColor(sf::Color::Transparent);

    // Quit button
    auto textQuit = sf::Text("QUIT", bitFont, 34);
    textQuit.setPosition({262, 560});
    textQuit.setFillColor(sf::Color::Green);

    auto quitButton = sf::RectangleShape(sf::Vector2f(150, 50));
    quitButton.setPosition({253, 550});
    quitButton.setFillColor(sf::Color::Transparent);

    // Pause window
    sf::RectangleShape pauseWindow(sf::Vector2f(desktopMode.width, desktopMode.height));
    pauseWindow.setFillColor(sf::Color::Black);
    pauseWindow.setPosition(0,0);

    // Pause button
    auto textPause = sf::Text("PAUSE", bitFont, 30);
    textPause.setPosition({0, 10});
    textPause.setFillColor(sf::Color::White);

    auto pauseButton = sf::RectangleShape(sf::Vector2f(150, 45));
    pauseButton.setPosition({0, 0});
    pauseButton.setFillColor(sf::Color::Transparent);

    // Resume button
    sf::Text resumeText("RESUME", bitFont, 24);
    resumeText.setPosition((desktopMode.width - 300) / 2 + 25, (desktopMode.height - 200) / 2);

    // Quit to menu button
    sf::Text quitMenuText("QUIT", bitFont, 24);
    quitMenuText.setPosition((desktopMode.width - 300) / 2 + 50, (desktopMode.height - 200) / 2 + 50);

    // Save button
    sf::Text saveText("SAVE", bitFont, 24);
    saveText.setPosition((desktopMode.width - 300) / 2 + 50, (desktopMode.height - 200) / 2 + 100);

    sf::Texture bgMenuTexture;
    if (!bgMenuTexture.loadFromFile("assets//backgroundProject.jpg"))
        return EXIT_FAILURE;

    sf::Texture bgGameTexture;
    if(!bgGameTexture.loadFromFile("assets//backgroundProjectGame.jpg"))
        return EXIT_FAILURE;

    bool change = true;

    GameState gameState = MENU;

    std::vector<FallingWord> fallingWords;
    std::string currentInput;
    sf::Clock clock;
    srand(static_cast<unsigned int>(time(nullptr)));
    int lives;

    sf::Sprite bgMenu;
    bgMenu.setTexture(bgMenuTexture);
    sf::Sprite bgGame;
    bgGame.setTexture(bgGameTexture);

    while (window.isOpen()) {

        auto event = sf::Event();
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;

                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        auto mousePos = sf::Mouse::getPosition(window);
                        if (gameState == MENU) {
                            if (quitButton.getGlobalBounds().contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                                window.close();
                            } else if (startButton.getGlobalBounds().contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                                gameState = PLAYING;
                                fallingWords.clear();
                                currentInput.clear();
                                clock.restart();
                                counter = 0;
                                lives = 3;
                                change = true;
                            } else if (scoreButton.getGlobalBounds().contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                                gameState = SCOREBOARD;
                                change = true;
                            }else if (settingsButton.getGlobalBounds().contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                                gameState = SETTINGS;
                                change = true;
                            } else if (loadButton.getGlobalBounds().contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                                std::fstream file("assets//save.txt");
                                if (file.peek() == std::ifstream::traits_type::eof()) {
                                    break;
                                }

                                loadSave(counter, lives, fallingWords, currentFont, currentFontSize);
                                gameState = PLAYING;
                                clock.restart();
                                change = true;
                            }
                        } else if (gameState == GAME_OVER) {
                            gameState = MENU;
                            change = true;
                        } else if(gameState == SCOREBOARD){
                            gameState = MENU;
                            change = true;
                        } else if (gameState == PLAYING){
                            if(pauseButton.getGlobalBounds().contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})){
                                saveGameState(counter, lives, fallingWords, currentFont, currentFontSize);
                                gameState = PAUSED;
                                change = true;
                            }
                        }else if (gameState == PAUSED){
                            if (quitMenuText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                gameState = MENU;
                                change = true;
                            } else if (saveText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                saveGameState(counter, lives, fallingWords, currentFont, currentFontSize);
                            } else if (resumeText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                loadSave(counter, lives, fallingWords, currentFont, currentFontSize);
                                gameState = PLAYING;
                                clock.restart();
                                change = true;
                            }
                        } else if (gameState == SETTINGS) {
                            if(fontChangeRightButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                if (currentFontType == ARIAL) {
                                    currentFontType = BIT_FONT;
                                    currentFont = arial;
                                    change = true;
                                } else if (currentFontType == BIT_FONT) {
                                    currentFontType = ARIAL;
                                    currentFont = arial;
                                    change = true;
                                }
                            } else if (fontChangeLeftButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                if (currentFontType == BIT_FONT) {
                                    currentFontType = ARIAL;
                                    currentFont = arial;
                                    change = true;
                                } else {
                                    currentFontType = BIT_FONT;
                                    currentFont = bitFont;
                                    change = true;
                                }
                            } else if (fontIncreaseButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                currentFontSize = std::min(40, currentFontSize + 1);
                                change = true;
                                for (auto& word : fallingWords) {
                                    word.changeFontSize(currentFontSize);
                                }
                            } else if (fontDecreaseButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                currentFontSize = std::max(4, currentFontSize - 1);
                                change = true;
                                for (auto& word : fallingWords) {
                                    word.changeFontSize(currentFontSize);
                                }
                            }
                        }
                    }
                    break;

                case sf::Event::TextEntered:
                    if (gameState == PLAYING) {
                        if (event.text.unicode == '\b') { // backspace
                            if (!currentInput.empty()) {
                                currentInput.pop_back();
                            }
                        } else if (event.text.unicode == '\r') { // enter
                            for (auto it = fallingWords.begin(); it != fallingWords.end(); ++it) {
                                if (it->getWord() == currentInput) {
                                    fallingWords.erase(it);
                                    currentInput.clear();
                                    if(it->getWord().size() < 6){
                                        counter++;
                                    } else if (it->getWord().size() < 10 && it->getWord().size() >= 6){
                                        counter += 2;
                                    } else if (it-> getWord().size() >= 10){
                                        counter += 3;
                                    }
                                    break;
                                }
                            }
                        } else if (event.text.unicode < 128) {
                            currentInput += static_cast<char>(event.text.unicode);
                        }
                    }
                    break;

                case sf::Event::Resized:
                    view.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
                    view.setCenter(static_cast<float>(event.size.width) / 2, static_cast<float>(event.size.height) / 2);
                    window.setView(view);
                    change = true;
                    break;

                case sf::Event::KeyPressed:
                    if(gameState == GAME_OVER){
                        gameState = MENU;
                        change = true;
                    } else if(gameState == SCOREBOARD){
                        gameState = MENU;
                        change = true;
                    } else if(gameState == SETTINGS){
                        gameState = MENU;
                        change = true;
                    }

                default:
                    break;
            }
        }

        if (gameState == PLAYING) {
            float deltaTime = clock.restart().asSeconds();

            if (rand() % std::max(1, 300 - counter * 2) < 1.5) {
                std::string newWord = getRandomWord(wordList);
                float x = static_cast<float>(rand() % (window.getSize().x - 150));
                float speed = 70.0f + counter;
                fallingWords.emplace_back(newWord, x, 0.0f, bitFont, speed, currentFontSize);
            }

            for (auto& word : fallingWords) {
                word.update(deltaTime);
                word.highlight(currentInput);
            }

            // collision with the bottom
            for (auto it = fallingWords.begin(); it != fallingWords.end(); ) {
                if (it->checkCollision(window.getSize().y)) {
                    lives--;
                    if (lives == 0) {
                        gameState = GAME_OVER;
                        break;
                    }
                    it = fallingWords.erase(it);  // Remove word that reached the bottom
                } else {
                    ++it;
                }
            }
            change = true;
        }

        if (change) {
            window.clear();
            window.setView(view);

            if (gameState == MENU) {
                window.draw(bgMenu);
                window.draw(title);
                window.draw(startButton);
                window.draw(textStart);
                window.draw(loadButton);
                window.draw(textLoad);
                window.draw(scoreButton);
                window.draw(textScoreboard);
                window.draw(settingsButton);
                window.draw(textSettings);
                window.draw(quitButton);
                window.draw(textQuit);

            } else if (gameState == PLAYING) {
                window.draw(bgGame);
                window.draw(pauseButton);
                window.draw(textPause);

                sf::RectangleShape line({static_cast<float>(window.getSize().x), 4});
                line.setPosition(0, window.getSize().y - 100);
                line.setFillColor(sf::Color::White);
                window.draw(line);

                for (auto& word : fallingWords) {
                    word.changeFont(currentFont);
                    window.draw(word.getText());
                    window.draw(word.getMatchedText());
                }

                sf::Text inputText(currentInput, bitFont, 24);
                sf::FloatRect inputBounds = inputText.getGlobalBounds();

                float xPos = (window.getSize().x - inputBounds.width) / 2;
                float yPos = window.getSize().y - 75;

                inputText.setPosition(xPos, yPos);
                inputText.setFillColor(sf::Color::White);
                window.draw(inputText);

                sf::Text wordCountText("Score: " + std::to_string(counter), bitFont, 24);
                wordCountText.setPosition(10, window.getSize().y - 75);
                wordCountText.setFillColor(sf::Color::White);
                window.draw(wordCountText);

                sf::Text livesText("Lives: " + std::to_string(lives), bitFont, 24);
                livesText.setPosition(1600, window.getSize().y - 75);
                livesText.setFillColor(sf::Color::White);
                window.draw(livesText);

            } else if (gameState == GAME_OVER) {
                sf::Text gameOverText("Game Over! Press any key to return to menu.", bitFont, 35);
                gameOverText.setFillColor(sf::Color::Red);

                sf::FloatRect textBounds = gameOverText.getGlobalBounds();

                float xPos = (window.getSize().x - textBounds.width) / 2;
                float yPos = (window.getSize().y - textBounds.height) / 2;

                gameOverText.setPosition(xPos, yPos);

                window.draw(bgGame);
                window.draw(gameOverText);
                std::ofstream file("assets//save.txt", std::ios::trunc);
                scores.push_back(counter);
                saveScores();

            } else if (gameState == PAUSED) {
                window.draw(pauseWindow);
                window.draw(resumeText);
                window.draw(quitMenuText);
                window.draw(saveText);

            } else if (gameState == SCOREBOARD) {
                window.draw(bgMenu);

                std::ranges::sort(scores, std::greater<int>()); //check

                float yPos = 100.0f;
                int place = 1;
                for (int score : scores) {
                    std::string placeText;
                    switch (place) {
                        case 1:
                            placeText = "1st place:";
                            break;
                        case 2:
                            placeText = "2nd place:";
                            break;
                        case 3:
                            placeText = "3rd place:";
                            break;
                        default:
                            placeText = std::to_string(place) + "th place:";
                            break;
                    }

                    sf::Text scoreText(placeText + " " + std::to_string(score), bitFont, 24);
                    scoreText.setPosition(100.0f, yPos);
                    scoreText.setFillColor(sf::Color::Green);
                    window.draw(scoreText);
                    yPos += 30.0f;
                    place++;
                }

                sf::Text backText("Press any key to return to menu", bitFont, 24);
                backText.setPosition(100.0f, yPos);
                backText.setFillColor(sf::Color::Red);
                window.draw(backText);

            } else if (gameState == SETTINGS) {
                sf::Text backText("Press any key to return to menu", bitFont, 24);
                backText.setPosition(100.0f, 100.0f);
                backText.setFillColor(sf::Color::Red);

                sf::Text fontBitText("BitFont", bitFont, 34);
                fontBitText.setPosition(810, 460);
                fontBitText.setFillColor(sf::Color::Green);

                sf::Text fontArialText("Arial", arial, 34);
                fontArialText.setPosition(890, 460);
                fontArialText.setFillColor(sf::Color::Green);

                sf::Text currentFontSizeText(std::to_string(currentFontSize), bitFont, 34);
                currentFontSizeText.setPosition(840, 510);
                currentFontSizeText.setFillColor(sf::Color::Green);

                window.draw(bgMenu);
                window.draw(backText);
                window.draw(fontButton);
                window.draw(textFont);
                window.draw(fontDecreaseButton);
                window.draw(textFontDecrease);
                window.draw(fontIncreaseButton);
                window.draw(textFontIncrease);
                window.draw(currentFontSizeText);
                window.draw(fontChangeLeftButton);
                window.draw(textFontLeft);
                window.draw(fontChangeRightButton);
                window.draw(textFontRight);
                window.draw(textFontSize);

                if(currentFontType == BIT_FONT){
                    window.draw(fontBitText);
                } else if (currentFontType == ARIAL) {
                    window.draw(fontArialText);
                }
            }
            window.display();
            change = false;
        }
    }
    return 0;
}
