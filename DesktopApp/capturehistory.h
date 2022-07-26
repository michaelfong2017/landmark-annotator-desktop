#ifndef CAPTUREHISTORY_H
#define CAPTUREHISTORY_H

#include "stdafx.h"
#include <opencv2/opencv.hpp>

struct CaptureHistory {
    int imageType;
    cv::Mat capturedColorImage;
    cv::Mat capturedDepthImage;
    cv::Mat capturedColorToDepthImage;
    cv::Mat capturedDepthToColorImage;
    cv::Mat RANSACImage;
    QImage qColorImage;
    QImage qDepthImage;
    QImage qColorToDepthImage;
    QImage qDepthToColorImage;
    QImage qDepthToColorColorizedImage;
};
#endif
#pragma once
