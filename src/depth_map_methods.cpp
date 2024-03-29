#include "depth_map_methods.h"
#include <time.h>
#include <unistd.h>
#include <cmath>
#include <exception>
#include <vector>
#include "opencv2/legacy/legacy.hpp"

using namespace std;

//image stack constructor
image_stack::image_stack(char* img_dir,
                        int threshold,
                        int size,
                        int quantization,
                        int scaled_width,
                        int scaled_height):
                        threshold(threshold),
                        quantization(quantization),
                        size(size),
                        defocus(1),
                        img_dir(img_dir),
                        scaled_width(scaled_width),
                        scaled_height(scaled_height),
                        depth_of_field(0),
                        focus_depth(0){

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
    cout << "C++ Reallocate images" << endl;
    scaled_height = in_scaled_height;
    scaled_width = in_scaled_width;
    depth_map_scaled = Mat( scaled_height, scaled_width, CV_8U, numpy_depth_map);
    focused_scaled = Mat( scaled_height, scaled_width, CV_8UC3, numpy_focused);
    refocused = Mat( scaled_height, scaled_width, CV_8UC3, numpy_refocused);
}

void image_stack::resize()
{
    cout << "C++ Resize images" << endl;
    cv::resize(focused, focused_scaled, Size(scaled_width, scaled_height));
    cv::resize(depth_map, depth_map_scaled, Size(scaled_width, scaled_height));
    generate_blurred_images();
    cout << "Depth of field: " << depth_of_field << ", focus depth: " << focus_depth << endl;
    refocus(depth_of_field, focus_depth);
    //cv::resize(refocused, refocused, Size(scaled_width, scaled_height));
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
inline float image_stack::gaussian_depth_esstimation(int y, int x)
{
    int dm = 1;
    float max_focus = 0;
    float max_focus_minus = 0;
    float max_focus_plus = 0;

    for(int z = 2; z < size; z++)
    {
        //find peak
        if( focus_map_stack[z - 1].at<float>(y, x) > max_focus )
        {
            max_focus = focus_map_stack[z - 1].at<float>(y, x);
            dm = z - 1;
        }
    }

    int dmm = dm - 1;
    int dmp = dm + 1;

    max_focus_minus = focus_map_stack[dmm].at<float>(y,x);
    max_focus_plus = focus_map_stack[dmp].at<float>(y,x);

    float depth = depth_mean(max_focus, max_focus_plus, max_focus_minus, dm, dmp, dmm);

    if(max_focus > threshold && depth < size - 1){
    	//cout << depth << endl;
        if(depth < 0)
        {
            return 0;
        }
        else
        {
            return depth;
        }
	}
    else
    {
        return size - 1;
    }
}

inline float image_stack::depth_mean(float Fm,
                                     float Fmp,
                                     float Fmm,
                                     int dm,
                                     int dmp,
                                     int dmm)
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

//Function for generating a depth map
void image_stack::create_depth_map()
{
    Mat depth_map_coarse(height, width, CV_32F);
    Mat depth_map_gaussian(height, width, CV_32F);

    for(int row = 0; row < height; row++)
    {
        float* row_ptr_coarse = depth_map_coarse.ptr<float>(row);
        float* row_ptr_gauss = depth_map_gaussian.ptr<float>(row);

        for(int col = 0; col < width; col++)
        {
            row_ptr_coarse[col] = coarse_depth_esstimation(row, col);
            row_ptr_gauss[col] = gaussian_depth_esstimation(row, col);
        }
    }

    //Clear out the focus map stack as the data is no longer needed
    focus_map_stack.clear();

    cout << "Saving depth map to file" << endl;
    //Scale and save the gaussian the depth map
    depth_map_gaussian = (depth_map_gaussian * 255) / (size - 1);
    depth_map_gaussian.convertTo(depth_map_gaussian, CV_8U);
    imwrite( img_dir + "output/" + "gaussian_depth_map.png", depth_map_gaussian);

    //Save a raw version of the coarse depth map
    depth_map_coarse.convertTo(depth_map, CV_8U);
    imwrite( img_dir + "output/" + "depth_map.png", depth_map);

    //Scale and save the coarse the depth map
    depth_map_coarse = (depth_map_coarse * 255) / (size - 1);
    depth_map_coarse.convertTo(depth_map_coarse, CV_8U);
    imwrite( img_dir + "output/" + "coarse_depth_map.png", depth_map_coarse);

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

void image_stack::setDefocus(int in_defocus){
    defocus = in_defocus;
    generate_blurred_images();
}

//Function which blurs the infocus image from 0 blur to the max blur required
void image_stack::generate_blurred_images()
{
	clock_t init, final;
    init=clock();
    blurred.clear();

    cerr << "defocus: " << defocus << endl;

    for(int z = 0; z < size; z++){
    	GaussianBlur(focused_scaled, dst, Size(z * defocus * 2 + 1, z * defocus* 2 + 1), 0, 0);
    	//boxFilter(focused, dst, -1, Size(z * 6 + 1, z * 6 + 1));
    	blurred.push_back(dst.clone());
    }

	final = clock() - init;
    cout << "blured images " << (double)final / ((double)CLOCKS_PER_SEC) << endl;
}

//Generates a defocus map used to refocus an image
Mat image_stack::generate_defocus_map(int in_focus_depth, int in_depth_of_field){
    Mat defocus_map = abs(depth_map_scaled - in_focus_depth);
    defocus_map = defocus_map - in_depth_of_field;
    cv::threshold( defocus_map, defocus_map, 0.0, 0.0, THRESH_TOZERO );
    return defocus_map;
}

//Function used to artificially refocus an image
void image_stack::refocus(int in_depth_of_field, int in_focus_depth)
{
    depth_of_field = in_depth_of_field;
    focus_depth = in_focus_depth;


    Mat defocus_map = generate_defocus_map(focus_depth, depth_of_field);
    cout << "error1" << endl;
    refocus(defocus_map);
    cout << "error2" << endl;
}

//Function used to artificially refocus an image
void image_stack::refocus(Mat& defocus_map)
{
    cout << "Begin refocus" << endl;
    cout << "scaled width: " << scaled_width << endl;
    cout << "scaled height: " << scaled_height << endl;

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

    cout << "End refocus" << endl;
}

// This function focuses the image at different points providing unrealistic
// multiple focus points that would not be possible with real cameras
void image_stack::refocus_multiple(int focus_depth_1, int focus_depth_2)
{
    Mat defocus_1 = generate_defocus_map(focus_depth_1, 0);
    Mat defocus_2 = generate_defocus_map(focus_depth_2, 0);
    Mat multi_focus(scaled_height, scaled_width, CV_8U);

    //Select the smallest pixels from each array
    for(int row = 0; row < scaled_height; row++)
    {

        char* defocus_1_row_ptr = defocus_1.ptr<char>(row);
        char* defocus_2_row_ptr = defocus_2.ptr<char>(row);
        char* multi_focus_row_ptr = multi_focus.ptr<char>(row);

        for(int col = 0; col < scaled_width; col++)
        {
            if(defocus_1_row_ptr[col] < defocus_2_row_ptr[col])
            {
                multi_focus_row_ptr[col] = defocus_1_row_ptr[col];
            }
            else
            {
                multi_focus_row_ptr[col] = defocus_2_row_ptr[col];
            }
        }

    }

    refocus(multi_focus);
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
