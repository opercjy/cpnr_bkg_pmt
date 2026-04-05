#include "MyPhysicsList.hh"
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"

MyPhysicsList::MyPhysicsList() : Shielding() {
    // 1. 광학 물리(Scintillation, Cherenkov 등) 추가
    G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
    this->RegisterPhysics(opticalPhysics);
    
    // 2. [메모리 최적화] 2차 입자인 광자를 나중이 아닌 즉시 추적하여 소멸시킴
    G4OpticalParameters* optParam = G4OpticalParameters::Instance();
    optParam->SetScintTrackSecondariesFirst(true);
    optParam->SetCerenkovTrackSecondariesFirst(true);
}

MyPhysicsList::~MyPhysicsList() {}

void MyPhysicsList::SetCuts() {
    Shielding::SetCuts();
}
