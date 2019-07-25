#ifndef __DB_INTERACTION__
#define __DB_INTERACTION__

#include <cmath>
#include <complex>

class Interaction {
    public:
        Interaction() {
            
        }
        
        int selectionCenterX = -1;
        int selectionCenterY = -1;
        int selectionDeltaX = -1;
        int selectionDeltaY = -1;
        bool doSelection = false;
        bool doKeyframeAnimation = false;
        
        void Selection(FractalWindow &window, Fractalsolver::Parameters &op) {
            if (window.leftMouseDown()) {
                if (!doSelection) {
                    selectionCenterX = window.MouseX();
                    selectionCenterY = window.MouseY();
                    doSelection = true;
                } else {
                    selectionDeltaX = std::abs(selectionCenterX-window.MouseX());
                    selectionDeltaY = (int)(1.0*window.height/window.width*selectionDeltaX);
                    window.drawRect(selectionCenterX-selectionDeltaX, selectionCenterX+selectionDeltaX, selectionCenterY-selectionDeltaY, selectionCenterY+selectionDeltaY, COLOR_RED);
                }
            } else {
                doSelection = false;
            }
            if (!doSelection and selectionCenterX != -1) {
                /* temp */
                /* double currentScreenWidth = op.x1-op.x0;
                double currentScreenHeight = op.y1-op.y0;
                double newWantedPercentX0 = 1.0*(selectionCenterX-selectionDeltaX)/window.width;
                double newWantedPercentX1 = 1.0*(selectionCenterX+selectionDeltaX)/window.width;
                double newWantedPercentY0 = 1.0*(selectionCenterY-selectionDeltaY)/window.height;
                double newWantedPercentY1 = 1.0*(selectionCenterY+selectionDeltaY)/window.height;
                double oldscreenCoordX0 = op.x0;
                double oldscreenCoordY0 = op.y0;
                double x0 = oldscreenCoordX0 + newWantedPercentX0*currentScreenWidth; 
                double x1 = oldscreenCoordX0 + newWantedPercentX1*currentScreenWidth;
                double y0 = oldscreenCoordY0 + newWantedPercentY0*currentScreenHeight; 
                double y1 = oldscreenCoordY0 + newWantedPercentY1*currentScreenHeight; */
                /* end */
                //window.print(selectionCenterX,selectionCenterY,"X: [" + std::to_string(x0) + ":" + std::to_string(x1) + "] Y: [" + std::to_string(y0) + ":" + std::to_string(y1) + "]" );
                window.drawRect(selectionCenterX-selectionDeltaX, selectionCenterX+selectionDeltaX, selectionCenterY-selectionDeltaY, selectionCenterY+selectionDeltaY);
            }
            if (window.keyPressed(KEY_SPACE) and selectionCenterX != -1) {
                double currentScreenWidth = op.x1-op.x0;
                double currentScreenHeight = op.y1-op.y0;
                double newWantedPercentX0 = 1.0*(selectionCenterX-selectionDeltaX)/window.width;
                double newWantedPercentX1 = 1.0*(selectionCenterX+selectionDeltaX)/window.width;
                double newWantedPercentY0 = 1.0*(selectionCenterY-selectionDeltaY)/window.height;
                double newWantedPercentY1 = 1.0*(selectionCenterY+selectionDeltaY)/window.height;
                double oldscreenCoordX0 = op.x0;
                double oldscreenCoordY0 = op.y0;
                double x0 = oldscreenCoordX0 + newWantedPercentX0*currentScreenWidth; 
                double x1 = oldscreenCoordX0 + newWantedPercentX1*currentScreenWidth;
                double y0 = oldscreenCoordY0 + newWantedPercentY0*currentScreenHeight; 
                double y1 = oldscreenCoordY0 + newWantedPercentY1*currentScreenHeight;
                op = Fractalsolver::Parameters(x0,x1,y0,y1,op.numx,op.numy,op.maxIterations);
                op.changed = true;
                std::cout << "window: x0 = " << x0 << ", x1 = " << x1 << ", y0 = " << y0 << ", y1 = " << y1 << std::endl;
                selectionCenterX = -1;
            }
        }  
        
        double lerp(double v0, double v1, double t) {
            return (1.0 - t)*v0 + t*v1;
        }
        double lorg(double v0, double v1, double t) {
            return std::pow(v0,(1.0 - t))*std::pow(v1,t);
        }
        
        std::vector<Fractalsolver::Parameters> params;
        int animator_currentParams = 0;
        int framesPerOOM = 120;            
        
        /* TODO: Für Arbitrary precision muss das hier auch Arb. prec. sein */
        void keyFrameAnimation(FractalWindow &window, Fractalsolver::Parameters &p) {
            if (window.keyPressed(KEY_k) and !doKeyframeAnimation) {
                window.printFade( (int)(window.width/2)-50, 5, "Saving current frame in keyframe-array..." );
                /* First Element */
                if (params.size() == 0) {
                    params.push_back(p);
                } 
                /* Add Interpolated */
                /* Should move this to a seperate function to enable single interpolations */
                else {
                    Fractalsolver::Parameters last = params.back();
                    double factor = std::log10(p.zoom/last.zoom);
                    double max = (1.0*framesPerOOM)*factor;
                    for (double i = 1; i < max; i++) {
                        double zoom = lorg(last.zoom,p.zoom,i/max);
                        double centerX = p.centerX;//lerp(last.centerX,p.centerX,i/max);
                        double centerY = p.centerY;//lerp(last.centerY,p.centerY,i/max);
                        int it    = (int)lerp(last.maxIterations, p.maxIterations, i/max);
                        Fractalsolver::Parameters cp = Fractalsolver::Parameters(centerX,centerY,zoom,p.numx,p.numy,it);
                        params.push_back(cp);
                    }
                    params.push_back(p);
                }
                std::cout << "Length of params vector = " << params.size() << std::endl;
                for (int i = 0; i < params.size(); i++)
                    std::cout << "i = " << i << " -> x0 = " << params[i].x0 << ", x1 = " << params[i].x1 << ", y0 = " << params[i].y0 << ", y1 = " << params[i].y1 << ", centerX = " << params[i].centerX << ", centerY = " << params[i].centerY << ", zoom = " << params[i].zoom << std::endl;
            }
            if (window.keyPressed(KEY_a)) {
                doKeyframeAnimation = !doKeyframeAnimation;
                animator_currentParams = 0;
            }
            if (doKeyframeAnimation) {
                p = params[animator_currentParams];
                p.changed = true;
                p.saveToFile = true;
                std::string nu = "0000";
                p.frameName = "img-" + nu.substr(0,nu.length()-std::to_string(animator_currentParams).length()) + std::to_string(animator_currentParams);
                animator_currentParams++;
                if (animator_currentParams > params.size()) {
                    std::cout << "Keyframe animation ended" << std::endl;
                    animator_currentParams = 0;
                    p.changed = false;
                    doKeyframeAnimation = !doKeyframeAnimation;
                }
            }
        }
};

#endif