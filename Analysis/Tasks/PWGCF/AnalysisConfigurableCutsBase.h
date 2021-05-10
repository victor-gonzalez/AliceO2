// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef ANALYSISCONFIGURABLECUTSBASE_H
#define ANALYSISCONFIGURABLECUTSBASE_H

/// \file AnalysisConfigurableCutsBase.h
/// \brief Base class for the analysis configurable cuts support
///

#include <TBits.h>
#include <TNamed.h>

/// \class AnalysisConfigurableCutsBase
/// \brief Base class for analysis configurable cuts
///
/// Provides support for configuring a variable set of cuts
///

namespace o2
{
namespace analysis
{
class AnalysisConfigurableCutsBase : public TNamed
{
 public:
  /// \enum QALevel
  /// \brief The level of the QA histograms output
  enum QALevel {
    kQALevelNone,  ///< no QA histograms output
    kQALevelLight, ///< light QA histograms output
    kQALevelHeavy  ///< full QA histograms output
  };

  /// \enum EnergyValue
  /// \brief The collision energy for the production period
  enum EnergyValue {
    kUnset = 0,        ///< not defined
    k900GeV = 1,       ///< pp 900 GeV
    k2760GeV = 2,      ///< pp 2.76TeV
    k5TeV = 3,         ///< pp 5 TeV
    k7TeV = 4,         ///< pp 7 TeV
    k8TeV = 5,         ///< pp 8 TeV
    k13TeV = 6,        ///< pp 13 TeV
    kpPb5TeV = 7,      ///< pPb 5 TeV
    kpPb8TeV = 8,      ///< pPb 8 TeV
    kPbPb2760GeV = 9,  ///< PbPb 2.76TeV
    kPbPb5TeV = 10,    ///< PbPb 5 TeV
    kXeXe5440GeV = 11, ///< XeXe 5.44 TeV
  };

  /// \enum baseSystemsTrackingCuts
  /// \brief The ids of the systems used as base for the track cuts
  enum baseSystemsTrackCuts {
    kUnknownBase, ///< no system base
    k2010based,   ///< 2010 based system
    k2011based    ///< 2011 based system
  };

  AnalysisConfigurableCutsBase();
  AnalysisConfigurableCutsBase(Int_t nCuts, Int_t nParams, const char* name = "CS AnalysisCuts", const char* title = "CS AnalysisCuts");
  virtual ~AnalysisConfigurableCutsBase();

  /// Gets the current activated cuts
  /// \return the mask with the activated cuts
  const TBits* GetCutsActivatedMask() { return &fCutsActivatedMask; }

  /// Initializes the cuts
  /// \param name the name to assign to the histograms list
  virtual void InitCuts(const char* name) = 0;
  /// Sets the desired level for the QA histograms output
  /// \param level the desired QA histograms output level
  void SetQALevelOutput(QALevel level) { fQALevel = level; }
  /// Get the histograms list
  /// \return the histograms list
  virtual TList* GetHistogramsList() { return fHistogramsList; }

  /// Processes a potential change in the run number
  /// Pure virtual function
  virtual void NotifyRun() = 0;
  /// A new event is coming
  /// Pure virtual function
  virtual void NotifyCollision() = 0;
  static void NotifyRunGlobal();
  /// Prints the activated cuts mask
  /// \param opt the print options
  virtual void Print(Option_t* opt = "") const { fCutsActivatedMask.Print(opt); }

  /// Get the period name corresponding to the current analysis
  /// \return the period name of the current analysis
  static const char* GetPeriodName() { return fgPeriodName.Data(); }
  /// Get the period corresponding to the current analysis
  /// \return the period code of the current analysis
  static const char* GetGlobalPeriod() { return fgDataPeriod; }
  /// Get the anchor period corresponding to the current analysis
  /// \return the anchor period code of the current analysis
  static const char* GetGlobalAnchorPeriod() { return fgAnchorPeriod; }
  /// Is a Monte Carlo data set
  /// \return kTRUE if it is a Monte Carlo data set, kFALSE otherwise
  static Bool_t IsMC() { return fgIsMC; }
  /// Is a fast Monte Carlo data set (only MC truth)
  /// \return kTRUE if it is a fast Monte Carlo data set, kFALSE otherwise
  static Bool_t IsMConlyTruth() { return fgIsMConlyTruth; }

 protected:
  /// Set the value for a concrete cut
  /// Pure virtual function
  /// \param paramID the ID of the cut of interest
  /// \param value the value to assign to the cut
  /// \return kTRUE if the cut value was accepted
  virtual Bool_t SetCutAndParams(Int_t paramID, Int_t value) = 0;
  void PrintCutsWithValues() const;
  /// Print the cut with its value
  /// Pure virtual function
  /// \param paramID the ID of the cut of interest
  virtual void PrintCutWithParams(Int_t paramID) const = 0;

 private:
  static TString GetPeriodNameFromDataFilePath();
  static Int_t GetCurrentRunNumber();

 private:
  static TString fgPeriodName;       ///< the period name of the ongoing analysis
  static const char* fgDataPeriod;   ///< the global period of ongoing analysis
  static const char* fgAnchorPeriod; ///< the anchor period, different from #fgDataPeriod in case of MC data
  Int_t fNParams;                    ///< the number of cuts parameters
 protected:
  Int_t fNCuts;                          ///< the number of cuts
  QALevel fQALevel;                      ///< the level of the QA histograms output
                                         /// the external values for each of the cuts parameters
  Int_t* fParameters;                    //[fNParams]
  TBits fCutsEnabledMask;                ///< the mask of enabled cuts
  TBits fCutsActivatedMask;              ///< the mask of cut activated for the ongoing event
  const char* fDataPeriod;               ///< the current period under analysis. Occasionally could be different from the global period
  baseSystemsTrackCuts fTrackBaseSystem; ///< the base system for track cuts for current period under analysis

  static EnergyValue fgEnergy;   ///< the collision energy for the analysis period
  static Bool_t fgIsMC;          ///< MC flag from production information
  static Bool_t fgIsMConlyTruth; ///< fast MC flag (only kinematics) from production information

  TList* fHistogramsList; ///< the list of histograms used

  /// Copy constructor
  /// Not allowed. Forced private.
  AnalysisConfigurableCutsBase(const AnalysisConfigurableCutsBase&);
  /// Assignment operator
  /// Not allowed. Forced private.
  /// \return l-value reference object
  AnalysisConfigurableCutsBase& operator=(const AnalysisConfigurableCutsBase&);

  /// \cond CLASSIMP
  ClassDef(AnalysisConfigurableCutsBase, 1);
  /// \endcond
};

} // namespace analysis
} // namespace o2

#endif /* ANALYSISCONFIGURABLECUTSBASE_H */