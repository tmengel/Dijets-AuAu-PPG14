const double MIN_PT     = 5.0;

int clean_sim( 
    const std::string & infile = "/sphenix/tg/tg01/jets/tmengel/mdc2/may29/hadded_v5/run31_jet10_scaled_may29_flow0_matched_v1/all.root",
    const std::string & outfile = "cleaned_sim.root",
    const double weight = 1.0,
    const double pt1_min = 0.0,
    const double pt1_max = 1e6,
    const double r = 0.3
)
{
    float zvrtx;
    int cent;
    float sumeT_cemc, sumeT_hcalin, sumeT_hcalout, sumeT_all;
    std::vector<float> *rj_pt = 0;
    std::vector<float> *rj_eta = 0;
    std::vector<float> *rj_phi = 0;
    std::vector<float> *rj_e = 0;
    std::vector<float> *rj_unsub_pt = 0;
    std::vector<float> *rj_unsub_e = 0;
    std::vector<float> *tj_pt = 0;
    std::vector<float> *tj_eta = 0;
    std::vector<float> *tj_phi = 0;
    auto * fin = TFile::Open( infile.c_str(), "READ" );
    auto * tin = (TTree*) fin->Get("T");
    tin -> SetBranchStatus("*", 0);
    tin->SetBranchStatus("zvrtx", 1);
    tin->SetBranchStatus("cent", 1);
    tin->SetBranchStatus("sumeT_cemc", 1);
    tin->SetBranchStatus("sumeT_hcalin", 1);
    tin->SetBranchStatus("sumeT_hcalout", 1);
    tin->SetBranchStatus("rj_pt", 1);
    tin->SetBranchStatus("rj_e", 1);
    tin->SetBranchStatus("rj_eta", 1);
    tin->SetBranchStatus("rj_phi", 1);  
    tin->SetBranchStatus("rj_unsub_pt", 1);
    tin->SetBranchStatus("rj_unsub_e", 1);
    tin->SetBranchStatus("tj_pt", 1);
    tin->SetBranchStatus("tj_eta", 1);
    tin->SetBranchStatus("tj_phi", 1);
    tin->SetBranchAddress("zvrtx", &zvrtx);
    tin->SetBranchAddress("cent", &cent);
    tin->SetBranchAddress("sumeT_cemc", &sumeT_cemc);
    tin->SetBranchAddress("sumeT_hcalin", &sumeT_hcalin);
    tin->SetBranchAddress("sumeT_hcalout", &sumeT_hcalout);
    tin->SetBranchAddress("rj_pt", &rj_pt);
    tin->SetBranchAddress("rj_e", &rj_e);
    tin->SetBranchAddress("rj_eta", &rj_eta);
    tin->SetBranchAddress("rj_phi", &rj_phi);
    tin->SetBranchAddress("rj_unsub_pt", &rj_unsub_pt);
    tin->SetBranchAddress("rj_unsub_e", &rj_unsub_e);
    tin->SetBranchAddress("tj_pt", &tj_pt);
    tin->SetBranchAddress("tj_eta", &tj_eta);
    tin->SetBranchAddress("tj_phi", &tj_phi);
    int nentries = tin->GetEntries();
    std::cout << "Number of entries in input file: " << nentries << std::endl;


    auto * fout = TFile::Open( outfile.c_str(), "RECREATE" );
    
    ULong64_t gl1_scaled = 0xFFFFFFFFFFFFFFFF;
    int minbias = 1;
    double weight_factor = weight;
    auto * tout = new TTree("ttree", "cleaned tree");
    tout->Branch("weight", &weight_factor);
    tout->Branch("gl1_scaled", &gl1_scaled);
    tout->Branch("minbias", &minbias);
    tout->Branch("centrality", &cent);
    tout->Branch("mbd_vertex_z", &zvrtx);
    float mbd_Qsum = 0;
    tout->Branch("mbd_Qsum", &mbd_Qsum);
    float mbd_time_zero = 0;
    tout->Branch("mbd_time_zero", &mbd_time_zero);
    tout->Branch("sum_eT_cemc", &sumeT_cemc);
    tout->Branch("sum_eT_hcalin", &sumeT_hcalin);
    tout->Branch("sum_eT_hcalout", &sumeT_hcalout);
    tout->Branch("sum_eT_all", &sumeT_all);

    std::vector<float> rj_pt_cleaned;
    std::vector<float> rj_eta_cleaned;
    std::vector<float> rj_phi_cleaned;
    std::vector<float> rj_e_cleaned;
    std::vector<float> rj_unsub_pt_cleaned;
    std::vector<float> rj_unsub_e_cleaned;
    std::vector<float> tj_pt_cleaned;
    std::vector<float> tj_eta_cleaned;
    std::vector<float> tj_phi_cleaned;
    tout->Branch("jet_pt_3_sub", &rj_pt_cleaned);
    tout->Branch("jet_eta_3_sub", &rj_eta_cleaned);
    tout->Branch("jet_phi_3_sub", &rj_phi_cleaned);
    tout->Branch("jet_e_3_sub", &rj_e_cleaned);
    tout->Branch("jet_pt_unsub_3_sub", &rj_unsub_pt_cleaned);
    tout->Branch("jet_e_unsub_3_sub", &rj_unsub_e_cleaned);
    tout->Branch("jet_pt_3_truth", &tj_pt_cleaned);
    tout->Branch("jet_eta_3_truth", &tj_eta_cleaned);
    tout->Branch("jet_phi_3_truth", &tj_phi_cleaned);

    for (int i = 0; i < nentries; i++)
    {
        tin->GetEntry(i);
        if ( cent <= 0 || cent > 100 ) continue;
        if ( i % (nentries / 10) == 0 ) std::cout << "Processing entry " << i << " / " << nentries << std::endl;
        if (fabs(zvrtx) > 60.0) continue;

        mbd_Qsum = 0; // not available in this file, so just set to zero
        mbd_time_zero = 0; // not available in this file, so just set to zero

        rj_pt_cleaned.clear();
        rj_eta_cleaned.clear();
        rj_phi_cleaned.clear();
        rj_unsub_pt_cleaned.clear();
        rj_unsub_e_cleaned.clear();
        rj_e_cleaned.clear();
        tj_pt_cleaned.clear();
        tj_eta_cleaned.clear();
        tj_phi_cleaned.clear();

        // get pt of leading truth jet
        double lead_truth_pt = -1.0;
        for (const float pt : *tj_pt)
        {
            lead_truth_pt = std::max(lead_truth_pt, static_cast<double>(pt));
        }
        if (lead_truth_pt < pt1_min || lead_truth_pt > pt1_max) continue;

        sumeT_all = sumeT_cemc + sumeT_hcalin + sumeT_hcalout;
        std::vector<std::pair<int, float>> good_reco_jets;
        for (size_t j = 0; j < rj_pt->size(); j++)
        {
            if (rj_pt->at(j) < MIN_PT) continue;
            if (rj_e->at(j) <= 0.0) continue;
            good_reco_jets.emplace_back(j, rj_pt->at(j));
        }
        std::sort(good_reco_jets.begin(), good_reco_jets.end(), [](const std::pair<int, float> &a, const std::pair<int, float> &b) { return a.second > b.second; });

        std::vector<std::pair<int, float>> good_truth_jets;
        for (size_t j = 0; j < tj_pt->size(); j++)
        {
            if (tj_pt->at(j) < 1.0) continue; // only keep truth jets with pt > 1 GeV
            good_truth_jets.emplace_back(j, tj_pt->at(j));
        }
        std::sort(good_truth_jets.begin(), good_truth_jets.end(), [](const std::pair<int, float> &a, const std::pair<int, float> &b) { return a.second > b.second; });

        // match each reco jet to the first truth jet within dR < 0.75*R (highest pt)
        std::vector<bool> truth_matched(tj_pt->size(), false);
        for (const auto &reco_jet : good_reco_jets)
        {
            int reco_idx = reco_jet.first;
            float reco_pt = reco_jet.second;
            float reco_eta = rj_eta->at(reco_idx);
            float reco_phi = rj_phi->at(reco_idx);

            bool matched = false;
            int truth_idx = -1;
            for (const auto &truth_jet : good_truth_jets)
            {
                truth_idx = truth_jet.first;
                if (truth_matched[truth_idx]) continue; // already matched
                float truth_pt = truth_jet.second;
                float truth_eta = tj_eta->at(truth_idx);
                float truth_phi = tj_phi->at(truth_idx);
                float dphi = reco_phi - truth_phi;
                if (dphi > M_PI) dphi -= 2 * M_PI;
                if (dphi < -M_PI) dphi += 2 * M_PI;
                float deta = reco_eta - truth_eta;
                float dR = std::sqrt(deta * deta + dphi * dphi);

                if (dR < 0.75 * r)
                {
                    // match found
                    matched = true;
                    truth_matched[truth_idx] = true;
                    break;
                }
            }

            rj_pt_cleaned.push_back(reco_pt);
            rj_e_cleaned.push_back(rj_e->at(reco_idx));
            rj_eta_cleaned.push_back(reco_eta);
            rj_phi_cleaned.push_back(reco_phi);
            rj_unsub_pt_cleaned.push_back(rj_unsub_pt->at(reco_idx));
            rj_unsub_e_cleaned.push_back(rj_unsub_e->at(reco_idx));
            if (matched)
            {
                tj_pt_cleaned.push_back(tj_pt->at(truth_idx));
                tj_eta_cleaned.push_back(tj_eta->at(truth_idx));
                tj_phi_cleaned.push_back(tj_phi->at(truth_idx));
            }
            else
            {
                tj_pt_cleaned.push_back(-1.0);
                tj_eta_cleaned.push_back(-999.0);
                tj_phi_cleaned.push_back(-999.0);
            }
        }
        // the remaining unmatched truth jets are now added
        for (const auto &truth_jet : good_truth_jets)
        {
            int truth_idx = truth_jet.first;
            if (!truth_matched[truth_idx])
            {
                tj_pt_cleaned.push_back(tj_pt->at(truth_idx));
                tj_eta_cleaned.push_back(tj_eta->at(truth_idx));
                tj_phi_cleaned.push_back(tj_phi->at(truth_idx));
                rj_pt_cleaned.push_back(-1.0);
                rj_e_cleaned.push_back(-1.0);
                rj_eta_cleaned.push_back(-999.0);
                rj_phi_cleaned.push_back(-999.0);
                rj_unsub_pt_cleaned.push_back(-1.0);
                rj_unsub_e_cleaned.push_back(-1.0);
            }
        }
        tout->Fill();
    }

    std::cout << "Writing cleaned data to " << outfile << std::endl;
    fout->cd();
    tout->Write();
    fout->Close();
    std::cout << "Done!" << std::endl;
    return 0;
}

