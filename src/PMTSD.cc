#include "PMTSD.hh"
#include "EventAction.hh"
#include "G4EventManager.hh"  
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"

PMTSD::PMTSD(const G4String& name) : G4VSensitiveDetector(name), fNPE(0) {}

void PMTSD::Initialize(G4HCofThisEvent*) {
    fNPE = 0; // 이벤트 시작 시 NPE 초기화
}

G4bool PMTSD::ProcessHits(G4Step* step, G4TouchableHistory*) {
    G4Track* track = step->GetTrack();

    if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) return false;

    // 상태 머신 타이밍 불일치 방어
    fNPE++; 
    
    // 중복 카운팅을 방지하기 위해 광자를 즉시 소멸시킵니다.
    track->SetTrackStatus(fStopAndKill); 
    return true;
}

void PMTSD::EndOfEvent(G4HCofThisEvent*) {
    if (fNPE > 0) {
        auto eventAction = static_cast<EventAction*>(
            G4EventManager::GetEventManager()->GetUserEventAction()
        );
        if (eventAction) eventAction->AddNPE(fNPE);
    }
}