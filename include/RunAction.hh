#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

class G4Run;

class RunAction : public G4UserRunAction {
  public:
    RunAction();
    virtual ~RunAction() = default;

    virtual void BeginOfRunAction(const G4Run* run) override;
    virtual void EndOfRunAction(const G4Run* run) override;
};

#endif