int clean_all_sim()
{
    std::vector<std::string> files = {
        "/sphenix/tg/tg01/jets/tmengel/mdc2/may29/hadded_v5/run31_jet10_scaled_may29_flow0_matched_v1/all.root",
        "/sphenix/tg/tg01/jets/tmengel/mdc2/may29/hadded_v5/run31_jet20_scaled_may29_flow0_matched_v1/all.root",
        "/sphenix/tg/tg01/jets/tmengel/mdc2/may29/hadded_v5/run31_jet30_scaled_may29_flow0_matched_v1/all.root"
    };
    std::vector<std::string>outfiles = {
        "run31_emb_scaled_hijing_jet10_matched.root",
        "run31_emb_scaled_hijing_jet20_matched.root",
        "run31_emb_scaled_hijing_jet30_matched.root"
    };
    std::vector<double> weights = {
        3.997e6/3.0e6, // weight for jet10
        6.2623e4/3.0e6, // weight for jet20
        2.5298e3/3.0e6 // weight for jet30
    };
    std::vector<double> pt1_mins = {
        10.0, // min pt for jet10
        21.0, // min pt for jet20
        31.0  // min pt for jet30
    };
    std::vector<double> pt1_maxs = {
        21.0, // max pt for jet10
        31.0, // max pt for jet20
        41.0  // max pt for jet30
    };

    for (size_t i = 0; i < files.size(); i++)
    {
        std::cout << "Cleaning file: " << files[i] << std::endl;
        clean_sim(files[i], outfiles[i], weights[i], pt1_mins[i], pt1_maxs[i]);
    }
    return 0;
}

