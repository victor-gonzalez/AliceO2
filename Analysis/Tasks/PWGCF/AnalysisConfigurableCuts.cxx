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

#include "AnalysisConfigurableCuts.h"
#include <regex>

using namespace o2::analysis;

ClassImp(SimpleInclusiveCut);

SimpleInclusiveCut::SimpleInclusiveCut() : TNamed(),
                                           mX(0),
                                           mY(0.0)
{
  //
  // default constructor
  //
}

SimpleInclusiveCut::SimpleInclusiveCut(const char* name, int _x, float _y) : TNamed(name, name),
                                                                             mX(_x),
                                                                             mY(_y)
{
  //
  // explicit constructor
  //
}

SimpleInclusiveCut& SimpleInclusiveCut::operator=(const SimpleInclusiveCut& sic)
{
  //
  // assignment operator
  //
  if (this != &sic) {
    TNamed::operator=(sic);
    mX = sic.mX;
    mY = sic.mY;
  }
  return (*this);
}

/// Default constructor
template <typename TValueToFilter>
CutBrick<TValueToFilter>::CutBrick() : TNamed(),
                                       mState(kPASSIVE),
                                       mMode(kUNSELECTED)
{
}

/// Named constructor
/// \param name The name of the brick
template <typename TValueToFilter>
CutBrick<TValueToFilter>::CutBrick(const char* name) : TNamed(name, name),
                                                       mState(kPASSIVE),
                                                       mMode(kUNSELECTED)
{
}

templateClassImp(CutBrick);

/// Default constructor
template <typename TValueToFilter>
CutBrickLimit<TValueToFilter>::CutBrickLimit() : CutBrick<TValueToFilter>(),
                                                 mLimit(TValueToFilter(0))
{
}

/// Named constructor
/// \param name The name of the brick
/// \param value The limit value
template <typename TValueToFilter>
CutBrickLimit<TValueToFilter>::CutBrickLimit(const char* name, const TValueToFilter& value) : CutBrick<TValueToFilter>(name),
                                                                                              mLimit(value)
{
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
bool CutBrickLimit<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if (value < mLimit) {
    this->mState = this->kACTIVE;
    return true;
  } else {
    this->mState = this->kPASSIVE;
    return false;
  }
}

templateClassImp(CutBrickLimit);

/// Default constructor
template <typename TValueToFilter>
CutBrickThreshold<TValueToFilter>::CutBrickThreshold() : CutBrick<TValueToFilter>(),
                                                         mThreshold(0)
{
}

/// Named constructor
/// \param name The name of the brick
/// \param value The threshold value
template <typename TValueToFilter>
CutBrickThreshold<TValueToFilter>::CutBrickThreshold(const char* name, const TValueToFilter& value) : CutBrick<TValueToFilter>(name),
                                                                                                      mThreshold(value)
{
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
bool CutBrickThreshold<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if (mThreshold <= value) {
    this->mState = this->kACTIVE;
    return true;
  } else {
    this->mState = this->kPASSIVE;
    return false;
  }
}

templateClassImp(CutBrickThreshold);

/// Default constructor
template <typename TValueToFilter>
CutBrickRange<TValueToFilter>::CutBrickRange() : CutBrick<TValueToFilter>(),
                                                 mLow(0),
                                                 mHigh(0)
{
}

/// Named constructor
/// \param name The name of the brick
/// \param low The low value for the cut range
/// \param high The high value for the cut range
template <typename TValueToFilter>
CutBrickRange<TValueToFilter>::CutBrickRange(const char* name, const TValueToFilter& low, const TValueToFilter& high) : CutBrick<TValueToFilter>(name),
                                                                                                                        mLow(low),
                                                                                                                        mHigh(high)
{
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
bool CutBrickRange<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if ((mLow <= value) and (value < mHigh)) {
    this->mState = this->kACTIVE;
    return true;
  } else {
    this->mState = this->kPASSIVE;
    return false;
  }
}

templateClassImp(CutBrickRange);

/// Default constructor
template <typename TValueToFilter>
CutBrickExtToRange<TValueToFilter>::CutBrickExtToRange() : CutBrick<TValueToFilter>(),
                                                           mLow(0),
                                                           mHigh(0)
{
}

/// Named constructor
/// \param name The name of the brick
/// \param low The low value for the cut excluded range
/// \param high The high value for the cut excluded range
template <typename TValueToFilter>
CutBrickExtToRange<TValueToFilter>::CutBrickExtToRange(const char* name, const TValueToFilter& low, const TValueToFilter& high) : CutBrick<TValueToFilter>(name),
                                                                                                                                  mLow(low),
                                                                                                                                  mHigh(high)
{
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
bool CutBrickExtToRange<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if ((value < mLow) or (mHigh <= value)) {
    this->mState = this->kACTIVE;
    return true;
  } else {
    this->mState = this->kPASSIVE;
    return false;
  }
}

templateClassImp(CutBrickExtToRange);

/// Default constructor
template <typename TValueToFilter>
CutBrickSelectorMultipleRanges<TValueToFilter>::CutBrickSelectorMultipleRanges() : CutBrick<TValueToFilter>()
{
}

