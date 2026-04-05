#include "PrimaryGeneratorAction.hh"
#include "G4Event.hh"
#include "G4GeneralParticleSource.hh" // GPS 헤더 필수 포함

PrimaryGeneratorAction::PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(),
  fGPS(nullptr)
{
  // GPS 객체 생성
  fGPS = new G4GeneralParticleSource();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fGPS;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // GPS를 통해 이벤트 발생
  fGPS->GeneratePrimaryVertex(anEvent);
}
