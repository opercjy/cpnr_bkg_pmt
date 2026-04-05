#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "Randomize.hh"

// ★ [컴파일 에러 해결] 단위(MeV)와 포아송 난수(G4Poisson) 함수 헤더 추가
#include "G4SystemOfUnits.hh"
#include "G4Poisson.hh"

EventAction::EventAction() 
  : G4UserEventAction(), fEdepNaI(0.), fEdepAl(0.), fTotalNPE(0) {}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*) {
    fEdepNaI = 0.;
    fEdepAl = 0.;
    fTotalNPE = 0;
}

void EventAction::EndOfEventAction(const G4Event*) {
    // 하이브리드 NPE 역산
    if (fTotalNPE == 0 && fEdepNaI > 0.) {
        // G4SystemOfUnits.hh 덕분에 CLHEP::MeV 대신 그냥 MeV 사용 가능
        G4double meanNPE = (fEdepNaI / MeV) * 38000.0 * 0.28 * 0.65;
        fTotalNPE = G4Poisson(meanNPE);
    }

    if (fEdepNaI > 0. || fEdepAl > 0. || fTotalNPE > 0) {
        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->FillNtupleDColumn(0, fEdepNaI);
        analysisManager->FillNtupleDColumn(1, fEdepAl);
        analysisManager->FillNtupleIColumn(2, fTotalNPE);
        analysisManager->AddNtupleRow();
    }
}
