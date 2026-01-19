//calculating cuts 
#include "/sps/nemo/scratch/ykozina/Falaise/tutorial/MiModule/include/MiEvent.h"

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <string>
#include <sstream>

R__LOAD_LIBRARY(./libMiModule.so);


const std::string OUTPUT_DIR =
    "/sps/nemo/scratch/ykozina/Falaise/tutorial/CalibrationScript/Tutorial/OMdata_txt_to_hist/";

std::string make_om_key(MiGID* gid)
{
    std::ostringstream os;
    os << "T" << gid->gettype()
       << "_M" << gid->getmodule()
       << "_S" << gid->getside()
       << "_W" << gid->getwall()
       << "_C" << gid->getcolumn()
       << "_R" << gid->getrow();
    return os.str();
}

void reading_root_OM_SPECTRA()
{
    TFile* f = new TFile("Default.root");
    TTree* s = (TTree*) f->Get("Event");  // get tree with name "Event"
    if (!s) {
        std::cerr << "Tree 'Event' not found\n";
        f->Close();
        delete f;
        return;
    }

    const int N = s->GetEntries();
    const double BIN_KEV = 10.0;

    std::map< std::string, std::map<long long,long long> > spectraPerOM;

    MiEvent* Eve = new MiEvent();
    s->SetBranchAddress("Eventdata", &Eve);

    for (UInt_t i = 0; i < (UInt_t)N; i++) // loop over events
    {
        s->GetEntry(i);

        MiCD* cd = Eve->getCD();
        if (!cd) continue;

        int ncalo = cd->getnoofcaloh();  
        for (int ih = 0; ih < ncalo; ++ih) {
            MiCDCaloHit* hitp = cd->getcalohit(ih);  
            if (!hitp) continue;

            MiCDCaloHit& hit = *hitp;

            const double E_keV = hit.getE();   
            if (!std::isfinite(E_keV)) continue;

            MiGID* gid = hit.getGID();        
            if (!gid) continue;

            const std::string omKey = make_om_key(gid);

            long long binIdx = (long long) std::floor(E_keV / BIN_KEV);

            spectraPerOM[omKey][binIdx]++;
        }
    }

    for (const auto& omEntry : spectraPerOM) {
        const std::string& omKey     = omEntry.first;
        const auto&        binCounts = omEntry.second;

        std::ostringstream fname;

        fname << OUTPUT_DIR
              << omKey
              << "-Default_root-apply_1556_calib_to_2700_run_SNCUTS.txt";

        std::ofstream out(fname.str());
        if (!out) {
            std::cerr << "Cannot open output file " << fname.str() << "\n";
            continue;
        }

        for (const auto& kv : binCounts) {
            const long long binIdx = kv.first;
            const long long count  = kv.second;
            const double energy_keV_center = (binIdx + 0.5) * BIN_KEV;

            out << count << " "
                << std::fixed << std::setprecision(3)
                << energy_keV_center << "\n";
        }
        out.close();

        std::cout << "OM " << omKey
                  << ": " << binCounts.size()
                  << " bins written to " << fname.str()
                  << " (bin=" << BIN_KEV << " keV)\n";
    }

    f->Close();
    delete f;
}

