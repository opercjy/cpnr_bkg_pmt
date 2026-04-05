#include "DetectorConstruction.hh"
#include "PMTSD.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"
#include "G4Exception.hh"

#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    G4NistManager* nist = G4NistManager::Instance();
    
    G4Material* air    = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
    G4Material* al     = nist->FindOrBuildMaterial("G4_Al");
    G4Material* cu     = nist->FindOrBuildMaterial("G4_Cu");
    G4Material* fr4    = nist->FindOrBuildMaterial("G4_POLYCARBONATE"); 
    
    if (!fr4) {
        G4Exception("DetectorConstruction::Construct", "ERR_MAT_NOT_FOUND",
                    FatalException, "G4_POLYCARBONATE not found in NIST database!");
    }

    G4Material* teflon = nist->FindOrBuildMaterial("G4_TEFLON");
    G4Material* nai    = nist->FindOrBuildMaterial("G4_SODIUM_IODIDE");
    G4Material* glass  = nist->FindOrBuildMaterial("G4_GLASS_PLATE");
    
    nai->GetIonisation()->SetBirksConstant(0.0125 * mm/MeV);

    G4Material* base276L = new G4Material("Base276L_Mat", 1.8*g/cm3, 3);
    base276L->AddMaterial(al, 40*perCent);
    base276L->AddMaterial(cu, 20*perCent);
    base276L->AddMaterial(fr4, 40*perCent);

    G4Material* grease = new G4Material("Grease", 1.0*g/cm3, 2);
    grease->AddElement(nist->FindOrBuildElement("C"), 85*perCent);
    grease->AddElement(nist->FindOrBuildElement("H"), 15*perCent);

