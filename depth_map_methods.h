#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <list>

using namespace std;
using namespace cv;

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
    sum_modified_laplacian(int height, int width, int step):step(step), height(height), width(width){
    
        ML = Mat(height, width, CV_32F);
        SML = Mat(height, width, CV_32F);
    
    }
    Mat operator()(Mat& image);
private:
    int step;
    int height;
    int width;
    Mat ML;
    Mat SML;
};

//This is the image stack class
class image_stack{
public:
    //Constructor
    image_stack(int height, int width, int size);
    
    //This function is used to add an image to the stack
    void add(char* image_path);
    
    //Function for determining the focus maximum of a pixel
    //using coarse depth esstimation method
    inline float coarse_depth_esstimation(int y, int x);
    
    //Function for determing the focus maximum of a pixel
    //using guassian interpolation
    inline float guassian_depth_esstimation(int y, int x);
    
    //Mean depth interpolation calculations
    inline float depth_mean(float Fm, float Fmp, float Fmm, int dm, int dmp, int dmm);

    //Function for generating a depth map from a focus stack processed with a focus measure
    void create_depth_map();
    
private:
    int height;
    int width;
    int size;
    vector<Mat> stack;
    Mat depth_map;
    //Mat img_32f;
    //Mat img;
    sum_modified_laplacian SML;
    float* focus;
};
