#include "PMTSD.hh"
#include "EventAction.hh"
#include "G4EventManager.hh"  // ★ 스레드 안전성(Thread-safety)을 위한 헤더 추가
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

    // Geant4 물리엔진이 광자를 흡수하고 성공적으로 탐지하면 트랙 중지 및 카운트
    if (track->GetTrackStatus() == fStopAndKill && step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
        fNPE++; 
        return true;
    }
    return false;
}

void PMTSD::EndOfEvent(G4HCofThisEvent*) {
    if (fNPE > 0) {
        // ★ [MT 불안정성 해결] G4RunManager 캐스팅 대신 G4EventManager를 사용하여 스레드 세이프하게 EventAction에 접근
        auto eventAction = static_cast<EventAction*>(
            G4EventManager::GetEventManager()->GetUserEventAction()
        );
        if (eventAction) eventAction->AddNPE(fNPE);
    }
}
