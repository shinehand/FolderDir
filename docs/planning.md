# FolderDir 기획서 (Planning Document)
## Q-Dir 클론 멀티패널 폴더 뷰어 — C++ 구현

---

## 0. Q-Dir 비교 분석 및 고도화 기획 (Gap Analysis & Refinement)

> 최초 작성: 2026-03-25 — 기획팀 3명 (PM 1 · UX 디자이너 1 · 비즈니스 애널리스트 1)  
> 개정 v2: 2026-03-26 — 기획팀 4명 (PM 1 · UX 디자이너 1 · BA 1 · QA 1) — 기본 기능 버그 분석·UX 인터랙션 재명세  
> 검토: 개발팀 리더 + 개발자 3명

---

### 0.1 분석 방법 (Analysis Methodology)

1. **기획팀 3인 분석** — Q-Dir 공식 기능 목록과 현재 구현 코드를 각각 전수 검토 후 항목별로 비교·정리  
2. **개발팀 3인 회의** — 구현 난이도·우선순위·아키텍처 영향도를 평가하여 최적 개발 방향 결정  
3. **리더 컨펌** — 아래 "리더 확인" 체크리스트로 최종 승인 후 개발 착수

---

### 0.2 Q-Dir vs FolderDir 기능 비교표 (Feature Comparison)

#### 범례 (Legend)
| 아이콘 | 의미 |
|--------|------|
| ✅ | 구현 완료 (Implemented) |
| ⚠️ | 부분 구현 (Partial) |
| ❌ | 미구현 (Not Implemented) |

#### A. 레이아웃 & 패널 (Layout & Pane Management)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 1 / 2 / 3 / 4 패널 전환 | ✅ | ✅ | 동일 |
| 수직·수평 분할 레이아웃 | ✅ | ✅ | 동일 |
| 패널 크기 비율 드래그 조절 | ✅ | ✅ | 동일 |
| 패널 간 디렉터리 동기화 탐색 | ✅ | ❌ | **미구현** — 한 패널 이동 시 다른 패널 자동 동기화 |
| 패널 잠금 (이동 방지) | ✅ | ❌ | **미구현** — Lock Pane 기능 |
| 패널 복제 (Clone Pane) | ✅ | ❌ | **미구현** — 현재 패널 경로 그대로 복제 |
| 패널 레이아웃 즐겨찾기 (64개) | ✅ | ❌ | **미구현** — 자주 쓰는 레이아웃 저장/불러오기 |

#### B. 탭 (Tabs)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 패널 당 탭 지원 (최대 16개) | ✅ | ✅ | 동일 |
| 탭 열기 / 닫기 (Ctrl+T/W) | ✅ | ✅ | 동일 |
| 탭 우클릭 컨텍스트 메뉴 | ✅ | ✅ | 동일 |
| 탭 드래그 — 패널 간 이동 | ✅ | ✅ | Sprint 8 완료 — DraggableTabBar |
| 탭 이름 변경 | ✅ | ✅ | Sprint 5 완료 — 탭 더블클릭·우클릭 메뉴 |
| Ctrl+Tab — 다음 탭으로 이동 | ✅ | ✅ | Sprint 1 완료 — FolderPane::nextTab() |

#### C. 주소창 & 탐색 (Address Bar & Navigation)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 편집 가능한 경로 입력 | ✅ | ✅ | 동일 |
| 브레드크럼(Breadcrumb) UI | ✅ | ✅ | Sprint 2 완료 — BreadcrumbBar (클릭 가능 세그먼트 + 편집 모드 전환) |
| 뒤로 / 앞으로 / 위로 | ✅ | ✅ | 동일 |
| 폴더 트리 사이드바 | ✅ | ✅ | Sprint 3 완료 — FolderTreePanel (독 위젯, Ctrl+Shift+T 토글) |
| 특수 폴더 빠른 접근 (Desktop·Docs·Downloads) | ✅ | ❌ | **미구현** — 사이드바 즐겨찾기 고정 항목 (FolderTreePanel 확장 필요) |
| 네트워크/UNC 경로 지원 | ✅ | ⚠️ | 부분 지원 — UNC 경로 입력은 가능하나 전용 UI 없음 |
| FTP / SFTP 폴더 지원 | ✅ | ❌ | **미구현** — 원격 파일 시스템 탐색 |
| 드라이브 표시줄 | ✅ | ✅ | 동일 |

#### D. 보기 모드 (View Modes)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 상세 보기 (Details) | ✅ | ✅ | 동일 |
| 목록 보기 (List) | ✅ | ✅ | 동일 |
| 아이콘 보기 (Icons) | ✅ | ✅ | 동일 |
| 썸네일 보기 (Thumbnails) | ✅ | ✅ | 동일 |
| 타일 보기 (Tiles) | ✅ | ❌ | **미구현** |
| 엑스트라 라지 아이콘 | ✅ | ❌ | **미구현** |
| 컬럼 표시/숨김·순서 변경 | ✅ | ⚠️ | 부분 — 정렬 가능, 컬럼 추가/숨김 UI 없음 |
| 폴더 크기 컬럼 계산 | ✅ | ✅ | Sprint 7 완료 — FolderSizeWorker (비동기) |
| 파일/폴더 색상 코딩 | ✅ | ✅ | Sprint 6 완료 — ColorManager + ColorRulesDialog |
| 파일 색상 필터 하이라이트 | ✅ | ❌ | **미구현** — 파일 유형별 색상 강조 |
| 숨김 파일/폴더 토글 | ✅ | ✅ | 동일 |
| 정렬 (이름·크기·유형·날짜) | ✅ | ✅ | 동일 |
| 중복 파일 하이라이트 | ✅ | ❌ | **미구현** |

#### E. 파일 작업 (File Operations)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 복사 (F5 / Ctrl+C) | ✅ | ✅ | 동일 |
| 이동 (F6 / Ctrl+X+V) | ✅ | ✅ | 동일 |
| 삭제 (Delete / F8) | ✅ | ✅ | 동일 |
| 이름 변경 (F2) | ✅ | ✅ | 동일 |
| 새 폴더 (F7) | ✅ | ✅ | Sprint 1 완료 — F7 단축키 매핑 완료 |
| 새 파일 (Ctrl+N) | ✅ | ✅ | 동일 |
| 속성 대화상자 (Alt+Enter) | ✅ | ✅ | Sprint 1 완료 — showProperties() 전체 정보 다이얼로그 (이름·위치·크기·날짜·권한) |
| 이름 바꿔 복사 (F9) | ✅ | ✅ | Sprint 4 완료 — copyAndRename() |
| 진행 다이얼로그 (속도·ETA·취소) | ✅ | ✅ | 동일 |
| 충돌 처리 (덮어쓰기·건너뛰기·이름변경) | ✅ | ✅ | 동일 |
| 패널 간 드래그 앤 드롭 복사 | ✅ | ✅ | 동일 |
| Shift+드래그 이동 | ✅ | ⚠️ | 기본 드래그는 동작, Shift 구분 미흡 |
| Ctrl+Shift+드래그 심볼릭 링크 생성 | ✅ | ❌ | **미구현** |
| 폴더 비교 (Compare Folders) | ✅ | ❌ | **미구현** |
| 디렉터리 목록 내보내기 (XLS·CSV·TXT·HTML) | ✅ | ❌ | **미구현** |
| 디렉터리 목록 인쇄 | ✅ | ❌ | **미구현** |
| CRC 복사 검증 | ✅ | ⚠️ | 설정 존재, 로직 미구현 |
| 압축(ZIP) 탐색 | ✅ | ❌ | **미구현** — ZIP을 폴더처럼 탐색 |
| 휴지통 통합 (Recycle Bin) | ✅ | ⚠️ | 삭제 시 확인 다이얼로그만, 휴지통 이동 미구현 |

#### F. 검색 & 필터 (Search & Filter)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 패널 내 와일드카드 필터 | ✅ | ✅ | 동일 |
| 전체 검색 다이얼로그 (이름·내용·날짜·크기) | ✅ | ✅ | 동일 |
| 검색 결과 별도 탭 표시 | ✅ | ✅ | 동일 |
| 파일 확장자별 색상 필터 | ✅ | ❌ | **미구현** |
| 정규식(Regex) 검색 | ✅ | ❌ | **미구현** |

#### G. 미리보기 (Preview)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 이미지 미리보기 (JPEG·PNG·BMP·GIF) | ✅ | ✅ | 동일 |
| 텍스트 미리보기 | ✅ | ✅ | 동일 |
| PDF 미리보기 | ✅ | ❌ | **미구현** — Qt PDF 모듈 필요 |
| 화면 돋보기(Magnifier) | ✅ | ❌ | **미구현** |
| 파일 정보 패널 | ✅ | ✅ | 동일 |

#### H. 즐겨찾기 & 북마크 (Bookmarks & Favorites)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 즐겨찾기 추가 (Ctrl+D) | ✅ | ✅ | 동일 |
| 즐겨찾기 사이드바 | ✅ | ✅ | 동일 |
| 즐겨찾기 드래그 순서 변경 | ✅ | ✅ | 동일 |
| 즐겨찾기 내보내기·가져오기 (JSON) | ✅ | ✅ | Sprint 5 완료 — Bookmarks 메뉴 UI 연결 완료 |
| 레이아웃 즐겨찾기 (최대 64개) | ✅ | ❌ | **미구현** |
| 각 항목 사용자 정의 아이콘·이름 | ✅ | ❌ | **미구현** |

#### I. 단축키 (Keyboard Shortcuts)