int make_calib( 
    const std::string & infile = "/sphenix/user/tmengel/PPG14/macros/jetUEmods/dr03/rootfiles/dr_04_grl_v3.root",
    const std::string & outfile = "calib.root"
)
{

    float zvrtx;
    int cent;
    float mbd_q_N, mbd_q_S;
    float sum_eT_all;
    auto * fin = TFile::Open( infile.c_str(), "READ" );
    auto * tin = (TTree*) fin->Get("T");
    tin -> SetBranchStatus("*", 0);
    tin->SetBranchStatus("zvrtx", 1);
    tin->SetBranchStatus("cent", 1);
    tin->SetBranchStatus("mbd_q_N", 1);
    tin->SetBranchStatus("mbd_q_S", 1);
    tin->SetBranchStatus("sum_eT_all", 1);

    tin->SetBranchAddress("zvrtx", &zvrtx);
    tin->SetBranchAddress("cent", &cent);
    tin->SetBranchAddress("mbd_q_N", &mbd_q_N);
    tin->SetBranchAddress("mbd_q_S", &mbd_q_S);
    tin->SetBranchAddress("sum_eT_all", &sum_eT_all);

    int nentries = tin->GetEntries();
    std::cout << "Number of entries in input file: " << nentries << std::endl;

    TProfile * p_mbdQsum_sumeT_all = new TProfile("p_mbdQsum_sumeT_all", "p_mbdQsum_sumeT_all", 200, 0, 2000, "s");
    TProfile * p_mbdQsum_cent = new TProfile("p_mbdQsum_cent", "p_mbdQsum_cent", 200, 0, 2000, "s");
    TProfile * p_cent_mbdQsum = new TProfile("p_cent_mbdQsum", "p_cent_mbdQsum", 100, 0, 100, "s");
    TProfile * p_cent_sumeT_all = new TProfile("p_cent_sumeT_all", "p_cent_sumeT_all", 100, 0, 100, "s");

    TH2D * h_mbdQsum_sumeT_all = new TH2D("h_mbdQsum_sumeT_all", "h_mbdQsum_sumeT_all", 200, 0, 2000, 200, 0, 2000);
    TH2D * h_mbdQsum_cent = new TH2D("h_mbdQsum_cent", "h_mbdQsum_cent", 200, 0, 2000, 100, 0, 100);
    TH2D * h_cent_mbdQsum = new TH2D("h_cent_mbdQsum", "h_cent_mbdQsum", 100, 0, 100, 200, 0, 2000);
    TH2D * h_cent_sumeT_all = new TH2D("h_cent_sumeT_all", "h_cent_sumeT_all", 100, 0, 100, 200, 0, 2000);

    for (int i = 0; i < nentries; i++)
    {
        tin->GetEntry(i);
        if ( cent <= 0 || cent > 100 ) continue;
        if ( i % (nentries / 10) == 0 ) std::cout << "Processing entry " << i << " / " << nentries << std::endl;
        if (fabs(zvrtx) > 60.0) continue;

        float mbd_Qsum = mbd_q_N + mbd_q_S;
        p_mbdQsum_sumeT_all->Fill(mbd_Qsum, sum_eT_all);
        p_mbdQsum_cent->Fill(mbd_Qsum, cent);
        p_cent_mbdQsum->Fill(cent, mbd_Qsum);
        p_cent_sumeT_all->Fill(cent, sum_eT_all);
        h_mbdQsum_sumeT_all->Fill(mbd_Qsum, sum_eT_all);
        h_mbdQsum_cent->Fill(mbd_Qsum, cent);
        h_cent_mbdQsum->Fill(cent, mbd_Qsum);
        h_cent_sumeT_all->Fill(cent, sum_eT_all);
    }

    TProfile * p_mbdQsum_sumeT_all_post_calib = new TProfile("p_mbdQsum_sumeT_all_post_calib", "p_mbdQsum_sumeT_all_post_calib", 200, 0, 2000, "s");
    TProfile * p_mbdQsum_cent_post_calib = new TProfile("p_mbdQsum_cent_post_calib", "p_mbdQsum_cent_post_calib", 200, 0, 2000, "s");
    TProfile * p_cent_mbdQsum_post_calib = new TProfile("p_cent_mbdQsum_post_calib", "p_cent_mbdQsum_post_calib", 100, 0, 100, "s");
    TProfile * p_cent_sumeT_all_post_calib = new TProfile("p_cent_sumeT_all_post_calib", "p_cent_sumeT_all_post_calib", 100, 0, 100, "s");

    TH2D * h_mbdQsum_sumeT_all_post_calib = new TH2D("h_mbdQsum_sumeT_all_post_calib", "h_mbdQsum_sumeT_all_post_calib", 200, 0, 2000, 200, 0, 2000);
    TH2D * h_mbdQsum_cent_post_calib = new TH2D("h_mbdQsum_cent_post_calib", "h_mbdQsum_cent_post_calib", 200, 0, 2000, 100, 0, 100);
    TH2D * h_cent_mbdQsum_post_calib = new TH2D("h_cent_mbdQsum_post_calib", "h_cent_mbdQsum_post_calib", 100, 0, 100, 200, 0, 2000);
    TH2D * h_cent_sumeT_all_post_calib = new TH2D("h_cent_sumeT_all_post_calib", "h_cent_sumeT_all_post_calib", 100, 0, 100, 200, 0, 2000);

    TProfile * p_mbdQsum_eff = new TProfile("p_mbdQsum_sumeT_all_post_calib_eff", "p_mbdQsum_sumeT_all_post_calib_eff", 200, 0, 2000, "s");
    TProfile * p_cent_eff = new TProfile("p_cent_sumeT_all_post_calib_eff", "p_cent_sumeT_all_post_calib_eff", 100, 0, 100, "s");
    TProfile * p_sumeT_eff = new TProfile("p_sumeT_all_post_calib_eff", "p_sumeT_all_post_calib_eff", 200, 0, 2000, "s");
    // cut on +- 3.5 sigma of mean mbd_Qsum for each centrality bin
    int n_removed = 0;    
    for (int i = 1; i < nentries; i++)
    {
        tin->GetEntry(i);
        if ( cent <= 0 || cent > 100 ) continue;
        if ( i % (nentries / 10) == 0 ) std::cout << "Processing entry " << i << " / " << nentries << std::endl;
        if (fabs(zvrtx) > 60.0) continue;

        float mbd_Qsum = mbd_q_N + mbd_q_S;
        float mean = p_mbdQsum_sumeT_all->GetBinContent(p_mbdQsum_sumeT_all->FindBin(mbd_Qsum));
        float sigma = p_mbdQsum_sumeT_all->GetBinError(p_mbdQsum_sumeT_all->FindBin(mbd_Qsum));
        if (sigma == 0) continue;

        if (sum_eT_all < mean - 3.5 * sigma || sum_eT_all > mean + 3.5 * sigma)
        {
            n_removed++;
            p_mbdQsum_eff->Fill(mbd_Qsum, 0);
            p_cent_eff->Fill(cent, 0);
            p_sumeT_eff->Fill(sum_eT_all, 0);
            continue;
        } 
        p_mbdQsum_eff->Fill(mbd_Qsum, 1);
        p_cent_eff->Fill(cent, 1);
        p_sumeT_eff->Fill(sum_eT_all, 1);

        // fill post-calibration histograms
        p_mbdQsum_sumeT_all_post_calib->Fill(mbd_Qsum, sum_eT_all);
        p_mbdQsum_cent_post_calib->Fill(mbd_Qsum, cent);
        p_cent_mbdQsum_post_calib->Fill(cent, mbd_Qsum);
        p_cent_sumeT_all_post_calib->Fill(cent, sum_eT_all);
        h_mbdQsum_sumeT_all_post_calib->Fill(mbd_Qsum, sum_eT_all);
        h_mbdQsum_cent_post_calib->Fill(mbd_Qsum, cent);
        h_cent_mbdQsum_post_calib->Fill(cent, mbd_Qsum);
        h_cent_sumeT_all_post_calib->Fill(cent, sum_eT_all);
    }

    std::cout << "Number of entries removed: " << n_removed << std::endl;
    std::cout << "Number of entries kept: " << nentries - n_removed << std::endl;
    std::cout << "Writing calibration data to calib.root" << std::endl;

    std::cout << "Writing cleaned data to cleaned_data.root" << std::endl;
    auto * fout = TFile::Open( outfile.c_str(), "RECREATE" );
    fout->cd();
    p_mbdQsum_sumeT_all->Write();
    p_mbdQsum_cent->Write();
    p_cent_mbdQsum->Write();
    p_cent_sumeT_all->Write();
    h_mbdQsum_sumeT_all->Write();
    h_mbdQsum_cent->Write();
    h_cent_mbdQsum->Write();
    h_cent_sumeT_all->Write();
    h_mbdQsum_sumeT_all_post_calib->Write();
    h_mbdQsum_cent_post_calib->Write();
    h_cent_mbdQsum_post_calib->Write();
    h_cent_sumeT_all_post_calib->Write();
    p_mbdQsum_sumeT_all_post_calib->Write();
    p_mbdQsum_cent_post_calib->Write();
    p_cent_mbdQsum_post_calib->Write();
    p_cent_sumeT_all_post_calib->Write();
    p_mbdQsum_eff->Write();
    p_cent_eff->Write();
    p_sumeT_eff->Write();
    fout->Close();
    std::cout << "Done!" << std::endl;

    return 0;
}

