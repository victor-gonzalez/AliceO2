// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#ifndef ANALYSIS_CONFIGURABLE_CUTS_CLASSES_H
#define ANALYSIS_CONFIGURABLE_CUTS_CLASSES_H

#include <Rtypes.h>
#include <TString.h>
#include <TObject.h>
#include <TNamed.h>
#include <TMath.h>
#include <TList.h>

namespace o2
{
namespace analysis
{
/// \class EventSelectionCuts
/// \brief Class which implements configurable event selection cuts
///
class EventSelectionCuts
{
 private:
  int mOfflinetrigger = 1;                 ///< offline trigger, default MB = 1
  TString mCentmultestimator = "V0M";      ///< centrality / multiplicity estimation, default V0M
  int mRemovepileupcode = 1;               ///< Procedure for pile-up removal, default V0M vs TPCout tracks = 1
  TString mRemovepileupfn = "-2500+5.0*x"; ///< function for pile-up removal, procedure dependent, defaul V0M vs TPCout tracks for LHC15o HIR
  std::vector<std::vector<float>> mVertexZ{{-7.0, 7.0}, {-3.0, 3.0}, {-10.0, 10.0}};

 public:
  EventSelectionCuts(int ot, const char* ce, int rp, const char* rpf, std::vector<std::vector<float>> vtx) : mOfflinetrigger(ot),
                                                                                                             mCentmultestimator(ce),
                                                                                                             mRemovepileupcode(rp),
                                                                                                             mRemovepileupfn(rpf),
                                                                                                             mVertexZ(vtx)
  {
  }

  EventSelectionCuts() : mOfflinetrigger(0),
                         mCentmultestimator(""),
                         mRemovepileupcode(0),
                         mRemovepileupfn(""),
                         mVertexZ({{}})
  {
  }

 private:
  ClassDefNV(EventSelectionCuts, 1);
};

/// \class DptDptBinningCuts
/// \brief Class which implements configurable acceptance cuts
///
class DptDptBinningCuts
{
 public:
  int mZVtxbins = 28;       ///< the number of z_vtx bins default 28
  float mZVtxmin = -7.0;    ///< the minimum z_vtx value, default -7.0 cm
  float mZVtxmax = 7.0;     ///< the maximum z_vtx value, default 7.0 cm
  int mPTbins = 18;         ///< the number of pT bins, default 18
  float mPTmin = 0.2;       ///< the minimum pT value, default 0.2 GeV
  float mPTmax = 2.0;       ///< the maximum pT value, default 2.0 GeV
  int mEtabins = 16;        ///< the number of eta bins default 16
  float mEtamin = -0.8;     ///< the minimum eta value, default -0.8
  float mEtamax = 0.8;      ///< the maximum eta value, default 0.8
  int mPhibins = 72;        ///< the number of phi bins, default 72
  float mPhibinshift = 0.5; ///< the shift in the azimuthal origen, defoult 0.5, i.e half a bin

 private:
  ClassDefNV(DptDptBinningCuts, 1);
};

class SimpleInclusiveCut : public TNamed
{
 public:
  int mX = 1;
  float mY = 2.f;
  SimpleInclusiveCut();
  SimpleInclusiveCut(const char*, int, float);
  SimpleInclusiveCut(const SimpleInclusiveCut&) = default;
  ~SimpleInclusiveCut() override = default;

  SimpleInclusiveCut& operator=(const SimpleInclusiveCut&);

 private:
  ClassDef(SimpleInclusiveCut, 1);
};

/// \class CutBrick
/// \brief Virtual class which implements the base component of the selection cuts
///
template <typename TValueToFilter>
class CutBrick : public TNamed
{
 public:
  CutBrick();
  CutBrick(const char*);
  CutBrick(const CutBrick&) = delete;
  virtual ~CutBrick() override = default;
  CutBrick& operator=(const CutBrick&) = delete;

 public:
  /// Returns whether the cut brick is active alowing the selection
  /// \return true if the brick is active
  bool IsActive() { return mState == kACTIVE; }
  /// Returns whether the cut is brick is incorporated in the selection chain
  bool IsArmed() { return mMode == kSELECTED; }
  /// Pure virtual function. Filters the passed value
  /// The brick will change to active if the passed value
  /// fits within the brick scope
  /// \returns true if the value activated the brick
  virtual bool Filter(const TValueToFilter&) = 0;
  /// Pure virtual function. Return the length needed to code the brick status
  /// The length is in brick units. The actual length is implementation dependent
  /// \returns Brick length in units of bricks
  virtual int Length() = 0;

 protected:
  /// Set the status of the cut significative (or not) for the selection chain
  void Arm(bool doit = true) { doit ? mMode = kSELECTED : mMode = kUNSELECTED; }

 protected:
  /// \enum BrickStatus
  /// \brief The status of the brick
  enum BrickStatus {
    kPASSIVE, ///< if the passed value does not comply whith the brick condition
    kACTIVE   ///< if the passed value comply whith the brick condition
  };
  /// \enum BrickMode
  /// \brief The mode of operation of the brick
  enum BrickMode {
    kUNSELECTED, ///< if the status of the brick is not significative
    kSELECTED    ///< if the status of the brick is significative
  };

  BrickStatus mState = kPASSIVE;
  BrickMode mMode = kUNSELECTED;

