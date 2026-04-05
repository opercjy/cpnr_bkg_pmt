#include "MyPhysicsList.hh"
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"

MyPhysicsList::MyPhysicsList() : Shielding() {
    // 1. 광학 물리(Scintillation, Cherenkov 등) 추가
    G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
    this->RegisterPhysics(opticalPhysics);
    
    // 2. 광학 파라미터 최적화 및 제어
    G4OpticalParameters* optParam = G4OpticalParameters::Instance();
    optParam->SetScintTrackSecondariesFirst(true);
    optParam->SetCerenkovTrackSecondariesFirst(true);
    
    // PMT 광전 음극(Boundary)에서 흡수 및 검출(Detection) 판정을 받은 광자를 
    // Sensitive Detector(PMTSD)로 즉시 보고하도록 강제하는 Geant4 v11+ 필수 설정
    optParam->SetBoundaryInvokeSD(true);
}

MyPhysicsList::~MyPhysicsList() {}

void MyPhysicsList::SetCuts() {
    Shielding::SetCuts();
}