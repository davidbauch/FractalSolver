#ifndef __DB_FRACTALSOLVER__
#define __DB_FRACTALSOLVER__

#include "rendergrid.h"
//#include "complex.h"
#include <vector>
#include <cmath>
#include <complex>
#include <omp.h>

/*
    This class calculates the fractal and saves iterations/smoothing value in a rendergrid
*/

namespace Fractalsolver {
    using namespace std::complex_literals;
    class Parameters {
        public:
            double x0, x1, y0, y1, numx, numy, centerX, centerY, zoom;
            int maxIterations;
            bool changed;
            bool saveToFile;
            std::string frameName;
            /* Regular x,y-range */
            Parameters(double _x0 = -2.5, double _x1 = 1.0, double _y0 = -1.0, double _y1 = 1.0, double _numx = 300.0, double _numy = 300.0, int _maxIterations = 64) {
                x0 = _x0;
                x1 = _x1;
                y0 = _y0;
                y1 = _y1;
                centerX = _x0 + (_x1 - _x0)/2.0;
                centerY = _y0 + (_y1 - _y0)/2.0;
                zoom = 3.5/(_x1 - _x0);
                numx = _numx;
                numy = _numy;
                maxIterations = _maxIterations;
                changed = false;
                saveToFile = false;
            }
            /* Center + Zoom */
            Parameters(double _centerX = -0.75, double _centerY = 0, double _zoom = 1.0, double _numx = 300.0, double _numy = 300.0, int _maxIterations = 64) {
                x0 = -0.5*3.5/_zoom + _centerX;
                x1 = 0.5*3.5/_zoom + _centerX;
                y0 = -0.5*2.0/_zoom + _centerY;
                y1 = 0.5*2.0/_zoom + _centerY;
                centerX = _centerX;
                centerY = _centerY;
                zoom = _zoom;
                numx = _numx;
                numy = _numy;
                maxIterations = _maxIterations;
                changed = false;
                saveToFile = false;
            }
            /* Standard */
            Parameters(double _numx = 300.0, double _numy = 300.0, int _maxIterations = 64): Parameters(-0.75, 0.0, 1.0, _numx, _numy, _maxIterations) { }
    };
    
    class Settings {
        public:
            int colorModulo;
            int screenWidth;
            int screenHeight;
            double screenAspectRatio;
            std::string colorpalette;
            int colorpaletteReps;
            double centerX, centerY, zoom; 
            int iterations;
            
            Settings(std::string filepath) {
                FILE *file = fopen(filepath.c_str(),"r");
                if (file) {
                    fscanf(file, "%d\n", &colorModulo);
                    fscanf(file, "%d\n", &screenWidth);
                    fscanf(file, "%d\n", &screenHeight);
                    screenAspectRatio = (double)screenWidth/screenHeight;
                    char temp[255];
                    fscanf(file, "%s%d\n", &temp, &colorpaletteReps);
                    colorpalette = temp;
                    fscanf(file, "%le%le%le%d\n", &centerX, &centerY, &zoom, &iterations);
                    fclose(file);
                } else  {
                    std::cout << "Couldn't open settings from file '" << filepath << "'" << std::endl;
                    colorModulo = 64;
                    screenWidth = 400;
                    screenHeight = 400;
                    screenAspectRatio = 1.0;
                    colorpalette = "colorpalettes/heatmap.pal";
                    colorpaletteReps = 1;
                    centerX = -0.75;
                    centerY = 0.0;
                    zoom = 1.0;
                    iterations = 32;
                }                
                std::cout << "Input Settings from file '" << filepath << "'" << std::endl;
                std::cout << "Color Modulo: " << colorModulo << std::endl;
                std::cout << "Screen Width: " << screenWidth << std::endl;
                std::cout << "Screen Height: " << screenHeight << std::endl;
                std::cout << "Colorpalette file used: " << colorpalette << " at " << colorpaletteReps << " repetitions" << std::endl;
                std::cout << "Using initial Parameters: X0 = " << centerX << ", Y0 = " << centerY << ", zoom = " << zoom << " at " << iterations << " iterations" << std::endl;
            }
    };
    
    class Solver {
        void fillRendergrid(RenderGrid &rendergrid, Parameters p);
    };
    
    class DoublePrecisionMB : Solver {
        public:
            
            double escapeRadius, escapeRadiusSquared, log2n;
            int errorIterations;
            
            DoublePrecisionMB() {
                escapeRadius = 2.0;
                escapeRadiusSquared = 4.0;
                errorIterations = 5;
                log2n = std::log(2.0);
            }
            
            int inCardioid(double x, double y) {
                double q = (x-0.25)*(x-0.25) + y*y;
                return ( q*(q+(x-0.25)) <= 0.25*y*y ) || ( (x+1)*(x+1) + y*y <= 1.0/16.0 );
            }
            
