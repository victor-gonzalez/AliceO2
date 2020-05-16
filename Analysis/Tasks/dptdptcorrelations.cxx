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
#include "Framework/ASoAHelpers.h"
#include "Analysis/EventSelection.h"
#include "Analysis/Centrality.h"
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

namespace o2
{
  namespace aod
  {
    using CollisionEvSelCent = soa::Join<aod::Collisions, aod::EvSels, aod::Cents>::iterator;
    using FilteredTrack = soa::Filtered<aod::Tracks>::iterator;

    namespace dptdptcorrelations {
      DECLARE_SOA_COLUMN(TrackacceptedAsOne, trackacceptedasone, bool);
      DECLARE_SOA_COLUMN(TrackacceptedAsTwo, trackacceptedatwo, bool);
    } // namespace dptdptcorrelations
    DECLARE_SOA_TABLE(ScannedTracks, "AOD", "SCANNEDTRACKS", dptdptcorrelations::TrackacceptedAsOne, dptdptcorrelations::TrackacceptedAsTwo);
  } // namespace aod
} // namespace o2


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

  /// \enum SystemType
  /// \brief The type of the system under analysis
  enum SystemType {
    kNoSystem = 0,    ///< no system defined
    kpp,              ///< **p-p** system
    kpPb,             ///< **p-Pb** system
    kPbp,             ///< **Pb-p** system
    kPbPb,            ///< **Pb-Pb** system
    kXeXe,            ///< **Xe-Xe** system
    knSystems         ///< number of handled systems
  };

  SystemType fSystem = kNoSystem;
  TH1F *fhCentMultB = nullptr;
  TH1F *fhCentMultA = nullptr;
  TH1F *fhVertexZB = nullptr;
  TH1F *fhVertexZA = nullptr;
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

  bool IsEvtSelected(aod::CollisionEvSelCent const& collision) {
    if (collision.alias()[0]) {
      if (collision.sel7()) {
        return true;
      }
    }
    return false;
  }

  std::tuple<bool,bool> AcceptTrack(aod::FilteredTrack const &track) {
    bool asone = false;
    bool astwo = false;

    if (track.charge() > 0) {
      /* we have to check here with the configured track one */
      asone = true; /* for the time being +- correlations */
    }
    else if (track.charge() < 0) {
      /* we have to check here with the configured track one */
      astwo = true; /* for the time being +- correlations */
    }
    return std::make_tuple(asone, astwo); 
  }
}  /* end namespace dptdptcorrelations */


// Task for <dpt,dpt> correlations analysis
// FIXME: this should really inherit from AnalysisTask but
//        we need GCC 7.4+ for that

using namespace dptdptcorrelations;
struct DptDptCorrelationsFilteredAnalysisTask {

  OutputObj<TDirectory> fOutput{"DptDptCorrelationsGlobalInfo",OutputObjHandlingPolicy::AnalysisObject};

