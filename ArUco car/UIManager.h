#pragma once
#include "ContentManager.h"
#include "MarkerInfo.h"
#include <opencv2/core.hpp>
#include <string>
#include <vector>

enum class UIAction { None, Quit, Capture, ToggleInfo, ToggleSpecs, ToggleDebug, ScaleUp, ScaleDown };

// 화면 정보 표시 + 키보드 입력 처리
class UIManager {
public:
    explicit UIManager(const std::string& windowName = "AR Car Cards");
    ~UIManager();

    // 인식된 카드 목록/수, 카드별 라벨, 제원 패널, 도움말 표시
    void drawOverlayInfo(cv::Mat& frame,
                         const std::vector<std::pair<MarkerInfo, const CardContent*>>& cards,
                         double fps) const;
    void showMessage(const std::string& msg, int frames = 45);
    void show(const cv::Mat& frame);
    UIAction pollKey(int delayMs = 1) const;

    bool showInfo = true;     // 'i' : 카드 이름/ID 라벨
    bool showSpecs = true;    // 's' : 우측 제원 패널
    bool showDebug = false;   // 'd' : 마커 테두리(디버그)

private:
    void drawLabel(cv::Mat& frame, const MarkerInfo& m, const CardContent& c) const;
    void drawSpecPanel(cv::Mat& frame,
                       const std::vector<std::pair<MarkerInfo, const CardContent*>>& cards) const;
    std::string window_;
    std::string message_;
    mutable int messageFrames_ = 0;
    bool windowCreated_ = false;
};
