#ifndef __DB_WINDOW__
#define __DB_WINDOW__

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>// -lsfml-graphics -lsfml-window -lsfml-system

#include <iostream>
#include "rendergrid.h"
#include "color.h"
//#include "VideoEncoder.h"

/* Most basic colors */
#define COLOR_WHITE sf::Color(255,255,255)
#define COLOR_BLACK sf::Color(0,0,0)
#define COLOR_GREEN sf::Color(0,255,0)
#define COLOR_RED sf::Color(255,0,0)
#define COLOR_BLUE sf::Color(0,0,255)

/* Keys */
#define KEY_a sf::Keyboard::A
#define KEY_b sf::Keyboard::B
#define KEY_c sf::Keyboard::C
#define KEY_d sf::Keyboard::D
#define KEY_e sf::Keyboard::E
#define KEY_f sf::Keyboard::F
#define KEY_g sf::Keyboard::G
#define KEY_h sf::Keyboard::H
#define KEY_i sf::Keyboard::I
#define KEY_j sf::Keyboard::J
#define KEY_k sf::Keyboard::K
#define KEY_l sf::Keyboard::L
#define KEY_m sf::Keyboard::M
#define KEY_n sf::Keyboard::N
#define KEY_o sf::Keyboard::O
#define KEY_p sf::Keyboard::P
#define KEY_q sf::Keyboard::Q
#define KEY_r sf::Keyboard::R
#define KEY_s sf::Keyboard::S
#define KEY_t sf::Keyboard::T
#define KEY_u sf::Keyboard::U
#define KEY_v sf::Keyboard::V
#define KEY_w sf::Keyboard::W
#define KEY_x sf::Keyboard::X
#define KEY_y sf::Keyboard::Y
#define KEY_z sf::Keyboard::Z
#define KEY_UP sf::Keyboard::Up
#define KEY_DOWN sf::Keyboard::Down
#define KEY_LEFT sf::Keyboard::Left
#define KEY_RIGHT sf::Keyboard::Right
#define KEY_SPACE sf::Keyboard::Space
#define KEY_LSHIFT sf::Keyboard::LShift
#define KEY_RSHIFT sf::Keyboard::RShift
#define KEY_ESCAPE sf::Keyboard::Escape

class FractalWindow {
    protected:
        
        int mouseX, mouseY;
        bool mouseLB, mouseRB;
        
        class fadeText {
            public:
                int x, y;
                std::string text;
                double timeToDisplay;
                sf::Color color;
                double timeCreated;
                fadeText(int _x = 0, int _y = 0, std::string _text = "", double _timeToDisplay = 5, sf::Color _color = COLOR_WHITE): x(_x), y(_y), text(_text), timeToDisplay(_timeToDisplay), color(_color) {
                    timeCreated = omp_get_wtime();
                }
        };
        std::vector<fadeText> fadetexts;
        
    public:
        int width;
        int height;
        double frametime = 0; 
        int fps = 0;
        std::string name;
        double time_ticks = 0;
        int maxFPS = 60;
        sf::RenderWindow window;
        sf::Font font;
        sf::RenderTexture mainTexture;
        std::vector<sf::Vertex> pixMat;
        sf::Text printtext;
        sf::Clock clock;
        int texthight;
        int keyDown = -1;
        
        FractalWindow(int w = 300, int h = 300, std::string n = "Unnamed") {
            width = w;
            height = h;
            name = n;
            mainTexture.create(w,h);
            pixMat.reserve((w+1)*(h+1));
            font.loadFromFile("include/font.ttf");
            texthight = (int)(0.015*h);
            printtext.setFont(font);
            printtext.setCharacterSize(texthight);
            printtext.setOutlineColor(COLOR_BLACK);
            printtext.setOutlineThickness(1.0f);
            for(int i = 0; i < width; i++){
                 for(int j = 0; j < height; j++){
                    pixMat.push_back(sf::Vertex(sf::Vector2f(i+.5f, j+.5f), sf::Color(i%255,j%255,0)));
                }
            } 
        }
        
        void init() {
            window.create(sf::VideoMode(width,height,32),name, sf::Style::Default ,sf::ContextSettings(0,0,8,2,0));
            window.setVerticalSyncEnabled(true);
            mainTexture.setSmooth(true);
            //window.setFramerateLimit(2);
        }
        