| 단축키 | Q-Dir | FolderDir | 차이/비고 |
|--------|-------|-----------|-----------|
| F2 — 이름 변경 | ✅ | ✅ | 동일 |
| F3 — 파일 보기 (뷰어 열기) | ✅ | ✅ | Sprint 1 완료 — openWithViewer() |
| F4 — 파일 편집 (편집기 열기) | ✅ | ✅ | Sprint 1 완료 — openWithEditor() |
| F5 — 복사 | ✅ | ✅ | BUG-003 수정 완료 — FileSystemBrowser에서 F5 소비 제거; F5=Copy To (MainWindow 처리), 새로고침은 Ctrl+R |
| F6 — 이동 | ✅ | ✅ | Sprint 4 완료 — moveToPath() (MainWindow에서 처리) |
| F7 — 새 폴더 | ✅ | ✅ | Sprint 1 완료 — F7 매핑 완료 |
| F8 / Delete — 삭제 | ✅ | ✅ | 동일 |
| F9 — 이름 바꿔 복사 | ✅ | ✅ | Sprint 4 완료 — copyAndRename() |
| F10 — 종료 | ✅ | ✅ | Sprint 1 완료 — onExit() |
| Alt+Enter — 속성 | ✅ | ✅ | Sprint 1 완료 — showProperties() 단축키 매핑 |
| Ctrl+Tab — 다음 탭 | ✅ | ✅ | Sprint 1 완료 — FolderPane::nextTab() |
| Ctrl+Shift+C — 경로 복사 | ✅ | ✅ | 동일 |
| / 또는 \ — 필터 토글 | ✅ | ✅ | 동일 |

#### J. 설정 & 사용자화 (Settings & Customization)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 라이트/다크/시스템 테마 | ✅ | ✅ | 동일 |
| 다국어 지원 (한국어·영어 등) | ✅ | ⚠️ | tr() 마킹, .ts 번역 파일 미생성 |
| 세션 저장·복원 | ✅ | ✅ | 동일 |
| 컬럼 커스터마이즈 | ✅ | ⚠️ | 기본 컬럼만 |
| 포터블 모드 (INI 파일) | ✅ | ❌ | **미구현** — 레지스트리 대신 INI 사용 옵션 |
| 고대비(High Contrast) 모드 | ✅ | ❌ | **미구현** |
| 폰트·UI 크기 커스터마이즈 | ✅ | ❌ | **미구현** |
| 툴바 커스터마이즈 | ✅ | ❌ | **미구현** |
| 마우스 제스처 | ✅ | ❌ | **미구현** |
| 파일 연결 프로그램 지정 | ✅ | ❌ | **미구현** |

#### K. 터미널 & 외부 도구 (Terminal & External Tools)

| 기능 | Q-Dir | FolderDir | 차이/비고 |
|------|-------|-----------|-----------|
| 현재 경로로 터미널 열기 | ✅ | ✅ | 동일 |
| 파일 편집기 연동 (F4) | ✅ | ❌ | **미구현** |
| 파일 뷰어 연동 (F3) | ✅ | ❌ | **미구현** |
| 이메일 첨부 발송 | ✅ | ❌ | **미구현** |
| 사용자 정의 외부 도구 등록 | ✅ | ❌ | **미구현** |

---

### 0.3 미구현 기능 우선순위 (Missing Features Priority List)

> **기획팀 3인 합의 결과** (PM·UX·BA 공동 평가: 영향도 × 구현 난이도 × 사용자 요청 빈도)

#### 🔴 우선순위 1 — 필수 (Critical, Phase 5)

| # | 기능 | 이유 |
|---|------|------|
| 1 | **F-키 단축키 완성** (F3·F4·F5·F6·F7·F9·F10) | Q-Dir 사용자의 근육 기억(muscle memory) 직결 |
| 2 | **브레드크럼 주소창 UI** | UX 핵심 — 경로 계층 탐색 편의성 대폭 향상 |
| 3 | **폴더 트리 사이드바** | 계층적 탐색 필수; Q-Dir과 가장 눈에 띄는 차이 |
| 4 | **Ctrl+Tab 탭 전환** | 기본 기대 동작, 빠른 구현 가능 |
| 5 | **Alt+Enter 속성 다이얼로그** | 파일 상세정보·권한 확인은 일상적 작업 |
| 6 | **F7 새 폴더 단축키** | Q-Dir 표준 단축키 미지원으로 혼란 유발 |
| 7 | **즐겨찾기 내보내기·가져오기 메뉴 UI** | 코드는 있으나 UI 연결 필요 (낮은 구현 비용) |
| 8 | **CRC 복사 검증 실제 구현** | 설정만 있고 기능 없어 사용자 오해 발생 |

#### 🟠 우선순위 2 — 중요 (High, Phase 6)

| # | 기능 | 이유 |
|---|------|------|
| 9 | **파일/폴더 색상 코딩** | Q-Dir의 핵심 차별화 기능; 시각적 분류 |
| 10 | **폴더 크기 컬럼 계산** | 디스크 정리 워크플로에 필수 |
| 11 | **탭 드래그 — 패널 간 이동** | 멀티패널 UX의 자연스러운 확장 |
| 12 | **탭 이름 변경** | 많은 탭 사용 시 식별 편의 |
| 13 | **패널 동기화 탐색** | 미러링 작업 시 생산성 향상 |
| 14 | **패널 잠금 / 복제** | 고급 워크플로 지원 |
| 15 | **PDF 미리보기** | Qt PDF 모듈 추가로 구현 가능 |
| 16 | **정규식 검색** | 고급 사용자 요청 빈번 |
| 17 | **레이아웃 즐겨찾기 저장** | 반복 작업 자동화 |

#### 🟡 우선순위 3 — 있으면 좋음 (Nice-to-Have, Phase 7)

| # | 기능 | 이유 |
|---|------|------|
| 18 | **ZIP 파일 내부 탐색** | 자주 요청되는 기능 |
| 19 | **디렉터리 목록 내보내기 (CSV·XLS·TXT)** | 보고서·자산 관리 용도 |
| 20 | **폴더 비교** | 개발자/파워유저 대상 |
| 21 | **포터블 모드 (INI)** | 기업 배포 시나리오 |
| 22 | **한국어 번역 파일 (.ts/.qm)** | 내수 사용자 UX |
| 23 | **사용자 정의 외부 도구 등록** | 확장성 강화 |
| 24 | **휴지통 이동 (Recycle Bin)** | 영구 삭제 전 안전망 |
| 25 | **FTP/SFTP 원격 탐색** | 웹 개발자·시스템 관리자 대상 |
| 26 | **중복 파일 하이라이트** | 디스크 정리 워크플로 |
| 27 | **타일 보기 모드** | 추가 보기 옵션 |
| 28 | **툴바·컬럼 커스터마이즈** | 고급 사용자 개인화 |
| 29 | **마우스 제스처** | 파워유저 편의 |
| 30 | **고대비 모드 / 접근성** | 접근성 법규 준수 |

---

### 0.4 개발팀 3인 회의 결과 — 최적 개발 방향 (Dev Team Decision)

> 회의 일자: 2026-03-25  
> 참여: 개발자 A (Backend·FileOps), 개발자 B (UI·Qt), 개발자 C (Architecture·Cross-platform)

#### 아키텍처 원칙 합의

1. **기존 클래스 구조 유지** — 신규 기능은 기존 `FolderPane` / `FileSystemBrowser` / `MainWindow` 확장으로 구현  
2. **Qt 표준 API 우선** — 외부 라이브러리 추가 최소화; PDF는 `Qt6::Pdf` 모듈 선택  
3. **스레드 안전성 준수** — 모든 파일 I/O·폴더 크기 계산은 `QThread` / `QtConcurrent` 사용  
4. **역할 분담**:  
   - **개발자 A**: 파일 작업 (F-키·F6·F9·CRC·ZIP·휴지통·폴더 비교)  
   - **개발자 B**: UI 컴포넌트 (브레드크럼·트리사이드바·색상코딩·탭 드래그·속성 다이얼로그)  
   - **개발자 C**: 기반 시스템 (포터블 모드·i18n·레이아웃 즐겨찾기·패널 동기화·PDF)

#### 스프린트 계획 (2주 단위)

| 스프린트 | 담당 | 작업 내용 |
|---------|------|-----------|
| ~~Sprint 1~~ ✅ | B | F-키 완성 (F3·F4·F5·F6·F7·F9·F10) · Ctrl+Tab · Ctrl+W · Alt+Enter 속성 다이얼로그 |
| ~~Sprint 2~~ ✅ | B | 브레드크럼 주소창 UI (BreadcrumbBar 신규 클래스, AddressBar 대체) |
| ~~Sprint 3~~ ✅ | B | 폴더 트리 사이드바 (FolderTreePanel: QTreeView + DockWidget) |
| ~~Sprint 4~~ ✅ | A | F6 이동 다이얼로그 · F9 이름 바꿔 복사 · F7 새 폴더 매핑 |
| ~~Sprint 5~~ ✅ | A | 즐겨찾기 내보내기·가져오기 메뉴 UI 연결 · 탭 이름 변경(더블클릭) |
| ~~Sprint 5.5~~ ✅ | B | **긴급 버그 수정**: 더블클릭 이동 충돌(BUG-001) · 클릭-재클릭 이름변경(BUG-002) — `FileSystemBrowser` EditTrigger 수정 (§0.6.3) |
| ~~Sprint 6~~ ✅ | B | 파일/폴더 색상 코딩 (ColorManager 신규 모듈) |
| ~~Sprint 7~~ ✅ | A | 폴더 크기 컬럼 (백그라운드 스레드 계산) |
| ~~Sprint 8~~ ✅ | B | 탭 드래그·패널 간 이동 |
| ~~Sprint 9~~ ✅ | C | 패널 동기화 · 패널 잠금/복제 · GAP-001 CRC · GAP-002 휴지통 |
| ~~Sprint 10~~ ✅ | C | 레이아웃 즐겨찾기 저장 (최대 10개, `LayoutManager`) |
| Sprint 11 | C | PDF 미리보기 (Qt6::Pdf 모듈) |
| Sprint 12 | C | 포터블 모드 (INI 파일 옵션) |
| Sprint 13 | C | 한국어 번역 .ts/.qm 파일 생성 |
| Sprint 14 | A | ZIP 내부 탐색 (QuaZIP 또는 libarchive) |
| Sprint 15 | A | 폴더 비교 · 디렉터리 목록 내보내기 (CSV·TXT) |

---

### 0.5 리더 컨펌 체크리스트 (Leader Approval Checklist)

> 아래 항목을 확인 후 서명(또는 이슈 코멘트)하여 개발 착수를 승인합니다.  
> **Sprint 8 완료 후 갱신 — Sprint 9 착수 전 재서명 필요 (§0.7.4 참조)**

