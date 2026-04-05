#include "G4Types.hh"
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

// 운영체제 시그널 처리 및 런매니저 제어용 헤더 추가
#include "G4RunManager.hh" 
#include <csignal>         

#include "DetectorConstruction.hh"
#include "MyPhysicsList.hh"
#include "ActionInitialization.hh"

// =========================================================================
// 운영체제 시그널(강제 종료) 감지 및 안전 종료 핸들러
// =========================================================================
void GracefulShutdown(int signum) {
    G4cout << "\n\n=======================================================" << G4endl;
    G4cout << " [Emergency] 시스템 종료 시그널(" << signum << ") 감지!" << G4endl;
    G4cout << " 데이터 파손(Zombie ROOT) 방지를 위해 현재 런을 안전하게 셧다운합니다..." << G4endl;
    G4cout << " 워커 스레드의 현재 이벤트를 마치고 ROOT 데이터 저장을 시작합니다." << G4endl;
    G4cout << "=======================================================\n" << G4endl;

    G4RunManager* runManager = G4RunManager::GetRunManager();
    if (runManager) {
        // 인자로 전달된 'true'는 Soft Abort를 의미합니다.
        // 현재 처리 중인 이벤트까지만 안전하게 계산을 마치고, 남은 런을 취소한 뒤
        // 정상적으로 EndOfRunAction을 호출하여 디스크에 ROOT 파일을 씁니다.
        runManager->AbortRun(true);
    }
}

int main(int argc, char** argv) {
    // OS 강제 종료 시그널 맵핑
    // 사용자의 Ctrl+C(SIGINT) 또는 클러스터 서버의 Job Kill(SIGTERM)을 가로챕니다.
    std::signal(SIGINT, GracefulShutdown);
    std::signal(SIGTERM, GracefulShutdown);

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