        bool run() {
            keyDown = -1;
            window.clear();
            updateMouseState();
            //mainSprite.setScale(1.0f,-1.0f); 
            window.draw(sf::Sprite(mainTexture.getTexture()));
            /* Fade Texts: */
            for (int i = 0; i < fadetexts.size(); i++) {
                print(fadetexts[i].x, fadetexts[i].y, fadetexts[i].text, fadetexts[i].color);
                if (omp_get_wtime() > fadetexts[i].timeCreated + fadetexts[i].timeToDisplay)
                    fadetexts.erase(fadetexts.begin()+i);
            }
            /* Time, FPS */
            frametime = clock.restart().asMilliseconds();
            fps = (int)(1000.0/frametime);
            
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                if (event.type == sf::Event::KeyPressed)
                    keyDown = event.key.code;
            }
            return window.isOpen();
        }
        
        void flipscreen() {
            window.display();
        }
        
        void BlitRenderGrid(RenderGrid &rendergrid, ColorPalette &colorpalette, double scaling) {
            //pixMat.clear();
            
            #pragma omp parallel for collapse(2) schedule(dynamic)
            for(int i = 0; i < width; i++){
                 for(int j = 0; j < height; j++){
                    double colorvalue = std::fmod((1.0*rendergrid.at(i,j).iterations + rendergrid.at(i,j).mu),scaling)/scaling;
                    //ColorPalette::Color color = colorpalette.getColor(colorvalue);
                    ColorPalette::Color color = colorpalette.getColor(colorvalue,false,true); 
                    //pixMat.push_back(sf::Vertex(sf::Vector2f(i+.5f, height-j+.5f), color));
                    pixMat.at(i*height+height-1-j).color.r = color.r;
                    pixMat.at(i*height+height-1-j).color.g = color.g;
                    pixMat.at(i*height+height-1-j).color.b = color.b;
                    // std::cout << "Color " << std::to_string(color.r) << std::endl;
                    //std::cout << ", after " << std::to_string(pixMat[i].color.r) << std::endl;
                }
            } 
            tic(); mainTexture.draw(pixMat.data(),height*width,sf::Points); toc("mainTexture.draw()");
            //window.draw(sf::Sprite(mainTexture.getTexture()));
        }
        
        sf::Color convertColorToSFML(ColorPalette::Color &color) {
            return sf::Color(color.r, color.g, color.b);
        }
        
        void print(int x, int y, std::string text, sf::Color textcolor = COLOR_WHITE, int background = 0, sf::Color backgroundcolor = COLOR_BLACK) {
            printtext.setFillColor(textcolor);
            printtext.setPosition((float)x,(float)y);
            printtext.setString(text);
            window.draw(printtext);// std::cout << "Test " << text << std::endl;
        }
        void printFade(int x, int y, std::string text, double fadeTime = 2.0, sf::Color textcolor = COLOR_WHITE, int background = 0, sf::Color backgroundcolor = COLOR_BLACK) {
            fadetexts.push_back(fadeText(x,y,text,fadeTime,textcolor));
        }
        
        bool keyPressed(int key) {
            //if (keyDown == key)
            //    std::cout << "Key down? " << std::to_string(keyDown == key) << " and key is " << std::to_string(key) << std::endl;
            return keyDown == key;
        }
        void updateMouseState() {
            mouseLB = sf::Mouse::isButtonPressed(sf::Mouse::Left);
            mouseRB = sf::Mouse::isButtonPressed(sf::Mouse::Right);
            sf::Vector2i position = sf::Mouse::getPosition(window);
            mouseX = position.x;
            mouseY = position.y;
        }
        int MouseX() {
            return mouseX;
        }
        int MouseY() {
            return mouseY;
        }
        bool leftMouseDown() {
            return mouseLB;
        }
        bool rightMouseDown() {
            return mouseRB;
        }
        void horLine(int y0, int x0, int x1, sf::Color color = COLOR_WHITE) {
            sf::RectangleShape line(sf::Vector2f(x1-x0,1.0f));
            line.setPosition(sf::Vector2f(x0,y0));
            line.setFillColor(color);
            window.draw(line);
        }
        void verLine(int x0, int y0, int y1, sf::Color color = COLOR_WHITE) {
            sf::RectangleShape line(sf::Vector2f(y1-y0,1.0f));
            line.setPosition(sf::Vector2f(x0,y0));
            line.setFillColor(color);
            line.rotate(90);
            window.draw(line);
        }
        void drawRect(int x0, int x1, int y0, int y1, sf::Color color = COLOR_WHITE) {
            if (x0 <= 0) x0 = 0;
            if (x1 >= width) x1 = width;
            if (y0 <= 0) y0 = 0;
            if (y1 >= height) y1 = height;
            horLine(y0,x0,x1,COLOR_GREEN);
            horLine(y1,x0,x1,color);
            verLine(x0,y0,y1,color);
            verLine(x1,y0,y1,color);
        }
        
};
#endif