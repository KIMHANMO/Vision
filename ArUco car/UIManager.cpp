#include "UIManager.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cstdio>

namespace {
const cv::Scalar WHITE(255, 255, 255), BLACK(0, 0, 0), GREEN(80, 220, 80), GRAY(180, 180, 180);
const int FONT = cv::FONT_HERSHEY_SIMPLEX;

// 반투명 사각형 배경
void shadeRect(cv::Mat& frame, cv::Rect r, double alpha = 0.55) {
    r &= cv::Rect(0, 0, frame.cols, frame.rows);
    if (r.area() <= 0) return;
    cv::Mat roi = frame(r);
    cv::Mat dark(roi.size(), roi.type(), BLACK);
    cv::addWeighted(dark, alpha, roi, 1.0 - alpha, 0.0, roi);
}
} // namespace

UIManager::UIManager(const std::string& windowName) : window_(windowName) {}

UIManager::~UIManager() {
    if (windowCreated_) cv::destroyWindow(window_);
}

void UIManager::drawLabel(cv::Mat& frame, const MarkerInfo& m, const CardContent& c) const {
    std::string text = c.name + "  (ID " + std::to_string(c.id) + ")";
    int base = 0;
    cv::Size ts = cv::getTextSize(text, FONT, 0.6, 2, &base);
    cv::Rect br = m.boundingRect();
    cv::Point org(br.x + br.width / 2 - ts.width / 2, br.y - 12);
    if (org.y - ts.height < 0) org.y = br.y + br.height + ts.height + 12; // 위가 잘리면 아래쪽에 표시
    cv::rectangle(frame, { org.x - 6, org.y - ts.height - 6, ts.width + 12, ts.height + base + 10 },
                  c.color, cv::FILLED);
    cv::putText(frame, text, org, FONT, 0.6, WHITE, 2, cv::LINE_AA);
    if (m.lostFrames > 0)   // 잠깐 놓친 마커는 점선 느낌으로 표시
        cv::circle(frame, m.center(), 6, GRAY, 2);
}

void UIManager::drawSpecPanel(cv::Mat& frame,
                              const std::vector<std::pair<MarkerInfo, const CardContent*>>& cards) const {
    const int w = 300, lineH = 22, x = frame.cols - w - 10;
    int h = 10;
    for (const auto& [m, c] : cards) h += 34 + (int)c->specs.size() * lineH + 10;
    if (cards.empty()) return;
    shadeRect(frame, { x, 50, w, h });

    int y = 50 + 30;
    for (const auto& [m, c] : cards) {
        cv::rectangle(frame, { x + 10, y - 18, 8, 22 }, c->color, cv::FILLED);
        cv::putText(frame, c->name, { x + 26, y }, FONT, 0.65, WHITE, 2, cv::LINE_AA);
        y += 26;
        for (const auto& [k, v] : c->specs) {
            cv::putText(frame, k, { x + 26, y }, FONT, 0.45, GRAY, 1, cv::LINE_AA);
            cv::putText(frame, v, { x + 130, y }, FONT, 0.45, WHITE, 1, cv::LINE_AA);
            y += lineH;
        }
        y += 18;
    }
}

void UIManager::drawOverlayInfo(cv::Mat& frame,
                                const std::vector<std::pair<MarkerInfo, const CardContent*>>& cards,
                                double fps) const {
    // 상단 상태바: 인식된 카드 수 + ID 목록 + FPS
    shadeRect(frame, { 0, 0, frame.cols, 40 });
    std::string ids;
    for (const auto& [m, c] : cards) ids += (ids.empty() ? "" : ", ") + std::to_string(c->id);
    std::string status = "Cards: " + std::to_string(cards.size()) + (ids.empty() ? "" : "  [ID " + ids + "]");
    cv::putText(frame, status, { 12, 27 }, FONT, 0.7, GREEN, 2, cv::LINE_AA);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "FPS %.1f", fps);
    cv::putText(frame, buf, { frame.cols - 120, 27 }, FONT, 0.6, WHITE, 1, cv::LINE_AA);

    if (showInfo)
        for (const auto& [m, c] : cards) drawLabel(frame, m, *c);
    if (showSpecs) drawSpecPanel(frame, cards);

    // 하단 도움말
    const std::string help = "[q] Quit  [c] Capture  [i] Labels  [s] Specs  [d] Debug  [+/-] Size";
    shadeRect(frame, { 0, frame.rows - 30, frame.cols, 30 });
    cv::putText(frame, help, { 12, frame.rows - 10 }, FONT, 0.5, WHITE, 1, cv::LINE_AA);

    // 일시 메시지 (캡처 완료 등)
    if (messageFrames_ > 0) {
        int base = 0;
        cv::Size ts = cv::getTextSize(message_, FONT, 0.8, 2, &base);
        cv::Point org((frame.cols - ts.width) / 2, frame.rows / 2);
        shadeRect(frame, { org.x - 15, org.y - ts.height - 15, ts.width + 30, ts.height + 30 }, 0.7);
        cv::putText(frame, message_, org, FONT, 0.8, GREEN, 2, cv::LINE_AA);
        --messageFrames_;
    }
}

void UIManager::showMessage(const std::string& msg, int frames) {
    message_ = msg;
    messageFrames_ = frames;
}

void UIManager::show(const cv::Mat& frame) {
    if (!windowCreated_) {   // 첫 프레임에서 창 생성
        cv::namedWindow(window_, cv::WINDOW_AUTOSIZE);
        windowCreated_ = true;
    }
    cv::imshow(window_, frame);
}

UIAction UIManager::pollKey(int delayMs) const {
    int k = cv::waitKey(delayMs);
    if (k < 0) {
        // 창의 X 버튼으로 닫은 경우 종료
        if (windowCreated_ && cv::getWindowProperty(window_, cv::WND_PROP_VISIBLE) < 1) return UIAction::Quit;
        return UIAction::None;
    }
    switch (k & 0xFF) {
    case 'q': case 'Q': case 27: return UIAction::Quit;
    case 'c': case 'C': return UIAction::Capture;
    case 'i': case 'I': return UIAction::ToggleInfo;
    case 's': case 'S': return UIAction::ToggleSpecs;
    case 'd': case 'D': return UIAction::ToggleDebug;
    case '+': case '=': return UIAction::ScaleUp;
    case '-': case '_': return UIAction::ScaleDown;
    default: return UIAction::None;
    }
}
