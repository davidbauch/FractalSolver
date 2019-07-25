// -O3 -lsfml-graphics -lsfml-window -lsfml-system -lgmpxx -lgmp -fopenmp -std=c++17
// clang++ ultragigaomegafractals.cpp -O3 -lsfml-graphics -lsfml-window -lsfml-system -fopenmp -std=c++17 -I/Users/davidbauch/Downloads/SFML-2.5.1-macos-clang/include/ -L/Users/davidbauch/Downloads/SFML-2.5.1-macos-clang/lib/ -F/Users/davidbauch/Downloads/SFML-2.5.1-macos-clang/Frameworks/ -I/usr/local/Cellar/llvm/8.0.0/include/ -I/usr/include/
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
//#include <gmpxx.h> // -lgmpxx -lgmp
#include <iostream>
#define DO_MULTIFRED // -fopenmp
//#include "include/complex.h"
#include "include/tictoc.h"
#include "include/color.h"
#include "include/fractalsolver.h"
#include "include/rendergrid.h"
#include "include/windowPort.h"// -lsfml-graphics -lsfml-window -lsfml-system
#include "include/interaction.h"

#include <omp.h>



int main(int argc, char *argv[]) {
    
    Fractalsolver::Settings settings = Fractalsolver::Settings("settings.txt");
    FractalWindow window = FractalWindow(settings.screenWidth,settings.screenHeight,"Test");
    window.init();
    
    ColorPalette colorpalette = ColorPalette();
    colorpalette.readColorPaletteFromGnuplotDOTPAL(settings.colorpalette,settings.colorpaletteReps);
    colorpalette.initColors();
    tic(); RenderGrid mbgrid = RenderGrid(settings.screenWidth,settings.screenHeight); toc("Rendergrid ");
    Fractalsolver::Parameters parameters = Fractalsolver::Parameters(settings.centerX, settings.centerY, settings.zoom, settings.screenWidth,settings.screenHeight, settings.iterations);
    Fractalsolver::DoublePrecisionMB solver = Fractalsolver::DoublePrecisionMB();
    tic(); solver.fillRendergrid(mbgrid, parameters); toc("fillRendergrid ");
    tic(); window.BlitRenderGrid(mbgrid, colorpalette, settings.colorModulo); toc("blitRendergrid ");
    Interaction interaction = Interaction();
    
    while (window.run()) {

        window.print(5,5,"FPS: " + std::to_string(window.fps));
        window.print(5,5+window.texthight+2,"Frametime: " + std::to_string(window.frametime) + "ms");
        window.print(5,5+(window.texthight+2)*2,"Last Computation time: " + std::to_string(mbgrid.lastTimeFilled) + "s");
        window.print(5,5+(window.texthight+2)*3,"Zoom: " + std::to_string(3.0/(parameters.x1-parameters.x0)));
        window.print(5,5+(window.texthight+2)*4,"X: " + std::to_string(window.MouseX()) + "Y: " + std::to_string(window.MouseY()));
        window.print(5,5+(window.texthight+2)*5,"Current Screen: X0 = " + std::to_string(parameters.x0) + ", X1 = " + std::to_string(parameters.x1) + ", Y0 = " + std::to_string(parameters.y0) + ", Y1 = " + std::to_string(parameters.y1));
        
        /* Selection */
        interaction.Selection(window, parameters);
        interaction.keyFrameAnimation(window, parameters);
        
        if (window.keyPressed(KEY_m)) {
            parameters.maxIterations /= 2;
            parameters.changed = true;
        }
        if (window.keyPressed(KEY_p)) {
            parameters.maxIterations *= 2;
            parameters.changed = true;
        }
        
        if (parameters.changed) {
            tic(); solver.fillRendergrid(mbgrid, parameters); toc("fillRendergrid ");
            tic(); window.BlitRenderGrid(mbgrid, colorpalette, settings.colorModulo); toc("blitRendergrid ");
            window.print(100,-600,"Current mbgrid.at(10,10) = " + std::to_string(mbgrid.at(10,10).iterations) + ", Parameters x0 = " + std::to_string(parameters.x0));
            if (parameters.saveToFile) {
                window.mainTexture.getTexture().copyToImage().saveToFile("img/" + parameters.frameName+".png");
                parameters.saveToFile = false;
            }
            parameters.changed = false;
        }
        window.flipscreen();
    }
    
    return 0;
}