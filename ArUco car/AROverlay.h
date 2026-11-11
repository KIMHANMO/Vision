#pragma once
#include "MarkerInfo.h"
#include <opencv2/core.hpp>

// 마커 위치에 맞춰 이미지를 원근 변환하여 합성
class AROverlay {
public:
    // scale: 마커 대비 이미지 크기 배율 (1.0 = 마커와 같은 크기, 1.5 = 1.5배로 확대)
    explicit AROverlay(float scale = 1.0f, float alpha = 1.0f);

    void draw(cv::Mat& frame, const cv::Mat& image, const MarkerInfo& marker) const;
    void setScale(float s) { scale_ = s; }
    void setAlpha(float a) { alpha_ = a; }
    float scale() const { return scale_; }

private:
    std::vector<cv::Point2f> scaledQuad(const MarkerInfo& marker) const;
    float scale_;
    float alpha_;   // 1.0 = 불투명, 0.0 = 투명
};
