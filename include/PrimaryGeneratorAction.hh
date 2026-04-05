#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

// GPS 헤더 포함
class G4GeneralParticleSource; 
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event*);

  private:
    // G4ParticleGun* fParticleGun; // 기존 Gun은 주석 처리 또는 삭제
    G4GeneralParticleSource* fGPS;  // GPS 포인터 선언
};

#endif
