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

    G4double globalTime = step->GetPostStepPoint()->GetGlobalTime();
    if (globalTime > 10.0 * microsecond) return;

    G4LogicalVolume* volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
    G4String volName = volume->GetName();

    if (volName == "NaI") {
        fEventAction->AddEdepNaI(edep);
    } 
    else if (volName == "AlCasing" || volName == "Base276L") {
        fEventAction->AddEdepAl(edep);
    }
}