#include "depth_map_methods.h"
#include <time.h>
#include <cmath>
#include <exception>
using namespace std;

//image stack constructor
image_stack::image_stack(int height, 
						int width,
						int size,
						int threshold,
						char* output_img_dir):
						height(height),
						width(width),
						size(size),
						threshold(threshold),
						output_img_dir(output_img_dir),
						SML(height, width, 9),
						depth_map(height, width){}
    
//This function is used to add an image to the stack
void image_stack::add(char* image_path)
{
    jbutil::image<float> image_in;
        
    cout << image_path << endl;
	
    // load image
    ifstream file_in(image_path);
    image_in.load(file_in);
	
	// TODO probably going need to clone image_in as it will most certainly be 
	// referenced in the push_back function, maybe use dynamic memory mangement
	// when creating the image above
    focus_map_stack.push_back(SML(image_in));
}

/* Function for determining the focus maximum of a pixel
 * and thus estimating the depth in the scene at that
 * pixel
 */
inline float image_stack::coarse_depth_esstimation(int y, int x)
{
    int max_focus_depth = 0;
    float max_focus = 0;
    
    for(int z = 0; z < focus_map_stack.size(); z++)
    {
        if( focus_map_stack[z](0,y,x) > max_focus)
        {
            max_focus_depth = z;
            max_focus = focus_map_stack[z](0,y,x);
        }
    }
    
    if(max_focus > threshold){
		return max_focus_depth;
	}
	else
	{
		return size - 1;
	}
		
}

//Function for generating a depth map
void image_stack::create_depth_map()
{
    for(int y = 0; y < height; y++)
    {        
        for(int x = 0; x < width; x++)
        {
            depth_map( 0, y, x) = coarse_depth_esstimation(y, x);
        }
    }
    
    //Clear out the focus map stack as the data is no longer needed
    focus_map_stack.clear();
    
    //Save the image to file
    ofstream file_out( (output_img_dir + "depth_map.jpg").c_str());
    depth_map.save(file_out);
}

// Sum Modified Laplacian focus measure
jbutil::image<float> sum_modified_laplacian::operator()(jbutil::image<float>& img){
     
    cout << "Run SML algorithm " << endl;
	
    for(int y = step; y < height - step; y++)
    {   
        for(int x = step; x < width - step; x++)
        {
            ML(0, y, x)
            =
            abs( 2 * img(0, y, x) - img(0, y, x - step) - img(0, y, x + step) )
            +
            abs( 2 * img(0, y, x) - img(0, y - step, x) - img(0, y + step, x) );
        }
    }
	
	//TODO implement a box filter function
    SML = boxFilter(ML, step);
    cout << "Run Box filter " << endl;
    
    return SML;
}

/* This is a box filter function that will box filter
 * an image using the seperable filter method of adding
 * the image first in the y direction and then in the x
 * direction.
 * The filtering process can be accelerated by keeping a 
 * total of the average in each direction and just subtract 
 * the last value in the filter and add the new value.
 */
jbutil::image<float> boxFilter(const jbutil::image<float>& img, int step){
	
	jbutil::image<float> y_filtered(img.get_cols(), img.get_rows()), 
				 result(img.get_cols(), img.get_rows());
	
	//Loop over every pixel and sum in y direction
	for(int y = step; y < img.get_rows() - step; y++){
		for(int x = step; x < img.get_cols() -  step; x++){
			//Apply the vertical box filter here
			//y_filtered(0, y, x) = 0; //zero the image before adding
			for(int y_offset = - step; y_offset <= step*2; y_offset++){
				y_filtered(0, y, x) += img(0,y + y_offset,x);
			}
		}
	}
	
	//Loop over every pixel and sum in x direction
	for(int y = step; y < img.get_rows() - step; y++){
		for(int x = step; x < img.get_cols() -  step; x++){
			//Apply the horizontal box filter here
			//result(0, y, x) = 0; //zero the image before adding
			for(int x_offset = - step; x_offset <= step*2; x_offset++){
				result(0, y, x) = y_filtered(0,y,x + x_offset);
			}
		}
	}
	
	return result;
}
