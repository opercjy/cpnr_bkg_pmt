#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"      
#include "SteppingAction.hh"   
#include "TrackingAction.hh"   // ★ 광자 궤적 암살자 헤더 추가

void ActionInitialization::BuildForMaster() const {
    SetUserAction(new RunAction());
}

void ActionInitialization::Build() const {
    SetUserAction(new PrimaryGeneratorAction());
    SetUserAction(new RunAction());
    
    EventAction* eventAction = new EventAction();
    SetUserAction(eventAction);
    SetUserAction(new SteppingAction(eventAction));
    
    // ★ 메모리 OOM 방지용 TrackingAction 등록! (1억 개 런의 핵심)
    SetUserAction(new TrackingAction());
}
