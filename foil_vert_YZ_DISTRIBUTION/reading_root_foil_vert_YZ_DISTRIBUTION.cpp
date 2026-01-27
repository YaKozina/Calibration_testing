#include "/sps/nemo/scratch/ykozina/Falaise/tutorial/MiModule/include/MiEvent.h"

#include <iostream>
#include <cmath>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TCanvas.h"

R__LOAD_LIBRARY(./libMiModule.so);

void reading_root_foil_vert_YZ_DISTRIBUTION()
{
  TFile* f = new TFile("Default.root","READ");
  TTree* s = (TTree*) f->Get("Event");
  if (!s) { std::cerr << "Tree 'Event' not found\n"; return; }

  MiEvent* Eve = new MiEvent();
  s->SetBranchAddress("Eventdata", &Eve);

  // 2D
  TH2D* hYZ = new TH2D("hFoilYZ",
                       "Foil vertices;Y;Z",
                       400, -3000, 3000,
                       400, -2000, 2000);

  TH1D* hY  = new TH1D("hFoilY",
                       "Foil vertex Y;Y;Entries",
                       400, -3000, 3000);

  TH1D* hZ  = new TH1D("hFoilZ",
                       "Foil vertex Z;Z;Entries",
                       400, -3000, 3000);

  long long totVert = 0, totFoil = 0;

  Long64_t N = s->GetEntries();
  for (Long64_t i = 0; i < N; ++i) {
    s->GetEntry(i);

    int nPart = Eve->getPTDNoPart();
    for (int ip = 0; ip < nPart; ++ip) {

      int nVert = Eve->getPTDNoVert(ip);
      for (int iv = 0; iv < nVert; ++iv) {
        totVert++;

        std::string where = Eve->getPTDVertpos(ip, iv);

        if (totVert < 20) {
          std::cout << "Example vertex label: '" << where << "'\n";
        }

        if (where == "reference source plane") {
          double y = Eve->getPTDverY(ip, iv);
          double z = Eve->getPTDverZ(ip, iv);
          if (!std::isfinite(y) || !std::isfinite(z)) continue;

          hYZ->Fill(y, z);
          hY->Fill(y);
          hZ->Fill(z);

          totFoil++;
        }
      }
    }
  }


  TFile* out = new TFile("FoilYZ-1556_calib_to_1556-75-200-mm_run.root", "RECREATE");
  hYZ->Write();
  hY->Write();
  hZ->Write();
  out->Close();

  TCanvas* c2 = new TCanvas("cFoilYZ","Foil YZ",900,700);
  hYZ->SetMinimum(1);
  hYZ->Draw("COLZ");

  TCanvas* cY = new TCanvas("cFoilY","Foil Y",900,600);
  hY->Draw("HIST");

  TCanvas* cZ = new TCanvas("cFoilZ","Foil Z",900,600);
  hZ->Draw("HIST");

  std::cout << "Filled foil vertices: " << totFoil
            << " (hist entries: " << hYZ->GetEntries() << ")\n";

  f->Close();
}

