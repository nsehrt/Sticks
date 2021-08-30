#include <SFML/Graphics.hpp>
#include "gametime.h"
#include <vector>
#include <iostream>

struct Point
{
    sf::Vector2f Position{};
    sf::Vector2f PrevPosition{};
    bool Locked = false;
};

struct Stick
{
    Point* PointA = nullptr;
    Point* PointB = nullptr;
    float Length{};
};

constexpr float pRadius = 10.0f;
constexpr int sIter = 5;
const sf::Color bg{ 227,227,227,255 };
const sf::Color pc{ 167,97,198,255 };
const sf::Color sc{ 141,141,141,255 };
const sf::Color fc{ 207,54,104,255 };

inline float Vec2Magnitude(const sf::Vector2f& vector)
{
    return std::hypot(vector.x, vector.y);
}

inline float Vec2Distance(const sf::Vector2f& p1, const sf::Vector2f& p2)
{
    return static_cast<float>(std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2)));
}

inline sf::Vector2f Vec2Normalize(const sf::Vector2f& vector)
{
    const float mag = Vec2Magnitude(vector);
    if(mag == 0.0f)
    {
        return { 0,0 };
    }
    return { vector.x / mag, vector.y / mag };
}

inline float PointInCircle(const sf::Vector2f& circleCenter, const sf::Vector2f& point)
{
    const auto pos = circleCenter - point;

    if(std::sqrt(pos.x * pos.x + pos.y * pos.y) < pRadius)
    {
        return true;
    }
    return false;
}

inline float PointStick(const Stick& stick, const sf::Vector2f& point)
{
    const float epsilon = 2.5f;

    const float length = Vec2Distance(stick.PointA->Position, stick.PointB->Position);
    const float d1 = Vec2Distance(point, stick.PointA->Position);
    const float d2 = Vec2Distance(point, stick.PointB->Position);

    if(d1 + d2 >= length - epsilon && d1 + d2 <= length + epsilon)
    {
        return true;
    }

    return false;
}




int main()
{
    bool setupPhase = true;
    float simTime = 0.f;
    bool rightMousePressed = false;
    Point* startStick = nullptr;

    sf::RenderWindow window(sf::VideoMode(800, 800), "Sticks!");
    window.setVerticalSyncEnabled(true);
    std::vector<Point> points{};
    std::vector<Stick> sticks{};
    GameTime gt{};

    sf::CircleShape pointShape{};
    pointShape.setRadius(pRadius);
    

    sf::Vertex stickLine[] = { sf::Vertex{}, sf::Vertex{} };
    sf::Vertex previewLine[] = { sf::Vertex{}, sf::Vertex{} };
    stickLine[0].color = sc;
    stickLine[1].color = sc;
    previewLine[0].color = sc;
    previewLine[1].color = sc;

    gt.reset();


    while(window.isOpen())
    {
        gt.update();

        const auto pos = sf::Vector2f{ sf::Mouse::getPosition(window) };

        sf::Event event;

        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();

            if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                setupPhase = !setupPhase;
            }

            if(setupPhase)
            {

                if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F)
                {
                    points.clear();

                    for(int i = 0; i < 15; i++)
                    {
                        for(int j = 0; j < 15; j++)
                        {
                            points.push_back(Point{ sf::Vector2f{50 + i * 50.f, 50 + j * 50.f}, sf::Vector2f{50 + i * 50.f, 50 + j * 50.f}, false });
                        }
                    }

                    for(int i = 0; i < 15 * 15; i++)
                    {
                        if(i == 0 || (i+1) % 15 != 0)
                            sticks.push_back(Stick{ &points[i], &points[i + 1], 50.f });
                        if(i < 15*14)
                            sticks.push_back(Stick{ &points[i], &points[i + 15], 50.f });
                    }
                    

                }

                //add/edit point
                if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {

                    //check already exists
                    Point* exists = nullptr;
                    for(auto& p : points)
                    {
                        if(PointInCircle(p.Position, pos))
                        {
                            exists = &p;
                        }
                    }

                    if(exists)
                    {
                        exists->Locked = !exists->Locked;
                    }
                    else
                    {
                        points.push_back(Point{ pos, pos, false });
                        sticks.clear();
                    }

                }

                if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
                {
                    rightMousePressed = false;
                    for(auto& p : points)
                    {
                        if(PointInCircle(p.Position, pos))
                        {
                            startStick = &p;
                            rightMousePressed = true;
                            break;
                        }
                    }

                }

                if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right && rightMousePressed)
                {
                    
                    for(auto& p : points)
                    {
                        if(PointInCircle(p.Position, pos))
                        {
                            if(&p != startStick)
                            {
                                const auto dist = Vec2Distance(startStick->Position, p.Position);
                                sticks.push_back(Stick{ startStick, &p, dist });
                                break;
                            }
                        }
                    }

                    startStick = nullptr;
                    rightMousePressed = false;
                }



            }
            else
            {
                if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
                {
                    rightMousePressed = true;
                }

                if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right)
                {
                    rightMousePressed = false;
                }

            }



        }


        //Simulate
        if(!setupPhase)
        {

            if(rightMousePressed)
            {
                int i = 0;
                bool hit = false;
                for(auto& s : sticks)
                {
                    if(PointStick(s, pos))
                    {
                        hit = true;
                        break;
                    }
                    i++;
                }

                if(hit)
                {
                    sticks.erase(sticks.begin() + i);
                }
            }

            for(auto& p : points)
            {
                if(!p.Locked)
                {
                    const auto beforePos = p.Position;
                    p.Position += p.Position - p.PrevPosition;
                    p.Position += sf::Vector2f{ 0.0,1.0f } * 9.81f * 200.0f * gt.getDelta().asSeconds() * gt.getDelta().asSeconds();
                    p.PrevPosition = beforePos;
                }
            }

            for(auto i = 0; i < sIter; i++)
            {
                for(auto& s : sticks)
                {
                    const auto stickCenter = (s.PointA->Position + s.PointB->Position) / 2.0f;
                    const auto stickDir = Vec2Normalize(s.PointA->Position - s.PointB->Position);

                    if(!s.PointA->Locked)
                    {
                        s.PointA->Position = stickCenter + stickDir * s.Length / 2.0f;
                    }
                    if(!s.PointB->Locked)
                    {
                        s.PointB->Position = stickCenter - stickDir * s.Length / 2.0f;
                    }
                }
            }
        }


        window.clear(bg);

        for(const auto& s : sticks)
        {
            stickLine[0].position = s.PointA->Position;
            stickLine[1].position = s.PointB->Position;
            window.draw(stickLine, 2, sf::Lines);
        }

        if(startStick)
        {
            previewLine[0].position = startStick->Position;
            previewLine[1].position = sf::Vector2f{ sf::Mouse::getPosition(window) };
            window.draw(previewLine, 2, sf::Lines);
        }
        

        for(const auto& p : points)
        {
            pointShape.setPosition(p.Position - sf::Vector2f{pRadius, pRadius});
            pointShape.setFillColor(p.Locked ? fc : pc);
            window.draw(pointShape);
        }

        window.display();

    }

    return 0;
}