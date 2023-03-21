#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"
#include "Generators/GeneratorPythia8.h"
#include "Pythia8/HeavyIons.h"
#include "Pythia8/Pythia.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TRandom3.h"

#include <iostream>
#include <string>
#include <map>
#include <unordered_set>

using namespace Pythia8;

class GeneratorPythia8GapTriggeredHF : public o2::eventgen::GeneratorPythia8 {
public:

  /// default constructor
  GeneratorPythia8GapTriggeredHF() = default;

  /// constructor
  GeneratorPythia8GapTriggeredHF(int inputTriggerRatio = 5, int quarkPdg = 4) {

    mGeneratedEvents = 0;
    mHadronPdg = 0; // unless differently set, we do not trigger on specific hadron species
    mQuarkPdg = quarkPdg;
    mInverseTriggerRatio = inputTriggerRatio;
    mQuarkRapidityMin = -1.5;
    mQuarkRapidityMax = -1.5;
    mHadRapidityMin = -1.5;
    mHadRapidityMax = -1.5;
  }

  ///  Destructor
  ~GeneratorPythia8GapTriggeredHF() = default;

  void addTriggerOnHadron(int hadPdg) { mHadronPdg = hadPdg; };
  void setQuarkRapidity(float yMin, float yMax)
  {
    mQuarkRapidityMin = yMin;
    mQuarkRapidityMax = yMax;
  };
  void setHadronRapidity(float yMin, float yMax)
  {
    mHadRapidityMin = yMin;
    mHadRapidityMax = yMax;
  };

protected:
  //__________________________________________________________________
  bool generateEvent() override {

    // Simple straightforward check to alternate generators
    if (mGeneratedEvents % mInverseTriggerRatio == 0) {
      // Generate event of interest
      bool genOk = false;
      while (!genOk) {
        if (GeneratorPythia8::generateEvent()) {
          genOk = selectEvent(mPythia.event);
        }        
      }
    } else {
      // Generate minimum-bias event
      bool genOk = false;
      while (!genOk) {
        genOk = GeneratorPythia8::generateEvent();
      }
    }

    mGeneratedEvents++;

    return true;
  }

  bool selectEvent(const Pythia8::Event& event)
  {
    bool isGoodAtPartonLevel = false, isGoodAtHadronLevel = (mHadronPdg != 0) ? false : true;
    for (auto iPart{0}; iPart < event.size(); ++iPart) {

      // search for Q-Qbar mother with at least one Q in rapidity window
      if (!isGoodAtPartonLevel) {
        auto daughterList = event[iPart].daughterList();
        bool hasQ = false, hasQbar = false, atSelectedY = false;
        for (auto iDau : daughterList) {
          if (event[iDau].id() == mQuarkPdg)
            hasQ = true;
          if (event[iDau].id() == -mQuarkPdg)
            hasQbar = true;
          if ((event[iDau].y() > mQuarkRapidityMin) && (event[iDau].y() < mQuarkRapidityMax))
            atSelectedY = true;
        }
        if (hasQ && hasQbar && atSelectedY) {
          isGoodAtPartonLevel = true;
        }
      }

      // search for hadron in rapidity window
      if (!isGoodAtHadronLevel) {
        int id = std::abs(event[iPart].id());
        float rap = event[iPart].y();
        if (id == mHadronPdg && rap > mHadRapidityMin && rap < mHadRapidityMax) {
          isGoodAtHadronLevel = true;
        }
      }

      // we send the trigger
      if (isGoodAtPartonLevel && isGoodAtHadronLevel) {
        return true;
      }
    }

    return false;
  };

private:
  // Interface to override import particles
  Pythia8::Event mOutputEvent;

  // Properties of selection
  int mQuarkPdg;
  float mQuarkRapidityMin;
  float mQuarkRapidityMax;
  int mHadronPdg;
  float mHadRapidityMin;
  float mHadRapidityMax;

  // Control gap-triggering
  unsigned long long mGeneratedEvents;
  int mInverseTriggerRatio;
};

// Predefined generators:

// Charm-enriched
FairGenerator *GeneratorPythia8GapTriggeredCharm(int inputTriggerRatio, float yMin=-1.5, float yMax=1.5) {
  auto myGen = new GeneratorPythia8GapTriggeredHF(inputTriggerRatio, 4);
  myGen->setQuarkRapidity(yMin, yMax);
  return myGen;
}

// Beauty-enriched
FairGenerator *GeneratorPythia8GapTriggeredBeauty(int inputTriggerRatio, float yMin=-1.5, float yMax=1.5) {
  auto myGen = new GeneratorPythia8GapTriggeredHF(inputTriggerRatio, 5);
  myGen->setQuarkRapidity(yMin, yMax);
  return myGen;
}