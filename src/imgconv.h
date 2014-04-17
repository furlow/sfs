#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

namespace imgconv{
    //Function to convert a Mat to copy/convert to a numpy array
    void mat2numpy(char* numpy_img, Mat& mat_img);
}