- [x] Q-Dir vs FolderDir 비교표 (§0.2) 내용 동의 — Sprint 8 완료 기준 재점검 완료 (§0.7.1)
- [x] 미구현 기능 우선순위 (§0.3) 동의
- [x] 스프린트 계획 및 담당자 배정 (§0.4) 동의
- [x] 아키텍처 원칙 (§0.4) 동의
- [x] Phase 5~7 (Sprint 1~8) 완료 확인
- [ ] Phase 8 (Sprint 9~11) 범위 승인 — §0.7.4 체크리스트 참조
- [ ] 잔여 격차 (GAP-001 CRC·GAP-002 휴지통) 처리 방향 확인
- [ ] 예산/일정 검토 완료 (Sprint 9~15 범위)
- [ ] **리더 서명**: _________________________  날짜: _____________

---

### 0.6 Q-Dir UX 참조 분석 및 기본 인터랙션 패턴 명세 (Q-Dir UX Reference & Basic Interaction Spec)

> 작성 일자: 2026-03-26  
> 작성 팀: 기획팀 4명 (PM 1 · UX 디자이너 1 · BA 1 · QA 1)  
> **배경**: QA 검수 결과 기본 폴더 탐색 인터랙션(더블클릭 이동, 클릭-재클릭 이름변경)이 정상 동작하지 않음을 확인. 기획·개발팀이 Q-Dir 원본 UI를 재분석하여 인터랙션 패턴을 명확히 명세화하고 즉시 수정 착수.

---

#### 0.6.1 Q-Dir 레이아웃 시각적 참조 (Q-Dir Visual Layout Reference)