/// Named constructor
/// \param name The name of the brick
/// \param edges Vector with the ranges edges
template <typename TValueToFilter>
CutBrickSelectorMultipleRanges<TValueToFilter>::CutBrickSelectorMultipleRanges(const char* name, const std::vector<TValueToFilter>& edges) : CutBrick<TValueToFilter>(name)
{
  for (auto edge : edges) {
    mEdges.push_back(edge);
  }
  for (int i = 1; i < mEdges.size(); ++i) {
    mActive.push_back(false);
  }
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
bool CutBrickSelectorMultipleRanges<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if ((mEdges.front() <= value) and (value < mEdges.back())) {
    this->mState = this->kACTIVE;
    for (int i = 0; i < mActive.size(); ++i) {
      if (value < mEdges[i + 1]) {
        mActive[i] = true;
      } else {
        mActive[i] = false;
      }
    }
    return true;
  } else {
    this->mState = this->kPASSIVE;
    for (int i = 0; i < mActive.size(); ++i) {
      mActive[i] = false;
    }
    return false;
  }
}

templateClassImp(CutBriCutBrickSelectorMultipleRangesckExtToRange);

/// Default constructor
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>::CutWithVariations() : TNamed(),
                                                         mAllowSeveralDefaults(false)
{
  mDefaultBricks.SetOwner(true);
  mVariationBricks.SetOwner(true);
}

/// Named constructor
/// \param name The name of the brick
/// \param cutstr The string associated with the cut
/// \param severaldefaults The cut should allow multiple defaults values or not
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>::CutWithVariations(const char* name, const char* cutstr, bool severaldefaults) : TNamed(name, cutstr),
                                                                                                                   mAllowSeveralDefaults(severaldefaults)
{
  mDefaultBricks.SetOwner(true);
  mVariationBricks.SetOwner(true);
}

/// \brief Cut string constructor
/// \param cutstr The cuts string
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>::CutWithVariations(const TString& cutstr) : TNamed(),
                                                                              mAllowSeveralDefaults(false)
{
  mDefaultBricks.SetOwner(true);
  mVariationBricks.SetOwner(true);

  ConstructCutFromString(cutstr);
}

/// \brief Construct the cut from a cut string
/// \param cutstr The cuts string
/// The cut string should have the structure
///    name{def,def,..,def[;alt,alt,...,alt]}
/// where each of the def and alt are basic cut bricks
/// If the cut string is correctly parsed the cut is correctly built
/// if not a fatal exception is rised
template <typename TValueToFilter>
void CutWithVariations<TValueToFilter>::ConstructCutFromString(const TString& cutstr)
{
  std::regex cutregex("^\w+\{[wd.{}]\},}*{\{[wd.{}]\}}{\}|;{\{[wd.{}]\},}*{\{[wd.{}]\}}\}}$",
                      std::regex_constants::ECMAScript | std::regex_constants::icase)
}

/// \brief Stores the brick with a default value for the cut
/// \param brick pointer to the brick to incorporate
/// \returns true if the brick was successfully added
/// If several default values are allowed it is only required
/// that the name of the new default brick were unique
/// If only one default value is allowed it is required that
/// no previous default were stored
/// If any of the above conditions fails the brick is not
/// added and false is returned
template <typename TValueToFilter>
bool CutWithVariations<TValueToFilter>::AddDefaultBrick(CutBrick<TValueToFilter>* brick)
{
  if (mAllowSeveralDefaults) {
    if (mDefaultBricks.FindObject(brick->GetName())) {
      return false;
    } else {
      mDefaultBricks.Add(brick);
      return true;
    }
  } else {
    if (mDefaultBricks.GetEntries() > 0) {
      return false;
    } else {
      mDefaultBricks.Add(brick);
      return true;
    }
  }
}

/// \brief Stores the brick with the variation to the default value for the cut
/// \param brick pointer to the brick to incorporate
/// \returns true if the brick was successfully added
/// It is required that the brick name were unique in the list of variation values brick
template <typename TValueToFilter>
bool CutWithVariations<TValueToFilter>::AddVariationBrick(CutBrick<TValueToFilter>* brick)
{
  if (mVariationBricks.FindObject(brick->GetName)) {
    return false;
  } else {
    mVariationBricks.Add(brick);
    return true;
  }
}

/// Filters the passed value
/// The bricks on the default values list and in the variation
/// values list will change to active or passive accordingly to the passed value
/// \param value The value to filter
/// \returns true if the value activated any of the bricks

template <typename TValueToFilter>
bool CutWithVariations<TValueToFilter>::Filter(const TValueToFilter& value)
{
  bool active = false;
  for (int i = 0; i < mDefaultBricks.GetEntries(); ++i) {
    active = active or ((CutBrick<TValueToFilter>*)mDefaultBricks.At(i))->Filter(value);
  }
  for (int i = 0; i < mVariationBricks.GetEntries(); ++i) {
    active = active or ((CutBrick<TValueToFilter>*)mVariationBricks.At(i))->Filter(value);
  }
  return active;
}

/// Return the length needed to code the cut
/// The length is in brick units. The actual length is implementation dependent
/// \returns Cut length in units of bricks
template <typename TValueToFilter>
int CutWithVariations<TValueToFilter>::Length()
{
  /* TODO: should a single default cut without variations return zero length? */
  int length = 0;
  for (int i = 0; i < mDefaultBricks.GetEntries(); ++i) {
    length += ((CutBrick<TValueToFilter>*)mDefaultBricks.At(i))->Length();
  }
  for (int i = 0; i < mVariationBricks.GetEntries(); ++i) {
    length += ((CutBrick<TValueToFilter>*)mVariationBricks.At(i))->Length();
  }
  return length;
}

templateClassImp(CutWithVariations);