  ClassDef(CutBrick, 1);
};

/// \class CutBrickLimit
/// \brief Class which implements a limiting cut brick.
/// The brick will be active if the filtered value is below the limit
template <typename TValueToFilter>
class CutBrickLimit : public CutBrick<TValueToFilter>
{
 public:
  CutBrickLimit();
  CutBrickLimit(const char*, const TValueToFilter&);
  virtual ~CutBrickLimit() override = default;
  CutBrickLimit(const CutBrickLimit&) = delete;
  CutBrickLimit& operator=(const CutBrickLimit&) = delete;

  virtual bool Filter(const TValueToFilter&);
  virtual int Length() { return 1; }

 private:
  TValueToFilter mLimit; ///< the limiting upper value
  ClassDef(CutBrickLimit, 1);
};

/// \class CutBrickThreshold
/// \brief Class which implements a threshold cut brick.
/// The brick will be active if the filtered value is above the threshold
template <typename TValueToFilter>
class CutBrickThreshold : public CutBrick<TValueToFilter>
{
 public:
  CutBrickThreshold();
  CutBrickThreshold(const char*, const TValueToFilter&);
  virtual ~CutBrickThreshold() override = default;
  CutBrickThreshold(const CutBrickThreshold&) = delete;
  CutBrickThreshold& operator=(const CutBrickThreshold&) = delete;

  virtual bool Filter(const TValueToFilter&);
  virtual int Length() { return 1; }

 private:
  TValueToFilter mThreshold; ///< the threshold value
  ClassDef(CutBrickThreshold, 1);
};

/// \class CutBrickRange
/// \brief Class which implements a range cut brick.
/// The brick will be active if the filtered value is within the range
template <typename TValueToFilter>
class CutBrickRange : public CutBrick<TValueToFilter>
{
 public:
  CutBrickRange();
  CutBrickRange(const char*, const TValueToFilter&, const TValueToFilter&);
  virtual ~CutBrickRange() override = default;
  CutBrickRange(const CutBrickRange&) = delete;
  CutBrickRange& operator=(const CutBrickRange&) = delete;

  virtual bool Filter(const TValueToFilter&);
  virtual int Length() { return 1; }

 private:
  TValueToFilter mLow;  ///< the lower value of the range
  TValueToFilter mHigh; ///< the upper value of the range
  ClassDef(CutBrickRange, 1);
};

/// \class CutBrickExtToRange
/// \brief Class which implements an external to range cut brick.
/// The brick will be active if the filtered value is outside the range
template <typename TValueToFilter>
class CutBrickExtToRange : public CutBrick<TValueToFilter>
{
 public:
  CutBrickExtToRange();
  CutBrickExtToRange(const char*, const TValueToFilter&, const TValueToFilter&);
  virtual ~CutBrickExtToRange() override = default;
  CutBrickExtToRange(const CutBrickExtToRange&) = delete;
  CutBrickExtToRange& operator=(const CutBrickExtToRange&) = delete;

  virtual bool Filter(const TValueToFilter&);
  virtual int Length() { return 1; }

 private:
  TValueToFilter mLow;  ///< the lower value of the range
  TValueToFilter mHigh; ///< the upper value of the range
  ClassDef(CutBrickExtToRange, 1);
};

/// \class CutBrickSelectorMultipleRanges
/// \brief Class which implements a string as selector an multiple ranges
/// The brick will be active if the filtered value is inside any of the multiple ranges
/// Otherwise it will be passive
/// Each range might be active in its own if the filtered value is within its scope
template <typename TValueToFilter>
class CutBrickSelectorMultipleRanges : public CutBrick<TValueToFilter>
{
 public:
  CutBrickSelectorMultipleRanges();
  CutBrickSelectorMultipleRanges(const char*, const std::vector<TValueToFilter>&);
  virtual ~CutBrickSelectorMultipleRanges() override = default;
  CutBrickSelectorMultipleRanges(const CutBrickSelectorMultipleRanges&) = delete;
  CutBrickSelectorMultipleRanges& operator=(const CutBrickSelectorMultipleRanges&) = delete;

  virtual bool Filter(const TValueToFilter&);
  /// Return the length needed to code the brick status
  /// The length is in brick units. The actual length is implementation dependent
  /// \returns Brick length in units of bricks
  virtual int Length() { return mActive.size(); }

 private:
  std::vector<TValueToFilter> mEdges; ///< the value of the ranges edges (len = nranges + 1)
  std::vector<bool> mActive;          ///< if the associated range is active with the passed value to filter (len = nranges)
  ClassDef(CutBrickSelectorMultipleRanges, 1);
};

/// \class CutWithVariations
/// \brief Class which implements a cut with its default configuration
/// and its potential variations to be used for systematic tests
template <typename TValueToFilter>
class CutWithVariations : public TNamed
{
 public:
  CutWithVariations();
  CutWithVariations(const char*, const char*, bool);
  CutWithVariations(const TString&);
  virtual ~CutWithVariations() override = default;
  CutWithVariations(const CutWithVariations&) = delete;
  CutWithVariations& operator=(const CutWithVariations&) = delete;

  bool AddDefaultBrick(CutBrick<TValueToFilter>* brick);
  bool AddVariationBrick(CutBrick<TValueToFilter>* brick);
  bool Filter(const TValueToFilter&);
  int Length();

 private:
  bool mAllowSeveralDefaults; ///< true if allows to store several cut default values
  TList mDefaultBricks;       ///< the list with the cut default values bricks
  TList mVariationBricks;     ///< the list with the cut variation values bricks
};

} // namespace analysis
} // namespace o2
#endif // ANALYSIS_CONFIGURABLE_CUTS_CLASSES_H