            void fillRendergrid(RenderGrid &rendergrid, Parameters &p) {
                /* Extracting numx,numy from the parameters class so openmp can parallise the loop */
                int numx = p.numx; int numy = p.numy; double tt = omp_get_wtime();
                #pragma omp parallel for collapse(2) schedule(dynamic)
                for (int x = 0; x < numx; x++) {
                    for (int y = 0; y < numy; y++) {
                        /* Coordinates */
                        double re = p.x0 + (p.x1 - p.x0)*(1.0*x)/p.numx;
                        double im = p.y0 + (p.y1 - p.y0)*(1.0*y)/p.numy;
                        /* Cardioid Check */
                        /* Helper Variables */
                        double oldRe = 0; double oldIm = 0; 
                        double newRe = 0; double newIm = 0;
                        double tempNewRe = 0; double tempNewIm = 0;
                        if (!inCardioid(re,im)) {                            
                            /* Calculate Iterations + Error Iterations */
                            int it;
                            for (it = 1; it <= p.maxIterations; it++){
                                oldRe = newRe; oldIm = newIm;
                                newRe = tempNewRe - tempNewIm + re;
                                newIm = 2.0*oldRe*oldIm + im;
                                tempNewRe = newRe*newRe;
                                tempNewIm = newIm*newIm;
                                if (tempNewRe + tempNewIm > escapeRadiusSquared) 
                                    break;
                            }
                            for (int i = 0; i <= errorIterations; i++){
                                oldRe = newRe; oldIm = newIm;
                                newRe = tempNewRe - tempNewIm + re;
                                newIm = 2.0*oldRe*oldIm + im;
                                tempNewRe = newRe*newRe;
                                tempNewIm = newIm*newIm;
                                it++;
                            }
                            rendergrid.at(x,y).iterations = it;
                        } else {
                            rendergrid.at(x,y).iterations = p.maxIterations;
                        }
                        if (rendergrid.at(x,y).iterations < p.maxIterations)
                            rendergrid.at(x,y).mu = 1.0 - std::real(std::log(std::log(tempNewRe + tempNewIm)/2.0)/log2n);
                        else {
                            rendergrid.at(x,y).iterations = 0;
                            rendergrid.at(x,y).mu = 0;
                        }
                        //std::cout << "Values at(" << re << ":" << im << "): It = " << rendergrid.at(x,y).iterations << ", mu = " << rendergrid.at(x,y).mu << std::endl;
                    }
                }
                rendergrid.lastTimeFilled = omp_get_wtime()-tt;
            }
    };
    
    /*
        Fills the Rendergrid with a given fractalfunction
    */
    class DoublePrecisionPerturbated : Solver {
        public:
            int radius = 1; // plus and minus on x,y axis
            double escapeRadius, escapeRadiusSquared, log2n;
            int errorIterations;
            
            DoublePrecisionPerturbated() {
                escapeRadius = 2.0;
                escapeRadiusSquared = 4.0;
                errorIterations = 5;
                log2n = std::log(2.0);
            }
            /* 
                Calculate at point P(x,y) with arbitrary precision, then approximate neighbouring values
            */
            void fillRendergrid(RenderGrid &rendergrid, Parameters p) {
                
                /* For parallising allocating should be inside the loop! */
                std::vector<std::complex<double>> P, A, B, C;
                P.reserve(p.maxIterations); A.reserve(p.maxIterations); B.reserve(p.maxIterations); C.reserve(p.maxIterations);
                int numx = p.numx; int numy = p.numy; double tt = omp_get_wtime();
                //#pragma omp parallel for collapse(2) schedule(dynamic)
                for (int x = 0; x < numx; x += 1+2*radius) {
                    for (int y = 0; y < numy; y += 1+2*radius) {
                        A[0] = 1; B[0] = 0; C[0] = 0;
                        
                        /* Coordinates */
                        double re = p.x0 + (p.x1 - p.x0)*(1.0*x)/p.numx;
                        double im = p.y0 + (p.y1 - p.y0)*(1.0*y)/p.numy;
                        std::complex<double> z0(re,im);
                        std::complex<double> z = 0;
                        /* Cardioid Check */
              
                        /* Calculate Iterations + Error Iterations */
                        int it;
                        for (it = 1; it <= p.maxIterations; it++){
                            z = z*z + z0;
                            if (real(z)*real(z) + imag(z)*imag(z) > escapeRadiusSquared) 
                                break;
                        }
                        for (int i = 0; i <= errorIterations; i++){
                            z = z*z + z0;
                            it++;
                        }
                        
                        auto Anp1 = (2.0*z*A.back() + 1.0);
                        auto Bnp1 = (2.0*z*B.back() + A.back()*A.back());
                        auto Cnp1 = (2.0*z*C.back() + 2.0*A.back()*B.back());
                        P.push_back(z);
                        A.push_back(Anp1);
                        B.push_back(Bnp1);
                        C.push_back(Cnp1);
                        
                        rendergrid.at(x,y).iterations = it;

                        if (rendergrid.at(x,y).iterations < p.maxIterations)
                            rendergrid.at(x,y).mu = 1.0 - std::real(std::log(std::log(std::real(z)*std::real(z) + std::imag(z)*std::imag(z))/2.0)/log2n);
                        else {
                            rendergrid.at(x,y).iterations = 0;
                            rendergrid.at(x,y).mu = 0;
                        }
                        
                    }
                }
                rendergrid.lastTimeFilled = omp_get_wtime()-tt;
            }
    };
    
}

#endif