Q-Dir(https://www.q-dir.com/)의 실제 화면 구조를 분석하여 FolderDir의 기준 레이아웃으로 정의한다.

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ [File▾] [Edit▾] [View▾] [Extras▾] [?]                          [_][□][X]  │  ← 메뉴바
├─────────────────────────────────────────────────────────────────────────────┤
│ [⬆][⬅][➡]  [🔍]  [★]  [⚙]  [🖥1][🖥2][🖥3][🖥4]  [📋레이아웃▾]          │  ← 툴바
├──────────────────────────────────────────────────────────────────────────────┤
│ [C:] [D:] [E:] [F:] [G:] [H:] [I:] [네트워크▾]                            │  ← 드라이브바
├────────────────────────┬────────────────────────────────────────────────────┤
│ [📁 Tab1: C:\Users] [+]│ [📁 Tab1: D:\Work] [📁 Tab2: D:\Docs] [+]        │
│ ┌──────────────────────┤ ┌──────────────────────────────────────────────────┤
│ │📍 C: › Users › Alice │ │📍 D: › Work                                     │  ← 브레드크럼
│ ├──────────────────────┤ ├──────────────────────────────────────────────────┤
│ │ 이름     ▲│크기│유형 │ │ 이름       ▲│크기  │유형  │수정일              │
│ ├──────────────────────┤ ├──────────────────────────────────────────────────┤
│ │📁 Documents          │ │📁 ProjectA   │   -  │폴더  │2025-01-10         │
│ │📁 Downloads          │ │📁 ProjectB   │   -  │폴더  │2025-02-15         │
│ │📁 Desktop            │ │📄 README.md  │  4 KB│.md   │2025-03-01         │
│ │📄 notes.txt          │ │📄 TODO.txt   │  1 KB│.txt  │2025-03-20         │
│ └──────────────────────┘ └──────────────────────────────────────────────────┘
├────────────────────────┬────────────────────────────────────────────────────┤
│ [📁 Tab1: E:\Media] [+]│ [📁 Tab1: C:\Temp] [+]                           │
│ ┌──────────────────────┤ ┌──────────────────────────────────────────────────┤
│ │📍 E: › Media         │ │📍 C: › Temp                                     │
│ ├──────────────────────┤ ├──────────────────────────────────────────────────┤
│ │ 이름     ▲│크기│유형 │ │ 이름       ▲│크기  │유형  │수정일              │
│ ├──────────────────────┤ ├──────────────────────────────────────────────────┤
│ │📁 Videos             │ │📄 log1.txt   │  2 KB│.txt  │2025-03-25         │
│ │📁 Photos             │ │📄 cache.tmp  │ 10 KB│.tmp  │2025-03-25         │
│ │🖼 cover.jpg          │ │                                                  │
│ └──────────────────────┘ └──────────────────────────────────────────────────┘
├─────────────────────────────────────────────────────────────────────────────┤
│ 선택: 2개 (8.5 KB)  │  전체: 1,024개  │  여유 공간: C: 45.2 GB            │  ← 상태바
└─────────────────────────────────────────────────────────────────────────────┘
```

**Q-Dir 레이아웃 핵심 특징:**
| 요소 | Q-Dir 실제 동작 | FolderDir 현재 상태 | 조치 필요 여부 |
|------|----------------|---------------------|---------------|
| 기본 분할 | 4분할(2×2), 동일 비율 | ✅ 동일 | 없음 |
| 탭바 위치 | 각 패널 상단 (파일뷰 위) | ✅ 동일 | 없음 |
| 브레드크럼 | 탭바 바로 아래 경로 세그먼트 버튼 | ✅ 구현됨 | 없음 |
| 드라이브바 | 메뉴바 바로 아래 가로 나열 | ✅ 구현됨 | 없음 |
| 파일 목록 | 이름·크기·유형·수정일 컬럼 | ✅ 구현됨 | 없음 |
| 더블클릭 동작 | 폴더 → 이동, 파일 → 기본 앱 실행 | ✅ **Sprint 5.5에서 수정됨** | 없음 |
| 클릭-재클릭 이름변경 | 선택된 항목 재클릭 시 인라인 편집 | ✅ **Sprint 5.5에서 수정됨** | 없음 |
| 상태바 | 선택 정보·전체 수·디스크 여유 | ✅ 구현됨 | 없음 |

---

#### 0.6.2 파일 목록 마우스 인터랙션 패턴 명세 (File List Mouse Interaction Spec)

Q-Dir과 Windows 탐색기가 공통으로 구현하는 표준 인터랙션 패턴을 기준으로 명세화한다.

##### A. 클릭 인터랙션 (Click Interactions)

| 동작 | 대상 | 결과 | 구현 방법 |
|------|------|------|-----------|
| 단일 클릭 | 파일/폴더 | 항목 선택 (포커스 이동) | `QAbstractItemView` 기본 동작 |
| 단일 클릭 | **이미 선택된** 파일/폴더 | **인라인 이름 변경 시작** | `EditTrigger::SelectedClicked` |
| 더블 클릭 | **폴더** | **해당 폴더로 이동** | `activated` 시그널 → `setPath()` |
| 더블 클릭 | **파일** | **기본 앱으로 파일 열기** | `activated` 시그널 → `QDesktopServices::openUrl()` |
| 우클릭 | 파일/폴더 | 컨텍스트 메뉴 표시 | `customContextMenuRequested` 시그널 |
| 우클릭 | 빈 영역 | 새 폴더·새 파일·붙여넣기 메뉴 | 위와 동일 |
| Ctrl+클릭 | 파일/폴더 | 다중 선택 추가 | `ExtendedSelection` 모드 기본 동작 |
| Shift+클릭 | 파일/폴더 | 범위 선택 | `ExtendedSelection` 모드 기본 동작 |

> **⚠️ 중요 구현 규칙**:  
> `DoubleClicked` EditTrigger를 **반드시 제거**해야 한다. 이 트리거가 활성화된 상태에서  
> 더블클릭하면 `activated` 시그널(폴더 이동)과 편집 위젯 열기가 동시에 발생하여  
> 시각적 깜빡임·비정상 동작이 나타난다.  
>  
> **올바른 EditTrigger 설정**: `SelectedClicked | EditKeyPressed`

##### B. 키보드 인터랙션 (Keyboard Interactions)

| 키 | 결과 |
|----|------|
| Enter / Return | 폴더 이동 또는 파일 열기 (`activated`와 동일) |
| F2 | 선택 항목 인라인 이름 변경 |
| Backspace | 상위 폴더로 이동 |
| Alt + ← | 뒤로 |
| Alt + → | 앞으로 |
| Alt + ↑ | 상위 폴더 |
| Space | 선택 항목 미리보기 토글 (macOS Quick Look 스타일) |
| Escape | 편집 취소 또는 필터바 닫기 |

##### C. 이름 변경 인터랙션 상세 (Rename Interaction Detail)

```
[사용자 액션 흐름]

1. 항목 첫 번째 클릭
   └─▶ 항목 선택됨 (파란색 하이라이트)
   
2. 이미 선택된 항목 두 번째 클릭 (500ms 이상 간격)
   └─▶ 인라인 편집 시작: 파일명 텍스트박스 활성화
       └─▶ 텍스트 전체 선택 (Ctrl+A 불필요)
       └─▶ 사용자 텍스트 입력
       └─▶ Enter → 이름 변경 확정
       └─▶ Escape → 이름 변경 취소
   
3. 더블클릭 (빠른 두 번 클릭 = 이름변경 아님)
   └─▶ 폴더: 해당 폴더로 이동
   └─▶ 파일: 기본 앱으로 열기

참고: Qt의 SelectedClicked 트리거는 내부적으로 더블클릭과 SelectedClicked를
자동으로 구분함 (타이머 기반). 별도 타이머 구현 불필요.
```

---

#### 0.6.3 기본 기능 버그 목록 및 즉시 수정 항목 (Critical Bug List & Immediate Fixes)

> 기획팀 4인 + 개발팀 리더 + 개발자 3인 공동 검수 결과 (2026-03-26)

| # | 버그/미구현 | 증상 | 원인 분석 | 수정 방법 |
|---|------------|------|-----------|-----------|
| BUG-001 | **더블클릭 이동 + 편집 충돌** | 폴더 더블클릭 시 이름 편집 위젯이 순간 표시되었다 사라짐 | `QTreeView/QListView` 기본 `EditTrigger`에 `DoubleClicked`가 포함되어 `activated` 시그널과 충돌 | `setEditTriggers(SelectedClicked \| EditKeyPressed)` |
| BUG-002 | **클릭-재클릭 이름변경 미구현** | 선택된 항목 재클릭 시 이름 변경 시작 안 됨 | `SelectedClicked` EditTrigger 미설정 | `setEditTriggers`에 `SelectedClicked` 추가 |

**수정 파일**: `src/FileSystemBrowser.cpp` — `setupUi()` 함수의 `m_detailsView`·`m_listView` 초기화 블록

```cpp
// ✅ 올바른 설정 (BUG-001·BUG-002 동시 해결)
m_detailsView->setEditTriggers(QAbstractItemView::SelectedClicked |
                               QAbstractItemView::EditKeyPressed);
m_listView->setEditTriggers(QAbstractItemView::SelectedClicked |
                            QAbstractItemView::EditKeyPressed);

// ❌ 기존 문제: Qt 기본값 (DoubleClicked | EditKeyPressed) — 명시적으로 재정의해야 함
```

---

#### 0.6.4 Sprint 5.5 — 기본 UX 버그 수정 (Basic UX Bug Fixes)

> 현재 Phase 5 Sprint 5까지 완료 후 Phase 6(Sprint 6~15)로 이어지는 계획에서,  
> 기획팀 4인 재검수 결과 기본 인터랙션 버그(BUG-001, BUG-002)가 발견됨.  
> Phase 6 시작 전 즉시 수정 착수.

| 스프린트 | 담당 | 작업 내용 | 상태 |
|---------|------|-----------|------|
| **Sprint 5.5 (긴급 버그 수정)** | B | BUG-001·BUG-002 수정: `FileSystemBrowser` EditTrigger 재설정 | ✅ **완료** |
| ~~Sprint 6~~ ✅ | B | 파일/폴더 색상 코딩 (ColorManager 신규 모듈) | ✅ **완료** |
| ~~Sprint 7~~ ✅ | A | 폴더 크기 컬럼 (백그라운드 스레드 계산) | ✅ **완료** |
| ~~Sprint 8~~ ✅ | B | 탭 드래그·패널 간 이동 | ✅ **완료** |

---

#### 0.6.5 개발팀 리더 + 팀원 3인 회의 결과 — 기본 UX 수정 (Dev Team Meeting Notes: 2026-03-26)

> 참여: 개발 리더 · 개발자 A · 개발자 B · 개발자 C

**회의 결론:**

1. **BUG-001·BUG-002 즉시 수정** — `FileSystemBrowser::setupUi()`에서 `QTreeView`·`QListView` 양쪽에 `setEditTriggers(SelectedClicked | EditKeyPressed)` 적용. 변경 범위 최소 (4줄 추가). 사이드이펙트 없음.

2. **검증 방법**:
   - 폴더 더블클릭 → 해당 폴더로 이동 확인 (편집 위젯 나타나지 않아야 함)
   - 항목 단일 클릭 → 선택 → 재클릭 → 인라인 편집 텍스트박스 표시 확인
   - F2 키 → 인라인 편집 텍스트박스 표시 확인 (기존 기능 보존)
   - Enter 키 → 폴더 이동 / 파일 열기 확인 (기존 기능 보존)

3. **아키텍처 영향도**: 없음 (단순 Qt 속성 설정).

4. **우선순위 재확인**: 기본 마우스 인터랙션은 모든 사용자가 매번 사용하는 1순위 기능. 색상 코딩·폴더 크기 등 고급 기능보다 먼저 수정 필수.

---

### 0.7 기획·리더 전수 재점검 결과 (PM & Leader Re-Audit — Sprint 8 완료 후)

> 작성 일자: 2026-03-26  
> 검토자: PM · UX 디자이너 · 개발 리더  
> 목적: Sprint 1~8 완료 후 §0.2 비교표의 **스테일(stale) 항목 전수 정리**, 잔여 격차(Gap) 재확인, 신규 발견 버그 목록화

---

#### 0.7.1 §0.2 비교표 업데이트 사항 요약 (Completed in Sprints 1~8)

아래는 이번 재점검에서 `❌`/`⚠️` → `✅`로 정정한 항목 목록이다.  
(§0.2 표에는 이미 직접 수정되어 있음.)

| 기능 | 이전 상태 | 수정 후 | 구현 위치 |
|------|-----------|---------|-----------|
| 브레드크럼 UI | ❌ | ✅ | `BreadcrumbBar` — Sprint 2 |
| 폴더 트리 사이드바 | ❌ | ✅ | `FolderTreePanel` — Sprint 3 |
| 탭 이름 변경 | ❌ | ✅ | `FolderPane::onTabDoubleClicked` — Sprint 5 |
| Ctrl+Tab 탭 전환 | ❌ | ✅ | `FolderPane::nextTab()` — Sprint 1 |
| 새 폴더 F7 단축키 | ⚠️ | ✅ | `FileSystemBrowser` + `MainWindow` — Sprint 1 |
| 속성 다이얼로그 (Alt+Enter) | ⚠️ | ✅ | `showProperties()` 전체 정보 — Sprint 1 |
| F3 파일 보기 | ❌ | ✅ | `openWithViewer()` — Sprint 1 |
| F4 파일 편집 | ❌ | ✅ | `openWithEditor()` — Sprint 1 |
| F6 이동 | ❌ | ✅ | `moveToPath()` in MainWindow — Sprint 4 |
| F9 이름 바꿔 복사 | ❌ | ✅ | `copyAndRename()` — Sprint 4 |
| F10 종료 | ❌ | ✅ | `onExit()` — Sprint 1 |
| 즐겨찾기 내보내기·가져오기 메뉴 UI | ⚠️ | ✅ | Bookmarks 메뉴 연결 — Sprint 5 |
| 파일/폴더 색상 코딩 | ❌ | ✅ | `ColorManager` + `ColorRulesDialog` — Sprint 6 |
| 폴더 크기 컬럼 계산 | ❌ | ✅ | `FolderSizeWorker` — Sprint 7 |
| 탭 드래그 — 패널 간 이동 | ❌ | ✅ | `DraggableTabBar` — Sprint 8 |

---

#### 0.7.2 잔여 격차 목록 (Remaining Gaps — Sprint 9+ 대상)

> 아래는 현재도 미구현 상태인 항목의 완전한 목록이다.

##### 🔴 핵심 버그·기능 누락 (즉시 처리 권고)

| # | 항목 | 현재 상태 | 권고 조치 |
|---|------|-----------|-----------|
| **~~BUG-003~~** ✅ | **F5 키 동작 충돌** — 파일 목록 포커스 시 F5가 `refresh()`를 호출하고 이벤트를 소비하여 MainWindow의 "Copy To" 핸들러에 도달하지 못함 | 수정 완료 | `FileSystemBrowser::keyPressEvent` 에서 F5 case 삭제; 새로고침은 Ctrl+R 전용. MainWindow에서 F5 = Copy To 단일화. |
| GAP-001 | **CRC 복사 검증 미구현** — 설정 다이얼로그에 체크박스 있으나 `FileOperations.cpp` 에 실제 검증 로직 없음 | 설정만 존재 | `QCryptographicHash` (SHA-1 또는 MD5)로 복사 후 검증 구현 |
| GAP-002 | **휴지통(Recycle Bin) 미구현** — 삭제 시 `QFile::remove()` 로 영구 삭제만 수행, `QFile::moveToTrash()` 미사용 | 영구 삭제 | `QFile::moveToTrash()` 사용하도록 FileOperations 수정; 설정에서 "영구 삭제 / 휴지통 이동" 선택 추가 |

##### 🟠 우선순위 2 잔여 항목 (Sprint 9~11 예정)

| # | 항목 | 현재 상태 |
|---|------|-----------|
| SP-9 | 패널 동기화 탐색 — 한 패널 이동 시 다른 패널 자동 동기화 | ❌ 미구현 |
| SP-9 | 패널 잠금 (Lock Pane) | ❌ 미구현 |
| SP-9 | 패널 복제 (Clone Pane) | ❌ 미구현 |
| SP-10 | 레이아웃 즐겨찾기 저장 (최대 64개) | ❌ 미구현 |
| SP-11 | PDF 미리보기 (Qt6::Pdf 모듈) | ❌ 미구현 |
| SP-11 | 정규식(Regex) 검색 | ❌ 미구현 |

##### 🟡 우선순위 3 잔여 항목 (Sprint 12~15 예정)

| # | 항목 | 현재 상태 |
|---|------|-----------|
| SP-12 | 포터블 모드 (INI 파일) | ❌ 미구현 |
| SP-13 | 한국어 번역 .ts/.qm 파일 | ❌ 미구현 |
| SP-14 | ZIP 내부 탐색 | ❌ 미구현 |
| SP-15 | 폴더 비교 | ❌ 미구현 |
| SP-15 | 디렉터리 목록 내보내기 (CSV·TXT) | ❌ 미구현 |
| — | 특수 폴더 빠른 접근 (Desktop·Docs·Downloads) | ❌ 미구현 |
| — | 파일 색상 필터 하이라이트 | ❌ 미구현 |
| — | 정규식(Regex) 검색 | ❌ 미구현 |
| — | 중복 파일 하이라이트 | ❌ 미구현 |
| — | 타일 보기 / 엑스트라 라지 아이콘 보기 | ❌ 미구현 |
| — | 컬럼 표시/숨김 UI | ❌ 미구현 |
| — | 패널 레이아웃 즐겨찾기 (64개) | ❌ 미구현 |
| — | 즐겨찾기 항목 사용자 정의 아이콘·이름 | ❌ 미구현 |
| — | 툴바 커스터마이즈 | ❌ 미구현 |
| — | 마우스 제스처 | ❌ 미구현 |
| — | 고대비(High Contrast) 모드 | ❌ 미구현 |
| — | FTP/SFTP 원격 탐색 | ❌ 미구현 (장기 과제) |

---

#### 0.7.3 §7.1 모듈 목록 정정 사항 (Module List Corrections)

- **`TabBar.h/cpp`** 항목 제거: 실제로 존재하지 않는 파일이었음. 탭바는 `FolderPane.h/cpp` 내부의 Qt `QTabBar` 및 `DraggableTabBar` (Sprint 8에서 `FolderPane.cpp`에 로컬 클래스로 구현) 로 처리됨.
- **`AddressBar.h/cpp`** 레거시 파일로 표시: `BreadcrumbBar`로 대체되었으나 파일이 남아 있음. 향후 정리 필요.
- Sprint 2~8에서 추가된 신규 파일 6개(`BreadcrumbBar`, `FolderTreePanel`, `ColorManager`, `ColorRulesDialog`, `FolderSizeWorker`, 그리고 드라이브바 `DriveBar`) 가 §7.1 현재 구현 모듈 목록에 반영됨.

---

#### 0.7.4 리더 최종 확인 체크리스트 (Leader Sign-off — Sprint 8 완료 시점)

- [ ] §0.2 비교표 업데이트 내용 (§0.7.1) 검토 완료
- [x] BUG-003 (F5 충돌) 수정 완료 — `FileSystemBrowser` F5 case 삭제, Ctrl+R로 통일
- [ ] GAP-001 (CRC 미구현) 처리 방향 확인 및 담당자 지정
- [ ] GAP-002 (휴지통 미구현) 처리 방향 확인 및 담당자 지정
- [ ] Sprint 9 착수 범위 (패널 동기화·잠금·복제) 승인
- [ ] **리더 서명**: _________________________  날짜: _____________

---

### 0.8 기획팀 UX 심층 감사 결과 (UX Deep-Audit — 2026-03-26)

> 작성 일자: 2026-03-26  
> 작성 팀: 기획팀 PM · UX 디자이너  
> 목적: 현재 구현 코드(`src/`)를 전수 분석하여 **사용자 체감 UX 관점**에서 누락·불일치·개선 가능 항목을 도출하고 즉시 수정할 것과 백로그에 추가할 것을 구분

---

#### 0.8.1 발견된 UX 문제 목록 (Findings)

| # | 구분 | 위치 | 문제 내용 | 심각도 | 상태 |
|---|------|------|-----------|--------|------|
| UX-001 | **레이아웃 오류** | `FolderPane::setupUi()` | 탭바가 브레드크럼 아래에 배치됨. Q-Dir 및 §0.6.1 레이아웃 기준서에 따르면 탭이 맨 위, 브레드크럼이 그 아래 위치해야 함. | 🔴 High | ✅ 수정 완료 |
| UX-002 | **상태바 미완성** | `MainWindow::updateStatusBar()` | `m_statusItems` 레이블이 생성되어 있으나 값이 채워지지 않음 — 현재 폴더의 항목 수가 상태바에 표시되지 않음 | 🔴 High | ✅ 수정 완료 |
| UX-003 | **보기 모드 전환 불가** | `MainWindow::setupMenuBar()` | `FileSystemBrowser`에 `setViewMode()` API가 있고 `ViewMode` 열거형도 정의되어 있으나, View 메뉴에 보기 모드 전환 항목이 없어 사용자가 Details/List/Icons/Thumbnails를 변경할 방법이 없음 | 🔴 High | ✅ 수정 완료 |
| UX-004 | **탭 닫기 버튼 항상 표시** | `FolderPane::setupUi()` | `setTabsClosable(true)` 고정 설정으로 탭이 1개일 때도 닫기 버튼이 표시됨. 클릭해도 아무 일이 없어 혼란을 유발. 브라우저 관례상 탭이 2개 이상일 때만 닫기 버튼 노출 | 🟠 Medium | ✅ 수정 완료 |
| UX-005 | **주소창 오류 피드백 부족** | `BreadcrumbBar::onReturnPressed()` | 존재하지 않는 경로 입력 시 빨간 테두리만 표시, 오류 이유를 설명하는 툴팁/문구 없음. 사용자가 왜 실패했는지 알 수 없음 | 🟠 Medium | ✅ 수정 완료 |
| UX-006 | **툴바 Refresh 버튼 없음** | `MainWindow::setupToolBar()` | 가장 자주 사용하는 작업 중 하나인 새로고침(Ctrl+R)이 툴바에 없음. 메뉴나 단축키를 모르면 접근 불가 | 🟠 Medium | ✅ 수정 완료 |
| UX-007 | **패널 전환 툴바 버튼 없음** | `MainWindow::setupToolBar()` | 1/2/3/4 패널 전환이 View > Layout 서브메뉴에만 있어 접근이 깊음. Q-Dir은 툴바에 1~4 버튼을 제공함 | 🟠 Medium | ✅ 수정 완료 |
| UX-008 | **About 다이얼로그 내용 시효 만료** | `MainWindow::setupMenuBar()` | "v1.0.2", "Phase 5 features"로 표기되어 있어 Sprint 8 이후 현재 상태를 반영하지 못함 | 🟡 Low | ✅ 수정 완료 |
| UX-009 | **탭 Ctrl+T 단축키 충돌 가능성** | `FolderPane::setupUi()` | "+" 버튼에 `setShortcut(Ctrl+T)`가 설정되어 있어 각 패널의 숨겨진 QPushButton이 Ctrl+T를 소비할 수 있음. MainWindow에서 Ctrl+T 미처리 시 예상치 못한 동작 가능 | 🟠 Medium | ✅ 수정 완료 (shortcut 제거, MainWindow 핸들러로 일원화) |
| UX-010 | **경로 변경 시 상태바 미갱신** | `MainWindow::setupConnections()` | `pathChanged` 시그널 연결에서 `updateStatusBar()`가 호출되지 않아 폴더 이동 시 `m_statusItems` 항목 수가 갱신되지 않음 | 🔴 High | ✅ 수정 완료 |

---

#### 0.8.2 즉시 수정 결과 요약 (Fixes Applied)

| 수정 파일 | 수정 내용 |
|-----------|-----------|
| `src/FolderPane.cpp` | UX-001: 레이아웃 순서 변경 — tabWidget 먼저, m_addressBar 두 번째 |
| `src/FolderPane.cpp` | UX-004: `setTabsClosable(false)` 기본값 + `updateTabCloseButtons()` 추가 — 탭 수에 따라 동적 제어 |
| `src/FolderPane.cpp` | UX-009: "+" 버튼 `setShortcut` 제거 |
| `src/FolderPane.h` | `updateTabCloseButtons()` 메서드 선언 추가 |
| `src/BreadcrumbBar.cpp` | UX-005: 유효하지 않은 경로 입력 시 `setToolTip("Path does not exist: ...")` 추가; 성공 시 툴팁 초기화 |
| `src/MainWindow.cpp` | UX-002 + UX-010: `updateStatusBar()`에 항목 수 계산 추가; `pathChanged` 시그널에서 `updateStatusBar()` 호출 |
| `src/MainWindow.cpp` | UX-003: View 메뉴에 "View Mode" 서브메뉴 추가 (Details / List / Icons / Thumbnails, QActionGroup 배타 선택) |
| `src/MainWindow.cpp` | UX-006: 툴바에 Refresh 버튼 추가 (Ctrl+R) |
| `src/MainWindow.cpp` | UX-007: 툴바에 패널 전환 버튼 1/2/3/4 추가 (Ctrl+F1~F4 단축키) |
| `src/MainWindow.cpp` | UX-008: About 다이얼로그 버전/기능 목록 업데이트 (v1.1.0, Sprint 8 반영) |
| `src/MainWindow.h` | `onSetViewMode(ViewMode)` 슬롯, 뷰모드 액션 멤버, `#include QActionGroup` 추가 |

---

#### 0.8.3 백로그 — 추가 UX 개선 사항 (Backlog Items, Sprint 9+)

> 이번 감사에서 발견됐으나 즉시 수정 범위를 벗어나는 항목 (별도 스프린트 계획 필요)

| # | 항목 | 설명 | 권고 스프린트 |
|---|------|------|---------------|
| UX-B01 | **뷰 모드 퍼-패널 독립성** | 현재 View Mode 메뉴는 모든 패널에 동시 적용됨. Q-Dir처럼 패널별 독립 뷰 모드 선택이 필요 (각 패널 우클릭 컨텍스트 메뉴 또는 툴바에 드롭다운 추가) | ✅ 수정 완료 |
| UX-B02 | **컬럼 표시/숨김 UI** | 상세 보기에서 이름·크기·유형·수정일 컬럼을 사용자가 헤더 우클릭으로 숨기거나 표시할 수 없음 | ✅ 수정 완료 |
| UX-B03 | **탭에 파일 경로 아이콘** | 탭 레이블에 폴더 아이콘(QFileIconProvider)을 표시하면 다수 탭 구분이 용이 | ✅ 수정 완료 |
| UX-B04 | **드래그 앤 드롭 시각 피드백** | 파일을 다른 패널로 드래그할 때 드롭 가능 영역 하이라이트가 없음. QDrag 픽스맵과 `dragMoveEvent` 스타일 추가 필요 | ✅ 수정 완료 |
| UX-B05 | **북마크 사이드바 드래그 순서 변경** | `QListWidget` 기반 북마크 사이드바가 드래그 재정렬을 지원하지 않음 (`DragDropMode::InternalMove` 미설정) | ✅ 수정 완료 |
| UX-B06 | **특수 폴더 빠른 접근** | 폴더 트리 상단에 Desktop·Documents·Downloads 고정 항목 추가 (`QStandardPaths` 활용) | ✅ 수정 완료 |
| UX-B07 | **패널 전환 시 포커스 표시** | 활성 패널 border 색은 있으나 비활성 상태에서 어느 패널이 활성인지 초기 시작 시 명확하지 않음 — 초기 로드 시 `pane[0]->setActive(true)` 호출 필요 | ✅ 수정 완료 |
| UX-B08 | **패널 1개 모드에서 레이아웃 버튼 상태** | 툴바의 패널 수 버튼이 현재 상태를 반영하는 시각적 활성 상태(checked/pressed)가 없음 | ✅ 수정 완료 |

> **UX 백로그 전 항목 완료 — Sprint 9 종료**

---

#### 0.8.5 추가 개선 사항 수집 Pass 1 (Sprint 9 — 2026-03-26)

> 백로그 UX-B03/B05/B06/B08 를 Sprint 9에서 구현 완료

| 수정 파일 | 수정 내용 |
|-----------|-----------|
| `src/MainWindow.h` | `m_actPane1~4` 멤버 추가, `m_bookmarkList` 멤버 추가, `QListWidget` include |
| `src/MainWindow.cpp` | UX-B08: 패널 버튼 `QActionGroup` 배타 그룹 + `setCheckable(true)`; `applyLayout()` 에서 `setChecked` 동기화 |
| `src/MainWindow.cpp` | UX-B05: `m_bookmarkList` 드래그 모드 `InternalMove`; `rowsMoved` → `BookmarkManager::move()` 연결 |
| `src/FolderPane.cpp` | UX-B03: `addTabInternal()` 및 `onBrowserPathChanged()` 에서 `QFileIconProvider`로 탭 아이콘 설정 |
| `src/FolderTreePanel.h` | UX-B06: `m_quickAccess(QListWidget)` 멤버 추가, `buildQuickAccess()` 선언 |
| `src/FolderTreePanel.cpp` | UX-B06: "Quick Access" 섹션 (Home/Desktop/Documents/Downloads/Music/Pictures/Videos) 폴더 트리 상단에 추가 |

#### 0.8.6 추가 개선 사항 수집 Pass 2 (Sprint 9 — 2026-03-26)

> 백로그 UX-B01/B02/B04 를 Sprint 9에서 구현 완료 — 전체 UX 백로그 종료

| 수정 파일 | 수정 내용 |
|-----------|-----------|
| `src/FolderPane.h` | UX-B01: `viewMode()`/`setViewMode()` 공개 API, `m_viewModeBtn`, `m_viewMode`, `m_dropHighlight` 멤버 추가; `dragLeaveEvent` override 추가 |
| `src/FolderPane.cpp` | UX-B01: `setupUi()` 에 뷰 모드 드롭다운 버튼(`QToolButton + QMenu`) 추가; `setViewMode()`, `syncViewModeButton()` 구현; 새 탭 생성 시 현재 뷰 모드 자동 적용 |
| `src/FolderPane.cpp` | UX-B04: `dragEnterEvent` 에서 `text/uri-list` MIME 수락 추가; `dragLeaveEvent` 추가; `setActiveStyle()` 에 드롭 하이라이트(파란 점선) 분기 추가 |
| `src/MainWindow.cpp` | UX-B01: `onSetViewMode()` 를 전체 패널 → 활성 패널 전용으로 변경 |
| `src/FileSystemBrowser.h` | UX-B02: `onHeaderContextMenu(const QPoint&)` 슬롯 선언 |
| `src/FileSystemBrowser.cpp` | UX-B02: 헤더에 `Qt::CustomContextMenu` 정책 설정; `onHeaderContextMenu()` 구현 (Size/Type/Date Modified 체크 토글) |

---

### 0.9 기획·개발팀 재편 및 Sprint 10 착수 (Team Re-formation & Sprint 10 — 2026-03-31)

> 작성 일자: 2026-03-31  
> 작성 팀: 기획팀 (PM · UX 디자이너 · BA · QA) + 개발팀 (리더 · 개발자 A/B/C)  
> 참고 문서: `docs/team.md` (팀 구성·협업 프로세스 신규 명문화)

---

#### 0.9.1 기획팀 현황 감사 (Planning Team Audit — Sprint 9 이후)

Sprint 9 완료 기준 전수 재점검 결과:

| 구분 | 항목 | 이전 상태 | 현재 상태 |
|------|------|-----------|-----------|
| SP-9 | 패널 동기화 탐색 | ❌ | ✅ `m_paneSyncEnabled` + `onTogglePaneSync()` — `MainWindow` |
| SP-9 | 패널 잠금 (Lock Pane) | ❌ | ✅ `FolderPane::setLocked()` + `onLockPane()` — `MainWindow` |
| SP-9 | 패널 복제 (Clone Pane) | ❌ | ✅ `onClonePane()` — `MainWindow` |
| GAP-001 | CRC 복사 검증 | ⚠️ 설정만 | ✅ `FileOperation::fileHash()` (SHA-256) + `m_verifyChecksum` |
| GAP-002 | 휴지통 이동 | ⚠️ 영구삭제만 | ✅ `QFile::moveToTrash()` + `m_useTrash` — `FileOperations.cpp` |
| UX-B01~B08 | UX 백로그 전체 | 부분 | ✅ Sprint 9에서 전 항목 완료 |

##### 기획팀 발견 신규 문제 (New Issues — Sprint 9 이후)

| ID | 항목 | 심각도 | 설명 |
|----|------|--------|------|
| UX-C01 | **레이아웃 프리셋 저장 UI 없음** | 🟠 High | 패널 수·경로 조합을 자주 바꾸는 사용자가 매번 수동 재구성해야 함. "즐겨찾기 레이아웃" 저장/불러오기 기능 요청 다수. |
| UX-C02 | **레이아웃 메뉴 반응성** | 🟡 Medium | 레이아웃 프리셋 메뉴가 동적으로 갱신되어야 함 (저장 즉시 메뉴에 반영). |

#### 0.9.2 개발팀 회의 결과 (Dev Team Decision — Sprint 10)

> 회의 일자: 2026-03-31  
> 참여: 개발 리더 · 개발자 C (Architecture)

**Sprint 10 목표**: `LayoutManager` 신규 모듈 구현 — 레이아웃 즐겨찾기 저장/불러오기 (최대 10개 프리셋)

##### 아키텍처 결정

1. `LayoutPreset` 구조체: 이름·패널 수·패널별 탭 경로 목록 저장
2. `LayoutManager` 클래스: `QSettings` 의 `Layouts` 그룹에 영속화; `presetsChanged()` 시그널로 메뉴 동적 갱신
3. `MainWindow` 통합: Tools > Layout Presets 서브메뉴, `Ctrl+Shift+S` 단축키로 저장
4. 기존 세션 저장(`saveSession()`)과 별도 관리하여 충돌 방지

#### 0.9.3 Sprint 10 구현 내역 (Sprint 10 Implementation)

##### 신규 파일

| 파일 | 내용 |
|------|------|
| `src/LayoutManager.h` | `LayoutPreset` 구조체 + `LayoutManager` 클래스 선언 (최대 10 프리셋) |
| `src/LayoutManager.cpp` | `loadAll()` / `saveAll()` / `save()` / `remove()` / `find()` 구현 |
| `docs/team.md` | 기획팀·개발팀 구성, 역할·책임, 협업 프로세스 명문화 |

##### 수정 파일

| 파일 | 수정 내용 |
|------|-----------|
| `CMakeLists.txt` | `LayoutManager.h/cpp` SOURCES·HEADERS 목록에 추가 |
| `src/MainWindow.h` | `LayoutManager *m_layoutManager`, `QMenu *m_layoutPresetsMenu` 멤버 추가; SP-10 슬롯 4개 선언 |
| `src/MainWindow.cpp` | `LayoutManager` 인스턴스 생성; Tools 메뉴에 "Layout Presets" 서브메뉴 추가; `onSaveLayoutPreset()` / `onLoadLayoutPreset()` / `onDeleteLayoutPreset()` / `rebuildLayoutPresetsMenu()` 구현 |

##### 사용자 흐름 (User Story)

1. 사용자가 4패널에서 각 패널에 자주 쓰는 폴더를 열어 놓은 상태
2. **Tools > Layout Presets > Save Current Layout…** 클릭 (또는 `Ctrl+Shift+S`)
3. 이름 입력 다이얼로그 → "Work Setup" 입력 → OK
4. 메뉴에 "Work Setup" 항목 즉시 추가됨
5. 패널 배치를 바꾼 후 **Tools > Layout Presets > Work Setup** 클릭 → 저장된 경로·패널 수 복원
6. "Delete" 항목으로 불필요한 프리셋 삭제 가능

#### 0.9.4 리더 최종 확인 체크리스트 (Leader Sign-off — Sprint 10)

- [x] Sprint 9 완료 확인 (패널 동기화·잠금·복제, UX 백로그 전체)
- [x] GAP-001 (CRC) / GAP-002 (휴지통) 완료 확인
- [x] Sprint 10 `LayoutManager` 구현 완료
- [x] `docs/team.md` 팀 구성 문서 신규 작성
- [ ] Sprint 11 착수 범위 (PDF 미리보기·정규식 검색) 승인
- [ ] **리더 서명**: _________________________  날짜: _____________

---

| 항목 | 내용 |
|------|------|
| 프로젝트명 | FolderDir |
| 목표 | Q-Dir 수준의 멀티패널 파일 탐색기 C++ 구현 |
| 플랫폼 | Windows / Linux / macOS (Qt 크로스플랫폼) |
| 언어 | C++17 |
| UI 프레임워크 | Qt 6 (Qt 5.15 호환 유지) |
| 빌드 시스템 | CMake 3.16+ |

---

## 2. 핵심 기능 명세 (Core Feature Specification)

### 2.1 멀티패널 레이아웃 (Multi-Pane Layout)

- **패널 구성**: 1 / 2(수직·수평) / 3 / 4 패널 전환 가능
- **기본 레이아웃**: 4분할(상단 2 + 하단 2), 드래그로 비율 조절
- **패널 독립성**: 각 패널은 별도 경로, 보기 모드, 필터를 유지
- **패널 동기화 옵션**: 특정 패널들 사이에서 디렉터리를 동기화 탐색 가능

### 2.2 파일 탐색 (File Navigation)

- **주소 표시줄**: 각 패널 상단에 편집 가능한 경로 입력창 (`breadcrumb` 스타일 + 텍스트 편집 모드 토글)
- **히스토리 탐색**: 뒤로(Alt+←) / 앞으로(Alt+→) / 위로(Alt+↑)
- **트리 뷰**: 좌측 사이드바에 폴더 트리 (선택적 표시)
- **즐겨찾기 패널**: 즐겨찾기 폴더 빠른 접근 사이드바
- **드라이브 표시줄**: 상단에 드라이브/볼륨 버튼 나열 (클릭 시 해당 루트로 이동)

### 2.3 파일 보기 모드 (View Modes)

| 모드 | 설명 |
|------|------|
| 아이콘 보기 | 크기 조절 가능한 아이콘 + 파일명 |
| 목록 보기 | 작은 아이콘 + 파일명 (다열) |
| 상세 보기 | 이름·크기·유형·수정일 컬럼 정렬·조절 가능 |
| 썸네일 보기 | 이미지·문서 미리보기 썸네일 |

- 각 패널별 독립 보기 모드 설정
- 컬럼 헤더 클릭으로 정렬 (이름·크기·유형·날짜)
- 숨김 파일/폴더 표시 토글

### 2.4 탭 지원 (Tab Support per Pane)

- 각 패널에 여러 탭 보유 (브라우저 탭 UX)
- 탭 드래그로 패널 간 이동
- 탭 우클릭 → 탭 복사·닫기·이름 변경
- Ctrl+T: 새 탭 / Ctrl+W: 탭 닫기

### 2.5 파일 작업 (File Operations)

| 작업 | 단축키 | 설명 |
|------|--------|------|
| 복사 | Ctrl+C / F5 | 선택 항목 복사 |
| 잘라내기 | Ctrl+X | 이동용 선택 |
| 붙여넣기 | Ctrl+V / F6 | 대상 폴더에 붙여넣기 |
| 삭제 | Delete / F8 | 휴지통 또는 영구 삭제 |
| 이름 변경 | F2 | 인라인 편집 |
| 새 폴더 생성 | F7 / Ctrl+Shift+N | 현재 패널 위치에 폴더 생성 |
| 새 파일 생성 | Ctrl+N | 빈 파일 생성 |
| 속성 보기 | Alt+Enter | 파일/폴더 속성 대화상자 |
| 경로 복사 | Ctrl+Shift+C | 선택 항목의 전체 경로 클립보드 복사 |

- **진행 표시 다이얼로그**: 대용량 복사·이동 시 속도·남은 시간 표시 + 취소 버튼
- **충돌 처리**: 덮어쓰기 / 이름 변경 / 건너뛰기 선택 다이얼로그
- **패널 간 드래그 앤 드롭**: 복사(기본) / 이동(Shift+드래그) / 링크(Ctrl+Shift+드래그)

### 2.6 검색 및 필터 (Search & Filter)

- **패널 내 빠른 필터**: 패널 하단 필터바에 와일드카드 입력 (예: `*.cpp`)
- **전체 검색**: Ctrl+F → 이름·내용·날짜·크기 조건 복합 검색
- **검색 결과 패널**: 검색 결과를 별도 탭에 표시

### 2.7 미리 보기 패널 (Preview Panel)

- 우측 또는 하단에 선택 파일 미리 보기
- 지원 형식: 이미지(JPEG·PNG·BMP·GIF·SVG), 텍스트, PDF (Qt PDF 모듈)
- 단축키: Ctrl+P로 미리 보기 패널 토글

### 2.8 즐겨찾기 / 북마크 (Bookmarks)

- Ctrl+D: 현재 경로를 즐겨찾기에 추가
- 즐겨찾기 사이드바에서 드래그로 순서 변경
- 즐겨찾기 내보내기 / 가져오기 (JSON 형식)
- 각 항목에 사용자 정의 아이콘 및 이름 설정

### 2.9 세션 및 설정 (Session & Settings)

- **세션 저장**: 종료 시 각 패널의 경로·탭·보기 모드 저장 → 다음 실행 시 복원
- **설정 다이얼로그**: 일반·외관·파일 작업·단축키·플러그인 탭
- **테마**: 라이트 / 다크 / 시스템 따름
- **언어**: 한국어 / 영어 (i18n 지원)
- **설정 저장소**: `QSettings` (플랫폼 기본 레지스트리 또는 INI 파일)

### 2.10 상태 표시줄 (Status Bar)

- 선택 항목 수, 총 크기
- 현재 폴더 내 항목 수
- 여유 디스크 공간
- 복사·붙여넣기 진행 중 상태

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 3.1 성능

- 10만 개 이상 파일이 있는 폴더도 **지연 로딩**으로 빠른 표시 (가상화 모델 활용)
- 썸네일은 **백그라운드 스레드**에서 비동기 생성 (UI 블로킹 없음)
- 파일 작업은 **워커 스레드** 분리, UI 스레드 차단 없음
- 시작 시간: 콜드 스타트 2초 이내

### 3.2 안정성

- 파일 작업 중 예외 발생 시 원본 데이터 보존 (롤백 가능한 단계 설계)
- 무결성: 복사 후 CRC 검증 옵션
- 잘못된 경로 입력 시 명확한 오류 메시지 표시 (크래시 없음)

### 3.3 접근성

- 키보드만으로 모든 기능 사용 가능
- 고대비(High Contrast) 모드 지원
- 스크린 리더 지원 (QAccessible)

---

## 4. 화면 설계 (Screen Design)

```
┌─────────────────────────────────────────────────────────────────────────┐
│ [File] [Edit] [View] [Tools] [Bookmarks] [Help]          [⚙] [🌙/☀]    │  ← Menu bar
├─────────────────────────────────────────────────────────────────────────┤
│ [↑][←][→] [Drive: C:▾][D:▾][E:▾]  [🔍 Search] [Bookmark★] [Preview◧] │  ← Toolbar
├────────────────────────────┬────────────────────────────────────────────┤
│ ▾ 🖥 This PC               │ ▾ 🖥 This PC                               │
│   📁 Desktop               │   📁 Documents                            │  ← Pane 1 (left top) | Pane 2 (right top)
│   📁 Documents             │                                           │
│ ──────────────────────     │                                           │
│ [Tab1: C:\Users] [Tab2+]   │ [Tab1: D:\Projects] [Tab2+]              │
│ ┌ Path: C:\Users\_______┐  │ ┌ Path: D:\Projects\__________┐         │
│ │ Name    │Size│Type │Date│ │ │ Name     │Size│Type│Date    │         │
│ │📁 Alice │  - │Dir  │... │ │ │📁 AppA   │  - │Dir │...    │         │
│ │📁 Bob   │  - │Dir  │... │ │ │📁 AppB   │  - │Dir │...    │         │
│ │📄 file  │12K │.txt │... │ │ │📄 README │ 4K │.md │...    │         │
│ └────────────────────────┘ │ └──────────────────────────────┘         │
├────────────────────────────┼────────────────────────────────────────────┤
│ [Tab1: E:\Media] [Tab2+]   │ [Tab1: C:\tmp] [Tab2+]                   │  ← Pane 3 (left bottom) | Pane 4 (right bottom)
│ ┌ Path: E:\Media\_______┐  │ ┌ Path: C:\tmp\________________┐         │
│ │ Name    │Size│Type │Date│ │ │ Name     │Size│Type│Date    │         │
│ │🖼 img1  │ 5M │.jpg │... │ │ │📄 log.txt│ 1K │.txt│...    │         │
│ └────────────────────────┘ │ └──────────────────────────────┘         │
├─────────────────────────────────────────────────────────────────────────┤
│ 3 items selected (12.5 MB) │ 1,024 items │ Free: 45.2 GB              │  ← Status bar
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 5. 단축키 일람 (Keyboard Shortcuts)

| 단축키 | 기능 |
|--------|------|
| F2 | 이름 변경 |
| F3 | 파일 보기 |
| F4 | 파일 편집 |
| F5 | 복사 |
| F6 | 이동 |
| F7 | 새 폴더 |
| F8 / Delete | 삭제 |
| F9 | 이름 바꿔 복사 |
| F10 | 종료 |
| Alt + ← | 뒤로 |
| Alt + → | 앞으로 |
| Alt + ↑ | 상위 폴더 |
| Alt + Enter | 속성 |
| Ctrl + A | 전체 선택 |
| Ctrl + C | 복사 |
| Ctrl + X | 잘라내기 |
| Ctrl + V | 붙여넣기 |
| Ctrl + D | 즐겨찾기 추가 |
| Ctrl + F | 검색 |
| Ctrl + H | 숨김 파일 토글 |
| Ctrl + N | 새 파일 |
| Ctrl + Shift + N | 새 폴더 |
| Ctrl + P | 미리보기 패널 |
| Ctrl + T | 새 탭 |
| Ctrl + W | 탭 닫기 |
| Ctrl + Tab | 다음 탭 |
| Ctrl + 1~4 | 패널 1~4 활성화 |
| Ctrl + Shift + C | 경로 복사 |

---

## 6. 기술 스택 결정 (Technology Stack)

| 구성 요소 | 채택 기술 | 이유 |
|-----------|-----------|------|
| GUI 프레임워크 | Qt 6 (Qt5.15 호환) | 크로스플랫폼, 성숙한 파일시스템 API |
| 빌드 시스템 | CMake 3.16+ | 업계 표준, CLion/VSCode 통합 |
| 파일 모델 | QFileSystemModel + 커스텀 프록시 | 가상화 지원, Qt 내장 |
| 썸네일 | QImageReader + QThreadPool | 비동기 비차단 로딩 |
| 설정 | QSettings | 플랫폼 통합, JSON 확장 가능 |
| 파일 작업 | QFile + QtConcurrent | 스레드 안전, 진행 추적 |
| 아이콘 | QFileIconProvider + 커스텀 SVG | 플랫폼 네이티브 아이콘 |
| 언어 | C++17 | 구조화된 바인딩, std::filesystem 활용 |

---

## 7. 모듈 구조 (Module Structure)

### 7.1 현재 구현 모듈 (Existing — as of Sprint 8)

```
src/
├── main.cpp                   # 진입점
├── MainWindow.h/cpp           # 메인 윈도우 (패널 배치, 메뉴, 툴바)
├── FolderPane.h/cpp           # 개별 패널 (BreadcrumbBar + 탭바 + 파일뷰)
│                              #   └─ DraggableTabBar (embedded) — Sprint 8
├── FileSystemBrowser.h/cpp    # 파일 목록 뷰 (트리/리스트/아이콘/썸네일)
├── FileSystemModel.h/cpp      # QFileSystemModel 래퍼 (색상 코딩·폴더 크기 통합)
├── BreadcrumbBar.h/cpp        # 클릭 가능 경로 세그먼트 주소창 — Sprint 2
├── FolderTreePanel.h/cpp      # 폴더 트리 사이드바 (DockWidget) — Sprint 3
├── ColorManager.h/cpp         # 파일/폴더 색상 코딩 규칙 관리 — Sprint 6
├── ColorRulesDialog.h/cpp     # 색상 규칙 편집 다이얼로그 — Sprint 6
├── FolderSizeWorker.h/cpp     # 폴더 크기 비동기 계산 (QtConcurrent) — Sprint 7
├── AddressBar.h/cpp           # (레거시 — BreadcrumbBar로 대체됨, 미사용)
├── FileOperations.h/cpp       # 복사·이동·삭제·이름변경 (스레드)
├── FileOperationDialog.h/cpp  # 파일 작업 진행 다이얼로그
├── BookmarkManager.h/cpp      # 즐겨찾기 관리 (JSON 내보내기·가져오기)
├── DriveBar.h/cpp             # 드라이브/볼륨 버튼 바
├── PreviewPanel.h/cpp         # 미리보기 패널
├── SettingsManager.h/cpp      # 설정 관리
├── SettingsDialog.h/cpp       # 설정 다이얼로그 UI
└── SearchDialog.h/cpp         # 파일 검색 다이얼로그
```

### 7.2 신규 추가 모듈 현황 (New Modules — Done vs. Planned)

```
src/
# ── Sprint 2-9 에서 이미 구현 완료 ─────────────────────────────────────────
├── BreadcrumbBar.h/cpp        ✅ 완료 — 클릭 가능 경로 세그먼트 주소창
├── FolderTreePanel.h/cpp      ✅ 완료 — 폴더 트리 사이드바 (DockWidget)
├── ColorManager.h/cpp         ✅ 완료 — 파일/폴더 색상 코딩 규칙
├── ColorRulesDialog.h/cpp     ✅ 완료 — 색상 규칙 편집 다이얼로그
├── FolderSizeWorker.h/cpp     ✅ 완료 — 폴더 크기 비동기 계산
├── LayoutManager.h/cpp        ✅ 완료 — 레이아웃 즐겨찾기 저장·불러오기 (Sprint 10)

# ── Sprint 11~15 에서 구현 예정 ──────────────────────────────────────────
├── ZipBrowser.h/cpp           🔲 예정 — ZIP 파일 내부 탐색 (Sprint 14)
├── ExportDialog.h/cpp         🔲 예정 — 디렉터리 목록 내보내기 CSV·TXT (Sprint 15)
└── FolderCompareDialog.h/cpp  🔲 예정 — 폴더 비교 다이얼로그 (Sprint 15)
```

---

## 8. 개발 로드맵 (Development Roadmap)

### 8.1 완료된 단계 (Completed Phases)

| 단계 | 내용 | 상태 |
|------|------|------|
| Phase 1 | 기본 멀티패널 + 파일 탐색 + 상세 보기 | ✅ 완료 |
| Phase 2 | 탭 지원 + 파일 작업 (복사·이동·삭제) + 주소창 | ✅ 완료 |
| Phase 3 | 즐겨찾기 + 검색 + 필터 + 설정 | ✅ 완료 |
| Phase 4 | 썸네일 + 미리보기 패널 + 드라이브바 + 비동기 썸네일 | ✅ 완료 |

### 8.2 신규 단계 — Q-Dir 고도화 (New Phases: Q-Dir Parity)

> §0.3·§0.4 기획·개발팀 합의 결과를 반영한 우선순위 기반 계획

| 단계 | 내용 | 우선순위 | 담당 |
|------|------|---------|------|
| **Phase 5** | **[🔴 필수] F-키 완성·Ctrl+Tab·Alt+Enter 속성·브레드크럼·폴더 트리 사이드바** | 🔴 Critical | Dev B |
| **Phase 6** | **[🔴 필수] F6 이동 다이얼로그·F9 이름 바꿔 복사·CRC 검증·즐겨찾기 내보내기 UI** | 🔴 Critical | Dev A |
| **Phase 7** | **[🟠 중요] 파일/폴더 색상 코딩·폴더 크기 컬럼·탭 드래그 패널 간 이동·탭 이름 변경** | 🟠 High | Dev A+B |
| **Phase 8** | **[🟠 중요] 패널 동기화·패널 잠금/복제·레이아웃 즐겨찾기·PDF 미리보기·정규식 검색** | 🟠 High | Dev C |
| **Phase 9** | **[🟡 선택] ZIP 탐색·폴더 비교·디렉터리 내보내기·포터블 모드·한국어 번역** | 🟡 Medium | Dev A+C |
| **Phase 10** | **[🟡 선택] 툴바 커스터마이즈·외부 도구 등록·마우스 제스처·고대비·접근성** | 🟡 Low | Dev B+C |

### 8.3 Phase 5 상세 작업 항목 (Phase 5 Detail)

> 리더 컨펌 후 즉시 착수 (Sprint 1~3)

#### Sprint 1 — 단축키 완성 (Dev B) ✅ 완료
- [x] F3 — 기본 파일 뷰어 연동 (`openWithViewer()`: QDesktopServices)
- [x] F4 — 기본 텍스트 편집기 연동 (`openWithEditor()`: 플랫폼별 에디터 자동 선택)
- [x] F5 — 복사 다이얼로그 (`copyToPath()`: 대상 경로 입력, 다른 패널 경로 기본값)
- [x] F6 — 이동 다이얼로그 (`moveToPath()`: 대상 경로 입력)
- [x] F7 — 새 폴더 (MainWindow::keyPressEvent → onNewFolder())
- [x] F9 — 이름 바꿔 복사 다이얼로그 (`copyAndRename()`)
- [x] F10 — 애플리케이션 종료
- [x] Ctrl+Tab — 현재 패널 다음 탭으로 이동 (`FolderPane::nextTab()`)
- [x] Ctrl+W — 탭 닫기 (MainWindow::keyPressEvent에 추가)
- [x] Alt+Enter — 파일/폴더 속성 다이얼로그 (전체 정보·권한, `showProperties()` QDialog)

#### Sprint 2 — 브레드크럼 주소창 (Dev B) ✅ 완료
- [x] `BreadcrumbBar` 클래스 신규 작성 (`src/BreadcrumbBar.h/cpp`)
- [x] 경로 세그먼트를 버튼으로 렌더링, 클릭 시 해당 경로로 이동
- [x] 텍스트 편집 모드 토글 (브레드크럼 ↔ 텍스트 입력 전환: 더블클릭 또는 ✎ 버튼)
- [x] Escape 키로 편집 취소, Enter/Go 버튼으로 이동 확정
- [x] `FolderPane` 에서 `AddressBar` 를 `BreadcrumbBar` 로 교체 (동일 인터페이스)

#### Sprint 3 — 폴더 트리 사이드바 (Dev B) ✅ 완료
- [x] `FolderTreePanel` 클래스 작성 (`src/FolderTreePanel.h/cpp`)
- [x] `QFileSystemModel` (디렉터리 전용) 기반 계층 트리 표시
- [x] 트리 항목 클릭 시 활성 패널 이동
- [x] 트리 자동 확장/선택 동기화 (`setActivePath()`)
- [x] View 메뉴에 "Folder Tree" 토글 항목 추가 (Ctrl+Shift+T)
- [x] 패널 전환 시 트리 자동 동기화 (`onActivePaneChanged`)

#### Sprint 4 & 5 — 추가 개선 (Dev A) ✅ 완료
- [x] F6 이동·F9 복사+이름변경 다이얼로그 (FileSystemBrowser 레벨)
- [x] 즐겨찾기 내보내기·가져오기 메뉴 UI (Bookmarks 메뉴에 추가)
- [x] 탭 이름 변경 (더블클릭 또는 우클릭 컨텍스트 메뉴)

#### Sprint 5.5 — 긴급 기본 UX 버그 수정 (Dev B) ✅ 완료
- [x] BUG-001 수정: `QTreeView`·`QListView` 에서 `DoubleClicked` EditTrigger 제거
  → 폴더 더블클릭 시 이름편집 위젯 충돌 없이 해당 폴더로 이동
- [x] BUG-002 수정: `SelectedClicked` EditTrigger 추가
  → 이미 선택된 항목 재클릭 시 인라인 이름 변경 시작 (Windows 탐색기 동일 동작)
- [x] 두 변경 모두 `FileSystemBrowser::setupUi()` 내 4줄 추가로 최소 범위 수정

---

## 0.11 Sprint 11 — 워크스페이스 탭 & Windows SmartScreen 대응 (Dev C) ✅ 완료

> 작성: 2026-03-31

### 변경 내용 요약

#### 1. Windows SmartScreen 완화 — 앱 매니페스트 추가
- `resources/FolderDir.manifest` 신규 작성
  - `requestedExecutionLevel level="asInvoker"` — 불필요한 UAC 상승 방지
  - `compatibility` 블록 — Windows 7/8/8.1/10/11 명시 지원 선언
  - `dpiAwareness` PerMonitorV2 — 고해상도 모니터 대응
  - `longPathAware` true — 긴 경로명 지원
- `resources/app.rc` 에 `1 RT_MANIFEST "FolderDir.manifest"` 추가 → 실행 파일 내 매니페스트 임베딩

#### 2. 워크스페이스 탭 (Top-level Workspace Tabs)
- **`WorkspaceWidget`** 신규 클래스 (`src/WorkspaceWidget.h/cpp`)
  - 기존 2×2 QSplitter 레이아웃 + FolderPane 4개를 캡슐화
  - 독립적인 패널 수 설정 (1/2/3/4)
  - 워크스페이스별 패널 동기화 (SP-9 per-workspace)
  - 세션 저장/복원 (`saveToSettings`, `restoreFromSettings`)
  - 레이아웃 프리셋 복원 (`restoreFromPreset`)
- **`MainWindow`** 수정
  - 기존 고정 4-패널 레이아웃 → `QTabWidget` (워크스페이스 탭) 으로 교체
  - 각 탭이 독립적인 `WorkspaceWidget` 을 가짐
  - 상단 우측 "+" 버튼으로 새 워크스페이스 탭 추가
  - 탭 더블클릭 / 우클릭 메뉴로 탭 이름 변경 및 닫기
  - `Ctrl+Alt+T` — 새 워크스페이스 탭
  - `Ctrl+Alt+W` — 현재 워크스페이스 닫기
  - View > Workspaces 메뉴에 워크스페이스 관리 항목 추가
  - 세션 저장/복원: 워크스페이스별 데이터 + 탭 레이블 저장
  - 구형 단일 워크스페이스 세션과의 하위 호환 복원 지원

#### 3. 각 패널의 주소창 (기존 기능 확인)
- `FolderPane` 은 이미 `BreadcrumbBar` (클릭형 경로 세그먼트 + 편집 모드) 를 내장
- 모든 워크스페이스의 모든 패널이 개별 주소창을 가짐

#### 체크리스트
- [x] `resources/FolderDir.manifest` 신규 생성
- [x] `resources/app.rc` 매니페스트 임베딩
- [x] `src/WorkspaceWidget.h/cpp` 신규 생성
- [x] `src/MainWindow.h` — 워크스페이스 탭 멤버/메서드로 교체
- [x] `src/MainWindow.cpp` — 워크스페이스 탭 전체 구현
- [x] `CMakeLists.txt` — WorkspaceWidget 소스/헤더 추가
- [x] 빌드 성공 확인
