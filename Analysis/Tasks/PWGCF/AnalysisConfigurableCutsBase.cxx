// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <TTree.h>
#include <TFile.h>

#include "Framework/AnalysisDataModel.h"
#include "Framework/ASoAHelpers.h"
#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"
#include "Framework/ASoAHelpers.h"

#include "AnalysisConfigurableCutsBase.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::analysis;

/// \file AnalysisConfigurableCutsBase.cxx
/// \brief Implementation of base analysis configurable cuts class

/// \cond CLASSIMP
ClassImp(AnalysisConfigurableCutsBase);
/// \endcond

TString AnalysisConfigurableCutsBase::fgPeriodName = "";
const char* AnalysisConfigurableCutsBase::fgDataPeriod = "";
const char* AnalysisConfigurableCutsBase::fgAnchorPeriod = "";
AnalysisConfigurableCutsBase::EnergyValue AnalysisConfigurableCutsBase::fgEnergy = AnalysisConfigurableCutsBase::kUnset;
Bool_t AnalysisConfigurableCutsBase::fgIsMC = kFALSE;
Bool_t AnalysisConfigurableCutsBase::fgIsMConlyTruth = kFALSE;

/// Default constructor for serialization
AnalysisConfigurableCutsBase::AnalysisConfigurableCutsBase() : TNamed(),
                                                               fNParams(0),
                                                               fNCuts(0),
                                                               fQALevel(kQALevelLight),
                                                               fParameters(nullptr),
                                                               fCutsEnabledMask(TBits()),
                                                               fCutsActivatedMask(TBits()),
                                                               fDataPeriod(""),
                                                               fHistogramsList(nullptr)
{
}

/// Constructor
/// Allocates the needed memory for the number of cuts to support
/// \param nCuts the number of cuts to support
/// \param nParams the number of cuts parameters
/// \param name name of the event cuts
/// \param title title of the event cuts
AnalysisConfigurableCutsBase::AnalysisConfigurableCutsBase(Int_t nCuts, Int_t nParams, const char* name, const char* title) : TNamed(name, title),
                                                                                                                              fNParams(nParams),
                                                                                                                              fNCuts(nCuts),
                                                                                                                              fQALevel(kQALevelLight),
                                                                                                                              fCutsEnabledMask(TBits(nCuts)),
                                                                                                                              fCutsActivatedMask(TBits(nCuts)),
                                                                                                                              fDataPeriod(""),
                                                                                                                              fHistogramsList(nullptr)
{
  fParameters = new Int_t[nParams];
  for (Int_t i = 0; i < nParams; i++) {
    fParameters[i] = 0;
  }
  fCutsEnabledMask.ResetAllBits();
  fCutsActivatedMask.ResetAllBits();
}

/// Destructor
AnalysisConfigurableCutsBase::~AnalysisConfigurableCutsBase()
{
  if (fParameters != nullptr)
    delete[] fParameters;
}

/// The run to analyze has potentially changed
///
/// Stores the needed production information if required
void AnalysisConfigurableCutsBase::NotifyRunGlobal()
{

  TString szLHCPeriod = GetPeriodNameFromDataFilePath();

  /* if period has not changed do nothing */
  if (szLHCPeriod.EqualTo(fgPeriodName))
    return;

  /* period name has changed */
  LOGF(INFO, "Data period has changed. New data period: %s", szLHCPeriod.Data());
  fgPeriodName = szLHCPeriod;
  fgDataPeriod = "";
  fgAnchorPeriod = "";
  fgIsMC = kFALSE;
  fgEnergy = kUnset;

  /* TODO: probably we would need to use run numbers in here */
  /* TODO: we have to set the base system for track cuts */

  fgDataPeriod = "LHC15o";
  fgAnchorPeriod = "LHC15o";
  fgEnergy = kPbPb5TeV;
  fgIsMC = false;
  fgIsMConlyTruth = false;
}

/// Extract the period name from the data file path
/// \return the name of the period associated with the input data file
TString AnalysisConfigurableCutsBase::GetPeriodNameFromDataFilePath()
{
  /* TODO: we have to figure out how to do this in o2 */
  TString szPeriodName = "LHC15o";

  return szPeriodName;
}

/// Get the current run number being (or going to be) analyzed
/// \return the current run number
Int_t AnalysisConfigurableCutsBase::GetCurrentRunNumber()
{
  /* TODO: we have to learn how to do this */
  Int_t runno = -1;
  return runno;
}

/// Prints the cuts information and values in an usere friendly way
/// Ask to each cut to print its own information
void AnalysisConfigurableCutsBase::PrintCutsWithValues() const
{
  // Print out current Cut Selection with value
  LOGF(INFO, "\n=========== %s information ===============", GetName());
  LOGF(INFO, "Cuts values: ");
  for (Int_t i = 0; i < fNParams; i++) {
    LOGF(INFO, "%d", fParameters[i]);
  }
  LOGF(INFO, "\nIndividual cut information");

  for (Int_t i = 0; i < fNParams; i++) {
    PrintCutWithParams(i);
  }
  LOGF(INFO, "=========== %s information end ===========", GetName());
}
