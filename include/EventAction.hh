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

    // SteppingAction과 PMTSD에서 호출할 인라인 누적 함수들
    void AddEdepNaI(G4double edep) { fEdepNaI += edep; }
    void AddEdepAl(G4double edep)  { fEdepAl += edep; }
    void AddNPE(G4int npe)         { fTotalNPE += npe; } // ★ 단순 정수형 합산 함수 추가

  private:
    G4double fEdepNaI;
    G4double fEdepAl;
    
    // ★ 기존 fHCID 제거: HitsCollection 대신 단순 변수로 NPE를 관리합니다.
    G4int fTotalNPE; 
};

#endif
