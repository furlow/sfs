#include "depth_map_methods.h"
#include <time.h>
#include <cmath>
#include <exception>
#include <vector>
#include "opencv2/legacy/legacy.hpp"

using namespace std;

//image stack constructor
image_stack::image_stack(char* img_dir,
                        int threshold,
                        int size,
                        int scaled_width,
                        int scaled_height):
                        threshold(threshold),
                        size(size),
                        img_dir(img_dir),
                        scaled_width(scaled_width),
                        scaled_height(scaled_height){

                            cout << "Threshold: " << threshold << endl;
                        }

//For loading already computed depth map and fused image
void image_stack::load(int in_size)
{
    size = in_size;
    depth_map = imread(img_dir + "output/" + "depth_map.png", CV_LOAD_IMAGE_GRAYSCALE);
    cv::resize(depth_map, depth_map_scaled, Size(scaled_width, scaled_height));

    focused = imread(img_dir + "output/" + "fused_focus.png");
    cv::cvtColor(focused, focused, CV_BGR2RGB);
    cv::resize(focused, focused_scaled, Size(scaled_width, scaled_height));

    cluster();
    generate_blurred_images();
}

//Function used to map pointers from scaled numpy image data for use in the
//C++ code
void image_stack::allocate(char* numpy_depth_map,
                            char* numpy_focused,
                            char* numpy_refocused,
                            int in_scaled_width,
                            int in_scaled_height)
{
    scaled_height = in_scaled_height;
    scaled_width = in_scaled_width;
    depth_map_scaled = Mat( scaled_height, scaled_width, CV_8U, numpy_depth_map);
    focused_scaled = Mat( scaled_height, scaled_width, CV_8UC3, numpy_focused);
    refocused = Mat( scaled_height, scaled_width, CV_8UC3, numpy_refocused);
}

//This function is used to add an image to the stack
void image_stack::add(char* image_path)
{

    Mat colour_image, img_32f, gray_image;
    colour_image = imread(image_path);

    if(raw_stack.size() == 0)
    {
        height = colour_image.rows;
        width = colour_image.cols;
        SML = new sum_modified_laplacian(colour_image.rows, colour_image.cols, 21);
        depth_map = Mat(colour_image.rows, colour_image.cols, CV_8U);
        focused = Mat(colour_image.rows, colour_image.cols, CV_8UC3);
    }

    raw_stack.push_back(colour_image.clone());
    cvtColor(colour_image, gray_image, CV_BGR2GRAY);
    gray_image.convertTo(img_32f, CV_32F);
    focus_map_stack.push_back((*SML)(img_32f).clone());
}

//Function for determining the focus maximum of a pixel
inline char image_stack::coarse_depth_esstimation(int y, int x)
{
    int max_focus_depth = 0;
    float max_focus = 0;

    for(int z = 0; z < size; z++)
    {

        float focus = focus_map_stack[z].at<float>(y,x);

        if( focus > max_focus)
        {
            max_focus_depth = z;
            max_focus = focus;
        }
    }

    if(max_focus > threshold)
    {
		return max_focus_depth;
	}
	else
	{
		return size - 1;
	}

}

//Function for determining the focus maximum of a pixel
/*inline float image_stack::gaussian_depth_esstimation(int y, int x)
{
    int dm = 0;
    float max_focus = 0;
    float max_focus_minus = 0;
    float max_focus_plus = 0;

    for(int z = 2; z < size; z++)
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

    float depth = depth_mean(max_focus, max_focus_plus, max_focus_minus, dm, dmp, dmm);

    if(max_focus > threshold && depth < size && depth >= 0){
    	//cout << depth << endl;
    	return depth;
	}
	else
	{
		return size - 1;
	}
}*/

/*inline float image_stack::depth_mean
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
}*/

//Function for generating a depth map
void image_stack::create_depth_map()
{
    for(int y = 0; y < height; y++)
    {

        char* y_ptr = depth_map.ptr<char>(y);

        for(int x = 0; x < width; x++)
        {
            y_ptr[x] = coarse_depth_esstimation(y, x);
        }
    }

    //Clear out the focus map stack as the data is no longer needed
    focus_map_stack.clear();

    cout << "Saving depth map to file" << endl;
    imwrite( img_dir + "output/" + "depth_map.png", depth_map);
    cluster();
    cv::resize(depth_map, depth_map_scaled, Size(scaled_width, scaled_height));
}

void image_stack::fuse_focus()
{

    Vector<Vec3b*> raw_stack_y_ptr;

    cout << "Running fuse focus" << endl;

    for(int y = 0; y < height; y++)
    {

        Vec3b* focused_y_ptr = focused.ptr<Vec3b>(y);
        char* depth_map_y_ptr = depth_map.ptr<char>(y);

        for(int i = 0; i < size; i++){
        	raw_stack_y_ptr.push_back( raw_stack[i].ptr<Vec3b>(y) );
        }

        //Add pointer array of each image row in the raw_stack

        for(int x = 0; x < width; x++)
        {
            focused_y_ptr[x] = raw_stack_y_ptr[ depth_map_y_ptr[x] ][x];
        }

        raw_stack_y_ptr.clear();
    }

    //Clear out the raw_stack as the data is no longer needed
    raw_stack.clear();

    imwrite( img_dir + "output/" + "fused_focus.png", focused);
    cv::cvtColor(focused, focused, CV_BGR2RGB);
    cv::resize(focused, focused_scaled, Size(scaled_width, scaled_height));
    generate_blurred_images();
}

