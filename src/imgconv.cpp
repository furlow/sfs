#include "imgconv.h"

using namespace imgconv;

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
