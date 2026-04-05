#ifndef MyPhysicsList_h
#define MyPhysicsList_h 1

#include "Shielding.hh"

class MyPhysicsList: public Shielding {
  public:
    MyPhysicsList();
    virtual ~MyPhysicsList();
    virtual void SetCuts() override;
};

#endif
