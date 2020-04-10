// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#include "Framework/runDataProcessing.h"
#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"
#include <TROOT.h>
#include <TList.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TProfile3D.h>

#include <cmath>

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;

namespace dptdptcorrelations {
  /* all this bins have to be make configurable */
  int ptbins = 18;
  float ptlow = 0.2, ptup = 2.0;
  int etabins = 16;
  float etalow = -0.8, etaup = 0.8;
  int zvtxbins = 40;
  float zvtxlow = -10.0, zvtxup = 10.0;
  int phibins = 72;
  std::string fTaskConfigurationString = "PendingToConfigure";

  TH1F *fhPtB = nullptr;
  TH1F *fhPtA = nullptr;
  TH1F *fhPtPosB = nullptr;
  TH1F *fhPtPosA = nullptr;
  TH1F *fhPtNegB = nullptr;
  TH1F *fhPtNegA = nullptr;

  TH1F *fhEtaB = nullptr;
  TH1F *fhEtaA = nullptr;

  TH1F *fhPhiB = nullptr;
  TH1F *fhPhiA = nullptr;

  TH2F *fhEtaVsPhiB = nullptr;
  TH2F *fhEtaVsPhiA = nullptr;

  TH2F *fhPtVsEtaB = nullptr;
  TH2F *fhPtVsEtaA = nullptr;
}  /* end namespace dptdptcorrelations */


// Task for <dpt,dpt> correlations analysis
// FIXME: this should really inherit from AnalysisTask but
//        we need GCC 7.4+ for that

using namespace dptdptcorrelations;
struct DptDptCorrelationsFilteredAnalysisTask {

  OutputObj<TDirectory> fOutput{"DptDptCorrelationsGlobalInfo",OutputObjHandlingPolicy::AnalysisObject};

