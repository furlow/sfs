#include "depth_map_methods.h"

//image stack constructor
image_stack::image_stack(int height, int width)
{
    depth_map = Mat(height, width, CV_32F);
}
    
//This function is used to add an image to the stack
void image_stack::add(char* image_path)
{
    
    Mat img(height, width, CV_32F);
    img = imread(image_path, 0);
    sum_modified_laplacian SML(9);
    img = SML(img);
    stack.push_back(img);
}

//Function for determining the focus maximum of a pixel
inline float image_stack::coarse_depth_esstimation(int y, int x)
{
    int max_focus_depth = 0;
    float max_focus = 0;
    
    for(int depth = 0; depth < stack.size(); depth++)
    {
        if(stack[depth].at<float>(y,x) > max_focus)
        {
            max_focus_depth = depth;
            max_focus = stack[depth].at<float>(y,x);
        }
    }
    
    return max_focus_depth;
}

//Function for generating a depth map from a focus stack processed with a focus measure
void image_stack::create_depth_map()
{
    
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            depth_map.at<float>(y,x) = coarse_depth_esstimation(y,x);
        }
    }
    
    stretch_factor = 255 / stack.size() - 1;
    
}

/* Sum Modified Laplacian focus measure
 */
Mat sum_modified_laplacian::operator()(Mat& image){
    
    Mat ML(image.rows, image.cols, CV_32F);
    Mat SML(image.rows, image.cols, CV_32F);
    
    for(int x = step; x < image.cols - step; x++)
    {
        for(int y = step; y < image.rows - step; y++)
        {
            ML.at<float>(y, x) = abs( 2 * image.at<float>(y, x) - image.at<float>(y, x - step) - image.at<float>(y, x + step))
                                +
                                 abs( 2 * image.at<float>(y, x) - image.at<float>(y - step , x) - image.at<float>(y + step, x));
        }
    }
    boxFilter(ML, SML, -1, Size(2*step+1,2*step+1), Point(-1,-1), false);
    return SML;
}