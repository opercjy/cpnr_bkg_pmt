# RI_Decay: NaI(Tl) PMT Environmental Radioactivity Simulation

[![Geant4](https://img.shields.io/badge/Geant4-v11.x-blue.svg)](https://geant4.web.cern.ch/)
[![ROOT](https://img.shields.io/badge/ROOT-v6.3x-blue.svg)](https://root.cern/)
[![OS](https://img.shields.io/badge/OS-Rocky%20Linux%209-green.svg)](https://rockylinux.org/)

<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/9e4d6a01-ad51-4593-aca4-1a026bfdf9e9" />
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/94bcb0ae-0264-4caa-8c3d-fca7353f742d" />

## 1. Overview & Design Philosophy (개요 및 설계 철학)
본 프로젝트는 **Geant4**와 **CERN ROOT**를 기반으로 구축된 몬테카를로 전산모사 툴킷입니다. 초저방사능(Ultra-low Background) 실험을 위한 환경 방사능 기여도를 평가하며, 성능의 한계를 극복하기 위해 아래와 같은 핵심 아키텍처가 적용되었습니다.

초보 연구원을 위해 본 시스템에 적용된 **기술적 당위성(Why we do this)**을 먼저 브리핑합니다.

* **왜 도커(Docker)를 사용하는가?**
  Geant4와 ROOT는 운영체제 및 시스템 라이브러리(C++ 버전, Qt 등) 의존성이 매우 높습니다. 로컬 PC에 직접 설치할 경우 발생하는 수많은 충돌 에러를 원천 차단하기 위해, 전 세계 어디서든 **100% 동일한 런타임 환경**을 보장하는 도커 컨테이너를 사용합니다.
* **왜 광학 물리(Optical Physics)를 끄고 하이브리드(Hybrid) 연산을 하는가?**
  1 MeV의 에너지가 결정에 흡수될 때마다 약 38,000개의 물리적 광자가 생성됩니다. 1,000만(10M+) 이벤트를 모사할 경우 수조 개의 광자 궤적(Tracking)을 연산해야 하므로 메모리 붕괴(Segfault/OOM)와 심각한 CPU 병목이 발생합니다. 이를 타파하기 위해, 대규모 시뮬레이션 시에는 매크로에서 물리적 빛 생성을 차단하되, C++ 커널에서 결정 침적 에너지를 기반으로 **포아송 통계 분포(G4Poisson)를 이용해 광전자 수(NPE)를 역산($O(1)$ 속도)**하는 초고속 하이브리드 최적화 기법을 적용했습니다.
* **왜 10 µs의 Time Window(시간 지연 컷오프)를 적용했는가?**
  U-238 등 자연방사능 동위원소는 붕괴 사슬을 거치며 수 일에서 수억 년에 걸쳐 에너지를 방출합니다. Geant4의 기본 Radioactive Decay는 이 모든 시간을 한 번에 추적합니다. 시간 컷오프를 주지 않으면 수억 년 뒤의 에너지까지 단일 프롬프트 이벤트(Pile-up)로 합산되어 물리적 스펙트럼이 완전히 왜곡됩니다. 따라서 실제 검출기의 DAQ Gate 성능과 동일한 10 µs의 컷오프를 `SteppingAction.cc`에 강제하여 무결성을 보장했습니다.

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
│   ├── SteppingAction.hh         # 에너지 수집 및 10 µs DAQ Gate 컷오프
│   └── TrackingAction.hh         # 광학 광자(Optical Photon) 추적 프루닝
│
├── src/                    # C++ 소스 파일 (.cc)
│   └── *.cc                # 헤더에 대응하는 구현부 파일들
│
├── macros/                 # Geant4 매크로 및 분석 스크립트
│   ├── vis.mac             # 일반 입자 궤적 및 기하구조 시각화
│   ├── vis_optics.mac      # 광학 궤적 디버깅 및 투시도 시각화
│   ├── shrinkwrap_bkg.mac  # 반사체 및 랩핑 물질 표면 방사능 평가 (Surface Bkg)
│   ├── full_bkg.mac        # 외부 환경 전체 차폐체 방사능 평가 (Volume Bkg)
│   ├── k40_decay.mac       # K-40 단일 핵종 모사
│   ├── u238_chain.mac      # U-238 붕괴 사슬 모사
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

### 3.2. 컨테이너 실행 및 환경 자동화 (Run the Container)
Geant4의 GUI 시각화 창을 로컬 모니터로 포워딩하고, 코드를 실시간 동기화하여 개발하기 위해 아래와 같이 컨테이너를 실행합니다.

**[옵션 1: 일회성 기본 실행]** 터미널에 직접 명령어를 입력하여 실행하는 방법입니다.

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

**[옵션 2: 아키텍트 추천 - Bash 함수로 영구 자동화]** 매번 위 긴 명령어를 입력하는 것은 비효율적입니다. 호스트 PC의 `~/.bashrc` (또는 Mac의 경우 `~/.zshrc`) 파일 맨 끝에 아래의 쉘 함수를 추가해 두면, 명령어 하나로 즉시 환경에 접속할 수 있습니다.

```bash
# ~/.bashrc 파일 맨 아래에 추가
rung4qt6() {
    xhost +local:docker > /dev/null 2>&1
    docker run -it --rm \
        -e DISPLAY=$DISPLAY \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        -v $(pwd):/work \
        naisim-env:1.0
}
```

위 코드를 파일에 추가하고 저장한 뒤, 터미널에서 `source ~/.bashrc`를 한 번 실행하여 환경을 갱신하십시오.  
이제부터는 터미널에서 **`rung4qt6`** 라고 입력하기만 하면, 즉시 Geant4 11.x Qt6 개발 환경 도커 컨테이너로 진입하게 됩니다.

### 3.3. 컨테이너 심화 실행 및 관리 (Advanced Docker Management)
연구 인프라 운영 목적에 따라 컨테이너를 백그라운드에서 실행하거나 포트를 연결해야 할 수 있습니다.

**[백그라운드 실행 및 포트 연결 (Detached & Port Forwarding)]**
GUI 렌더링이 필요 없는 대규모 배치(Batch) 스크립트를 서버 백그라운드에서 돌리거나, 도커 내부의 JupyterLab 등 웹 서비스 포트를 로컬 호스트와 연결할 때 사용합니다.
```bash
# -d (백그라운드 실행 모드), -p (포트 포워딩, 예: 로컬 8888번을 컨테이너 8888번으로 연결)
docker run -d -p 8888:8888 -v $(pwd):/work naisim-env:1.0
```

**[도커 이미지 및 컨테이너 관리 명령어]**
하드 디스크 용량 확보 및 이미지 관리를 위한 필수 명령어 모음입니다.
```bash
# 1. 현재 로컬에 생성(빌드)된 도커 이미지 목록 확인
docker images

# 2. 현재 실행 중인 컨테이너 상태 확인
docker ps

# 3. 특정 도커 이미지 삭제 (예: naisim-env:1.0 삭제)
docker rmi naisim-env:1.0  # 또는 docker rmi <IMAGE_ID>

# 4. [권장] 안 쓰는 더미 데이터(캐시, 컨테이너, 이미지) 일괄 대청소
docker system prune -a
```

---

## 4. Macro-Driven Architecture (매크로 기반 제어 시스템)
본 시스템은 런타임 효율을 위해 모든 물리적 제어와 출력 설정을 **매크로 파일(`.mac`)**로 통제합니다. 특히 대규모 프로덕션 매크로들은 목적에 따라 방사선원의 기하학적 분포(Geometry)와 방출 각도(Angular Distribution)가 아키텍처 수준에서 철저히 분리 최적화되어 있습니다.

### [1] 디버깅 및 시각화용 매크로 (GUI 모드)
* **`vis.mac` / `vis_optics.mac`**
  대규모 런을 돌리기 전, 기하 구조가 올바른지 검증하고 입자의 거동을 시각적으로 확인합니다. GUI 메모리 보호를 위해 `vis.mac`은 광자 렌더링을 원천 차단하며, `vis_optics.mac`은 광학 추적에 특화된 Wireframe 투시도와 입자별 색상 매핑을 제공합니다.

### [2] 초고속 프로덕션 전용 매크로 (Batch 모드)
모든 프로덕션 매크로는 내부에 `/process/optical/processActivation Scintillation false` 옵션을 강제하여 물리적 빛 생성을 차단, C++의 '하이브리드 NPE 역산 시스템'을 발동시킵니다. 시뮬레이션이 모사하고자 하는 **물리적 기원(Physical Origin)**에 따라 아래와 같이 두 가지 핵심 아키텍처로 나뉩니다.

#### A. 국소 표면 오염 평가: `shrinkwrap_bkg.mac`
NaI 결정을 얇게 감싸는 반사체 및 랩핑 물질 내벽의 미세 오염(Surface Background)을 평가하기 위한 극한 최적화 모드입니다.
* **기하학적 스케일 (Surface):** 방사선원 타입을 `/gps/pos/type Surface`로 설정하고, 2"x2" 검출기에 밀착된 초소형 실린더(R=3cm, Half-Z=5cm) 껍데기에서 입자가 발생하도록 제한합니다.
* **입체각 바이어싱 (Cosine Biasing):** 검출기 표면에서 발생한 입자가 바깥 허공으로 날아가는 연산력 낭비를 막기 위해, 방출 각도를 `/gps/ang/type cos`로 강제합니다. 입자가 무조건 검출기 중심부(법선 방향)를 향해 쏘아지도록 제어하여 시뮬레이션 효율을 수십 배 극대화합니다.

#### B. 전역 체적 방사능 평가: `full_bkg.mac`
실험실 외곽 구조물, 공기 중 라돈, 전체 차폐체 등 외부 환경에서 기인하는 주변 방사능(Volume Background)을 정밀하게 모사하기 위한 모드입니다.
* **기하학적 스케일 (Volume):** 방사선원 타입을 `/gps/pos/type Volume`으로 설정하고, 시뮬레이션의 최외곽 경계인 World 볼륨 전체에 꽉 차는 거대 실린더(R=1m, Half-Z=1m) 영역 내에서 방사선이 무작위로 발생하도록 설정합니다.
* **물리적 무결성 (Isotropic):** 외부 환경 방사능의 자연적 무작위성을 정확히 모사하기 위해 `/gps/ang/type iso`를 적용합니다. 인위적인 방향성 조작 없이 4$\pi$ 전방위 입체각으로 입자가 쏟아지도록 하여 완벽한 통계적 요동을 구현합니다.

#### C. 단일 핵종 모사: `k40_decay.mac`, `u238_chain.mac`
특정 NORM(자연방사능) 동위원소의 순수 에너지 스펙트럼과 역산된 NPE 분포 데이터를 개별적으로 추출할 때 사용하는 모듈화된 매크로입니다.

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

# 3-2. [배치 모드] 특정 목적의 매크로를 로드하여 대규모 하이브리드 연산 실행
./RI_Sim macros/shrinkwrap_bkg.mac  # 반사체 랩핑 물질 표면 방사능 평가
./RI_Sim macros/full_bkg.mac        # 외부 환경 전체 차폐체 모사
./RI_Sim macros/k40_decay.mac       # K-40 단일 핵종 모사
./RI_Sim macros/u238_chain.mac      # U-238 붕괴 사슬 모사
```

---

## 6. Data Analysis (데이터 분석 파이프라인)
매크로 실행이 완료되면 `build/` 폴더 내에 시뮬레이션 결과가 담긴 ROOT Ntuple 파일(예: `shrinkwrap_bkg_10M.root`)이 자동 생성됩니다. 내장된 Python 스크립트를 통해 직관적으로 데이터를 시각화할 수 있습니다.

```bash
# Rocky Linux 9 (도커 환경)에서는 시스템 호환성을 위해 python3 명령어를 명시적으로 사용합니다.
python3 macros/plot_spectra.py
```

이 파이썬 스크립트는 Geant4가 출력한 원시 데이터를 1차원 히스토그램으로 변환하여, 순수 침적 에너지(MC Total Energy)와 통계적 요동이 반영된 광전자 수(NPE)의 상관관계를 서브플롯(Subplot) 형태로 즉시 도출합니다.

## 7. Appendix: Architecture Defense Logic (전문가 아키텍처 방어 논리)
본 전산모사 코드는 단순한 작동을 넘어, 초저방사능(Ultra-low Background) 광학 물리 시뮬레이션에서 흔히 발생하는 시스템적 오류와 물리적 왜곡을 방어하기 위해 치밀하게 설계되었습니다. 타 연구자와의 교차 검증 및 디펜스를 위한 핵심 아키텍처 논리를 명시합니다.

### 7.1. 왜 스코어링(Scoring) 대신 SD(Sensitive Detector)를 구현했는가?
Geant4에 내장된 커맨드 기반 스코어링(Command-based Scorer)을 사용하지 않고 `PMTSD` 클래스를 직접 구현한 이유는 **광전 음극(Photocathode)의 양자 역학적 특수성** 때문입니다.
* 스코어링 메쉬는 '에너지 침적량'을 측정하는 데 유리하지만, PMT 표면은 에너지가 쌓이는 곳이 아니라 **광학 광자(Optical Photon)가 광전자(Photoelectron)로 변환되는 곳**입니다.
* 커널에서 `optParam->SetBoundaryInvokeSD(true)`를 활성화하여, Geant4의 `G4OpBoundaryProcess`가 PMT 표면(`dielectric_metal`)에 도달한 광자에 대해 28%의 양자 효율(QE) 확률 연산을 먼저 수행하도록 설계했습니다.
* 즉, 물리 엔진의 순수한 양자 변환 주사위를 통과한 진짜 '광전자'들만 SD로 보고되므로, 유저가 데이터 후처리 단계에서 임의로 QE 상수를 곱하는 작위적인 방식을 원천 배제하고 물리적 무결성을 100% 확보했습니다.

### 7.2. 분광 특성 매개변수의 물리적 당위성 (Detector Spectral Properties)
`DetectorConstruction.cc`에 주입된 광학 매개변수들은 실험실 환경의 미시적 물리 현상을 현실화하는 핵심 인자입니다.
* **`RAYLEIGH` (레일리 산란 = 40 cm):** 이 값이 없으면 광자가 진공처럼 일직선으로 날아가 PMT 중앙으로 비정상적인 집중(Focusing)이 일어납니다. 두꺼운 NaI 결정 내부에서 푸른빛(415 nm)이 겪는 40cm 단위의 산란을 구현하여, 현실적인 집광 시간(Timing)과 궤적 난반사를 모사했습니다.
* **`UNIFIED` 모델과 `groundfrontpainted`:** 테플론 반사체를 낡은 `GLISUR` 거울면 모델로 모사하면 시간 분해능이 왜곡됩니다. 최신 `UNIFIED` 모델을 통해 테플론 랩핑에 닿은 빛이 98%의 확률로 완벽하게 사방으로 흩뿌려지는 **람베르트 난반사(Lambertian Diffuse Reflection)**를 강제했습니다.
* **광전 음극의 `REFLECTIVITY` (0.0):** PMT 유리와 내부 진공 사이의 `dielectric_metal` 경계에서 빛이 결정 쪽으로 다시 튕겨 나가는 비물리적 반사를 억제하고, 오직 28%의 흡수/검출 상호작용만 일어나도록 통제했습니다.

### 7.3. 경계면 스텝 사이즈 아티팩트(Step Size Artifact) 2중 방어망
몬테카를로 광학 시뮬레이션의 고질적 병폐인 **'경계면 무한 루프(부동소수점 한계로 스텝 사이즈가 0에 수렴하는 현상)'**를 방어하기 위해 커널 레벨의 2중 차단 로직이 적용되어 있습니다.
1. **SD 단의 강제 처형 (Explicit Kill):** 광자가 28%의 QE를 뚫고 `PMTSD`에 진입하여 계측된 직후, `track->SetTrackStatus(fStopAndKill)`가 호출됩니다. 검출이 완료된 입자를 즉시 소멸시켜 경계면에 갇혀 마이크로 스텝을 유발할 여지를 제거했습니다.
2. **경계면 흡수 메커니즘 (Boundary Absorption):** 검출 확률 28%를 통과하지 못한 나머지 72%의 광자들은 `REFLECTIVITY 0.0` 설정에 의해 경계면에서 즉각 `fAbsorption`(흡수) 판정을 받고 소멸합니다. 결과적으로 경계면에서 무의미하게 맴도는 광자가 수학적으로 존재할 수 없는 매우 견고한(Robust) 상태를 유지합니다.
