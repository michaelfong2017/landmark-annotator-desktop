#ifndef CAPTUREHISTORY_H
#define CAPTUREHISTORY_H

#include "stdafx.h"
#include <opencv2/opencv.hpp>

struct CaptureHistory {
    int imageType;
    QString imageName;
    cv::Mat capturedColorImage;
    cv::Mat capturedDepthImage;
    cv::Mat capturedColorToDepthImage;
    cv::Mat capturedDepthToColorImage;
    cv::Mat FourChannelPNG;
    cv::Mat RANSACImage;
    QImage qColorImage;
    QImage qDepthImage;
    QImage qColorToDepthImage;
    QImage qDepthToColorImage;
    QImage qDepthToColorColorizedImage;
    QRect clip_rect;
};
#endif
#pragma once
