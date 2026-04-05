#pragma once
#include "G4VSensitiveDetector.hh"
class EventAction;

class PMTSD : public G4VSensitiveDetector {
  public:
    PMTSD(const G4String& name);
    virtual ~PMTSD() = default;

    virtual void Initialize(G4HCofThisEvent* hce) override;
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    virtual void EndOfEvent(G4HCofThisEvent* hce) override;

  private:
    G4int fNPE;
};