int clean_data( 
    const std::string & infile = "/sphenix/user/tmengel/PPG14/macros/jetUEmods/dr03/rootfiles/dr_04_grl_v3.root",
    const std::string & outfile = "run2auau_ana509_2024p022_v001_r03_jets.root",
    const std::string & calib = "run2auau_ana509_2024p022_v001_sumeT_calib.root"
)
{

    
    

    // load the calibration file
    auto * fin_calib = TFile::Open( calib.c_str(), "READ" );
    auto * p_mbdQsum_sumeT_all = (TProfile*) fin_calib->Get("p_mbdQsum_sumeT_all") ->Clone( "p_mbdQsum_sumeT_all" );
    p_mbdQsum_sumeT_all->SetDirectory(0);
    if ( !p_mbdQsum_sumeT_all )
    {
        std::cout << "Error: could not find p_mbdQsum_sumeT_all in calibration file" << std::endl;
        return 1;
    }
    fin_calib->Close();



    float zvrtx;
    int cent;
    float mbd_q_N, mbd_q_S, mbd_t_N, mbd_t_S;
    float sum_eT_cemc, sum_eT_hcalin, sum_eT_hcalout, sum_eT_all;
    std::vector<float> *jet_E = 0;
    std::vector<float> *jet_pT = 0;
    std::vector<float> *jet_eta = 0;
    std::vector<float> *jet_phi = 0;
    std::vector<float> *jet_unsub_pT = 0;
    std::vector<float> *jet_unsub_E = 0;
    auto * fin = TFile::Open( infile.c_str(), "READ" );
    auto * tin = (TTree*) fin->Get("T");
    tin->SetBranchAddress("zvrtx", &zvrtx);
    tin->SetBranchAddress("cent", &cent);
    tin->SetBranchAddress("mbd_q_N", &mbd_q_N);
    tin->SetBranchAddress("mbd_q_S", &mbd_q_S);
    tin->SetBranchAddress("mbd_t_N", &mbd_t_N);
    tin->SetBranchAddress("mbd_t_S", &mbd_t_S);
    tin->SetBranchAddress("sum_eT_cemc", &sum_eT_cemc);
    tin->SetBranchAddress("sum_eT_hcalin", &sum_eT_hcalin);
    tin->SetBranchAddress("sum_eT_hcalout", &sum_eT_hcalout);
    tin->SetBranchAddress("sum_eT_all", &sum_eT_all);
    tin->SetBranchAddress("jet_E", &jet_E);
    tin->SetBranchAddress("jet_pT", &jet_pT);
    tin->SetBranchAddress("jet_eta", &jet_eta);
    tin->SetBranchAddress("jet_phi", &jet_phi);
    tin->SetBranchAddress("jet_unsub_pT", &jet_unsub_pT);
    tin->SetBranchAddress("jet_unsub_E", &jet_unsub_E);

    int nentries = tin->GetEntries();
    std::cout << "Number of entries in input file: " << nentries << std::endl;

    TH2D * h_mbdQsum_sumeT_all = new TH2D("h_mbdQsum_sumeT_all", "h_mbdQsum_sumeT_all", 200, 0, 2000, 200, 0, 2000);

  
    auto * fout = TFile::Open( outfile.c_str(), "RECREATE" );
    auto * tout = new TTree( "ttree", "cleaned data" );
    // just make gl1 all 1s
    ULong64_t gl1_scaled = 0xFFFFFFFFFFFFFFFF;
    int minbias = 1;
    tout->Branch("gl1_scaled", &gl1_scaled);
    tout->Branch("minbias", &minbias);
    tout->Branch("centrality", &cent);
    tout->Branch("mbd_vertex_z", &zvrtx);
    float mbd_Qsum = 0;
    tout->Branch("mbd_Qsum", &mbd_Qsum);
    float mbd_time_zero = 0;
    tout->Branch("mbd_time_zero", &mbd_time_zero);
    tout->Branch("sum_eT_cemc", &sum_eT_cemc);
    tout->Branch("sum_eT_hcalin", &sum_eT_hcalin);
    tout->Branch("sum_eT_hcalout", &sum_eT_hcalout);
    tout->Branch("sum_eT_all", &sum_eT_all);
    std::vector<float>  jet_E_cleaned;
    std::vector<float>  jet_pT_cleaned;
    std::vector<float>  jet_eta_cleaned;
    std::vector<float>  jet_phi_cleaned;
    std::vector<float>  jet_unsub_pT_cleaned;
    std::vector<float>  jet_unsub_E_cleaned;
    tout->Branch("jet_e_3_sub", &jet_E_cleaned);
    tout->Branch("jet_pt_3_sub", &jet_pT_cleaned);
    tout->Branch("jet_eta_3_sub", &jet_eta_cleaned);
    tout->Branch("jet_phi_3_sub", &jet_phi_cleaned);
    tout->Branch("jet_pt_unsub_3_sub", &jet_unsub_pT_cleaned);
    tout->Branch("jet_e_unsub_3_sub", &jet_unsub_E_cleaned);

    for (int i = 0; i < nentries; i++)
    {
        tin->GetEntry(i);
        if ( cent <= 0 || cent > 100 ) continue;
        if ( i % (nentries / 10) == 0 ) std::cout << "Processing entry " << i << " / " << nentries << std::endl;

        if (fabs(zvrtx) > 60.0) continue;

        
       
        jet_E_cleaned.clear();
        jet_pT_cleaned.clear();
        jet_eta_cleaned.clear();
        jet_phi_cleaned.clear();
        jet_unsub_pT_cleaned.clear();
        jet_unsub_E_cleaned.clear();
        
        gl1_scaled = 0xFFFFFFFFFFFFFFFF;
        minbias = 1;
        mbd_time_zero = mbd_t_N + mbd_t_S;
        mbd_Qsum = mbd_q_N + mbd_q_S;

        // apply calibration to sum_eT_all
        double mean = p_mbdQsum_sumeT_all->GetBinContent(p_mbdQsum_sumeT_all->FindBin(mbd_Qsum));
        double sigma = p_mbdQsum_sumeT_all->GetBinError(p_mbdQsum_sumeT_all->FindBin(mbd_Qsum));
        if (sigma == 0) continue;
        if (sum_eT_all < mean - 3.5 * sigma || sum_eT_all > mean + 3.5 * sigma) continue;
        h_mbdQsum_sumeT_all->Fill(mbd_Qsum, sum_eT_all);

        std::vector<std::pair<int, float>> jet_pt_pairs{};
        for (size_t j = 0; j < jet_pT->size(); j++)
        {
            if (jet_pT->at(j) < MIN_PT) continue;
            if (jet_E->at(j) <= 0.0) continue;
            jet_pt_pairs.push_back(std::make_pair(j, jet_pT->at(j)));
        }
        //sort by pt
        std::sort(jet_pt_pairs.begin(), jet_pt_pairs.end(), [](const std::pair<int, float> &a, const std::pair<int, float> &b) {
            return a.second > b.second;
        });
        // now fill the cleaned vectors
        for (const auto & jet_pair : jet_pt_pairs)
        {
            int j = jet_pair.first;
            jet_E_cleaned.push_back(jet_E->at(j));
            jet_pT_cleaned.push_back(jet_pT->at(j));
            jet_eta_cleaned.push_back(jet_eta->at(j));
            jet_phi_cleaned.push_back(jet_phi->at(j));
            jet_unsub_pT_cleaned.push_back(jet_unsub_pT->at(j));
            jet_unsub_E_cleaned.push_back(jet_unsub_E->at(j));
        }

        tout->Fill();
    }
    std::cout << "Writing cleaned data to " << outfile << std::endl;
    fout->cd();
    tout->Write();
    h_mbdQsum_sumeT_all->Write();
    p_mbdQsum_sumeT_all->Write();
    fout->Close();
    fin->Close();
    return 0;
}

int preprocess()
{
    std::string calibfile = "run2auau_ana509_2024p022_v001_sumeT_calib.root";
    bool calib_exists = gSystem->AccessPathName(calibfile.c_str());
    bool overwrite_calib = false;
    if (!calib_exists || overwrite_calib ) make_calib();

    bool clean_sim_data = false;
    if ( clean_sim_data ) clean_all_sim();

    bool clean_real_data = true;
    if ( clean_real_data ) clean_data( 
        "/sphenix/user/tmengel/PPG14/macros/jetUEmods/dr03/rootfiles/dr_04_grl_v3.root",
        "run2auau_ana509_2024p022_v001_r03_jets.root",
        calibfile
    );

    return 0;
}

