#include "TrackingAction.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4TrackingManager.hh"
#include "G4GenericMessenger.hh" // ★ 매크로 통신용 헤더

TrackingAction::TrackingAction() : G4UserTrackingAction(), fStoreOpticalTraj(false) {
    // 디폴트는 false로 설정 (대규모 런 메모리 보호)
    
    // 매크로에서 제어할 수 있도록 커스텀 UI 명령어 등록
    fMessenger = new G4GenericMessenger(this, "/RI_Sim/tracking/", "Optical photon tracking control");
    
    // /RI_Sim/tracking/storeOpticalTraj 커맨드를 fStoreOpticalTraj 변수와 연결
    fMessenger->DeclareProperty("storeOpticalTraj", fStoreOpticalTraj, 
                                "Set true to store optical photon trajectories for visualization");
}

TrackingAction::~TrackingAction() {
    delete fMessenger;
}

void TrackingAction::PreUserTrackingAction(const G4Track* track) {
    // 광학 광자인 경우에만 매크로에서 설정한 값(fStoreOpticalTraj)을 적용
    if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
        fpTrackingManager->SetStoreTrajectory(fStoreOpticalTraj);
    }
}