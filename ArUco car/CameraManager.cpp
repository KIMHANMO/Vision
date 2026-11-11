#include "CameraManager.h"
#include <iostream>

CameraManager::~CameraManager() { release(); }

bool CameraManager::open(int index, int width, int height) {
#ifdef _WIN32
    // Windows에서는 DirectShow 백엔드가 카메라 열기/해상도 설정이 빠르고 안정적
    cap_.open(index, cv::CAP_DSHOW);
    if (!cap_.isOpened()) cap_.open(index);
#else
    cap_.open(index);
#endif
    if (!cap_.isOpened()) {
        std::cerr << "[CameraManager] Cannot open camera " << index << "\n";
        return false;
    }
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    std::cout << "[CameraManager] Camera " << index << " opened: "
              << frameSize().width << "x" << frameSize().height << "\n";
    return true;
}

bool CameraManager::read(cv::Mat& frame) {
    if (!cap_.isOpened()) return false;
    return cap_.read(frame) && !frame.empty();
}

void CameraManager::release() {
    if (cap_.isOpened()) cap_.release();
}

cv::Size CameraManager::frameSize() const {
    return { static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_WIDTH)),
             static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_HEIGHT)) };
}
