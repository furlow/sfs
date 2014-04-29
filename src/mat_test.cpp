#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <list>
#include <string>

using namespace std;
using namespace cv;

int main(void){


    Mat test(1, 1, CV_8UC3);

    cout << test << endl;

    char* test_ptr = test.ptr<char>(0);

    test_ptr[0] = 98;
    test_ptr[1] = 34;
    test_ptr[2] = 45;

    cout << (int)test_ptr[0] << endl;
    cout << (int)test_ptr[1] << endl;
    cout << (int)test_ptr[2] << endl;


    Vec3b* test_vec3b_ptr = test.ptr<Vec3b>(0);

    cout << (int)test_vec3b_ptr->val[0] << endl;
    cout << (int)test_vec3b_ptr->val[1] << endl;
    cout << (int)test_vec3b_ptr->val[2] << endl;




}
