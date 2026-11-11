#pragma once
#include <opencv2/core.hpp>
#include <vector>

// 인식된 마커 1개의 정보 (ID + 네 모서리 좌표)
class MarkerInfo {
public:
    MarkerInfo() = default;
    MarkerInfo(int id, const std::vector<cv::Point2f>& corners);

    int id() const { return id_; }
    // 모서리 순서: 0=좌상, 1=우상, 2=우하, 3=좌하 (ArUco 기본 순서)
    const std::vector<cv::Point2f>& corners() const { return corners_; }
    cv::Point2f center() const;
    float sideLength() const;           // 화면상 평균 한 변 길이(px)
    cv::Rect boundingRect() const;

    int lostFrames = 0;                 // 0 = 이번 프레임에 검출됨, >0 = 직전 위치를 유지 중

private:
    int id_ = -1;
    std::vector<cv::Point2f> corners_;
};
