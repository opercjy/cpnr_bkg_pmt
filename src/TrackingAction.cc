#include "TrackingAction.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4TrackingManager.hh"

TrackingAction::TrackingAction() : G4UserTrackingAction() {}

TrackingAction::~TrackingAction() {}

void TrackingAction::PreUserTrackingAction(const G4Track* track) {
    if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
        fpTrackingManager->SetStoreTrajectory(false);
    }
}
