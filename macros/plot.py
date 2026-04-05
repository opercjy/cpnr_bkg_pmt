import uproot
import numpy as np
import matplotlib.pyplot as plt
import mplhep as hep

# 고에너지 물리(HEP) 논문용 표준 스타일 적용
hep.style.use(hep.style.ROOT)

def smear_energy(energy_array):
    """ 물리적 에너지 분해능을 반영하는 Gaussian Smearing 함수 """
    smeared = np.zeros_like(energy_array)
    valid = energy_array > 0
    E = energy_array[valid]
    
    # 에너지 분해능 공식 적용 (에너지가 높을수록 분해능 개선)
    sigma = E * np.sqrt((0.05**2) / E + 0.02**2)
    smeared[valid] = np.random.normal(E, sigma)
    return smeared

def main():
    # 1. ROOT 파일 및 Tree 로드 (1,000만 개 런 파일명 기준)
    file_name = "shrinkwrap_bkg_1M260405.root" 
    try:
        events = uproot.open(f"{file_name}:EventTree")
    except Exception as e:
        print(f"❌ Error opening file: {e}")
        print("💡 팁: 만약 테스트용 1M 런을 분석하시려면, 코드 상단의 file_name을 'shrinkwrap_bkg_1M.root'로 수정해주세요.")
        return

    # 2. 브랜치 데이터를 Numpy 배열로 추출
    edep_nai = events["Edep_NaI"].array(library="np")
    edep_al  = events["Edep_Al"].array(library="np")
    npe      = events["NPE"].array(library="np")

    # 3. 0인 데이터(배경 노이즈) 필터링 및 스미어링 적용
    edep_nai_clean = edep_nai[edep_nai > 0]
    smeared_nai = smear_energy(edep_nai_clean)
    
    edep_al_clean = edep_al[edep_al > 0]
    npe_clean = npe[npe > 0]

    # =========================================================
    # 플로팅 (1행 3열 구조)
    # =========================================================
    fig, axes = plt.subplots(1, 3, figsize=(24, 7))
    
    # [Panel 1] NaI(Tl) Smeared Spectrum
    # np.histogram을 통해 데이터 개수와 구간을 미리 계산 (mplhep 에러 방지)
    counts_nai, bins_nai = np.histogram(smeared_nai, bins=500, range=(0, 5))
    hep.histplot(counts_nai, bins=bins_nai, ax=axes[0], color='black', histtype='step', 
                 linewidth=1.5, label='MC Total (Smeared)')
    axes[0].set_yscale('log')
    axes[0].set_xlim(0, 5)
    axes[0].set_ylim(1, 1e5)
    axes[0].set_xlabel("Energy (MeV)", loc='right')
    axes[0].set_ylabel("Counts / 10 keV", loc='top')
    axes[0].set_title("NaI(Tl) Crystal Spectrum", fontsize=22, pad=15)
    axes[0].legend(loc='upper right', fontsize=14)
    axes[0].grid(True, which="both", axis="y", linestyle="--", alpha=0.5)

    # [Panel 2] Al Housing Spectrum
    counts_al, bins_al = np.histogram(edep_al_clean, bins=500, range=(0, 5))
    hep.histplot(counts_al, bins=bins_al, ax=axes[1], color='firebrick', histtype='step', 
                 linewidth=1.5, label='Al Housing & Base')
    axes[1].set_yscale('log')
    axes[1].set_xlim(0, 5)
    axes[1].set_ylim(1, 1e5)
    axes[1].set_xlabel("Energy (MeV)", loc='right')
    axes[1].set_ylabel("Counts / 10 keV", loc='top')
    axes[1].set_title("Al Housing Energy Deposition", fontsize=22, pad=15)
    axes[1].legend(loc='upper right', fontsize=14)
    axes[1].grid(True, which="both", axis="y", linestyle="--", alpha=0.5)

    # [Panel 3] NPE (Number of Photoelectrons) Distribution
    npe_max = max(npe_clean) if len(npe_clean) > 0 else 10000
    counts_npe, bins_npe = np.histogram(npe_clean, bins=500, range=(0, npe_max))
    hep.histplot(counts_npe, bins=bins_npe, ax=axes[2], color='royalblue', histtype='step', 
                 linewidth=1.5, label='Hybrid NPE')
    axes[2].set_yscale('log')
    axes[2].set_xlim(0, npe_max)
    axes[2].set_ylim(1, 1e5)
    axes[2].set_xlabel("Number of Photoelectrons (NPE)", loc='right')
    axes[2].set_ylabel("Counts", loc='top')
    axes[2].set_title("PMT Photoelectron Distribution", fontsize=22, pad=15)
    axes[2].legend(loc='upper right', fontsize=14)
    axes[2].grid(True, which="both", axis="y", linestyle="--", alpha=0.5)

    # =========================================================
    # 여백 조절 및 출력
    # =========================================================
    plt.tight_layout(pad=2.0)
    plt.savefig("Simulation_Results_3Panels.png", dpi=300) 
    
    print("✅ 스펙트럼 시각화 완료: 'Simulation_Results_3Panels.png' 파일이 생성되었습니다.")
    
    # 이미지 팝업 표시 (반드시 savefig 이후에 호출해야 함)
    plt.show()

if __name__ == "__main__":
    main()
