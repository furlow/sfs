#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <list>
#include <string>

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
class sum_modified_laplacian: public focus_measure
{
  public:
    sum_modified_laplacian
    (int height, int width, int step):step(step), height(height), width(width)
    {

        ML = Mat(height, width, CV_32F);
        SML = Mat(height, width, CV_32F);
    }

    Mat operator()(Mat& image);
  private:
    int step;
    int height;
    int width;
    Mat ML; //need to remove these when un needed
    Mat SML;
};

//This is the image stack class
class image_stack{
public:
    //Constructor
    image_stack(char* img_dir,
                int threshold,
                int size,
                int quantization,
                int scaled_width,
                int scaled_height);

    //Function used to map pointers from scaled numpy image data for use in the
    //C++ code
    void allocate( char* numpy_depth_map,
                                char* numpy_focused,
                                char* numpyrefocused,
                                int in_scaled_width,
                                int in_scaled_height);

    //Function for loading already computed depth map and fused image
    void load(int in_size);

    //This function is used to add an image to the stack
    void add(char* image_path);

    //Function for generating a depth map from a focus stack processed with a focus measure
    void create_depth_map();

    //Function for determining the focus maximum of a pixel
    //using coarse depth esstimation method
    inline char coarse_depth_esstimation(int y, int x);

    //Function for determing the focus maximum of a pixel
    //using guassian interpolation
    inline float gaussian_depth_esstimation(int y, int x);

    //Mean depth interpolation calculations
    inline float depth_mean
    (float Fm, float Fmp, float Fmm, int dm, int dmp, int dmm);

    //Function for generating an all in focus image
    void fuse_focus();

    //Function for generating blurred images for use in the refocus algorithm
    void generate_blurred_images();

    //Generates a defocus map used to refocus an image
    Mat generate_defocus_map(int in_focus_depth, int in_depth_of_feild);

    //This function allows the defocus amount to be set
    void setDefocus(int in_defocus);

    //Function for artificially refocusing an image
    void refocus(int in_depth_of_feild, int in_focus_depth);

    //Refocus an image by suppling a defocus map
    void refocus(Mat& defocus_map);

    // This function focuses the image at different points providing unrealistic
    // multiple focus points that would not be possible with real cameras
    void refocus_multiple(int focus_depth_1, int focus_depth_2);

    //Function to set resize parameters for output display size
    void resize(int in_scaled_width, int in_scaled_height);

    //Function used to apply boxfilter blur to a single pixel
    inline void boxfilter_single_pixel(int y, int x, int ksize);

private:
    int height;
    int width;
    int size;
    int quantization;
    int scaled_width;
    int scaled_height;
    int rows_double;
    int cols_double;
    int depth_of_feild;
    int focus_depth;
    int defocus;
    float threshold;
    string img_dir;
    vector<Mat> raw_stack;
    vector<Mat> focus_map_stack;
    vector<Mat> blurred;
    sum_modified_laplacian* SML;
    Mat focused;
    Mat refocused;
    Mat depth_map;
    Mat depth_map_quantized;
    Mat focused_scaled;
    Mat depth_map_scaled;
    Mat dst; //temporary storage of an array
};
