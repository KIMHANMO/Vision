#include "AROverlay.h"
#include <opencv2/imgproc.hpp>
#if CV_VERSION_MAJOR >= 5
#include <opencv2/geometry.hpp>
#endif
#include <algorithm>

AROverlay::AROverlay(float scale, float alpha) : scale_(scale), alpha_(alpha) {}

// 마커 중심 기준으로 네 모서리를 scale 배 확장
std::vector<cv::Point2f> AROverlay::scaledQuad(const MarkerInfo& marker) const {
    cv::Point2f c = marker.center();
    std::vector<cv::Point2f> q;
    for (const auto& p : marker.corners()) q.push_back(c + (p - c) * scale_);
    return q;
}

void AROverlay::draw(cv::Mat& frame, const cv::Mat& image, const MarkerInfo& marker) const {
    if (frame.empty() || image.empty() || marker.corners().size() != 4) return;

    const auto dst = scaledQuad(marker);
    const std::vector<cv::Point2f> src = {
        { 0.f, 0.f }, { (float)image.cols, 0.f },
        { (float)image.cols, (float)image.rows }, { 0.f, (float)image.rows } };

    // 프레임 전체가 아닌, 마커가 차지하는 영역(ROI)만 변환하여 속도 향상
    cv::Rect roi = cv::boundingRect(dst) & cv::Rect(0, 0, frame.cols, frame.rows);
    if (roi.area() <= 0) return;

    std::vector<cv::Point2f> dstLocal;
    for (const auto& p : dst) dstLocal.push_back(p - cv::Point2f((float)roi.x, (float)roi.y));

    cv::Mat H = cv::getPerspectiveTransform(src, dstLocal);
    cv::Mat warped, mask;
    cv::warpPerspective(image, warped, H, roi.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);
    cv::warpPerspective(cv::Mat(image.size(), CV_8UC1, cv::Scalar(255)), mask, H, roi.size(),
                        cv::INTER_NEAREST, cv::BORDER_CONSTANT);

    cv::Mat target = frame(roi);
    if (alpha_ >= 0.999f) {
        warped.copyTo(target, mask);
    } else {
        cv::Mat blended;
        cv::addWeighted(warped, alpha_, target, 1.0 - alpha_, 0.0, blended);
        blended.copyTo(target, mask);
    }
}
