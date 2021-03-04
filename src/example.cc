// C++ standard library headers
#include <iostream>
#include <vector>

// ROOT headers
#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <TLorentzVector.h>
#include <TH1D.h>

// our own headers
// these files are located in the include directory
#include "tcnt.hh" // tcnt prints the incremented counter value every second

using std::cout;
using std::cerr;
using std::endl;
using ivanp::tcnt;

int main(int argc, char* argv[]) {
  if (argc != 3) { // require two command line arguments
    cout << "usage: " << argv[0] << " data.root histograms.root\n";
    return 1;
  }

  // open ROOT file for writing
  // this is the file that will contain our histograms
  TFile fout(argv[2],"recreate");
  if (fout.IsZombie()) {
    cerr << "\033[31m" "cannot open output ROOT file" "\033[0m\n";
    return 1;
  }

  // create histograms with the desired binning
  // TH1D constructor arguments: name, title, nbins, xmin, xmax
  TH1D* h_m_yy  = new TH1D("m_yy","",110,105,160);
  TH1D* h_pT_j1 = new TH1D("pT_j1","",500,0,1000);

  // open input ROOT file for reading
  cout << "reading input ROOT file \"" << argv[1] <<'\"'<< endl;
  TFile fin(argv[1]);
  if (fin.IsZombie()) {
    cerr << "\033[31m" "cannot open input ROOT file" "\033[0m\n";
    return 1;
  }

  // get the tree called "HGamData" and make sure it exists
  TTree* tree = fin.Get<TTree>("HGamData");
  if (!tree) {
    cerr << "\033[31m" "cannot get TTree \"HGamData\"" "\033[0m\n";
    return 1;
  }
  cout << '\n';

  // setup readers for the branches of the input TTree
  TTreeReader reader(tree);
  TTreeReaderValue<UInt_t> b_njets(reader,"njets");
  TTreeReaderArray<Float_t>
    b_photon_pt (reader,"photon_pt" ),
    b_photon_eta(reader,"photon_eta"),
    b_photon_phi(reader,"photon_phi"),
    b_photon_m  (reader,"photon_m"  ),
    b_jet_pt (reader,"jet_pt" ),
    b_jet_eta(reader,"jet_eta"),
    b_jet_phi(reader,"jet_phi"),
    b_jet_m  (reader,"jet_m"  );

  // particle containers
  std::vector<TLorentzVector>
    photons(2), // we know that we'll have 2 photons
    jets; // the number of jets will vary between events

  // loop over all events in the data tree
  for (tcnt cnt(reader.GetEntries()); reader.Next(); ++cnt) {

    // construct 4-vectors of photons' momenta
    for (unsigned i=0; i<2; ++i) {
      photons[i].SetPtEtaPhiM(
        b_photon_pt [i],
        b_photon_eta[i],
        b_photon_phi[i],
        b_photon_m  [i]
      );
    }

    // construct 4-vectors of jets' momenta
    const unsigned njets = *b_njets;
    jets.resize(njets);
    for (unsigned i=0; i<njets; ++i) {
      jets[i].SetPtEtaPhiM(
        b_jet_pt [i],
        b_jet_eta[i],
        b_jet_phi[i],
        b_jet_m  [i]
      );
    }

    // 4-momentum of the combined system of two photons
    const TLorentzVector diphoton = photons[0] + photons[1];

    const double m_yy = diphoton.M(); // diphoton mass

    h_m_yy->Fill(m_yy); // fill the diphoton mass histogram

    if (njets < 1) continue; // require at least one jet
                             // for the remained of the loop cycle

    h_pT_j1->Fill(jets[0].Pt()); // fill the leading jet pT histogram

  } // end event loop

  fout.Write(); // write changes to the output file

  // NOTE:
  // Even though the histograms were dynamically allocated
  // using the `new` operator, they don't need to be explicitly deleted.
  // The distructor of the TFile they belong to will deallocate them.

} // end main
