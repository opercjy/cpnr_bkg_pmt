#include "RunAction.hh"
#include "G4AnalysisManager.hh"

RunAction::RunAction() : G4UserRunAction() {
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetDefaultFileType("root");
    analysisManager->SetNtupleMerging(true);
    analysisManager->SetVerboseLevel(1);

    analysisManager->CreateNtuple("EventTree", "Alpha Spectra 905 Data");
    analysisManager->CreateNtupleDColumn("Edep_NaI");
    analysisManager->CreateNtupleDColumn("Edep_Al");
    analysisManager->CreateNtupleIColumn("NPE");
    analysisManager->FinishNtuple();
}

void RunAction::BeginOfRunAction(const G4Run*) {
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->OpenFile();
}

void RunAction::EndOfRunAction(const G4Run*) {
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
}
