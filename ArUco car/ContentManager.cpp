#include "ContentManager.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

// 카드(차종) 데이터 등록 — 카드를 추가하려면 여기에 한 줄 추가 + content/ 에 이미지 추가
// 제원은 차급별 예시값
void ContentManager::registerDefaults() {
    contents_.clear();
    contents_[0] = { 0, "SEDAN", "car_0.png",
        { {"Size", "4900x1860x1445 mm"}, {"Engine", "2.0L Gasoline"}, {"Power", "160 hp"},
          {"Fuel eff.", "12.5 km/L"}, {"Seats", "5"} },
        cv::Scalar(200, 110, 52), cv::Mat() };
    contents_[1] = { 1, "SUV", "car_1.png",
        { {"Size", "4800x1900x1700 mm"}, {"Engine", "2.2L Diesel"}, {"Power", "202 hp"},
          {"Fuel eff.", "13.0 km/L"}, {"Seats", "7"} },
        cv::Scalar(100, 150, 40), cv::Mat() };
    contents_[2] = { 2, "SPORTS CAR", "car_2.png",
        { {"Size", "4500x1900x1300 mm"}, {"Engine", "3.0L Twin-turbo"}, {"Power", "400 hp"},
          {"0-100 km/h", "4.2 s"}, {"Seats", "2"} },
        cv::Scalar(50, 50, 210), cv::Mat() };
    contents_[3] = { 3, "TRUCK", "car_3.png",
        { {"Size", "5100x1740x1970 mm"}, {"Engine", "2.5L Diesel"}, {"Power", "133 hp"},
          {"Payload", "1000 kg"}, {"Seats", "3"} },
        cv::Scalar(30, 150, 230), cv::Mat() };
}

bool ContentManager::load(const std::string& contentDir) {
    registerDefaults();
    std::string dir = contentDir;
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') dir += '/';

    bool allLoaded = true;
    for (auto& [id, c] : contents_) {
        c.image = cv::imread(dir + c.imageFile, cv::IMREAD_COLOR);
        if (c.image.empty()) {
            std::cerr << "[ContentManager] Missing image: " << dir + c.imageFile
                      << " -> using placeholder\n";
            c.image = makePlaceholder(c);
            allLoaded = false;
        }
    }
    std::cout << "[ContentManager] " << contents_.size() << " cards registered\n";
    return allLoaded;
}

const CardContent* ContentManager::find(int id) const {
    auto it = contents_.find(id);
    return it == contents_.end() ? nullptr : &it->second;
}

// 이미지 파일이 없을 때 프로그램이 멈추지 않도록 영문 제원 카드를 직접 그림
cv::Mat ContentManager::makePlaceholder(const CardContent& c) {
    cv::Mat img(600, 600, CV_8UC3, cv::Scalar(250, 250, 250));
    cv::rectangle(img, { 0, 0, 600, 90 }, c.color, cv::FILLED);
    cv::putText(img, c.name, { 25, 62 }, cv::FONT_HERSHEY_DUPLEX, 1.6, cv::Scalar(255, 255, 255), 2);
    int y = 160;
    for (const auto& [k, v] : c.specs) {
        cv::putText(img, k, { 30, y }, cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(90, 90, 90), 2);
        cv::putText(img, v, { 250, y }, cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(20, 20, 20), 2);
        y += 80;
    }
    return img;
}
