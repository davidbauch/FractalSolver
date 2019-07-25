#ifndef __DB_RENDERGRID__
#define __DB_RENDERGRID__

/* 
    This class provides a vector to be rendered 
    Includes: Iterations (int), Smoothing value (mu)
*/

/* vector class */
#include <vector>

/* Gridclass */
class RenderGrid {
    public:
        /* Subclass for storing number of iterations and smoothing value (mu%1) */
        class GridValue {
            public:
                int iterations;
                double mu;
                /* Constructor */
                GridValue(int it = 0, double mu = 0.0): iterations(it), mu(mu) {}
        };
        
        int numRows, numCols;
        std::vector<GridValue> values;
        
        double lastTimeFilled;
        
        /* Constructors */
        RenderGrid(int r = 0, int c = 0): numRows(r), numCols(c) {
            values.reserve(r*c);
            for (int i = 0; i < r*c; i++)
                values[i] = GridValue();
            lastTimeFilled = 0;
        }
        
        /* Get Value in Grid at (i,j) */
        GridValue &at(int x, int y) {
            //if (x >= 0) and (x < numCols) and (y >= 0) and (y < numRows) {
                return values[y*numRows + x];
            //}
            return values[0];
        }
};



#endif