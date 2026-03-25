# FolderDir

**FolderDir**는 [Q-Dir](https://www.softwareok.com/?seite=Freeware/Q-Dir)에서 영감을 받아 C++17과 Qt로 제작된 다중 창 파일 관리자입니다.  
크기 조절 가능한 분할 창 레이아웃으로 **최대 4개의 디렉터리를 동시에** 볼 수 있으며, 탭, 즐겨찾기, 파일 작업 등 다양한 기능을 제공합니다.

---

## ✨ 주요 기능

| 기능 | 설명 |
|------|------|
| **다중 창 레이아웃** | 1 / 2 / 3 / 4개의 창, 크기 조절 가능한 분할기 (기본값: 2×2 격자) |
| **창별 탭** | 각 창에서 무제한 탭 지원; Ctrl+T / Ctrl+W로 열기/닫기 |
| **주소 표시줄** | 유효성 검사가 포함된 편집 가능한 경로; 입력 후 Enter로 이동 |
| **드라이브 표시줄** | 마운트된 드라이브/볼륨으로 한 번에 이동 |
| **파일 작업** | 복사(F5), 이동(F6), 삭제(F8/Del), 이름 바꾸기(F2), 새 폴더(F7) |
| **비동기 진행 표시** | 대용량 복사/이동 작업 시 별도 스레드에서 속도 및 예상 완료 시간 표시 |
| **충돌 해결** | 파일별 대화상자: 덮어쓰기 / 건너뛰기 / 이름 바꾸기 / 취소 ("전체" 옵션 포함) |
| **즐겨찾기** | Ctrl+D로 추가; 사이드바 목록; JSON 내보내기/가져오기 |
| **검색** | 전체 검색 대화상자: 이름 와일드카드, 내용 텍스트, 날짜 및 크기 범위 |
| **미리 보기 패널** | 이미지(비동기 썸네일), 일반 텍스트/코드, 일반 파일 정보 |
| **필터 표시줄** | 창별 글로브 필터(예: `*.cpp`), 각 창 하단에 표시 |
| **보기 모드** | 자세히 / 목록 / 아이콘 / 썸네일 — 창별로 전환 가능 |
| **숨김 파일 표시** | Ctrl+H 토글 |
| **다크 / 라이트 테마** | 시스템 / 라이트 / 다크 — QApplication 팔레트로 적용 |
| **세션 복원** | 다음 실행 시 모든 창의 탭 및 경로 재오픈 |
| **설정 대화상자** | 일반 · 외관 · 파일 작업 · 단축키 |
| **터미널 열기** | 현재 디렉터리에서 시스템 터미널 열기 |

---

## 🗂 프로젝트 구조

```
FolderDir/
├── CMakeLists.txt            # CMake 빌드 파일 (Qt5 또는 Qt6)
├── docs/
│   └── planning.md           # 상세 기획 문서 (한국어 + 영어)
├── resources/
│   ├── resources.qrc         # Qt 리소스 파일
│   └── icons/
│       └── app.png           # 애플리케이션 아이콘
└── src/
    ├── main.cpp
    ├── MainWindow.h/cpp      # 메인 윈도우: 메뉴, 툴바, 창 레이아웃
    ├── FolderPane.h/cpp      # 개별 창: 탭 + 주소 표시줄 + 보기
    ├── AddressBar.h/cpp      # 편집 가능한 경로 표시줄
    ├── FileSystemBrowser.h/cpp  # 파일 목록 (자세히/목록/아이콘 보기)
    ├── FileSystemModel.h/cpp    # QFileSystemModel 래퍼 + 필터
    ├── FileOperations.h/cpp     # 비동기 복사/이동/삭제 워커
    ├── FileOperationDialog.h/cpp  # 충돌 해결 기능이 포함된 진행 대화상자
    ├── BookmarkManager.h/cpp    # 즐겨찾기 지속성 (JSON)
    ├── PreviewPanel.h/cpp       # 비동기 이미지/텍스트/정보 미리 보기
    ├── SettingsManager.h/cpp    # QSettings를 통한 타입화된 설정
    ├── SettingsDialog.h/cpp     # 설정 UI
    ├── SearchDialog.h/cpp       # 검색 UI + 백그라운드 워커
    └── DriveBar.h/cpp           # 드라이브/볼륨 빠른 접근 표시줄
```

---

## 🔧 빌드 요구사항

| 의존성 | 버전 | 비고 |
|--------|------|------|
| C++ 컴파일러 | C++17 | GCC 8+, Clang 7+, MSVC 2019+ |
| CMake | 3.16+ | |
| Qt | **6.x** (권장) 또는 **5.15** | Widgets, Concurrent 모듈 |

### Qt 설치

**Ubuntu/Debian**
```bash
sudo apt install qt6-base-dev qt6-base-dev-tools cmake ninja-build
# 또는 Qt5:
sudo apt install qtbase5-dev qt5-default cmake
```

**macOS (Homebrew)**
```bash
brew install qt cmake ninja
export CMAKE_PREFIX_PATH=$(brew --prefix qt)
```

**Windows**
```
https://www.qt.io/download 에서 Qt 온라인 인스톨러를 다운로드하여 *MSVC* 또는 *MinGW* 컴포넌트를 설치하세요.
```

---

## 🚀 빌드 및 실행

```bash
# 클론
git clone https://github.com/shinehand/FolderDir.git
cd FolderDir

# 구성
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 빌드
cmake --build build --parallel

# 실행
./build/FolderDir          # Linux / macOS
build\FolderDir.exe        # Windows
```

### 디버그 빌드
```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
```

---

## ⌨️ 키보드 단축키

| 단축키 | 동작 |
|--------|------|
| **Alt+←** | 뒤로 이동 |
| **Alt+→** | 앞으로 이동 |
| **Alt+↑** | 상위 디렉터리로 이동 |
| **F2** | 선택 항목 이름 바꾸기 |
| **F5** | 복사 |
| **F6** | 이동 |
| **F7** / **Ctrl+Shift+N** | 새 폴더 |
| **Delete** / **F8** | 선택 항목 삭제 |
| **Ctrl+C** | 클립보드에 복사 |
| **Ctrl+X** | 잘라내기 |
| **Ctrl+V** | 붙여넣기 |
| **Ctrl+A** | 전체 선택 |
| **Ctrl+Shift+C** | 전체 경로를 클립보드에 복사 |
| **Ctrl+D** | 현재 폴더를 즐겨찾기에 추가 |
| **Ctrl+F** | 검색 대화상자 열기 |
| **Ctrl+H** | 숨김 파일 표시/숨기기 |
| **Ctrl+P** | 미리 보기 패널 표시/숨기기 |
| **Ctrl+T** | 새 탭 |
| **Ctrl+W** | 현재 탭 닫기 |
| **Ctrl+1..4** | 1–4번 창 활성화 |
| **/** 또는 **\\** | 필터 표시줄 토글 |
| **F5** (보기에서) | 새로 고침 |
| **Backspace** | 상위 디렉터리로 이동 |

---

## 🏗 아키텍처 노트

- **스레드 안전성**: 모든 파일 작업은 `FileOperation`을 통해 전용 `QThread`에서 실행됩니다.  
  진행 신호는 `Qt::QueuedConnection`을 사용하여 결과를 GUI 스레드로 안전하게 전달합니다.
- **가상 모델**: `QFileSystemModel`은 주문형 디렉터리 로딩을 처리하여, 100,000개 이상의 파일이 있는 디렉터리에서도 UI가 반응적으로 동작합니다.
- **비동기 썸네일**: `PreviewPanel`은 `QtConcurrent::run` + `QFutureWatcher`를 사용하여 이벤트 루프를 차단하지 않고 이미지를 디코딩합니다.
- **충돌 해결**: 차단된 워커는 뮤텍스에서 대기하며, GUI가 해결 후 `FileOperation::resolveConflict()`를 호출하여 차단을 해제합니다 — GUI 스레드에서 바쁜 대기 없음.
- **세션 지속성**: `QSettings`는 창 수, 탭 경로, 화면 위치 및 모든 설정을 저장합니다. 옵션이 활성화된 경우 시작 시 자동으로 복원됩니다.

---

## 📋 로드맵

- [ ] Phase 1 ✅ — 다중 창 레이아웃, 파일 탐색, 자세히 보기
- [ ] Phase 2 ✅ — 탭, 파일 작업, 주소 표시줄
- [ ] Phase 3 ✅ — 즐겨찾기, 검색, 필터, 설정
- [ ] Phase 4 ✅ — 미리 보기 패널, 드라이브 표시줄, 비동기 썸네일
- [ ] Phase 5 — 썸네일 캐싱, 다국어 지원(한국어), 플러그인 시스템, 포터블 모드

---

## 📄 라이선스

MIT — 자세한 내용은 [LICENSE](LICENSE)를 참조하세요.
