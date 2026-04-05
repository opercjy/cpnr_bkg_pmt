// --- src/SteppingAction.cc ---
#include "SteppingAction.hh"
#include "EventAction.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolume.hh"

SteppingAction::SteppingAction(EventAction* eventAction)
: G4UserSteppingAction(), fEventAction(eventAction) {}

void SteppingAction::UserSteppingAction(const G4Step* step) {
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep <= 0.) return;

    // ★ 아키텍트 개입: DAQ Gate Window (10 us) 복원
    // G4RadioactiveDecay의 장수명 지연 붕괴 에너지가 단일 이벤트로 Pile-up 되는 왜곡 차단
    G4double globalTime = step->GetPostStepPoint()->GetGlobalTime();
    if (globalTime > 10.0 * microsecond) return;

    // 현재 스텝이 발생한 논리 볼륨 이름 가져오기
    G4LogicalVolume* volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
    G4String volName = volume->GetName();

    // 볼륨 이름 매칭 후 에너지 누적
    if (volName == "NaI") {
        fEventAction->AddEdepNaI(edep);
    } 
    else if (volName == "AlCasing" || volName == "Base276L") {
        fEventAction->AddEdepAl(edep);
    }
}