//Function which blurs the infocus image from 0 blur to the max blur required
void image_stack::generate_blurred_images()
{

	clock_t init, final;
    init=clock();

    for(int z = 0; z < size; z++){
    	GaussianBlur(focused_scaled, dst, Size(z * 2 + 1, z * 2 + 1), 0, 0);
    	//boxFilter(focused, dst, -1, Size(z * 6 + 1, z * 6 + 1));
    	blurred.push_back(dst.clone());
    }

	final = clock() - init;
    cout << "blured images " << (double)final / ((double)CLOCKS_PER_SEC) << endl;

}

//Function used to artificially refocus an image
void image_stack::refocus(int in_depth_of_field, int in_focus_depth)
{

    depth_of_feild = in_depth_of_field;
    focus_depth = in_focus_depth;

    Mat defocus_map = abs(depth_map_scaled - focus_depth);
    defocus_map = defocus_map - depth_of_feild;
    cv::threshold( defocus_map, defocus_map, 0.0, 0.0, THRESH_TOZERO );

    Vec3b* blurred_stack_y_ptr[size];

    //For loop which selects the correct pixels based on the depth map value
    for(int y = 0; y < scaled_height; y++)
    {

        Vec3b* refocused_y_ptr = refocused.ptr<Vec3b>(y);
        char* defocus_map_y_ptr = defocus_map.ptr<char>(y);


        for(int i = 0; i < size; i++)
        {
        	blurred_stack_y_ptr[i] = blurred[i].ptr<Vec3b>(y);
        }

        //Add pointer array of each image row in the raw_stack

        for(int x = 0; x < scaled_width; x++)
        {
            refocused_y_ptr[x] = blurred_stack_y_ptr[defocus_map_y_ptr[x]][x];
        }
    }
}

//This function is most likely obsolete
inline void image_stack::boxfilter_single_pixel(int y, int x, int ksize)
{

	int ksize_sq = pow(ksize*2 + 1, 2);
    int col_temp, row_temp;
	int b_sum = 0, g_sum = 0, r_sum = 0;
    Vec3b* focused_row_ptr = NULL;

	for(int row = y - ksize;
        row <= y + ksize;
        row++
        )
	{
        if(row < focused.rows)
        {
            row_temp = abs(row);
        }
        else
        {
            row_temp = rows_double - row - 1;
        }

        focused_row_ptr = focused_scaled.ptr<Vec3b>(row_temp);

		for(int col = x - ksize;
            col <= x + ksize;
            col++
            )
		{
            if(col < focused.cols)
            {
                col_temp = abs(col);
            }
            else
            {
                col_temp = cols_double - col - 1;
            }
            b_sum += focused_row_ptr[col_temp].val[0];
            g_sum += focused_row_ptr[col_temp].val[1];
            r_sum += focused_row_ptr[col_temp].val[2];
		}
	}

	Vec3b* pixel = refocused.ptr<Vec3b>(y, x);

	pixel->val[0] = b_sum / ksize_sq;
	pixel->val[1] = g_sum / ksize_sq;
	pixel->val[2] = r_sum / ksize_sq;
}

/* Sum Modified Laplacian focus measure
 */
Mat sum_modified_laplacian::operator()(Mat& image)
{

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
        }
    }


    boxFilter(ML, SML, -1, Size(2*step+1,2*step+1), Point(-1,-1), false);
    return SML;
}

void image_stack::resize(int in_scaled_width, int in_scaled_height)
{
    cv::resize(focused, focused_scaled, Size(scaled_width, scaled_height));
    cv::resize(depth_map, depth_map_scaled, Size(scaled_width, scaled_height));
    generate_blurred_images();
    refocus(depth_of_feild, focus_depth);
    cv::resize(refocused, refocused, Size(scaled_width, scaled_height));
}

void image_stack::cluster()
{

    cout << "Apply the bilateralFilter" << endl;
    int morph_size = 5;
    dst = depth_map * (255/size - 1);
    cv::resize(dst, dst, Size(scaled_width, scaled_height));
    namedWindow("depth map");
    imshow("depth map", dst);

    Mat element = getStructuringElement( MORPH_ELLIPSE, Size( 2*morph_size + 1, 2*morph_size+1 ) );
    morphologyEx(depth_map, depth_map, MORPH_CLOSE, element);
    dst = depth_map * (255/size - 1);
    cv::resize(dst, dst, Size(scaled_width, scaled_height));
    namedWindow("depth map bilateral");
    imshow("depth map bilateral", dst);
}
