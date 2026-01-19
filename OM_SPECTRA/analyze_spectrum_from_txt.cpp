#include <vector>
#include <utility>
#include <cstdio>
#include <cmath>
#include <string>

// ROOT
#include <TCanvas.h>
#include <TH1D.h>
#include <TFile.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <TAxis.h>

// ============================================================
TH1D* make_hist_from_txt(const char* infile,
                         double binWidth_keV = 10.0)
{
  FILE* f = fopen(infile, "r");
  if (!f) {
    printf("Cannot open %s\n", infile);
    return nullptr;
  }

  std::vector<std::pair<long long,double>> rows;
  rows.reserve(100000);

  char line[256];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#' || line[0] == '\n') continue;
    long long cnt = 0;
    double E = 0.0;
    if (sscanf(line, "%lld %lf", &cnt, &E) == 2 && cnt > 0) {
      rows.emplace_back(cnt, E);
    }
  }
  fclose(f);

  if (rows.empty()) {
    printf("No data in %s\n", infile);
    return nullptr;
  }

  double minE = rows.front().second;
  double maxE = rows.front().second;
  for (auto &r : rows) {
    if (r.second < minE) minE = r.second;
    if (r.second > maxE) maxE = r.second;
  }

  const double xmin_hist = std::min(0.0, std::floor(minE / binWidth_keV) * binWidth_keV);
  const double xmax_hist = std::max(3600.0, std::ceil (maxE / binWidth_keV) * binWidth_keV);
  const int    nbins     = (int)std::ceil((xmax_hist - xmin_hist) / binWidth_keV);

  TString base = gSystem->BaseName(infile); 
  base.ReplaceAll(".txt", "");
  TString hname  = "h_" + base;
  TString htitle = "Energy spectrum " + base;

  TH1D* h = new TH1D(hname, htitle,
                     nbins, xmin_hist, xmin_hist + nbins*binWidth_keV);

  for (auto &r : rows) {
    h->Fill(r.second, (double)r.first);
  }

  h->SetLineColor(kBlue+2);
  h->SetLineWidth(1);
  h->SetFillColor(kBlue-9);   
  h->SetFillStyle(1001);      
  h->SetOption("HIST");       

  printf("Created hist %s from %s: %d bins, X[%g, %g] keV (bin=%g keV)\n",
         hname.Data(), infile,
         nbins, xmin_hist, xmin_hist + nbins*binWidth_keV, binWidth_keV);

  return h;
}

// ============================================================
void analyze_spectrum_from_txt(const char* indir =
    "/sps/nemo/scratch/ykozina/Falaise/tutorial/CalibrationScript/Tutorial/OMdata_txt_to_hist",
    double binWidth_keV = 10.0,
    const char* outroot = "OM_spectra.root")
{
  gStyle->SetOptStat(0);

  TSystemDirectory dir("specdir", indir);
  TList* files = dir.GetListOfFiles();
  if (!files) {
    printf("No files found in directory %s\n", indir);
    return;
  }

  TFile fout(outroot, "RECREATE");
  if (fout.IsZombie()) {
    printf("Cannot create output ROOT file %s\n", outroot);
    return;
  }

  TIter next(files);
  TSystemFile* file;
  int nHists = 0;

  while ((file = (TSystemFile*)next())) {
    TString fname = file->GetName();

    if (file->IsDirectory()) continue;
    if (!fname.EndsWith(".txt")) continue; 

    TString fullpath = TString(indir) + "/" + fname;

    TH1D* h = make_hist_from_txt(fullpath.Data(), binWidth_keV);
    if (!h) continue;

    h->GetXaxis()->SetRangeUser(0.0, 3600.0);

    h->Write();
    nHists++;
  }

  fout.Close();

  printf("Done. Wrote %d histograms to %s\n", nHists, outroot);
}

