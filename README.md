# 📌 KRAFTON Jungle Tech Lab Week 04

---

## 🚀 주요 기능 구현

- **Wavefront OBJ/MTL 파싱 → 엔진 구조 변환**
    - `v/vt/vn`, `f`, `mtllib`, `usemtl`, `Ka/Kd/Ks/Ns`, `map_Kd` 지원
    - 파이프라인: `.OBJ → FObjInfo(Intermediate) → FStaticMesh(Cooked)`
- **바이너리 베이크 & 캐시 (재로딩 가속)**
    - 최초 파싱 후 `Data/Binary/{Base}.bin` 저장
    - OBJ 최신성 비교 후 `.bin` 우선 로드
    - 사용 모듈: `FArchive`, `FWindowsBinWriter`, `FWindowsBinReader`
- **정적 메시 시스템 & 렌더링**
    - `UStaticMesh`, `UStaticMeshComponent` → 씬 배치 및 Transform(이동/회전/스케일) 조작
    - 드롭리스트 UI에서 `TObjectIterator<UStaticMesh>`로 메시 선택 → Property 창에 Assign
- **다중 머티리얼(섹션) 렌더링**
    - OBJ `usemtl` 그룹을 `FStaticMeshSection`으로 변환 → 섹션 단위 드로우
- **다중 뷰포트 & 스플리터**
    - 상/하(수평) + 좌/우(수직) 스플리터 → 총 4개 Viewport
    - 드래그 리사이즈 가능, 위치는 `Editor.ini`에 저장/복원
    - Orthographic/Perspective 동시 렌더링, 선택 뷰포트 메인 카메라 승격
- **셰이더: `StaticMeshShader.hlsl`**
    - POSITION / NORMAL / COLOR / TEXCOORD 입력
    - UV Scroll (체크박스 + 속도) → 픽셀 셰이더 반영
- **OBJ Viewer 실행 구성**
    - `ObjViewerDebug | x64` 빌드 시 별도 Viewer 실행

---

## 📂 프로젝트 구조 (요약)

- **솔루션**: `GTL.sln`
- **엔진**: `Engine/`

**주요 디렉터리**

- OBJ 파이프라인: `Mesh/StaticMesh/{ObjImporter, ObjManager, StaticMesh}.*`
- 컴포넌트: `Mesh/StaticMeshComponent.*`
- 렌더러: `Render/Renderer/{Renderer, Pipeline}.*`
- 셰이더: `Data/Shader/StaticMeshShader.hlsl` (+ Text, Slate, LineInstanced)
- 뷰포트 & 스플리터: `Slate/{Viewport, Splitter, SplitterH, SplitterV}.*`
- 뷰포트 매니저: `Manager/Viewport/ViewportManager.*`
- UI 위젯: `Render/UI/Widget/ActorDetailWidget.cpp`
- 경로/설정: `Manager/Path/PathManager.*` (`editor.ini`, `Data/Binary/`)
- 엔트리: `main.cpp`, `Core/ClientApp.*`

---

## ⚙️ 빌드 및 실행

**사전 요구사항**

- Visual Studio 2022
- PlatformToolset v143
- Windows 10 SDK

**빌드 절차**

1. `GTL.sln` 열기 → `Engine` 프로젝트 선택
2. 구성: `ObjViewerDebug | x64` (뷰어 실행) 또는 `Debug/Release | x64`
3. 빌드 후 실행 (기준 경로: `Data/`)

**자산 배치**

- OBJ/MTL/Texture는 `Engine/Data/` 하위에 배치
- 예: `car.obj`, `Demon.obj`, `Bear.obj`, `Cube.obj`
- 최초 실행 시 `.bin` 생성 → 이후 바이너리 로드로 빠른 시작

---

## 🔍 구현 상세 포인트

- **Wavefront OBJ 포맷/파싱**
    - `ParseObjFile / ParseMtlFile` → 위치/노멀/UV/머티리얼 속성 수집
    - Face 인덱스 변환: 1-based → 0-based
- **데이터 변환 (Intermediate → Cooked)**
    - `ConvertObjToStaticMesh()`에서 `(pos, uv, normal)` 키로 유니크 정점 맵핑 → 중복 제거
    - `usemtl` → `FStaticMeshSection`, `FStaticMaterial` 구성
- **Blender ↔ Engine 파이프라인**
    - `map_Kd`는 `Data/Texture` 상대경로 기준
    - `UPathManager`가 폴더 구조(`Shader/Texture/World/Config/Binary`) 보장
- **StaticMesh 시스템**
    - `UStaticMesh`: Cooked 데이터 보관
    - `UStaticMeshComponent`: 씬 배치, Bounds 계산, 원시 데이터 접근
    - Transform 조작: 기존 Primitive와 동일 (기즈모/입력)
- **UI 연계 (Drop List Assign)**
    - `ActorDetailWidget`에서 `TObjectIterator<UStaticMesh>` 열거
    - 선택 결과 → `UStaticMeshComponent::SetStaticMeshByPath()`
- **셰이더 처리**
    - MaterialParams 상수 버퍼: `UseTexture`, `UVScrollSpeed`, `Time`, 색/스펙
    - 텍스처 미사용 → DiffuseColor, 사용 시 → `map_Kd` SRV 샘플링
- **Multiple Material (Section)**
    - `FStaticMesh.Sections`에 `usemtl` 단위로 등록 → 섹션별 드로우
- **Viewport & Splitter**
    - 4 Viewport 구성 (`SSplitterH` + `SSplitterV`)
    - 드래그 리사이즈 / 최소 크기 보장 / Hover 하이라이트
- **스플리터 상태 저장/복원**
    - `Editor.ini`에 좌표 기록 (예: `SplitterH`, `TopSplitterV`, `BottomSplitterV`)
- **Orthogonal View 동기화**
    - Ortho 조작 시 다른 Ortho 뷰 동기 이동
- **UE 유사 구조 차용**
    - `SViewport`, `UViewportManager` → UE `FViewport/FViewportClient` 컨셉 반영
- **카메라 저장/복원**
    - `FLevelSerializer`가 Perspective 카메라 메타데이터(JSON) 저장/로드
- **에셋 매니저 (FObjManager)**
    - Vertex/Index 버퍼 생성
    - 텍스처 SRV 캐싱 (DirectXTK WIC)
- **Binary Bake**
    - OBJ 최신성 비교 → `.bin` 직렬화/역직렬화로 빠른 로딩
- **UV Scroll**
    - Detail 창 체크박스/속도 → `UStaticMeshComponent` 시간 누적 관리
    - 셰이더 파라미터(`UVScrollSpeed/Time`)로 전달

---

## 🙌 크레딧

- Team: **Game Tech Lab Week 4 — Team 3**
- Tech: **DirectX 11, ImGui, DirectXTK(WIC Texture)**