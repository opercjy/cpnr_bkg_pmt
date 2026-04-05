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
    
    // 1. 물질 정의
    G4Material* air    = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
    G4Material* al     = nist->FindOrBuildMaterial("G4_Al");
    G4Material* cu     = nist->FindOrBuildMaterial("G4_Cu");
    
    // ★ [Segfault 해결] G4_EPOXY 대신 실존하는 G4_POLYCARBONATE 사용
    G4Material* fr4    = nist->FindOrBuildMaterial("G4_POLYCARBONATE"); 
    
    // ★ [방어적 널 체크] 포인터 할당 실패 시 즉각적인 예외 처리
    if (!fr4) {
        G4Exception("DetectorConstruction::Construct", "ERR_MAT_NOT_FOUND",
                    FatalException, "G4_POLYCARBONATE not found in NIST database!");
    }

    G4Material* teflon = nist->FindOrBuildMaterial("G4_TEFLON");
    G4Material* nai    = nist->FindOrBuildMaterial("G4_SODIUM_IODIDE");
    G4Material* glass  = nist->FindOrBuildMaterial("G4_GLASS_PLATE");
    
    // 알파선 퀜칭을 위한 Birks Constant 적용
    nai->GetIonisation()->SetBirksConstant(0.0125 * mm/MeV);

    // Base 276L 불감 물질 세팅 (Al 40%, Cu 20%, FR4 40%, 밀도 1.8 g/cm3)
    G4Material* base276L = new G4Material("Base276L_Mat", 1.8*g/cm3, 3);
    base276L->AddMaterial(al, 40*perCent);
    base276L->AddMaterial(cu, 20*perCent);
    base276L->AddMaterial(fr4, 40*perCent);

    G4Material* grease = new G4Material("Grease", 1.0*g/cm3, 2);
    grease->AddElement(nist->FindOrBuildElement("C"), 85*perCent);
    grease->AddElement(nist->FindOrBuildElement("H"), 15*perCent);

    // --- 광학 특성 ---
    std::vector<G4double> photonEnergy = {2.5 * eV, 3.0 * eV, 3.5 * eV};
    std::vector<G4double> rindexNaI  = {1.85, 1.85, 1.85};
    std::vector<G4double> scintYield = {0.1, 1.0, 0.1};
    std::vector<G4double> absNaI     = {100.*cm, 100.*cm, 100.*cm};

    G4MaterialPropertiesTable* mptNaI = new G4MaterialPropertiesTable();
    mptNaI->AddProperty("RINDEX", photonEnergy, rindexNaI);
    mptNaI->AddProperty("ABSLENGTH", photonEnergy, absNaI);
    mptNaI->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, scintYield);
    mptNaI->AddConstProperty("SCINTILLATIONYIELD", 38000. / MeV);
    mptNaI->AddConstProperty("RESOLUTIONSCALE", 1.0);
    mptNaI->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 250. * ns);
    nai->SetMaterialPropertiesTable(mptNaI);

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

    G4OpticalSurface* refSurface = new G4OpticalSurface("ReflectorSurface", glisur, ground, dielectric_metal);
    std::vector<G4double> refTeflon = {0.98, 0.98, 0.98};
    G4MaterialPropertiesTable* mptRef = new G4MaterialPropertiesTable();
    mptRef->AddProperty("REFLECTIVITY", photonEnergy, refTeflon);
    refSurface->SetMaterialPropertiesTable(mptRef);

    G4OpticalSurface* pmtSurface = new G4OpticalSurface("PhotocathodeSurface", glisur, polished, dielectric_metal);
    std::vector<G4double> refPMT = {0.0, 0.0, 0.0}; 
    std::vector<G4double> effPMT = {0.28, 0.28, 0.28}; 
    G4MaterialPropertiesTable* mptPMT = new G4MaterialPropertiesTable();
    mptPMT->AddProperty("REFLECTIVITY", photonEnergy, refPMT);
    mptPMT->AddProperty("EFFICIENCY", photonEnergy, effPMT);
    pmtSurface->SetMaterialPropertiesTable(mptPMT);

    // 2. 기하 구조 생성
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
