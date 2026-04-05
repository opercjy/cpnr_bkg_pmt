#include "G4Types.hh"
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "DetectorConstruction.hh"
#include "MyPhysicsList.hh"
#include "ActionInitialization.hh"

int main(int argc, char** argv) {
    G4UIExecutive* ui = nullptr;
    if (argc == 1) ui = new G4UIExecutive(argc, argv);

    // 1. RunManager 생성 (멀티스레딩 지원 버전)
    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

    // 2. 필터링된 물리 리스트 및 기하 구조 등록
    runManager->SetUserInitialization(new DetectorConstruction());
    runManager->SetUserInitialization(new MyPhysicsList());
    runManager->SetUserInitialization(new ActionInitialization());

    // 3. 시각화 매니저
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    if (!ui) {
        // 배치 모드
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    } else {
        // GUI 대화형 모드
        UImanager->ApplyCommand("/control/execute macros/vis_optics.mac");
        ui->SessionStart();
        delete ui;
    }

    delete visManager;
    delete runManager;

    return 0;
}