  OutputObj<TH1F> fOutPtA{"fHistPtA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtPosA{"fHistPtPosA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtNegA{"fHistPtNegA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutEtaA{"fHistEtaA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPhiA{"fHistPhiA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH2F> fOutEtaVsPhiA{"CSTaskEtaVsPhiA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH2F> fOutPtVsEtaA{"fhPtVsEtaA_%s", OutputObjHandlingPolicy::AnalysisObject};

//  OutputObj<TProfile3D> fhPt3DB{TProfile3D(Form("fhPt3DB_%s",fTaskConfigurationString.c_str()),"p_{T} vs #eta, #phi and vtx_{z} before;#eta;#phi;vtx_{z}",etabins,etalow,etaup,phibins,0.0,2*TMath::Pi(),zvtxbins,zvtxlow,zvtxup)};
//  OutputObj<TProfile3D> fhPt3DA{TProfile3D(Form("fhPt3DA_%s",fTaskConfigurationString.c_str()),"p_{T} vs #eta, #phi and vtx_{z};#eta;#phi;vtx_{z}",etabins,etalow,etaup,phibins,0.0,2*TMath::Pi(),zvtxbins,zvtxlow,zvtxup)};


  void init(InitContext const&) {
    fhPtA = new TH1F("fHistPtA", "p_{T} distribution for reconstructed;p_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhPtPosA = new TH1F("fHistPtPosA", "P_{T} distribution for reconstructed (#{+});P_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhPtNegA = new TH1F("fHistPtNegA", "P_{T} distribution for reconstructed (#{-});P_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhEtaA = new TH1F("fHistEtaA","#eta distribution for reconstructed;#eta;counts",etabins, etalow, etaup);
    fhPhiA = new TH1F("fHistPhiA","#phi distribution for reconstructed;#phi;counts",360, 0.0, 2*M_PI);
    fhEtaVsPhiA = new TH2F(TString::Format("CSTaskEtaVsPhiA_%s",fTaskConfigurationString.c_str()),"#eta vs #phi;#phi;#eta", 360, 0.0, 2*M_PI, 100, -2.0, 2.0);
    fhPtVsEtaA = new TH2F(TString::Format("fhPtVsEtaA_%s",fTaskConfigurationString.c_str()),"p_{T} vs #eta;#eta;p_{T} (GeV/c)",etabins,etalow,etaup,200,0.0,10.0);

    fOutPtA.setObject(fhPtA);
    fOutPtPosA.setObject(fhPtPosA);
    fOutPtNegA.setObject(fhPtNegA);
    fOutEtaA.setObject(fhEtaA);
    fOutPhiA.setObject(fhPhiA);
    fOutEtaVsPhiA.setObject(fhEtaVsPhiA);
    fOutPtVsEtaA.setObject(fhPtVsEtaA);
  }

  Filter etaFilter = (aod::track::tgl < float(tan(0.5*M_PI - 2.0*atan(exp(etalow))))) && (aod::track::tgl > float(tan(0.5*M_PI - 2.0*atan(exp(etaup)))));
  Filter ptFilter = ((aod::track::signed1Pt < float(1.0/ptlow)) && (aod::track::signed1Pt > float(1.0/ptup))) || ((aod::track::signed1Pt < - float(1.0/ptup)) && (aod::track::signed1Pt > - float(1.0/ptlow)));

  void process(aod::Collision const& collision, soa::Filtered<aod::Tracks> const& ftracks)
  {

    LOGF(INFO,"Alive! Got a new collision with %d filtered tracks", ftracks.size());
    for (auto& track : ftracks) {
      fhPtA->Fill(track.pt());
      fhEtaA->Fill(track.eta());
      fhPhiA->Fill(track.phi());
      fhEtaVsPhiA->Fill(track.phi(),track.eta());
      fhPtVsEtaA->Fill(track.eta(),track.pt());
      if (track.charge() > 0) {
        fhPtPosA->Fill(track.pt());
      }
      else {
        fhPtNegA->Fill(track.pt());
      }
    }
  }

};


struct DptDptCorrelationsUnFilteredAnalysisTask {

  OutputObj<TH1F> fOutPtB{"fHistPtB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtPosB{"fHistPtPosB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtNegB{"fHistPtNegB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutEtaB{"fHistEtaB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPhiB{"fHistPhiB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH2F> fOutEtaVsPhiB{"CSTaskEtaVsPhiB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH2F> fOutPtVsEtaB{"fhPtVsEtaB_%s", OutputObjHandlingPolicy::AnalysisObject};

  void init(InitContext const&) {
    fhPtB = new TH1F("fHistPtB", "p_{T} distribution for reconstructed before;p_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhPtPosB = new TH1F("fHistPtPosB", "P_{T} distribution for reconstructed (#{+}) before;P_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhPtNegB = new TH1F("fHistPtNegB", "P_{T} distribution for reconstructed (#{-}) before;P_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhEtaB = new TH1F("fHistEtaB","#eta distribution for reconstructed before;#eta;counts",etabins, etalow, etaup);
    fhPhiB = new TH1F("fHistPhiB","#phi distribution for reconstructed before;#phi;counts",360, 0.0, 2*M_PI);
    fhEtaVsPhiB = new TH2F(TString::Format("CSTaskEtaVsPhiB_%s",fTaskConfigurationString.c_str()),"#eta vs #phi before;#phi;#eta", 360, 0.0, 2*M_PI, 100, etalow, etaup);
    fhPtVsEtaB = new TH2F(TString::Format("fhPtVsEtaB_%s",fTaskConfigurationString.c_str()),"p_{T} vs #eta before;#eta;p_{T} (GeV/c)",etabins,etalow,etaup,200,0.0,10.0);

    fOutPtB.setObject(fhPtB);
    fOutPtPosB.setObject(fhPtPosB);
    fOutPtNegB.setObject(fhPtNegB);
    fOutEtaB.setObject(fhEtaB);
    fOutPhiB.setObject(fhPhiB);
    fOutEtaVsPhiB.setObject(fhEtaVsPhiB);
    fOutPtVsEtaB.setObject(fhPtVsEtaB);
  }

  void process(aod::Collision const& collision, aod::Tracks const& uftracks)
  {

    LOGF(INFO,"Alive! Got a new collision with %d unfiltered tracks", uftracks.size());
    for (auto& track : uftracks) {
      fhPtB->Fill(track.pt());
      fhEtaB->Fill(track.eta());
      fhPhiB->Fill(track.phi());
      fhEtaVsPhiB->Fill(track.phi(),track.eta());
      fhPtVsEtaB->Fill(track.eta(),track.pt());
      if (track.charge() > 0) {
        fhPtPosB->Fill(track.pt());
      }
      else {
        fhPtNegB->Fill(track.pt());
      }
    }
  }

};

// Task for building <dpt,dpt> correlations
struct DptDptCorrelationsTask {
  OutputObj<TH1F> fhPtPlus{TH1F("fHistPtPlus", "p_{T} distribution for reconstructed (+);p_{T} (GeV/c);dN/dP_{T} (c/GeV)", 50, 0.0, 5.0)};
  OutputObj<TH1F> fhPtMinus{TH1F("fHistPtMinus", "p_{T} distribution for reconstructed (-);p_{T} (GeV/c);dN/dP_{T} (c/GeV)", 50, 0.0, 5.0)};
  OutputObj<TH2F> etaphiplusH{TH2F("etaphiplus", "etaphi (+)", 100, 0., 2. * M_PI, 102, -2.01, 2.01)};
  OutputObj<TH2F> etaphiminusH{TH2F("etaphiminus", "etaphi (-)", 100, 0., 2. * M_PI, 102, -2.01, 2.01)};

  void init(InitContext const&) {
	etaphiplusH->SetMarkerStyle(kFullCircle);
	etaphiminusH->SetMarkerStyle(kFullCircle);
  }

  void process(aod::Collisions,aod::Tracks const& tracks)
  {
    for (auto& track : tracks) {
      if (track.charge() < 0) {
    	fhPtMinus->Fill(track.pt());
        etaphiminusH->Fill(track.phi(), track.eta());
      }
      else {
      	fhPtPlus->Fill(track.pt());
        etaphiplusH->Fill(track.phi(), track.eta());
      }
    }
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const&)
{
  return WorkflowSpec{
    adaptAnalysisTask<DptDptCorrelationsFilteredAnalysisTask>("DptDptCorrelationsFilteredAnalysisTask"),
    adaptAnalysisTask<DptDptCorrelationsUnFilteredAnalysisTask>("DptDptCorrelationsUnFilteredAnalysisTask"),
    adaptAnalysisTask<DptDptCorrelationsTask>("DptDptCorrelationsTask"),
  };
}
