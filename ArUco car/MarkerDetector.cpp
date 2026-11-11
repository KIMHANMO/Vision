#include "MarkerDetector.h"
#include <opencv2/imgproc.hpp>

MarkerDetector::MarkerDetector(int dictionaryId, int holdFrames)
    : holdFrames_(holdFrames) {
    cv::aruco::DetectorParameters params;
    params.cornerRefinementMethod = cv::aruco::CORNER_REFINE_SUBPIX;  // 모서리 떨림 감소
    detector_ = cv::aruco::ArucoDetector(cv::aruco::getPredefinedDictionary(dictionaryId), params);
}

std::vector<MarkerInfo> MarkerDetector::detect(const cv::Mat& frame) {
    lastIds_.clear();
    lastCorners_.clear();
    if (frame.empty()) return {};

    detector_.detectMarkers(frame, lastCorners_, lastIds_);

    // 이번 프레임 검출 결과 반영
    for (auto& [id, info] : tracked_) info.lostFrames++;
    for (size_t i = 0; i < lastIds_.size(); ++i)
        tracked_[lastIds_[i]] = MarkerInfo(lastIds_[i], lastCorners_[i]);

    // 오래 안 보인 마커 제거, 나머지 반환
    std::vector<MarkerInfo> result;
    for (auto it = tracked_.begin(); it != tracked_.end();) {
        if (it->second.lostFrames > holdFrames_) {
            it = tracked_.erase(it);
        } else {
            result.push_back(it->second);
            ++it;
        }
    }
    return result;
}

void MarkerDetector::drawDebug(cv::Mat& frame) const {
    if (!lastIds_.empty())
        cv::aruco::drawDetectedMarkers(frame, lastCorners_, lastIds_);
}
