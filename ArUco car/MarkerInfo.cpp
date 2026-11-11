#include "MarkerInfo.h"
#include <opencv2/imgproc.hpp>
#if CV_VERSION_MAJOR >= 5
#include <opencv2/geometry.hpp>
#endif
#include <cmath>

MarkerInfo::MarkerInfo(int id, const std::vector<cv::Point2f>& corners)
    : id_(id), corners_(corners) {}

cv::Point2f MarkerInfo::center() const {
    cv::Point2f c(0.f, 0.f);
    for (const auto& p : corners_) c += p;
    return corners_.empty() ? c : c * (1.0f / corners_.size());
}

float MarkerInfo::sideLength() const {
    if (corners_.size() != 4) return 0.f;
    float sum = 0.f;
    for (int i = 0; i < 4; ++i) {
        cv::Point2f d = corners_[(i + 1) % 4] - corners_[i];
        sum += std::sqrt(d.x * d.x + d.y * d.y);
    }
    return sum / 4.f;
}

cv::Rect MarkerInfo::boundingRect() const {
    return cv::boundingRect(corners_);
}
