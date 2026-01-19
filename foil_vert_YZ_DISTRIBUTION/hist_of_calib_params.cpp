#include <TFile.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TSystem.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>
#include <iostream>
#include <algorithm>

static std::vector<std::string> split_semicolon(const std::string &s)
{
  std::vector<std::string> out;
  out.reserve(16);
  std::string item;
  std::stringstream ss(s);
  while (std::getline(ss, item, ';')) {
    // trim spaces
    item.erase(item.begin(), std::find_if(item.begin(), item.end(), [](unsigned char c){ return !std::isspace(c); }));
    item.erase(std::find_if(item.rbegin(), item.rend(), [](unsigned char c){ return !std::isspace(c); }).base(), item.end());
    out.push_back(item);
  }
  return out;
}

static bool to_double(const std::string &s, double &x)
{
  // robust conversion (accepts scientific notation)
  char *end = nullptr;
  x = std::strtod(s.c_str(), &end);
  return end && end != s.c_str();
}

void hist_of_calib_params(const char *filename = "output_1556.csv",
                int nbins = 100,
                const char *outroot = "histograms-1556-calib.root",
                const char *outpdf  = "histograms-1556-calib.pdf")
{
  gStyle->SetOptStat(1110);

  const std::vector<std::string> wanted = {"a","b","chi2_A","chi2_B","loss"};

  // ---------- Pass 1: find column indices from header + compute min/max ----------
  std::ifstream fin(filename);
  if (!fin.is_open()) {
    std::cerr << "ERROR: cannot open file: " << filename << std::endl;
    return;
  }

  std::string line;
  if (!std::getline(fin, line)) {
    std::cerr << "ERROR: empty file\n";
    return;
  }

  // remove leading '#'
  if (!line.empty() && line[0] == '#') line.erase(0,1);

  auto headers = split_semicolon(line);
  if (headers.size() < 2) {
    std::cerr << "ERROR: header parsing failed\n";
    return;
  }

  std::unordered_map<std::string,int> colIndex;
  for (int i = 0; i < (int)headers.size(); ++i) colIndex[headers[i]] = i;

  for (auto &name : wanted) {
    if (!colIndex.count(name)) {
      std::cerr << "ERROR: column '" << name << "' not found in header.\n";
      std::cerr << "Header was: " << line << "\n";
      return;
    }
  }

  std::unordered_map<std::string,double> vmin, vmax;
  for (auto &name : wanted) {
    vmin[name] =  std::numeric_limits<double>::infinity();
    vmax[name] = -std::numeric_limits<double>::infinity();
  }

  long long nrows = 0;
  while (std::getline(fin, line)) {
    if (line.empty()) continue;
    if (!line.empty() && line[0] == '#') continue;

    auto fields = split_semicolon(line);
    if (fields.size() < headers.size()) continue; // skip malformed lines

    bool ok = true;
    for (auto &name : wanted) {
      double x;
      if (!to_double(fields[colIndex[name]], x)) { ok = false; break; }
      vmin[name] = std::min(vmin[name], x);
      vmax[name] = std::max(vmax[name], x);
    }
    if (ok) ++nrows;
  }
  fin.close();

  if (nrows == 0) {
    std::cerr << "ERROR: no valid data rows found.\n";
    return;
  }

  // Small padding so edge values are inside histogram range
  auto pad_range = [](double lo, double hi) {
    if (!(std::isfinite(lo) && std::isfinite(hi))) return std::pair<double,double>(0.0, 1.0);
    if (lo == hi) {
      double p = (lo == 0.0) ? 1.0 : std::abs(lo)*0.1;
      return std::pair<double,double>(lo - p, hi + p);
    }
    double p = 0.02 * (hi - lo);
    return std::pair<double,double>(lo - p, hi + p);
  };

  // ---------- Create histograms ----------
  TH1D *h_a     = nullptr;
  TH1D *h_b     = nullptr;
  TH1D *h_chi2A = nullptr;
  TH1D *h_chi2B = nullptr;
  TH1D *h_loss  = nullptr;

  {
    auto r = pad_range(vmin["a"], vmax["a"]);
    h_a = new TH1D("h_a", "a; a; Entries", nbins, r.first, r.second);
  }
  {
    auto r = pad_range(vmin["b"], vmax["b"]);
    h_b = new TH1D("h_b", "b; b; Entries", nbins, r.first, r.second);
  }
  {
    auto r = pad_range(vmin["chi2_A"], vmax["chi2_A"]);
    h_chi2A = new TH1D("h_chi2_A", "chi2_A; chi2_{A}; Entries", nbins, r.first, r.second);
  }
  {
    auto r = pad_range(vmin["chi2_B"], vmax["chi2_B"]);
    h_chi2B = new TH1D("h_chi2_B", "chi2_B; chi2_{B}; Entries", nbins, r.first, r.second);
  }
  {
    auto r = pad_range(vmin["loss"], vmax["loss"]);
    h_loss = new TH1D("h_loss", "loss; loss; Entries", nbins, r.first, r.second);
  }

  // ---------- Pass 2: fill histograms ----------
  fin.open(filename);
  std::getline(fin, line); // skip header

  while (std::getline(fin, line)) {
    if (line.empty()) continue;
    if (!line.empty() && line[0] == '#') continue;

    auto fields = split_semicolon(line);
    if (fields.size() < headers.size()) continue;

    double xa, xb, xA, xB, xl;
    if (!to_double(fields[colIndex["a"]], xa)) continue;
    if (!to_double(fields[colIndex["b"]], xb)) continue;
    if (!to_double(fields[colIndex["chi2_A"]], xA)) continue;
    if (!to_double(fields[colIndex["chi2_B"]], xB)) continue;
    if (!to_double(fields[colIndex["loss"]], xl)) continue;

    h_a->Fill(xa);
    h_b->Fill(xb);
    h_chi2A->Fill(xA);
    h_chi2B->Fill(xB);
    h_loss->Fill(xl);
  }
  fin.close();

  // ---------- Save ROOT file ----------
  TFile fout(outroot, "RECREATE");
  h_a->Write();
  h_b->Write();
  h_chi2A->Write();
  h_chi2B->Write();
  h_loss->Write();
  fout.Close();

  // ---------- Draw & save PDF ----------
  TCanvas c("c", "CSV histograms", 1200, 800);
  c.Divide(3,2);

  c.cd(1); h_a->Draw("HIST");
  c.cd(2); h_b->Draw("HIST");
  c.cd(3); h_chi2A->Draw("HIST");
  c.cd(4); h_chi2B->Draw("HIST");
  c.cd(5); h_loss->Draw("HIST");

  c.cd(6);
  // just print summary text on last pad
  TPaveText *pt = new TPaveText(.05,.05,.95,.95,"NDC");
  pt->AddText(Form("File: %s", filename));
  pt->AddText(Form("Valid rows: %lld", nrows));
  pt->AddText(Form("Saved: %s", outroot));
  pt->AddText(Form("Saved: %s", outpdf));
  pt->Draw();

  c.SaveAs(outpdf);

  std::cout << "Done.\n"
            << "  rows = " << nrows << "\n"
            << "  wrote ROOT: " << outroot << "\n"
            << "  wrote PDF : " << outpdf  << "\n";
}

