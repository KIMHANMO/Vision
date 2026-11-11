// AR 카드 인식 기반 증강현실 프로그램 (6조)
// 사용법: ARCardProject.exe [카메라번호] [content 폴더]
#include "ARCardService.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    AppConfig cfg;
    if (argc > 1) cfg.cameraIndex = std::stoi(argv[1]);
    if (argc > 2) cfg.contentDir = argv[2];

    ARCardService app(cfg);
    if (!app.initialize()) {
        std::cerr << "Initialization failed. Check the camera connection.\n";
        return 1;
    }
    app.run();
    return 0;
}
