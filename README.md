# Calibration_testing
scripts used to produce the energy spectra, vertex distributions e.g.

applied after https://github.com/YaKozina/Energy_Calibration

**OM_SPECTRA** folder contains :

**reading_root_OM_SPECTRA.cpp** - reads root file obtained after MiModule and saves txt files with data for each OM

**T1232_M0_S0_W0_C0_R10-Default_root-apply_1556_calib_to_1556_run_SNCUTS.txt** - **example txt file for OM with M0_S0_W0_C0_R10 keys**, **one** of obtained txt after applying reading_root_OM_SPECTRA.cpp

**analyze_spectrum_from_txt.cpp** - reads all txt files like T1232_M0_S0_W0_C0_R10-Default_root-apply_1556_calib_to_1556_run_SNCUTS.txt and makes **energy spectrum for EACH OM**


**foil_vert_YZ_DISTRIBUTION** folder contains :

**reading_root_foil_vert_YZ_DISTRIBUTION.cpp** - reads root file obtained after MiModule and saves root file with vertex data 

**hist_of_calib_params.cpp** - makes 2D histogram of the foil vertices (Y,Z coordinates)

**MiEvent** - files from MiModule with added features to read vertexes; weren't used for analysis exposed here but might be upraged and used later
