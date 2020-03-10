// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file GPUTPCStartHitsFinder.cxx
/// \author Sergey Gorbunov, Ivan Kisel, David Rohr

#include "GPUTPCStartHitsFinder.h"
#include "GPUTPCTracker.h"
#include "GPUCommonMath.h"

using namespace GPUCA_NAMESPACE::gpu;

#if defined(__HIPCC__) && defined(GPUCA_GPUCODE_DEVICE)
#define HIPGPUsharedref() __attribute__((address_space(3)))
#define HIPGPUglobalref() __attribute__((address_space(1)))
#define HIPGPUconstantref() __attribute__((address_space(4)))
#else
#define HIPGPUsharedref() GPUsharedref()
#define HIPGPUglobalref() GPUglobalref()
#define HIPGPUconstantref()
#endif
#ifdef GPUCA_OPENCL1
#define HIPTPCROW(x) GPUsharedref() MEM_LOCAL(x)
#else
#define HIPTPCROW(x) x
#endif

template <>
GPUdii() void GPUTPCStartHitsFinder::Thread<0>(int /*nBlocks*/, int nThreads, int iBlock, int iThread, GPUsharedref() MEM_LOCAL(GPUSharedMemory) & GPUrestrict() s, processorType& GPUrestrict() trackerX)
{
  // find start hits for tracklets
  HIPGPUconstantref() processorType& GPUrestrict() tracker = (HIPGPUconstantref() processorType&)trackerX;

  if (iThread == 0) {
    s.mIRow = iBlock + 1;
    s.mNRowStartHits = 0;
    if (s.mIRow <= GPUCA_ROW_COUNT - 4) {
      s.mNHits = tracker.mData.mRows[s.mIRow].mNHits;
    } else {
      s.mNHits = -1;
    }
  }
  GPUbarrier();
  GPUglobalref() const MEM_GLOBAL(GPUTPCRow) & GPUrestrict() row = tracker.mData.mRows[s.mIRow];
  GPUglobalref() const MEM_GLOBAL(GPUTPCRow) & GPUrestrict() rowUp = tracker.mData.mRows[s.mIRow + 2];
  for (int ih = iThread; ih < s.mNHits; ih += nThreads) {
    long int lHitNumberOffset = row.mHitNumberOffset;
    unsigned int linkUpData = tracker.mData.mLinkUpData[lHitNumberOffset + ih];

    if (tracker.mData.mLinkDownData[lHitNumberOffset + ih] == CALINK_INVAL && linkUpData != CALINK_INVAL && tracker.mData.mLinkUpData[rowUp.mHitNumberOffset + linkUpData] != CALINK_INVAL) {
#ifdef GPUCA_SORT_STARTHITS
      GPUglobalref() GPUTPCHitId* const GPUrestrict() startHits = tracker.mTrackletTmpStartHits + s.mIRow * tracker.mNMaxRowStartHits;
      unsigned int nextRowStartHits = CAMath::AtomicAddShared(&s.mNRowStartHits, 1);
      CONSTEXPR int errCode = GPUCA_ERROR_ROWSTARTHIT_OVERFLOW;
      if (nextRowStartHits + 1 >= tracker.mNMaxRowStartHits)
#else
      GPUglobalref() GPUTPCHitId* const GPUrestrict() startHits = tracker.mTrackletStartHits;
      unsigned int nextRowStartHits = CAMath::AtomicAdd(&tracker.mCommonMem->nTracklets, 1);
      CONSTEXPR int errCode = GPUCA_ERROR_TRACKLET_OVERFLOW;
      if (nextRowStartHits + 1 >= tracker.mNMaxStartHits)
#endif
      {
        trackerX.CommonMemory()->kernelError = errCode;
        CAMath::AtomicExch(&tracker.mCommonMem->nTracklets, 0);
        break;
      }
      startHits[nextRowStartHits].Set(s.mIRow, ih);
    }
  }
  GPUbarrier();

#ifdef GPUCA_SORT_STARTHITS
  if (iThread == 0) {
    unsigned int nOffset = CAMath::AtomicAdd(&tracker.mCommonMem->nTracklets, s.mNRowStartHits);
    tracker.mRowStartHitCountOffset[s.mIRow] = s.mNRowStartHits;
    if (nOffset + s.mNRowStartHits >= tracker.mNMaxStartHits) {
      trackerX.CommonMemory()->kernelError = GPUCA_ERROR_TRACKLET_OVERFLOW;
      CAMath::AtomicExch(&tracker.mCommonMem->nTracklets, 0);
    }
  }
#endif
}
