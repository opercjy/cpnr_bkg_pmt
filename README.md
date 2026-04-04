
# NaI(Tl) PMT Environmental Radioactivity Simulation

[![Geant4](https://img.shields.io/badge/Geant4-v11.2.1-blue.svg)](https://geant4.web.cern.ch/)
[![ROOT](https://img.shields.io/badge/ROOT-v6.30.04-blue.svg)](https://root.cern/)
[![OS](https://img.shields.io/badge/OS-Rocky%20Linux%209-green.svg)](https://rockylinux.org/)
[![IDE](https://img.shields.io/badge/IDE-Neovim%2FEmacs-orange.svg)]()

## 1. Overview (프로젝트 개요)
본 프로젝트는 **Geant4**와 **CERN ROOT**를 기반으로 구축된 몬테카를로 전산모사 툴킷입니다. NaI(Tl) 섬광 결정(Scintillator)과 광전자증폭관(PMT)으로 구성된 검출기 시스템을 정밀하게 모사하여, 저배경(Low-background) 실험을 위한 환경 방사능(Environmental Background Radiation)을 평가합니다.

**주요 모사 물리 현상:**
* **자연 방사능 붕괴 체인 (Radioactive Decay):** U-238, Th-232, K-40 등 NORM 동위원소
* **우주선 배경방사선 (Cosmic-ray Muons):** 뮤온 스펙트럼 및 각도 분포
* **광학 물리 (Optical Physics):** 섬광(Scintillation) 생성, 굴절, 반사 및 PMT 양자효율(Quantum Efficiency) 적용

---

## 2. Directory Structure (디렉토리 구조)
전산모사 코드의 디렉토리 구조는 다음과 같이 구성되어 있습니다.

```text
NaI_Sim_Project/
├── CMakeLists.txt          # CMake 빌드 설정 파일
├── Dockerfile              # All-in-one 전산모사/코딩 환경 구성 도커 파일
├── NaISim.cc               # Geant4 메인 실행 파일 (main 함수)
│
├── include/                # C++ 헤더 파일 (.hh) 디렉토리
│   ├── ActionInitialization.hh
│   ├── DetectorConstruction.hh   # NaI 결정, PMT, 차폐체 기하구조 정의
│   ├── PrimaryGeneratorAction.hh # 방사선원(GPS) 및 우주선(CRY) 발생 정의
│   ├── RunAction.hh              # ROOT Ntuple 초기화 및 저장
│   ├── EventAction.hh
│   └── SteppingAction.hh         # 광학 광자(Optical Photon) 추적 및 카운팅
│
├── src/                    # C++ 소스 파일 (.cc) 디렉토리
│   ├── ActionInitialization.cc
│   ├── DetectorConstruction.cc
│   ├── PrimaryGeneratorAction.cc
│   ├── RunAction.cc
│   ├── EventAction.cc
│   └── SteppingAction.cc
│
├── macros/                 # Geant4 매크로 스크립트 (.mac) 디렉토리
│   ├── vis.mac             # Qt6 / OpenGL 시각화 설정
│   ├── k40_decay.mac       # K-40 자연방사능 붕괴 테스트 매크로
│   └── u238_chain.mac      # U-238 붕괴 체인 테스트 매크로
│
└── build/                  # 컴파일된 바이너리 및 빌드 생성물 (Git 무시됨)
```

---

## 3. Docker Environment Setup (도커 환경 구축)
본 프로젝트는 의존성 충돌과 환경 차이로 인한 오류를 원천 차단하기 위해 **자체 Docker 컨테이너 환경**을 제공합니다. Geant4, ROOT, Neovim(LazyVim), Emacs, Nano 등 필수 물리 라이브러리와 C++ IDE가 모두 포함되어 있습니다.

### 3.1. 이미지 빌드 (Build the Image)
저장소를 클론한 후, 최상위 디렉토리에서 아래 명령어를 통해 도커 이미지를 생성합니다. (네트워크 오류 방지를 위해 `--network host` 옵션을 권장합니다.)
```bash
docker build --network host -t naisim-env:1.0 .
```

### 3.2. 컨테이너 실행 및 환경 변수 등록 (Run the Container)
Geant4의 **Qt6/X11 GUI 시각화 창**을 호스트(로컬) 모니터로 띄우고, 호스트의 작업 폴더를 컨테이너 내부와 동기화하기 위해 아래 환경 변수 및 볼륨 바인딩 옵션을 적용하여 실행합니다.

```bash
# 로컬 X11 서버 접근 권한 허용 (터미널에서 1회 실행)
xhost +local:docker

# 컨테이너 실행 (GUI 환경 변수 및 디렉토리 마운트 적용)
docker run -it --rm \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v $(pwd):/work \
    naisim-env:1.0
```
* `-e DISPLAY=$DISPLAY` & `-v /tmp/.X11-unix...`: Geant4 시각화(GUI) 창을 호스트 화면으로 포워딩합니다.
* `-v $(pwd):/work`: 호스트의 현재 소스코드 디렉토리를 도커 내부의 `/work` 폴더와 양방향으로 연결합니다. 도커 안에서 Neovim으로 코드를 수정하면 로컬에도 즉시 반영됩니다.

---

## 4. Docker Image Management (도커 이미지 관리 팁)
연구 과정에서 도커 용량이 부족해지거나 이전 이미지를 정리해야 할 때 다음 명령어들을 사용하십시오.

* **현재 설치된 이미지 목록 확인:**
  ```bash
  docker images
  ```
* **특정 이미지 삭제:**
  실패했거나 안 쓰는 이미지는 `IMAGE ID`를 확인하여 삭제합니다.
  ```bash
  docker rmi <IMAGE_ID>
  ```
* **[권장] 안 쓰는 도커 찌꺼기 일괄 대청소:**
  실행 중인 컨테이너와 연결되지 않은 모든 더미 이미지, 캐시, 네트워크를 한 번에 삭제하여 디스크 용량을 크게 확보합니다.
  ```bash
  docker system prune -a
  ```

---

## 5. Build & Run the Simulation (컴파일 및 실행)
도커 컨테이너 내부에 접속한 상태에서 아래 절차에 따라 시뮬레이션을 컴파일하고 실행합니다.

```bash
# 1. 빌드 폴더 생성 및 이동
mkdir build && cd build

# 2. CMake 구성 및 컴파일 (멀티코어 활용)
cmake ..
make -j$(nproc)

# 3-1. GUI 시각화 모드로 실행 (vis.mac 자동 로드)
./NaISim

# 3-2. 백그라운드 배치 모드로 특정 매크로 실행 (예: K-40 모사)
./NaISim -m ../macros/k40_decay.mac
```