  OutputObj<TH1F> fOutCentMultA{"CentralityA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutVertexZA{"VertexZA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtA{"fHistPtA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtPosA{"fHistPtPosA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtNegA{"fHistPtNegA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutEtaA{"fHistEtaA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPhiA{"fHistPhiA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH2F> fOutEtaVsPhiA{"CSTaskEtaVsPhiA", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH2F> fOutPtVsEtaA{"fhPtVsEtaA", OutputObjHandlingPolicy::AnalysisObject};

  Produces<aod::ScannedTracks> scannedtracks;

  void init(InitContext const&) {
    if(fSystem  > kPbp){
      fhCentMultA = new TH1F("CentralityA","Centrality; centrality (%)",100,0,100);
    }
    else {
      /* for pp, pPb and Pbp systems use multiplicity instead */
      fhCentMultA = new TH1F("MultiplicityA","Multiplicity (%); multiplicity (%)",100,0,100);
    }
    fhVertexZA = new TH1F("VertexZA","Vertex Z; z_{vtx}",60,-15,15);

    fhPtA = new TH1F("fHistPtA", "p_{T} distribution for reconstructed;p_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhPtPosA = new TH1F("fHistPtPosA", "P_{T} distribution for reconstructed (#{+});P_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhPtNegA = new TH1F("fHistPtNegA", "P_{T} distribution for reconstructed (#{-});P_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhEtaA = new TH1F("fHistEtaA","#eta distribution for reconstructed;#eta;counts",etabins, etalow, etaup);
    fhPhiA = new TH1F("fHistPhiA","#phi distribution for reconstructed;#phi;counts",360, 0.0, 2*M_PI);
    fhEtaVsPhiA = new TH2F(TString::Format("CSTaskEtaVsPhiA_%s",fTaskConfigurationString.c_str()),"#eta vs #phi;#phi;#eta", 360, 0.0, 2*M_PI, 100, -2.0, 2.0);
    fhPtVsEtaA = new TH2F(TString::Format("fhPtVsEtaA_%s",fTaskConfigurationString.c_str()),"p_{T} vs #eta;#eta;p_{T} (GeV/c)",etabins,etalow,etaup,200,0.0,10.0);

    fOutCentMultA.setObject(fhCentMultA);
    fOutVertexZA.setObject(fhVertexZA);
    fOutPtA.setObject(fhPtA);
    fOutPtPosA.setObject(fhPtPosA);
    fOutPtNegA.setObject(fhPtNegA);
    fOutEtaA.setObject(fhEtaA);
    fOutPhiA.setObject(fhPhiA);
    fOutEtaVsPhiA.setObject(fhEtaVsPhiA);
    fOutPtVsEtaA.setObject(fhPtVsEtaA);
  }

  float fPi = static_cast<float>(M_PI);

  Filter etaFilter = (aod::track::tgl < (tan(0.5f*fPi - 2.0f*atan(exp(etalow))))) && (aod::track::tgl > (tan(0.5f*fPi - 2.0f*atan(exp(etaup)))));
  Filter ptFilter = ((aod::track::signed1Pt < (1.0f/ptlow)) && (aod::track::signed1Pt > (1.0f/ptup))) || ((aod::track::signed1Pt < - (1.0f/ptup)) && (aod::track::signed1Pt > - (1.0f/ptlow)));

  void process(aod::CollisionEvSelCent const& collision, soa::Filtered<aod::Tracks> const& ftracks)
  {
//    LOGF(INFO,"New collision with %d filtered tracks", ftracks.size());
    if (IsEvtSelected(collision)) {
      fhCentMultA->Fill(collision.centV0M());
      fhVertexZA->Fill(collision.posZ());
//      LOGF(INFO,"New accepted collision with %d filtered tracks", ftracks.size());

      for (auto &track : ftracks) {
        auto [asone,astwo] = AcceptTrack(track);        
        if (asone or astwo) {
          /* the track has been accepted */
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
        scannedtracks(asone,astwo);
      }
//      for (auto& [t0, t1] : combinations(acceptedTracks, acceptedTracks)) {
//        LOGF(info, "Tracks pair: %d %d", t0.index(), t1.index());
//      }
    }
  }
};


struct DptDptCorrelationsUnFilteredAnalysisTask {

  OutputObj<TH1F> fOutCentMultB{"CentralityB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutVertexZB{"VertexZB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtB{"fHistPtB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtPosB{"fHistPtPosB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPtNegB{"fHistPtNegB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutEtaB{"fHistEtaB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH1F> fOutPhiB{"fHistPhiB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH2F> fOutEtaVsPhiB{"CSTaskEtaVsPhiB", OutputObjHandlingPolicy::AnalysisObject};
  OutputObj<TH2F> fOutPtVsEtaB{"fhPtVsEtaB_%s", OutputObjHandlingPolicy::AnalysisObject};

  void init(InitContext const&) {
    if(fSystem  > kPbp){
      fhCentMultB = new TH1F("CentralityB","Centrality before cut; centrality (%)",100,0,100);
    }
    else {
      /* for pp, pPb and Pbp systems use multiplicity instead */
      fhCentMultB = new TH1F("MultiplicityB","Multiplicity (%) before cut; multiplicity (%)",100,0,100);
    }
    fhVertexZB = new TH1F("VertexZB","Vertex Z; z_{vtx}",60,-15,15);

    fhPtB = new TH1F("fHistPtB", "p_{T} distribution for reconstructed before;p_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhPtPosB = new TH1F("fHistPtPosB", "P_{T} distribution for reconstructed (#{+}) before;P_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhPtNegB = new TH1F("fHistPtNegB", "P_{T} distribution for reconstructed (#{-}) before;P_{T} (GeV/c);dN/dP_{T} (c/GeV)", ptbins, ptlow, ptup);
    fhEtaB = new TH1F("fHistEtaB","#eta distribution for reconstructed before;#eta;counts",etabins, etalow, etaup);
    fhPhiB = new TH1F("fHistPhiB","#phi distribution for reconstructed before;#phi;counts",360, 0.0, 2*M_PI);
    fhEtaVsPhiB = new TH2F(TString::Format("CSTaskEtaVsPhiB_%s",fTaskConfigurationString.c_str()),"#eta vs #phi before;#phi;#eta", 360, 0.0, 2*M_PI, 100, etalow, etaup);
    fhPtVsEtaB = new TH2F(TString::Format("fhPtVsEtaB_%s",fTaskConfigurationString.c_str()),"p_{T} vs #eta before;#eta;p_{T} (GeV/c)",etabins,etalow,etaup,200,0.0,10.0);

    fOutCentMultB.setObject(fhCentMultB);
    fOutVertexZB.setObject(fhVertexZB);
    fOutPtB.setObject(fhPtB);
    fOutPtPosB.setObject(fhPtPosB);
    fOutPtNegB.setObject(fhPtNegB);
    fOutEtaB.setObject(fhEtaB);
    fOutPhiB.setObject(fhPhiB);
    fOutEtaVsPhiB.setObject(fhEtaVsPhiB);
    fOutPtVsEtaB.setObject(fhPtVsEtaB);
  }

  void process(aod::CollisionEvSelCent const& collision, aod::Tracks const& uftracks)
  {

//    LOGF(INFO,"New collision with %d unfiltered tracks", uftracks.size());
    fhCentMultB->Fill(collision.centV0M());
    fhVertexZB->Fill(collision.posZ());
    if (IsEvtSelected(collision)) {
//      LOGF(INFO,"New accepted collision with %d unfiltered tracks", uftracks.size());
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

  void process(aod::Collision const& collision,aod::Tracks const& tracks)
  {
//    LOGF(INFO,"New unfiltered collision with z_vtx: %f and with %d unfiltered tracks", collision.posZ(),tracks.size());
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