// =========================================================================
    // --- NaI(Tl) 광학 특성 및 발광 메커니즘 (Optical Properties) ---
    // =========================================================================
    
    // 1. 기준 광자 에너지 대역 (가시광선 영역)
    // NaI(Tl)의 최대 발광 파장은 약 415 nm이며, 이는 에너지로 환산 시 약 3.0 eV에 해당합니다.
    // 2.5 eV ~ 3.5 eV (약 350 nm ~ 500 nm) 대역을 정의하여 스펙트럼 범위를 모사합니다.
    std::vector<G4double> photonEnergy = {2.5 * eV, 3.0 * eV, 3.5 * eV};

    // 2. 굴절률 (Refractive Index)
    // NaI 결정의 굴절률은 약 1.85입니다. 이 값은 테플론(반사체)이나 
    // 광학 그리스(Optical Grease)와의 경계면에서 스넬의 법칙(Snell's Law)에 따른 
    // 전반사 및 굴절 각도를 결정하는 핵심 인자입니다.
    std::vector<G4double> rindexNaI  = {1.85, 1.85, 1.85};

    // 3. 발광 스펙트럼 (Scintillation Spectrum)
    // 3.0 eV (415 nm)에서 피크(1.0)를 가지며, 양옆 파장대역으로 줄어드는 형태의 발광 강도를 정의합니다.
    std::vector<G4double> scintYield = {0.1, 1.0, 0.1};

    // 4. 자체 흡수 길이 (Absorption Length)
    // NaI(Tl)가 자신이 만들어낸 빛에 대해 얼마나 투명한지를 나타냅니다. 
    // 100 cm는 빛이 결정 내부에서 거의 흡수되지 않고 매끄럽게 통과함을 의미합니다.
    std::vector<G4double> absNaI     = {100.*cm, 100.*cm, 100.*cm};

    G4MaterialPropertiesTable* mptNaI = new G4MaterialPropertiesTable();
    mptNaI->AddProperty("RINDEX", photonEnergy, rindexNaI);
    mptNaI->AddProperty("ABSLENGTH", photonEnergy, absNaI);
    mptNaI->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, scintYield);
    
    // 5. 광수율 (Light Yield)
    // 1 MeV의 에너지가 침적되었을 때 약 38,000개의 광학 광자(Optical Photon)가 생성됩니다.
    mptNaI->AddConstProperty("SCINTILLATIONYIELD", 38000. / MeV);
    
    // ★ [Geant4 v11 필수 파라미터]
    // 첫 번째 발광 컴포넌트가 전체 발광량에서 차지하는 비율(100% = 1.0)을 명시합니다.
    // (이 속성이 누락되면 Geant4 엔진은 광자를 단 한 개도 생성하지 않습니다.)
    mptNaI->AddConstProperty("SCINTILLATIONYIELD1", 1.0); 
    
    // 6. 에너지 분해능 스케일 (Resolution Scale)
    // 1.0은 광자 생성이 순수한 포아송 분포(Poisson statistics)의 통계적 요동을 따름을 의미합니다.
    mptNaI->AddConstProperty("RESOLUTIONSCALE", 1.0);

    // 7. 섬광 감쇠 시간 (Decay Time Constant)
    // NaI(Tl)의 대표적인 형광 감쇠 시간(Decay Time)인 250 ns를 적용합니다.
    mptNaI->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 250. * ns);
    
    nai->SetMaterialPropertiesTable(mptNaI);

    // =========================================================================
    // --- 경계면 매질 광학 특성 (Boundary Optical Properties) ---
    // =========================================================================
    // 빛이 NaI(1.85) -> 그리스(1.50) -> PMT 유리창(1.50) -> 진공(1.0)으로 
    // 이동하는 과정에서 경계면 굴절과 반사가 완벽하게 모사되도록 굴절률을 세팅합니다.
    std::vector<G4double> rindexGlass  = {1.50, 1.50, 1.50};
    std::vector<G4double> rindexGrease = {1.50, 1.50, 1.50};
    std::vector<G4double> rindexVacAir = {1.0, 1.0, 1.0};
    
    G4MaterialPropertiesTable* mptGlass = new G4MaterialPropertiesTable();
    mptGlass->AddProperty("RINDEX", photonEnergy, rindexGlass);
    glass->SetMaterialPropertiesTable(mptGlass);

    G4MaterialPropertiesTable* mptGrease = new G4MaterialPropertiesTable();
    mptGrease->AddProperty("RINDEX", photonEnergy, rindexGrease);
    grease->SetMaterialPropertiesTable(mptGrease);

    G4MaterialPropertiesTable* mptVacAir = new G4MaterialPropertiesTable();
    mptVacAir->AddProperty("RINDEX", photonEnergy, rindexVacAir);
    vacuum->SetMaterialPropertiesTable(mptVacAir);
    air->SetMaterialPropertiesTable(mptVacAir);

    // 테플론 반사체 (98% 반사율)
    G4OpticalSurface* refSurface = new G4OpticalSurface("ReflectorSurface", glisur, ground, dielectric_metal);
    std::vector<G4double> refTeflon = {0.98, 0.98, 0.98};
    G4MaterialPropertiesTable* mptRef = new G4MaterialPropertiesTable();
    mptRef->AddProperty("REFLECTIVITY", photonEnergy, refTeflon);
    refSurface->SetMaterialPropertiesTable(mptRef);

    // 광전 음극 (QE 28%)
    G4OpticalSurface* pmtSurface = new G4OpticalSurface("PhotocathodeSurface", glisur, polished, dielectric_metal);
    std::vector<G4double> refPMT = {0.0, 0.0, 0.0}; 
    std::vector<G4double> effPMT = {0.28, 0.28, 0.28}; 
    G4MaterialPropertiesTable* mptPMT = new G4MaterialPropertiesTable();
    mptPMT->AddProperty("REFLECTIVITY", photonEnergy, refPMT);
    mptPMT->AddProperty("EFFICIENCY", photonEnergy, effPMT);
    pmtSurface->SetMaterialPropertiesTable(mptPMT);

    // =========================================================================
    // 2. 기하 구조 생성
    // =========================================================================
    G4double rNaI         = 2.54*cm;  
    G4double hNaI         = 2.54*cm;  
    G4double thickRefl    = 1.5*mm;   
    G4double thickAl      = 1.0*mm;   
    G4double thickGrease  = 0.1*mm;   
    G4double thickWindow  = 2.0*mm;   
    G4double thickCathode = 0.5*mm;   
    G4double hPMT         = 4.0*cm;   
    G4double hBase        = 1.5*cm;   

    G4Box* solidWorld = new G4Box("World", 1.*m, 1.*m, 1.*m);
    fLogicWorld = new G4LogicalVolume(solidWorld, air, "World");
    G4VPhysicalVolume* physWorld = new G4PVPlacement(0, G4ThreeVector(), fLogicWorld, "World", 0, false, 0, true);

    G4double rCasing = rNaI + thickRefl + thickAl;
    G4double hCasing = (thickRefl + 2.*hNaI + thickGrease + thickWindow + thickCathode + 2.*hPMT) / 2.0;
    G4Tubs* solidCasing = new G4Tubs("AlCasing", 0., rCasing, hCasing + thickAl, 0.*deg, 360.*deg);
    fLogicHousing = new G4LogicalVolume(solidCasing, al, "AlCasing");
    new G4PVPlacement(0, G4ThreeVector(0,0,0), fLogicHousing, "AlCasing", fLogicWorld, false, 0, true);

    G4double currentZ = hCasing; 

    // Reflector Top
    currentZ -= thickRefl / 2.0;
    G4Tubs* refTop = new G4Tubs("RefTop", 0., rNaI + thickRefl, thickRefl/2.0, 0.*deg, 360.*deg);
    G4LogicalVolume* logicRefTop = new G4LogicalVolume(refTop, teflon, "ReflectorTop");
    new G4PVPlacement(0, G4ThreeVector(0, 0, currentZ), logicRefTop, "RefTop", fLogicHousing, false, 0, true);
    currentZ -= thickRefl / 2.0;

    // NaI Crystal & Reflector Side
    currentZ -= hNaI;
    G4Tubs* solidNaI = new G4Tubs("NaI", 0., rNaI, hNaI, 0.*deg, 360.*deg);
    fLogicNaI = new G4LogicalVolume(solidNaI, nai, "NaI");
    new G4PVPlacement(0, G4ThreeVector(0, 0, currentZ), fLogicNaI, "NaI", fLogicHousing, false, 0, true);
    
    G4Tubs* refSide = new G4Tubs("RefSide", rNaI, rNaI + thickRefl, hNaI, 0.*deg, 360.*deg);
    fLogicReflector = new G4LogicalVolume(refSide, teflon, "ReflectorSide");
    new G4PVPlacement(0, G4ThreeVector(0, 0, currentZ), fLogicReflector, "RefSide", fLogicHousing, false, 0, true);
    
    new G4LogicalSkinSurface("RefTopSkin", logicRefTop, refSurface);
    new G4LogicalSkinSurface("RefSideSkin", fLogicReflector, refSurface);
    currentZ -= hNaI;

    // Grease
    currentZ -= thickGrease / 2.0;
    G4Tubs* solidGrease = new G4Tubs("Grease", 0., rNaI, thickGrease/2.0, 0.*deg, 360.*deg);
    fLogicGrease = new G4LogicalVolume(solidGrease, grease, "Grease");
    new G4PVPlacement(0, G4ThreeVector(0, 0, currentZ), fLogicGrease, "Grease", fLogicHousing, false, 0, true);
    currentZ -= thickGrease / 2.0;

    // Window
    currentZ -= thickWindow / 2.0;
    G4Tubs* solidWindow = new G4Tubs("Window", 0., rNaI, thickWindow/2.0, 0.*deg, 360.*deg);
    fLogicWindow = new G4LogicalVolume(solidWindow, glass, "Window");
    new G4PVPlacement(0, G4ThreeVector(0, 0, currentZ), fLogicWindow, "Window", fLogicHousing, false, 0, true);
    currentZ -= thickWindow / 2.0;

    // Cathode
    currentZ -= thickCathode / 2.0;
    G4Tubs* solidCathode = new G4Tubs("Cathode", 0., rNaI, thickCathode/2.0, 0.*deg, 360.*deg);
    fLogicCathode = new G4LogicalVolume(solidCathode, vacuum, "Cathode");
    new G4PVPlacement(0, G4ThreeVector(0, 0, currentZ), fLogicCathode, "Cathode", fLogicHousing, false, 0, true);
    new G4LogicalSkinSurface("PhotocathodeSkin", fLogicCathode, pmtSurface); 
    currentZ -= thickCathode / 2.0;

    // PMT Body
    currentZ -= hPMT;
    G4Tubs* solidPMTBody = new G4Tubs("PMTBody", 0., rNaI, hPMT, 0.*deg, 360.*deg);
    fLogicPMTBody = new G4LogicalVolume(solidPMTBody, vacuum, "PMTBody");
    new G4PVPlacement(0, G4ThreeVector(0, 0, currentZ), fLogicPMTBody, "PMTBody", fLogicHousing, false, 0, true);

    // Base 276L
    G4Tubs* solidBase = new G4Tubs("Base276L", 0., rCasing, hBase, 0.*deg, 360.*deg);
    G4LogicalVolume* logicBase = new G4LogicalVolume(solidBase, base276L, "Base276L");
    G4double baseZ = -(hCasing + thickAl) - hBase;
    new G4PVPlacement(0, G4ThreeVector(0, 0, baseZ), logicBase, "Base276L", fLogicWorld, false, 0, true);

    fLogicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());
    return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
    G4SDManager* sdManager = G4SDManager::GetSDMpointer();
    PMTSD* pmtSD = new PMTSD("PMT_SD");
    sdManager->AddNewDetector(pmtSD);
    SetSensitiveDetector("Cathode", pmtSD);
}