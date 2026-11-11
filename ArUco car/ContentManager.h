#pragma once
#include <opencv2/core.hpp>
#include <map>
#include <string>
#include <utility>
#include <vector>

// 마커 ID에 대응하는 카드 콘텐츠 (차종)
struct CardContent {
    int id = -1;
    std::string name;      // 화면 표시용 이름 (cv::putText는 한글 미지원 → 영문)
    std::string imageFile; // AR 합성 이미지 파일명 (한글 제원이 그려진 이미지)
    std::vector<std::pair<std::string, std::string>> specs; // 제원 (항목, 값)
    cv::Scalar color;      // UI 강조색 (BGR)
    cv::Mat image;
};

// 마커 ID -> 콘텐츠 관리
class ContentManager {
public:
    bool load(const std::string& contentDir);   // 이미지 로드 (없으면 대체 이미지 생성)
    const CardContent* find(int id) const;
    size_t size() const { return contents_.size(); }

private:
    void registerDefaults();
    static cv::Mat makePlaceholder(const CardContent& c);
    std::map<int, CardContent> contents_;
};
