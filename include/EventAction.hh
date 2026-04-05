#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class EventAction : public G4UserEventAction {
  public:
    EventAction();
    virtual ~EventAction();

    virtual void BeginOfEventAction(const G4Event* event) override;
    virtual void EndOfEventAction(const G4Event* event) override;

    // ★ 시간(time)을 인자로 받아 상대 게이팅 처리를 수행
    void AddEdepNaI(G4double edep, G4double time);
    void AddEdepAl(G4double edep, G4double time);
    void AddNPE(G4int npe) { fTotalNPE += npe; } // 물리적 PMT 광전자 누적

  private:
    G4double fEdepNaI;
    G4double fEdepAl;
    G4int fTotalNPE;
    
    // ★ 실제 DAQ 시스템처럼 작동할 첫 타격 시간(Trigger Time)
    G4double fTriggerTime; 
};

#endif