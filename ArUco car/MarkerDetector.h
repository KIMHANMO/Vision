#pragma once
#include "MarkerInfo.h"
#include <opencv2/objdetect/aruco_detector.hpp>
#include <map>
#include <vector>

// 프레임에서 ArUco 마커 검출 (OpenCV 4.7+ API)
class MarkerDetector {
public:
    // holdFrames: 마커가 잠깐 안 보여도 직전 위치를 유지할 프레임 수 (깜빡임 방지)
    explicit MarkerDetector(int dictionaryId = cv::aruco::DICT_4X4_50, int holdFrames = 5);

    std::vector<MarkerInfo> detect(const cv::Mat& frame);
    void drawDebug(cv::Mat& frame) const;   // 검출된 마커 테두리/ID 표시 (디버그용)

private:
    cv::aruco::ArucoDetector detector_;
    int holdFrames_;
    std::map<int, MarkerInfo> tracked_;      // id -> 최근 정보
    std::vector<std::vector<cv::Point2f>> lastCorners_;
    std::vector<int> lastIds_;
};
