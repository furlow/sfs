#include "depth_map_methods.h"
#include <time.h>
#include <cmath>
#include <exception>
#include <vector>

//image stack constructor
image_stack::image_stack(int height, int width, int size):
height(height), width(width), size(size), SML(height, width, 9)
{
    depth_map = Mat(height, width, CV_32F);
    focused = Mat(height, width, CV_8UC3);
    refocused = Mat(height, width, CV_8UC3);
    namedWindow( "Depth Map", 0);
}
    
//This function is used to add an image to the stack
void image_stack::add(char* image_path)
{
    
    Mat img2, colour_image, img_32f, gray_image;
    
    cout << image_path << endl;
    
    clock_t init, final;
    
    init=clock();

    colour_image = imread(image_path);
    
    final=clock()-init;
    cout << "Read image " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    raw_stack.push_back(colour_image.clone());
        
    init=clock();
    
    cvtColor(colour_image, gray_image, CV_BGR2GRAY);
    gray_image.convertTo(img_32f, CV_32F);
    
    final=clock()-init;
    cout << "Convert image to float " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    init=clock();
    
    focus_map_stack.push_back(SML(img_32f).clone());
    
    final=clock()-init;
    cout << "Run SML and push into the stack " << (double)final / ((double)CLOCKS_PER_SEC) 
    << endl;
    
    cout << "Currently allocated stack vector capacity = " << raw_stack.capacity() << endl;
    
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
    
    for(int z = 0; z < focus_map_stack.size(); z++)
    {

        float* focus = focus_map_stack[z].ptr<float>(y,x);

        if( *focus > max_focus)
        {
            max_focus_depth = z;
            max_focus = *focus;
        }
    }
    
    return max_focus_depth;
}

//Function for determining the focus maximum of a pixel
inline float image_stack::guassian_depth_esstimation(int y, int x)
{
    int dm = 0;
    float max_focus = 0;
    float max_focus_minus = 0;
    float max_focus_plus = 0;
    
    for(int z = 2; z < focus_map_stack.size(); z++)
    {
        //find peak
        if( *(focus_map_stack[z - 1].ptr<float>(y,x)) > max_focus //&&
            //*(stack[z - 1].ptr<float>(y,x)) > *(stack[z].ptr<float>(y,x)) //&& 
             //stack[z - 1].at<float>(y,x) > stack[z - 2].at<float>(y,x)
          )
        {
            max_focus = *(focus_map_stack[z - 1].ptr<float>(y,x));
            dm = z - 1;
        }
    }
    
    int dmm = dm - 1;
    int dmp = dm + 1;

    max_focus_minus = *(focus_map_stack[dmm].ptr<float>(y,x));
    max_focus_plus = *(focus_map_stack[dmp].ptr<float>(y,x));
    
    return depth_mean(max_focus, max_focus_plus, max_focus_minus, dm, dmp, dmm);
}


inline float image_stack::depth_mean
(float Fm, float Fmp, float Fmm, int dm, int dmp, int dmm)
{
    float log_Fm = log(Fm);
    float log_Fmm = log(Fmm);
    float log_Fmp = log(Fmp);
    float log_fm_minus_fmp = (log_Fm - log_Fmp);
    float log_fm_minus_fmm = (log_Fm - log_Fmm);
    float dm_sq = pow(dm, 2);

    float d_mean = (
                  log_fm_minus_fmp*(dm_sq - pow(dmm, 2))
                  -
                  log_fm_minus_fmm*(dm_sq - pow(dmp, 2))
                 )
                 /
                 ( 2 * (log_fm_minus_fmm + log_fm_minus_fmp) );
    
    return d_mean;
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
        convertScaleAbs(stack[z],dst, 0.1);
        resize(dst, dst, Size(), 0.1, 0.1);
        imshow("Depth Map", dst);
        waitKey(0);
    }
    */

    for(int y = 0; y < height; y++)
    {
        
        float* y_ptr = depth_map.ptr<float>(y);
        
        for(int x = 0; x < width; x++)
        {
            y_ptr[x] = coarse_depth_esstimation(y, x);
        }
    }
    
    //Clear out the focus map stack as the data is no longer needed
    focus_map_stack.clear();
    
    final = clock() - init;
    cout << "Generate depth map " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    
    depth_map.convertTo(dst, CV_8U, 255 / (raw_stack.size() - 1));
    resize(dst, dst, Size(), 0.2, 0.2);
    namedWindow( "Depth Map", WINDOW_AUTOSIZE );
    imshow("Depth Map", dst);
    
    waitKey(0);
    
    cout << "Saving depth map to file" << endl;
    imwrite( "depth_map.jpg", dst );
    
    fuse_focus();
    
    int focus_depth;
    
    cout << "Input the focus depth:";
    
    cin >> focus_depth;
    
    refocus(0, focus_depth);
}

