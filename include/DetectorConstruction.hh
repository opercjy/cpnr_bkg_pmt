#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction {
  public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct() override;
    virtual void ConstructSDandField() override;

  private:
    G4LogicalVolume* fLogicWorld;
    G4LogicalVolume* fLogicShieldPb;
    G4LogicalVolume* fLogicShieldCu;
    G4LogicalVolume* fLogicHousing;
    G4LogicalVolume* fLogicReflector;
    G4LogicalVolume* fLogicNaI;
    G4LogicalVolume* fLogicGrease;
    G4LogicalVolume* fLogicWindow;
    G4LogicalVolume* fLogicCathode;
    G4LogicalVolume* fLogicPMTBody;
};

#endif
