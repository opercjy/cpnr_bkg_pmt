import uproot
import numpy as np
import matplotlib.pyplot as plt
import mplhep as hep

# 고에너지 물리(HEP) 논문용 표준 스타일 적용
hep.style.use(hep.style.ROOT)

def main():
    # 1. ROOT 파일 로드 (1,000만 개 런 파일명 기준)
    file_name = "full_bkg_10M.root" 
    try:
        events = uproot.open(f"{file_name}:EventTree")
    except Exception as e:
        print(f"❌ Error opening file: {e}")
        return

    # 2. 브랜치 데이터 추출
    edep_nai = events["Edep_NaI"].array(library="np")
    edep_al  = events["Edep_Al"].array(library="np")
    npe      = events["NPE"].array(library="np")

    # 3. 0인 데이터(배경 노이즈) 필터링
    edep_nai_clean = edep_nai[edep_nai > 0]
    edep_al_clean = edep_al[edep_al > 0]
    npe_clean = npe[npe > 0]

    # =========================================================
    # 물리적 캘리브레이션 (NPE -> Reconstructed MeV)
    # (필요시 이 상수를 조절하여 K-40 피크를 1.46 MeV에 맞추십시오)
    # =========================================================
    CALIBRATION_FACTOR = 6900.0
    reco_energy = npe_clean / CALIBRATION_FACTOR

    # =========================================================
    # 플로팅 (2행 2열 구조)
    # =========================================================
    fig, axes = plt.subplots(2, 2, figsize=(18, 14))
    
    # [Panel (0, 0): Top-Left] 순수 에너지 침적 (True Energy Deposition in NaI)
    counts_nai, bins_nai = np.histogram(edep_nai_clean, bins=500, range=(0, 5))
    hep.histplot(counts_nai, bins=bins_nai, ax=axes[0, 0], color='black', histtype='step', 
                 linewidth=1.5, label='True Edep (No Resolution)')
    axes[0, 0].set_yscale('log')
    axes[0, 0].set_xlim(0, 5)
    axes[0, 0].set_ylim(1, 1e5)
    axes[0, 0].set_xlabel("Energy (MeV)", loc='right')
    axes[0, 0].set_ylabel("Counts", loc='top')
    axes[0, 0].set_title("True Energy Deposition in NaI", fontsize=20, pad=15)
    axes[0, 0].legend(loc='upper right', fontsize=14)
    axes[0, 0].grid(True, which="both", axis="y", linestyle="--", alpha=0.5)

    # [Panel (0, 1): Top-Right] 알루미늄 하우징 및 베이스 침적 에너지
    counts_al, bins_al = np.histogram(edep_al_clean, bins=500, range=(0, 5))
    hep.histplot(counts_al, bins=bins_al, ax=axes[0, 1], color='forestgreen', histtype='step', 
                 linewidth=1.5, label='Al Housing & Base')
    axes[0, 1].set_yscale('log')
    axes[0, 1].set_xlim(0, 5)
    axes[0, 1].set_ylim(1, 1e5)
    axes[0, 1].set_xlabel("Energy (MeV)", loc='right')
    axes[0, 1].set_ylabel("Counts", loc='top')
    axes[0, 1].set_title("Al Housing Energy Deposition", fontsize=20, pad=15)
    axes[0, 1].legend(loc='upper right', fontsize=14)
    axes[0, 1].grid(True, which="both", axis="y", linestyle="--", alpha=0.5)

    # [Panel (1, 0): Bottom-Left] Geant4 물리 엔진이 출력한 순수 광전자 수 (NPE)
    counts_npe, bins_npe = np.histogram(npe_clean, bins=500, range=(0, 35000))
    hep.histplot(counts_npe, bins=bins_npe, ax=axes[1, 0], color='royalblue', histtype='step', 
                 linewidth=1.5, label='Raw PMT Signal (NPE)')
    axes[1, 0].set_yscale('log')
    axes[1, 0].set_xlim(0, 35000)
    axes[1, 0].set_ylim(1, 1e5)
    axes[1, 0].set_xlabel("Number of Photoelectrons (NPE)", loc='right')
    axes[1, 0].set_ylabel("Counts", loc='top')
    axes[1, 0].set_title("Raw PMT Optical Signal", fontsize=20, pad=15)
    axes[1, 0].legend(loc='upper right', fontsize=14)
    axes[1, 0].grid(True, which="both", axis="y", linestyle="--", alpha=0.5)

    # [Panel (1, 1): Bottom-Right] 재구성된 검출기 스펙트럼 (Reconstructed Energy)
    counts_reco, bins_reco = np.histogram(reco_energy, bins=500, range=(0, 5))
    hep.histplot(counts_reco, bins=bins_reco, ax=axes[1, 1], color='firebrick', histtype='step', 
                 linewidth=1.5, label='Reconstructed Energy')
    axes[1, 1].set_yscale('log')
    axes[1, 1].set_xlim(0, 5)
    axes[1, 1].set_ylim(1, 1e5)
    axes[1, 1].set_xlabel("Energy (MeV)", loc='right')
    axes[1, 1].set_ylabel("Counts", loc='top')
    axes[1, 1].set_title("Simulated Detector Spectrum", fontsize=20, pad=15)
    axes[1, 1].legend(loc='upper right', fontsize=14)
    axes[1, 1].grid(True, which="both", axis="y", linestyle="--", alpha=0.5)

    # 여백 조절 및 출력
    plt.tight_layout(pad=3.0)
    plt.savefig("Simulation_Results_2x2.png", dpi=300) 
    
    print("스펙트럼 시각화 완료: 'Simulation_Results_2x2.png'")
    plt.show()

if __name__ == "__main__":
    main()
