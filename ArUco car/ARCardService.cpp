#include "ARCardService.h"
#include <opencv2/core/utility.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

ARCardService::ARCardService(const AppConfig& cfg)
    : cfg_(cfg), overlay_(cfg.overlayScale) {}

bool ARCardService::initialize() {
    contents_.load(cfg_.contentDir);   // 이미지가 없어도 대체 이미지로 계속 진행
    if (!camera_.open(cfg_.cameraIndex, cfg_.width, cfg_.height)) return false;
    return true;
}

void ARCardService::processFrame(cv::Mat& frame) {
    // 1) 마커 인식
    std::vector<MarkerInfo> markers = detector_.detect(frame);

    // 2) 콘텐츠 검색 (등록되지 않은 ID는 무시)
    std::vector<std::pair<MarkerInfo, const CardContent*>> cards;
    for (const auto& m : markers)
        if (const CardContent* c = contents_.find(m.id())) cards.emplace_back(m, c);

    // ID 순으로 정렬해서 패널 순서가 흔들리지 않게
    std::sort(cards.begin(), cards.end(),
              [](const auto& a, const auto& b) { return a.second->id < b.second->id; });

    // 3) AR 합성 (멀리 있는 = 작은 마커부터 그려서 가까운 카드가 위에 오도록)
    auto drawOrder = cards;
    std::sort(drawOrder.begin(), drawOrder.end(),
              [](const auto& a, const auto& b) { return a.first.sideLength() < b.first.sideLength(); });
    for (const auto& [m, c] : drawOrder) overlay_.draw(frame, c->image, m);

    if (ui_.showDebug) detector_.drawDebug(frame);

    // 4) 정보 표시
    ui_.drawOverlayInfo(frame, cards, fps_);
}

void ARCardService::saveCapture(const cv::Mat& frame) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::create_directories(cfg_.captureDir, ec);

    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream name;
    name << cfg_.captureDir << "/capture_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".png";

    if (cv::imwrite(name.str(), frame)) {
        std::cout << "[ARCardService] Saved " << name.str() << "\n";
        ui_.showMessage("Saved: " + name.str());
    } else {
        ui_.showMessage("Capture failed");
    }
}

bool ARCardService::handleAction(UIAction action, const cv::Mat& frame) {
    switch (action) {
    case UIAction::Quit:        return false;
    case UIAction::Capture:     saveCapture(frame); break;
    case UIAction::ToggleInfo:  ui_.showInfo = !ui_.showInfo; break;
    case UIAction::ToggleSpecs: ui_.showSpecs = !ui_.showSpecs; break;
    case UIAction::ToggleDebug: ui_.showDebug = !ui_.showDebug; break;
    case UIAction::ScaleUp:     overlay_.setScale(std::min(3.0f, overlay_.scale() + 0.1f)); break;
    case UIAction::ScaleDown:   overlay_.setScale(std::max(0.5f, overlay_.scale() - 0.1f)); break;
    default: break;
    }
    return true;
}

void ARCardService::run() {
    cv::Mat frame;
    int failCount = 0;
    auto prev = std::chrono::steady_clock::now();

    while (true) {
        // 0) 영상 입력 (일시적 실패는 몇 번 재시도)
        if (!camera_.read(frame)) {
            if (++failCount > 30) {
                std::cerr << "[ARCardService] Camera read failed, exiting\n";
                break;
            }
            if (ui_.pollKey(10) == UIAction::Quit) break;
            continue;
        }
        failCount = 0;

        processFrame(frame);

        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - prev).count();
        prev = now;
        if (dt > 0) fps_ = fps_ * 0.9 + (1.0 / dt) * 0.1;   // 지수 이동평균

        ui_.show(frame);
        if (!handleAction(ui_.pollKey(1), frame)) break;
    }
    camera_.release();
}
