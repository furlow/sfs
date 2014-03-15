#include <iostream>
#include <list>
#include <string>
#include <vector>
#include "jbutil.h"

using namespace std;
     
/* The focus_measure class is pure virutal and it allows for any valid focus
 * measures to be implemented and used without out changing the underlying code
 * base
 * To define a focus measure create a derived class which inherits from the base
 * class focus_measure with public inheritance and overide the operator()
 * function with the focus measure you intend to implement
 */
class focus_measure{
public:
    virtual jbutil::image<float> operator()(jbutil::image<float>& img) = 0;
};

/* Sum Modified Laplacian focus measure
 */
class sum_modified_laplacian: public focus_measure{
public:
    sum_modified_laplacian
    (int height, int width, int step):step(step), height(height), width(width), 
	ML(height, width), SML(height, width){
    }
    jbutil::image<float> operator()(jbutil::image<float>& img);
private:
    int step;
    int height;
    int width;
    jbutil::image<float> ML; //need to remove these when un needed
    jbutil::image<float> SML;
};


/* boxFilter function for use in the Sum Modified Laplacian
 * focus measure filter
 */
jbutil::image<float> boxFilter(const jbutil::image<float>& img, int step);


/* This is the image stack class it is used to contain an image stack
 * for prcoessing into a depth map
 */
class image_stack{
public:
    //Constructor
    image_stack(int height, int width, int size, int threshold ,char* output_img_dir);
    
    //This function is used to add an image to the stack
    void add(char* image_path);
    
    //Function for generating a depth map from a focus stack processed with a focus measure
    void create_depth_map();
    
    //Function for determining the focus maximum of a pixel
    //using coarse depth esstimation method
    inline float coarse_depth_esstimation(int y, int x);
private:
    int height;
    int width;
    int size;
    float threshold;
    string output_img_dir;
    vector< jbutil::image<float> > focus_map_stack;
    jbutil::image<float> depth_map;
    sum_modified_laplacian SML;
};
