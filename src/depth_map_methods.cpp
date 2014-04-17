#include "depth_map_methods.h"
#include <time.h>
#include <cmath>
#include <exception>
#include <vector>
//#include <QPixmap>

using namespace std;

//image stack constructor
image_stack::image_stack(int height, int width, int size, int threshold, char* output_img_dir):
height(height), width(width), size(size), threshold(threshold) , output_img_dir(output_img_dir), SML(height, width, 9)
{
    depth_map = Mat(height, width, CV_32F);
    focused = Mat(height, width, CV_8UC3);
    refocused = Mat(height, width, CV_8UC3);
}

//This function is used to add an image to the stack
void image_stack::add(char* image_path)
{

    Mat colour_image, img_32f, gray_image;

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

    float depth = depth_mean(max_focus, max_focus_plus, max_focus_minus, dm, dmp, dmm);

    if(max_focus > threshold && depth < size && depth >= 0){
    	//cout << depth << endl;
    	return depth;
	}
	else
	{
		return size - 1;
	}
}


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
            y_ptr[x] = gaussian_depth_esstimation(y, x);
        }
    }

    //Clear out the focus map stack as the data is no longer needed
    focus_map_stack.clear();

    //Display the output image

    dst = depth_map * (255.0 / (size - 1));

    dst.convertTo(dst, CV_8U);

    //Save the image to file
    cout << "Saving depth map to file" << endl;
    imwrite( output_img_dir + "depth_map.jpg", dst);

    resize(dst, dst, Size(), 0.3, 0.3);
    namedWindow( "Depth Map", WINDOW_AUTOSIZE );
    imshow("Depth Map", dst);


}

void image_stack::fuse_focus(char* out_img){

    Vector<Vec3b*> raw_stack_y_ptr;

    cout << "Running fuse focus" << endl;

    for(int y = 0; y < height; y++)
    {

        Vec3b* focused_y_ptr = focused.ptr<Vec3b>(y);
        float* depth_map_y_ptr = depth_map.ptr<float>(y);

        for(int i = 0; i < size; i++){
        	raw_stack_y_ptr.push_back( raw_stack[i].ptr<Vec3b>(y) );
        }

        //Add pointer array of each image row in the raw_stack

        for(int x = 0; x < width; x++)
        {
        	//cout << int(depth_map_y_ptr[x]) << endl;
            focused_y_ptr[x] = raw_stack_y_ptr[ int(depth_map_y_ptr[x]) ][x];

        }

        raw_stack_y_ptr.clear();
    }

    //Clear out the raw_stack as the data is no longer needed
    raw_stack.clear();

    //Display the fused focus image
    resize(focused, dst, Size(), 0.2, 0.2);
    namedWindow( "Fused Focus", WINDOW_AUTOSIZE );
    imshow("Fused Focus", dst);

    //Save the fused focus image to file
    cout << "Saving fused image to file" << endl;
    imwrite( output_img_dir + "fused_focus.jpg", focused );

    mat2numpy(out_img, focused);

}

void image_stack::generate_blurred_images(){

	clock_t init, final;
    init=clock();

    //for loop which blurs the infocus image from 0 now blur to the max blur required
    for(int z = 0; z < size; z++){
    	GaussianBlur(focused, dst, Size(z * 6 + 1, z * 6 + 1), 0, 0);
    	//boxFilter(focused, dst, -1, Size(z * 6 + 1, z * 6 + 1));
    	blurred.push_back(dst.clone());
    }

	final = clock() - init;
    cout << "blur images" << (double)final / ((double)CLOCKS_PER_SEC) << endl;

}

//Function used to artificially refocus an image
void image_stack::refocus(int depth_of_field, int depth_focus_point, char* out_img){

	clock_t init, final;
    init=clock();

	if(blurred.size() != size) generate_blurred_images();

    Vec3b* blurred_stack_y_ptr[size];

    //for loop which selects the correct pixels based on the depth map value
    for(int y = 0; y < height; y++)
    {

        Vec3b* refocused_y_ptr = refocused.ptr<Vec3b>(y);
        float* depth_map_y_ptr = depth_map.ptr<float>(y);

        for(int i = 0; i < size; i++){
        	blurred_stack_y_ptr[i] = blurred[i].ptr<Vec3b>(y);
        }

        //Add pointer array of each image row in the raw_stack

        for(int x = 0; x < width; x++)
        {
            refocused_y_ptr[x] = blurred_stack_y_ptr[ abs(int(depth_map_y_ptr[x]) - depth_focus_point) ][x];

        }
    }

    final = clock() - init;
    cout << "Refocus image" << (double)final / ((double)CLOCKS_PER_SEC) << endl;

    init=clock();

    //Display the refocused image
    resize(refocused, dst, Size(), 0.3, 0.3);
    imshow("Fused Focus", dst);

    final = clock() - init;
    cout << "Displaying image" << (double)final / ((double)CLOCKS_PER_SEC) << endl;

    //Save the refocused image to file
    //cout << "Saving fused image to file" << endl;
    //imwrite( output_img_dir + "refocused.jpg", refocused );
    mat2numpy(out_img, refocused);
}

//This function is most likely obsolete
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

void mat2numpy(char* numpy_img, Mat& mat_img){

    for(int y = 0; y < mat_img.rows; y++){

        char* numpy_row_ptr = numpy_img + ( y * mat_img.cols * 3 );
        Vec3b* mat_row_ptr = mat_img.ptr<Vec3b>(y);

        for(int x = 0; x < mat_img.cols; x++){

            for(int c = 0; c < 3; c++){

                numpy_row_ptr[x + c] = mat_row_ptr[x].val[c];

            }
        }
    }
}
