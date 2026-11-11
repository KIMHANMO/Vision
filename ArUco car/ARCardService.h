#pragma once
#include "AROverlay.h"
#include "CameraManager.h"
#include "ContentManager.h"
#include "MarkerDetector.h"
#include "UIManager.h"
#include <string>

struct AppConfig {
    int cameraIndex = 0;
    int width = 1280;
    int height = 720;
    std::string contentDir = "content";
    std::string captureDir = "captures";
    float overlayScale = 1.0f;
};

// 전체 흐름 제어: 영상 입력 → 마커 인식 → 콘텐츠 검색 → AR 합성 → 결과 출력
class ARCardService {
public:
    explicit ARCardService(const AppConfig& cfg = AppConfig());
    bool initialize();
    void run();

private:
    void processFrame(cv::Mat& frame);
    bool handleAction(UIAction action, const cv::Mat& frame);
    void saveCapture(const cv::Mat& frame);

    AppConfig cfg_;
    CameraManager camera_;
    MarkerDetector detector_;
    ContentManager contents_;
    AROverlay overlay_;
    UIManager ui_;
    double fps_ = 0.0;
};
