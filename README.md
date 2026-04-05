# RI_Decay: NaI(Tl) PMT Environmental Radioactivity Simulation

[![Geant4](https://img.shields.io/badge/Geant4-v11.x-blue.svg)](https://geant4.web.cern.ch/)
[![ROOT](https://img.shields.io/badge/ROOT-v6.30.04-blue.svg)](https://root.cern/)
[![OS](https://img.shields.io/badge/OS-Rocky%20Linux%209-green.svg)](https://rockylinux.org/)

## 1. Overview & Design Philosophy (개요 및 설계 철학)
본 프로젝트는 **Geant4**와 **CERN ROOT**를 기반으로 구축된 몬테카를로 전산모사 툴킷입니다. 초저방사능(Ultra-low Background) 실험을 위한 환경 방사능 기여도를 평가하며, 성능의 한계를 극복하기 위해 아래와 같은 핵심 아키텍처가 적용되었습니다.

초보 연구원을 위해 본 시스템에 적용된 **기술적 당위성(Why we do this)**을 먼저 브리핑합니다.

* **왜 도커(Docker)를 사용하는가?**
  Geant4와 ROOT는 운영체제 및 시스템 라이브러리(C++ 버전, Qt 등) 의존성이 매우 높습니다. 로컬 PC에 직접 설치할 경우 발생하는 수많은 충돌 에러를 원천 차단하기 위해, 전 세계 어디서든 **100% 동일한 런타임 환경**을 보장하는 도커 컨테이너를 사용합니다.
* **왜 광학 물리(Optical Physics)를 끄고 하이브리드(Hybrid) 연산을 하는가?**
  1MeV의 에너지가 결정에 흡수될 때마다 약 38,000개의 물리적 광자가 생성됩니다. 1,000만(10M+) 이벤트를 모사할 경우 수조 개의 광자 궤적(Tracking)을 연산해야 하므로 메모리 붕괴(Segfault/OOM)와 심각한 CPU 병목이 발생합니다. 이를 타파하기 위해, 대규모 시뮬레이션 시에는 매크로에서 물리적 빛 생성을 차단하되, C++ 커널(`EventAction.cc`)에서 결정 침적 에너지를 기반으로 **포아송 통계 분포(G4Poisson)를 이용해 광전자 수(NPE)를 역산($O(1)$ 속도)**하는 초고속 하이브리드 최적화 기법을 적용했습니다.
* **왜 10µs의 Time Window(시간 지연 컷오프)를 적용했는가?**
  U-238 등 자연방사능 동위원소는 붕괴 사슬을 거치며 수 일에서 수억 년에 걸쳐 에너지를 방출합니다. Geant4의 기본 Radioactive Decay는 이 모든 시간을 한 번에 추적합니다. 시간 컷오프를 주지 않으면 수억 년 뒤의 에너지까지 단일 프롬프트 이벤트(Pile-up)로 합산되어 물리적 스펙트럼이 완전히 왜곡됩니다. 따라서 실제 검출기의 DAQ Gate 성능과 동일한 10µs의 컷오프를 `SteppingAction.cc`에 강제하여 무결성을 보장했습니다.

---

## 2. Directory Structure (디렉토리 구조)
전산모사 코드의 디렉토리 구조는 아래와 같습니다.

```text
RI_Decay/
├── CMakeLists.txt          # CMake 빌드 설정 파일 (C++17 표준 강제)
├── Dockerfile              # All-in-one 전산모사/코딩 환경 구성 도커 파일
├── main.cc                 # Geant4 메인 실행 파일 (진입점)
│
├── include/                # C++ 헤더 파일 (.hh)
│   ├── ActionInitialization.hh
│   ├── DetectorConstruction.hh   # NaI 결정, PMT, 차폐체 기하구조 정의
│   ├── MyPhysicsList.hh          # Shielding + G4OpticalPhysics 커스텀
│   ├── PMTSD.hh                  # PMT Sensitive Detector (광자 카운팅)
│   ├── PrimaryGeneratorAction.hh # 방사선원(GPS) 발생 정의
│   ├── RunAction.hh              # ROOT Ntuple I/O 파이프라인
│   ├── EventAction.hh            # 하이브리드 NPE 역산 및 데이터 합산
│   ├── SteppingAction.hh         # 에너지 수집 및 10µs DAQ Gate 컷오프
│   └── TrackingAction.hh         # 광학 광자(Optical Photon) 추적 프루닝
│
├── src/                    # C++ 소스 파일 (.cc)
│   └── *.cc                # 헤더에 대응하는 구현부 파일들
│
├── macros/                 # Geant4 매크로 및 분석 스크립트
│   ├── vis.mac             # 일반 입자 궤적 및 기하구조 시각화
│   ├── vis_optics.mac      # 광학 궤적 디버깅 및 투시도 시각화
│   ├── k40_decay.mac       # K-40 자연방사능 모사 (10M 하이브리드 연산)
│   ├── u238_chain.mac      # U-238 붕괴 사슬 모사 (10M 하이브리드 연산)
│   ├── full_bkg.mac        # 전체 차폐체 방사능 평가 매크로
│   └── plot_spectra.py     # 에너지 및 NPE 스펙트럼 시각화 Python 스크립트
│
└── build/                  # 컴파일된 바이너리 및 생성물 (Git 무시됨)
```

---

## 3. Docker Environment Setup (독립된 런타임 환경 구축)

### 3.1. 이미지 빌드 (Build the Image)
저장소를 클론한 후, 최상위 디렉토리에서 아래 명령어를 통해 도커 이미지를 생성합니다.
```bash
docker build -t naisim-env:1.0 .

# (참고) 사내 보안망 등으로 인해 패키지 다운로드 네트워크 에러가 발생할 경우,
# 호스트 네트워크 공유 옵션을 추가하십시오: docker build --network host -t naisim-env:1.0 .
```

### 3.2. 컨테이너 실행 (Run the Container)
Geant4의 GUI 시각화 창을 로컬 모니터로 포워딩하고, 코드를 실시간 동기화하기 위해 아래 명령어로 접속합니다.

```bash
# 로컬 X11 서버 접근 권한 허용 (호스트 터미널에서 1회 실행)
xhost +local:docker

# 컨테이너 실행 (볼륨 마운트 및 디스플레이 포워딩 적용)
docker run -it --rm \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v $(pwd):/work \
    naisim-env:1.0
```

---

## 4. Macro-Driven Architecture (매크로 기반 제어 시스템)
본 시스템은 런타임 효율을 위해 모든 물리적 제어와 출력 설정을 **매크로 파일(`.mac`)**로 통제합니다.

### 🔬 [1] 디버깅 및 시각화용 매크로 (GUI 모드)
* **`vis.mac` / `vis_optics.mac`**
  대규모 런을 돌리기 전, 기하 구조가 올바른지 검증하고 입자의 거동을 시각적으로 확인합니다. 메모리 보호를 위해 `vis.mac`은 광자 렌더링을 차단하고, `vis_optics.mac`은 광학 추적에 특화된 Wireframe 투시도를 제공합니다.

### 🚀 [2] 초고속 프로덕션 전용 매크로 (Batch 모드)
* **`k40_decay.mac`, `u238_chain.mac`, `full_bkg.mac` 등**
  매크로 내부에 `/process/optical/processActivation Scintillation false` 명령어가 삽입되어 있습니다. 물리적 빛 생성을 원천 차단하여 CPU 연산을 극도로 단축시키고, C++의 '하이브리드 NPE 역산 시스템'을 발동시킵니다. 1,000만(10M+) 이벤트도 수 분 내에 처리합니다.

---

## 5. Build & Run the Simulation (컴파일 및 실행)
도커 컨테이너 내부에 접속한 상태에서 아래 절차에 따라 시뮬레이션을 수행합니다. 실행 파일명은 `RI_Sim`으로 통일되어 있습니다.

```bash
# 1. 빌드 폴더 생성 및 이동
mkdir build && cd build

# 2. CMake 구성 및 멀티코어 컴파일 (-j 옵션으로 빌드 속도 극대화)
cmake ..
make -j$(nproc)

# 3-1. [GUI 모드] 시각화 창 띄우기 (vis.mac 자동 로드)
./RI_Sim

# 3-2. [배치 모드] 특정 매크로를 로드하여 대규모 하이브리드 연산 실행
./RI_Sim macros/k40_decay.mac
./RI_Sim macros/u238_chain.mac
./RI_Sim macros/full_bkg.mac
```

---

## 6. Data Analysis (데이터 분석 파이프라인)
매크로 실행이 완료되면 `build/` 폴더 내에 시뮬레이션 결과가 담긴 ROOT Ntuple 파일(예: `u238_chain_10M.root`)이 자동 생성됩니다. 내장된 Python 스크립트를 통해 직관적으로 데이터를 시각화할 수 있습니다.

```bash
# 에너지 스펙트럼 및 NPE 분포 플로팅 (uproot, matplotlib 기반)
python macros/plot_spectra.py
```
이 파이썬 스크립트는 Geant4가 출력한 원시 데이터를 1차원 히스토그램으로 변환하여, 순수 침적 에너지(MC Total Energy)와 통계적 요동이 반영된 광전자 수(NPE)의 상관관계를 서브플롯(Subplot) 형태로 즉시 도출합니다.
