#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"

EventAction::EventAction() 
  : G4UserEventAction(), fEdepNaI(0.), fEdepAl(0.), fTotalNPE(0), fTriggerTime(-1.0) {}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*) {
    fEdepNaI = 0.;
    fEdepAl = 0.;
    fTotalNPE = 0;
    fTriggerTime = -1.0; // 매 이벤트마다 트리거 대기 상태로 초기화
}

void EventAction::AddEdepNaI(G4double edep, G4double time) {
    if (fTriggerTime < 0.) fTriggerTime = time; // 에너지가 처음 침적된 순간 게이트 오픈!
    
    // 트리거 시점으로부터 10us Window 안에 들어온 에너지만 합산
    if (time - fTriggerTime <= 10.0 * microsecond) {
        fEdepNaI += edep;
    }
}

void EventAction::AddEdepAl(G4double edep, G4double time) {
    if (fTriggerTime < 0.) fTriggerTime = time;
    if (time - fTriggerTime <= 10.0 * microsecond) {
        fEdepAl += edep;
    }
}

void EventAction::EndOfEventAction(const G4Event*) { 

    if (fEdepNaI > 0. || fEdepAl > 0. || fTotalNPE > 0) {
        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->FillNtupleDColumn(0, fEdepNaI);
        analysisManager->FillNtupleDColumn(1, fEdepAl);
        analysisManager->FillNtupleIColumn(2, fTotalNPE);
        analysisManager->AddNtupleRow();
    }
}