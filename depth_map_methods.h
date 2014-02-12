//
//  depth_map_methods.h
//  
//
//  Created by Josh on 10/02/2014.
//
//
#ifndef ____depth_map_methods__
#define ____depth_map_methods__

#include <iostream>

#endif /* defined(____depth_map_methods__) */

//
//  depth_map_methods.cpp
//
//
//  Created by Josh on 10/02/2014.
//
//

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <list>

using namespace std;
using namespace cv;

//This is the image stack class
class image_stack{
public:
    //Constructor
    image_stack(int height, int width);
    
    //This function is used to add an image to the stack
    void add(char* image_path);
    
    //Function for determining the focus maximum of a pixel
    inline float coarse_depth_esstimation(int x, int y);
    
    //Function for generating a depth map from a focus stack processed with a focus measure
    void create_depth_map();
    
private:
    int height;
    int width;
    int stretch_factor;
    vector<Mat> stack;
    Mat depth_map;
};

/* How to define a focus measure class ??
 The class needs to be overidable and customizable for implementing
 the different meausres
 Use a derived class which inherits from the base class focus measure
 focus measure will have a few virtual methods which can then be overidden
 */
class focus_measure{
public:
    virtual Mat operator()(Mat& image) = 0;
};

/* Sum Modified Laplacian focus measure
 */
class sum_modified_laplacian: public focus_measure{
public:
    sum_modified_laplacian(int step):step(step){}
    Mat operator()(Mat& image);
private:
    int step;
};