#include "depth_map_methods.h"
#include <time.h>

//image stack constructor
image_stack::image_stack(int height, int width, int size):
height(height), width(width), size(size), SML(height, width, 4)
{
    depth_map = Mat(height, width, CV_32F);
    //img_32f = Mat(height, width, CV_32F);
    //img = Mat(height, width, CV_8U);

    //stack.resize(size);
    
    cout << "The size is " << size << endl;
    cout << "Currently allocated stack vector capacity = " << stack.capacity() << endl;
}
    
//This function is used to add an image to the stack
void image_stack::add(char* image_path)
{
    
    Mat dst, img2;
    
    cout << image_path << endl;
    
    clock_t init, final;
    
    init=clock();

    Mat img = imread(image_path, 0);
    
    final=clock()-init;
    cout << "Read image " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    namedWindow( "Depth Map", 0);
    //resize(img, dst, Size(), 0.1, 0.1);
    //imshow("Depth Map", dst);
    //waitKey(0);
    
    init=clock();
    
    Mat img_32f(height, width, CV_32F);

    img.convertTo(img_32f, CV_32F);
    
    final=clock()-init;
    cout << "Convert image to float " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    init=clock();
    
    stack.push_back(SML(img_32f).clone());
    
    final=clock()-init;
    cout << "Run SML and push into the stack " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    cout << "Currently allocated stack vector capacity = " << stack.capacity() << endl;
    
    //convertScaleAbs(img_32f,img2, 0.1);
    //resize(img2, dst, Size(), 0.1, 0.1);
    //imshow("Depth Map", dst);
    
    //waitKey(0);
    
}

//Function for determining the focus maximum of a pixel
inline float image_stack::coarse_depth_esstimation(int y, int x)
{
    int max_focus_depth = 0;
    float max_focus = 0;
    
    for(int z = 0; z < stack.size(); z++)
    {
        if( stack[z].at<float>(y,x) > max_focus)
        {
            max_focus_depth = z;
            max_focus = stack[z].at<float>(y,x);
        }
    }
    
    if(max_focus > 1800){
        return stack.size() - max_focus_depth;
    } else{
        return 0;
    }
}

//Function for generating a depth map from a focus stack processed with a focus measure
void image_stack::create_depth_map()
{
    clock_t init, final;

    init=clock();

    /*    
    for(int z = 0; z < stack.size(); z++)
    {
        cout << "image " << z << endl;
        convertScaleAbs(stack[z],img2, 0.1);
        resize(img2, dst, Size(), 0.1, 0.1);
        imshow("Depth Map", dst);
        waitKey(0);
    }
    */  

    for(int y = 0; y < height; y++)
    {
        
        float* y_ptr = depth_map.ptr<float>(y);
        
        for(int x = 0; x < width; x++)
        {
            y_ptr[x] = coarse_depth_esstimation(y,x);
        }
    }
    
    final = clock() - init;
    cout << "Generate depth map " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    Mat dst;
    convertScaleAbs(depth_map,dst, 255 / stack.size());
    resize(dst, dst, Size(), 0.2, 0.2);
    namedWindow( "Depth Map", WINDOW_AUTOSIZE );
    imshow("Depth Map", dst);
    
    waitKey(0);
}

/* Sum Modified Laplacian focus measure
 */
Mat sum_modified_laplacian::operator()(Mat& image){
    
    Mat img, dst;

    clock_t init, final;
    
    init=clock();
        
    for(int y = step; y < height - step; y++)
    {
        float* ML_ptr = ML.ptr<float>(y);
        float* img_ptr = image.ptr<float>(y);
        float* img_ptr_step_minus = image.ptr<float>(y - step);
        float* img_ptr_step_plus = image.ptr<float>(y + step);
        
        for(int x = step; x < width - step; x++)
        {
            ML_ptr[x]
            =
            abs( 2 * img_ptr[x] - img_ptr[x - step] - img_ptr[x + step])
            +
            abs( 2 * img_ptr[x] - img_ptr_step_minus[x] - img_ptr_step_plus[x]);
            
            //cout << ML_ptr[x] << endl;
            
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