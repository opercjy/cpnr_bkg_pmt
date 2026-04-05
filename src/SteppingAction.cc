#include "SteppingAction.hh"
#include "EventAction.hh"
#include "G4Step.hh"
#include "G4LogicalVolume.hh"

SteppingAction::SteppingAction(EventAction* eventAction)
: G4UserSteppingAction(), fEventAction(eventAction) {}

void SteppingAction::UserSteppingAction(const G4Step* step) {
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep <= 0.) return;

    // 수억 년 뒤에 발생한 붕괴라 할지라도 그 시간값을 그대로 EventAction에 전달합니다.
    G4double globalTime = step->GetPostStepPoint()->GetGlobalTime();

    G4LogicalVolume* volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
    G4String volName = volume->GetName();

    // 에너지가 침적된 절대 시간을 함께 전달
    if (volName == "NaI") {
        fEventAction->AddEdepNaI(edep, globalTime);
    } 
    else if (volName == "AlCasing" || volName == "Base276L") {
        fEventAction->AddEdepAl(edep, globalTime);
    }
}