void image_stack::fuse_focus(){
	
    clock_t init, final;
    init=clock();
    
    Vector<Vec3b*> raw_stack_y_ptr;
    
    cout << "Running fuse focus" << endl;

    for(int y = 0; y < height; y++)
    {
        
        Vec3b* focused_y_ptr = focused.ptr<Vec3b>(y);
        float* depth_map_y_ptr = depth_map.ptr<float>(y);
        for(int i = 0; i < raw_stack.size(); i++){
        	raw_stack_y_ptr.push_back( raw_stack[i].ptr<Vec3b>(y) );
        }
        
        //Add pointer array of each image row in the raw_stack
        
        for(int x = 0; x < width; x++)
        {
            focused_y_ptr[x] = raw_stack_y_ptr[ int(depth_map_y_ptr[x]) ][x];

        }
        
        raw_stack_y_ptr.clear();
    }
    
    //Clear out the raw_stack as the data is no longer needed
    raw_stack.clear();
    
    final = clock() - init;
    cout << "Generate fuse focused image " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    resize(focused, dst, Size(), 0.2, 0.2);
    namedWindow( "Depth Map", WINDOW_AUTOSIZE );
    imshow("Depth Map", dst);
    
    waitKey(0);
    
    cout << "Saving fused image to file" << endl;
    imwrite( "fused_focus.jpg", focused );
    
}

//function used to artificially refocus an image
void image_stack::refocus(int depth_of_feild, int depth_focus_point){

	int ksize;
	
	clock_t init, final;
    init=clock();

	for(int y = 0; y < height; y++)
	{
        for(int x = 0; x < width; x++)
        {
        	ksize = ( abs( *(depth_map.ptr<float>(y, x)) - depth_focus_point ) * 4 ) + 1;
        	boxfilter_single_pixel(y, x, ksize);
        }
    }
    
	final = clock() - init;
    cout << "refocused image " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
    resize(refocused, dst, Size(), 0.2, 0.2);
    namedWindow( "Depth Map", WINDOW_AUTOSIZE );
    imshow("Depth Map", dst);
    
    waitKey(0);
    
    cout << "Saving fused image to file" << endl;
    imwrite( "refocused.jpg", refocused );
	
}

inline void image_stack::boxfilter_single_pixel(int y, int x, int ksize){

	int ksize_half = (ksize / 2);
	int ksize_sq = ksize * ksize;
	//cout << "ksize_half " << ksize_half << endl;
	int b_sum = 0, g_sum = 0, r_sum = 0;
	int lower_lim_col, lower_lim_row;
	
	if(y - ksize_half < 0){ 
		lower_lim_row = 0;	
	}
	else
	{
		lower_lim_row = ksize_half;
	}
	
	if(x - ksize_half < 0){
		lower_lim_col = 0;
	}
	else
	{
		lower_lim_col =  ksize_half;
	}
		
	for(int row = y - lower_lim_row; row <= (y + ksize_half) && row < focused.rows; row++)
	{
		Vec3b* row_ptr = focused.ptr<Vec3b>(row);
		for(int col = x - lower_lim_col; col <= (x + ksize_half) && col < focused.cols; col++)
		{
			b_sum += row_ptr[col][0];
			g_sum += row_ptr[col][1];
			r_sum += row_ptr[col][2];
		}
	}
	
	Vec3b* pixel = refocused.ptr<Vec3b>(y);

	pixel[x][0] = (b_sum / ksize_sq);
	pixel[x][1] = (g_sum / ksize_sq);
	pixel[x][2] = (r_sum / ksize_sq);
}

/* Sum Modified Laplacian focus measure
 */
Mat sum_modified_laplacian::operator()(Mat& image){
    
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