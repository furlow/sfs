#include "depth_map_methods.h"
#include <time.h>

//image stack constructor
image_stack::image_stack(int height, int width, int size):height(height), width(width), size(size)
{
    depth_map = Mat(height, width, CV_32F);
    
    //stack.resize(size);
    
    cout << "The size is " << size << endl;
    cout << "Currently allocated stack vector capacity = " << stack.capacity() << endl;
}
    
//This function is used to add an image to the stack
void image_stack::add(char* image_path)
{
    
    clock_t init, final;
    
    init=clock();
    
    Mat img_32f(height, width, CV_32F);
    
    final=clock()-init;
    cout << "Create Mat " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    init=clock();

    Mat img = imread(image_path, 0);
    
    final=clock()-init;
    cout << "Read image " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    init=clock();

    img.convertTo(img_32f, CV_32F);
    
    final=clock()-init;
    cout << "Convert image to float " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    init=clock();

    sum_modified_laplacian SML(9);
    
    final=clock()-init;
    cout << "Create Sum modfied laplacian " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    init=clock();
    
    img_32f = SML(img_32f);
    
    final=clock()-init;
    cout << "Run sum modified laplacian " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    init=clock();
    
    stack.push_back(img_32f);
    
    final=clock()-init;
    cout << "Push into the stack " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    cout << "Currently allocated stack vector capacity = " << stack.capacity() << endl;
}

//Function for determining the focus maximum of a pixel
inline float image_stack::coarse_depth_esstimation(int y, int x)
{
    int max_focus_depth = 0;
    float max_focus = 0;
    
    for(int z = 0; z < stack.size(); z++)
    {
        float* img_ptr = (float*)(stack[z].data) + y*stack[z].cols + x;
        
        if( *img_ptr > max_focus)
        {
            max_focus_depth = z;
            max_focus = *img_ptr;
        }
    }
    return max_focus_depth;
}

//Function for generating a depth map from a focus stack processed with a focus measure
void image_stack::create_depth_map()
{
    
    cout << "Stack size " << stack.size() << endl;
    
    for(int y = 0; y < height; y++)
    {
        float* y_ptr = depth_map.ptr<float>(y);
        
        for(int x = 0; x < width; x++)
        {
            y_ptr[x] = coarse_depth_esstimation(y,x);
        }
    }
    
    //Mat img;
    //convertScaleAbs(depth_map,img);
    //namedWindow( "Depth Map", WINDOW_OPENGL );
    //imshow("Depth Map", img);
    
    //waitKey(0);
}

/* Sum Modified Laplacian focus measure
 */
Mat sum_modified_laplacian::operator()(Mat& image){
    
    Mat ML(image.rows, image.cols, CV_32F);
    Mat SML(image.rows, image.cols, CV_32F);
    
    float* img_ptr = (float*)(image.data);
    float* ML_ptr = (float*)(ML.data);
    
    clock_t init, final;
    
    init=clock();

    for(int y = step; y < image.rows - step; y++)
    {
        for(int x = step; x < image.cols - step; x++)
        {
            ML_ptr[y*image.cols + x]
            =
            abs( 2 * img_ptr[y*image.cols + x] - img_ptr[y*image.cols + x - step] - img_ptr[y*image.cols + x + step])
            +
            abs( 2 * img_ptr[y*image.cols + x] - img_ptr[(y - step)*image.cols + x] - img_ptr[(y + step)*image.cols + x]);
        }
    }
    
    final=clock()-init;
    cout << "Run SML algorithm " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    init=clock();
    
    boxFilter(ML, SML, -1, Size(2*step+1,2*step+1), Point(-1,-1), false);
    
    final=clock()-init;
    cout << "Run Box filter " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    return SML;
}