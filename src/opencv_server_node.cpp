#include <ros/ros.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <image_transport/image_transport.h>
// #include <cv_bridge/cv_bridge.h>
#include <cv_bridge_4_8_0/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/wechat_qrcode.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


std::string save_path;
std::string QR_file_name;
bool use_window = false;

void imageCallback(const sensor_msgs::ImageConstPtr& img){
    static int jump_cnt = 0;
    if(jump_cnt++ < 15)return;
    jump_cnt = 0;
    
    cv_bridge::CvImageConstPtr imgPtr;
    // try
    // {
        // Mat img_cp;
        imgPtr = cv_bridge::toCvShare(img, "bgr8");
        // img_cp = imgPtr->image;

        wechat_qrcode::WeChatQRCode detector{
                "/home/tspi/catkin_make/src/opencv_server/QR_module/detect.prototxt", 
                "/home/tspi/catkin_make/src/opencv_server/QR_module/detect.caffemodel",
                "/home/tspi/catkin_make/src/opencv_server/QR_module/sr.prototxt",     
                "/home/tspi/catkin_make/src/opencv_server/QR_module/sr.caffemodel"
        };
            
        vector<Mat> points;

        auto res = detector.detectAndDecode(imgPtr->image, points); 

        if(res.size() > 0){
                
            // imwrite(save_path + QR_file_name, imgPtr->image);
            imwrite("/home/tspi/Desktop/1.jpg", imgPtr->image);
            ROS_INFO("get QR:%s", res);

            if(use_window)
            {
                for(size_t idx = 0; idx < res.size(); idx ++){
                    Point pt1 = points[idx].at<Point2f>(0);
                    Point pt2 = points[idx].at<Point2f>(2);
                    Rect rect{pt1, pt2};
                    Point center = (pt1 + pt2) / 2;
                    // 绘制矩形框
                    rectangle(imgPtr->image, rect, {0,0,255}, 2);
                    circle(imgPtr->image, center, rect.height / 15, {0,0,255}, -1);
                    // 解码字符串
                    putText(imgPtr->image, res[idx], {pt1.x, pt2.y + 16}, 1, 1, {0,0,255});    
                }
            }
        }
            if(use_window)
	            imshow("view", imgPtr->image);

    // }
    // catch (std::exception& e)
    // {
    //     ROS_ERROR("ERROR!");
    // }
}
 
int main(int argc, char** argv){
    ros::init(argc, argv, "opencv_server");
	ros::NodeHandle nh;

    nh.getParam("/pic_save_path", save_path);
    nh.getParam("/QR_file_name", QR_file_name);
    nh.getParam("/use_window", use_window);

    if(use_window){
        namedWindow("view");
        startWindowThread();
    }

    image_transport::ImageTransport it(nh);
    image_transport::Subscriber sub = it.subscribe("/usb_cam/image_raw", 2000, imageCallback);

	ros::Rate loop_rate(2);
	while(ros::ok())
	{	
		ros::spinOnce();
		loop_rate.sleep();
	}
	
    return 0;
 }