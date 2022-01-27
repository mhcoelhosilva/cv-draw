#include <iostream>
#include <map>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

#include "../handy/Handy/BackgroundRemover.h"
#include "../handy/Handy/SkinDetector.h"
#include "../handy/Handy/FaceDetector.h"
#include "../handy/Handy/FingerCount.h"

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
    const string about =
        "This sample demonstrates Lucas-Kanade Optical Flow calculation.\n"
        "The example file can be downloaded from:\n"
        "  https://www.bogotobogo.com/python/OpenCV_Python/images/mean_shift_tracking/slow_traffic_small.mp4";
    const string keys =
        "{ h help |      | print this help message }"
        "{ @image | vtest.avi | path to image file }"
        "{ @camera | 0 | Camera device number.}";
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    
    int camera_device = parser.get<int>("@camera");
    VideoCapture capture;
    capture.open( camera_device );
    if (!capture.isOpened())
    {
        cout << "--(!)Error opening video capture\n";
        return 0;
    }
    
    BackgroundRemover backgroundRemover;
    SkinDetector skinDetector;
    FaceDetector faceDetector;
    FingerCount fingerCount;
    
    int trackLen = 50;
    int detectInterval = 5;
    vector<vector<Point2f>> tracks;
    int frameID = 0;
    int maxTrack = 500;
    
    // Create some random colors
    vector<Scalar> colors;
    RNG rng;
    for(int i = 0; i < maxTrack; i++)
    {
        int r = rng.uniform(0, 256);
        int g = rng.uniform(0, 256);
        int b = rng.uniform(0, 256);
        colors.push_back(Scalar(r,g,b));
    }
    vector<Point2f> p0, p1, p0r;
    vector<Point> fingerPoints;
    
    Mat frameOut, handMask, foreground, fingerCountDebug;
    
    Mat old_frame, old_gray;
    capture >> old_frame;
    cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
    
    Mat vis = Mat::zeros(old_frame.size(), old_frame.type());
    Mat sub = Mat::zeros(old_frame.size(), old_frame.type());
    sub.setTo(Scalar(12, 12, 12));
    
    while(true){
        Mat frame, frame_gray;
        capture >> frame;
        if (frame.empty())
            break;
        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
        frameOut = frame.clone();
        
        // re-calibrate periodically
        if (frameID % (detectInterval * 5) == 0)
        {
            backgroundRemover.calibrate(frame);
            skinDetector.calibrate(frame);
        }
        
        skinDetector.drawSkinColorSampler(frameOut);
        foreground = backgroundRemover.getForeground(frame);
        faceDetector.removeFaces(frame, foreground);
        handMask = skinDetector.getSkinMask(foreground);
        
        // fade out old paths
        vis -= sub;
        
        if (tracks.size() > maxTrack)
        {
            int diff = tracks.size() - maxTrack;
            tracks.erase(tracks.begin(), tracks.begin() + diff);
        }
        
        if (tracks.size() > 0)
        {
            // calculate optical flow
            vector<uchar> status;
            vector<float> err;
            TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), 10, 0.03);
            
            p0.clear();
            for (size_t i = 0; i < tracks.size(); ++i)
                p0.push_back(tracks[i].back());
            
            calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err, Size(15,15), 2, criteria);
            calcOpticalFlowPyrLK(frame_gray, old_gray, p1, p0r, status, err, Size(15,15), 2, criteria);
            
            std::vector<bool> good;
            for (size_t i = 0; i < p0.size(); ++i)
            {
                Point2f d = p0[i] - p0r[i];
                float maxD = std::max(d.x, d.y);
                good.push_back(maxD < 1.0f);
            }
            
            vector<vector<Point2f>> newTracks;
            size_t minSize = std::min(tracks.size(), p1.size());
            for(uint i = 0; i < minSize; i++)
            {
                // Select good points
                if(status[i] == 1 && good[i] == true)
                {
                    vector<Point2f> tr = tracks[i];
                    tr.push_back(p1[i]);
                    if (tr.size() > trackLen)
                        tr.erase(tr.begin());
                    newTracks.push_back(tr);
                    //circle(vis, p1[i], 5, colors[i], -1);
                }
            }
            tracks = newTracks;
            
            // draw the tracks
            for (size_t i = 0; i < tracks.size(); ++i)
                for (size_t j = 1; j < tracks[i].size(); ++j)
                    line(vis, tracks[i][j], tracks[i][j-1], colors[i], 2);
        }
        
        if (frameID % detectInterval == 0)
        {
            // don't look for corners where we already have them
            /*Mat mask = Mat::zeros(frame_gray.size(), frame_gray.type());
            mask.setTo(Scalar(255));
            for (size_t i = 0; i < tracks.size(); ++i)
                circle(mask, tracks[i].back(), 10, 0, -1);
            for (size_t i = 0; i < fingerPoints.size(); ++i)
                circle(mask, fingerPoints[i], 5, 0, -1);*/
            
            // trying to not check fingers where we already have them
            for (size_t i = 0; i < tracks.size(); ++i)
                circle(handMask, tracks[i].back(), 10, Scalar(0, 0, 0), -1);
            for (size_t i = 0; i < fingerPoints.size(); ++i)
                circle(handMask, fingerPoints[i], 5, Scalar(0, 0, 0), -1);
            
            fingerCountDebug = fingerCount.findFingersCount(handMask, frameOut, fingerPoints);
            
            for(uint i = 0; i < fingerPoints.size(); i++)
            {
                tracks.push_back( {fingerPoints[i]} );
            }
        }
        
        ++frameID;
            
        Mat img, flippedImg;
        add(frame, vis, img);
        flip(img, flippedImg, 1);
        imshow("Frame", flippedImg);
        
        /*imshow("output", frameOut);
        imshow("foreground", foreground);
        imshow("handMask", handMask);
        imshow("handDetection", fingerCountDebug);*/
        
        int key = waitKey(1);

        if (key == 27) // esc
            break;
        else if (key == 98) // b
            backgroundRemover.calibrate(frame);
        else if (key == 115) // s
            skinDetector.calibrate(frame);
        else if (key == 116) // t
            tracks.clear();
        
        // Now update the previous frame and previous points
        old_gray = frame_gray.clone();
    }
}
