#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"

class G4GenericMessenger;

class TrackingAction : public G4UserTrackingAction {
  public:
    TrackingAction();
    virtual ~TrackingAction();
    virtual void PreUserTrackingAction(const G4Track*) override;

  private:
    G4bool fStoreOpticalTraj;          // 매크로에서 받을 boolean 변수
    G4GenericMessenger* fMessenger;    // UI 커맨드 메신저
};

#endif