#include "AnaTree.h"

#include <fun4all/PHTFileServer.h>

#include <fun4all/Fun4AllReturnCodes.h>

#include <ffaobjects/EventHeaderv1.h>

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>
#include <phool/phool.h>

#include <globalvertex/GlobalVertex.h>
#include <globalvertex/GlobalVertexMapv1.h>

#include <centrality/CentralityInfov2.h>

#include <eventplaneinfo/Eventplaneinfo.h>
#include <eventplaneinfo/EventplaneinfoMap.h>

#include <calobase/RawTowerGeom.h>
#include <calobase/RawTowerGeomContainer.h>
#include <calobase/TowerInfo.h>
#include <calobase/TowerInfoContainer.h>

#include <ffarawobjects/Gl1Packetv2.h>

#include <jetbase/Jetv2.h>
#include <jetbase/JetContainerv1.h>

#include <jetbackground/TowerBackgroundv1.h>

#include <TTree.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <utility>

AnaTree::AnaTree( const std::string & outputfile )
  : SubsysReco("AnaTree")
  , m_output_filename( outputfile )
{}

int AnaTree::Init( PHCompositeNode * /*topNode*/ )
{
  
    if ( Verbosity () > 0 ) 
    {
        std::cout << "AnaTree::Init - opening file " << m_output_filename << std::endl;
    }

    // init tree
    PHTFileServer::get().open( m_output_filename, "RECREATE" );
    m_tree = new TTree( "T", "T" );
    m_tree -> Branch( "event_id", &m_event_id, "event_id/I" );
    if ( !m_gl1_node.empty() ) 
    {
        m_tree -> Branch( "s_triggervec", &s_triggervec, "s_triggervec/l" );
        m_tree -> Branch( "l_triggervec", &l_triggervec, "l_triggervec/l" );
    }
    if ( !m_zvrtx_node.empty() )
    {
        m_tree -> Branch( "zvrtx", &m_zvtx, "zvrtx/F" );
    }
    if ( !m_cent_node.empty() )
    {
        m_tree -> Branch( "cent", &m_cent, "cent/I" );
    }
    if ( !m_eventhead_node.empty() ) 
    {
        m_tree -> Branch( "b", &m_b, "b/F" );
        m_tree -> Branch( "ep_angle", &m_ep_angle, "ep_angle/F" );
        m_tree -> Branch( "ecc", &m_ecc, "ecc/F" );
        m_tree -> Branch( "psi", m_psi_arr, "psi[6]/F" );
        m_tree -> Branch( "ncoll", &m_ncoll, "ncoll/F" );
        m_tree -> Branch( "npart", &m_npart, "npart/F" );
        m_tree -> Branch( "runnumber", &m_runnumber, "runnumber/I" );
        m_tree -> Branch( "evtsequence", &m_evtsequence, "evtsequence/I" );
    }
    if ( m_save_full_calo )
    {
        m_tree -> Branch( "tower_E", m_tower_E, Form("tower_E[3][%d][%d]/F", k_ieta, k_iphi) );
    }
    if ( m_save_sumeT )
    {
        m_tree -> Branch( "sumeT", m_sumeT, "sumeT[3]/F" );
    }
    if ( m_save_sub1_sumeT )
    {
        m_tree -> Branch( "sumeT_sub1", m_sumeT_sub1, "sumeT_sub1[3]/F" );
    }
    if ( m_save_org_sumeT )
    {
        m_tree -> Branch( "sumeT_org", m_sumeT_org, "sumeT_org[3]/F" );
    }
    if ( m_do_towerbkgd ) 
    {
        m_tree -> Branch( "sub2_v2", &m_sub2_v2, "sub2_v2/F" );
        m_tree -> Branch( "sub2_flowfaliure", &m_sub2_flowfaliure, "sub2_flowfaliure/I" );
        m_tree -> Branch( "sub2_psi2", &m_sub2_psi2, "sub2_psi2/F" );
        m_tree -> Branch( "sub2_towerbkgd_ue", m_sub2_towerbkgd_ue, Form("sub2_towerbkgd_ue[3][%d]/F", k_ieta) );
    }
    if ( !m_sub1jet_node.empty() ) 
    {
        m_tree  -> Branch( "jet_E", &m_sub1_jet_E );
        m_tree  -> Branch( "jet_phi", &m_sub1_jet_phi );
        m_tree  -> Branch( "jet_eta", &m_sub1_jet_eta );
        m_tree  -> Branch( "jet_pT", &m_sub1_jet_pT );
        m_tree  -> Branch( "jet_unsub_pT", &m_sub1_jet_unsub_pT );
        m_tree  -> Branch( "jet_unsub_E", &m_sub1_jet_unsub_E );
    }
    if ( !m_truthjet_node.empty() ) 
    {
        m_tree  -> Branch("truth_jet_E", &m_truth_jet_E );
        m_tree  -> Branch("truth_jet_phi", &m_truth_jet_phi );
        m_tree  -> Branch("truth_jet_eta", &m_truth_jet_eta );
        m_tree  -> Branch("truth_jet_pT", &m_truth_jet_pT );
    }

    if ( Verbosity () > 0 )
    {
        std::cout << "AnaTree::Init - done" << std::endl;
    }

    return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTree::InitRun ( PHCompositeNode *topNode )
{
    if ( Verbosity () > 0 ) 
    {
        std::cout << "AnaTree::InitRun - initializing run with file " << m_output_filename << std::endl;
    }
    // get calo r 
    memset(m_calo_r, 0, sizeof(m_calo_r));
    const std::vector<std::pair< std::string, RawTowerDefs::CalorimeterId >> calo_info = {
        {"TOWERGEOM_CEMC", RawTowerDefs::CalorimeterId::CEMC}, 
        {"TOWERGEOM_HCALIN", RawTowerDefs::CalorimeterId::HCALIN}, 
        {"TOWERGEOM_HCALOUT", RawTowerDefs::CalorimeterId::HCALOUT} 
    };
    for ( const auto & [geo_node, calo_id] : calo_info )
    {
        auto * geom = LoadTowerGeomContainer( topNode, geo_node );
        const auto dummykey = RawTowerDefs::encode_towerid( calo_id, 0, 0 );
        auto * dummy_towergeom = geom->get_tower_geometry( dummykey );
        if ( ! dummy_towergeom ) 
        {
            std::cout << PHWHERE << " Failed to get tower geometry for " << geo_node << " with calo id " << static_cast<int>(calo_id) << std::endl;
            return Fun4AllReturnCodes::ABORTRUN;
        }
        m_calo_r[calo_id] = dummy_towergeom->get_center_radius();
        if ( Verbosity() > 0 ) 
        {
            std::cout << "AnaTree::Init - Calo " << geo_node << " with calo id " << static_cast<int>(calo_id) << " has radius " << m_calo_r[calo_id] << std::endl;
        }
    }

    m_event_id = -1;

    return Fun4AllReturnCodes::EVENT_OK;
}

int AnaTree::process_event( PHCompositeNode *topNode )
{
    m_event_id++; 

    if( Verbosity() > 1 ) 
    {
        std::cout << PHWHERE << " Processing event " << m_event_id << std::endl;
    }

    if ( Verbosity() > 10 )
    {
        std::cout << "Printing hcalout wave form and towerinfo for event " << m_event_id << std::endl;
        auto towerinfos_wf = LoadTowerInfoContainer( topNode, "WAVEFORM_HCALOUT" );
        auto towerinfos_calib = LoadTowerInfoContainer( topNode, "TOWERINFO_CALIB_HCALOUT" );
        auto towerinfos_uncalib = LoadTowerInfoContainer( topNode, "TOWERS_HCALOUT" );
        std::cout << "ieta, iphi, waveform->get_energy(), calib energy, uncalib energy" << std::endl;
        for ( unsigned int ich = 0; ich < towerinfos_wf->size(); ich++ ) 
        {
            auto tower_wf = towerinfos_wf->get_tower_at_channel(ich);
            auto tower_calib = towerinfos_calib->get_tower_at_channel(ich);
            auto tower_uncalib = towerinfos_uncalib->get_tower_at_channel(ich);
            if ( !tower_wf || !tower_calib || !tower_uncalib )
            {
                std::cout << PHWHERE << " Failed to get tower info for channel " << ich << std::endl;
                continue;
            }
            unsigned int key = towerinfos_wf->encode_key(ich);
            int ieta = towerinfos_wf->getTowerEtaBin(key);
            int iphi = towerinfos_wf->getTowerPhiBin(key);
            std::cout << ieta << ", " << iphi << ", " << tower_wf->get_energy() << ", " << tower_calib->get_energy() << ", " << tower_uncalib->get_energy() << std::endl;
        }
        return Fun4AllReturnCodes::EVENT_OK;
    }

    if ( !m_gl1_node.empty() )
    {  
        // get GL1
        s_triggervec = 0;
        l_triggervec = 0;
        auto * gl1 = findNode::getClass< Gl1Packetv2 >( topNode, m_gl1_node );
        if( !gl1 ) 
        {
            std::cout << PHWHERE << " No GL1 packet found! Abort." << std::endl;
            return Fun4AllReturnCodes::ABORTRUN;
        }
        s_triggervec = gl1 -> getScaledVector();
        l_triggervec = gl1 -> getLiveVector();
        if ( Verbosity() > 1 ) 
        {
            std::cout << PHWHERE << " - s_triggervec = " << std::hex << s_triggervec << ", l_triggervec = " << l_triggervec << std::dec << std::endl;
        }
    }

    if ( !m_cent_node.empty() ) 
    { 
        // get centrality
        m_cent = -1;
        auto * cent_node = findNode::getClass< CentralityInfo >( topNode, m_cent_node );
        if ( !cent_node ) 
        {
            std::cout << PHWHERE << m_cent_node << " node missing, Abort!." << std::endl;
            return Fun4AllReturnCodes::ABORTRUN;
        }
        m_cent = static_cast<int>( cent_node -> get_centrality_bin(CentralityInfo::PROP::mbd_NS) );
        if ( Verbosity() > 1 ) 
        {
            std::cout << PHWHERE << "- Centrality = " << m_cent << std::endl;
        }
    }

    if ( !m_zvrtx_node.empty() ) 
    { 
        // get zvtx

        m_zvtx = -999;
        GlobalVertex * vtx { nullptr };
        auto * vertexmap = findNode::getClass<GlobalVertexMap>( topNode, m_zvrtx_node );
        if ( !vertexmap  ) 
        {
            std::cout << PHWHERE << "" << m_zvrtx_node << " node missing, skipping event." << std::endl;
            return Fun4AllReturnCodes::ABORTRUN;
        }
        if ( vertexmap->empty() ) 
        {
            std::cout << PHWHERE << "" << m_zvrtx_node << " node has empty vertex map, skipping event." << std::endl;
            return Fun4AllReturnCodes::ABORTEVENT;
        }

        auto vertices = vertexmap -> get_gvtxs_with_type( { GlobalVertex::MBD } );
        if( !vertices.empty() )
        {
            vtx = vertices.at(0);
        }
        else 
        {
            vtx = vertexmap->begin()->second;
        }
        
        if ( vtx )
        {
            m_zvtx = vtx->get_z();
        }
        
        if ( std::isnan(m_zvtx) || std::abs(m_zvtx) > 1e3 )
        {
            static bool z_warning_once = true;
            if ( z_warning_once )
            {
                z_warning_once = false;
                std::cout << PHWHERE << " vertex z is " << m_zvtx << ", skipping event (further warnings will be suppressed)." << std::endl;
            }
            
            return Fun4AllReturnCodes::ABORTEVENT;
            
        }
 
        if ( Verbosity() > 1 ) 
        {
            std::cout << PHWHERE << " - zvtx = " << m_zvtx << std::endl;
        }

    }

    if ( !m_eventhead_node.empty() ) 
    { 
        // get event header info
        m_b = -999;
        m_ep_angle = -999;
        m_ecc = -999;
        // memset(m_psi_arr, -999, sizeof(m_psi_arr));
        std::fill(
            std::begin(m_psi_arr),
            std::end(m_psi_arr),
            -999
        );
        m_runnumber = -1;
        m_evtsequence = -1;

        auto * eventhead = findNode::getClass<EventHeader>( topNode, m_eventhead_node );
        if ( !eventhead ) 
        {
            std::cout << PHWHERE << " Input node " << m_eventhead_node << " Node missing, doing nothing." << std::endl;
            return Fun4AllReturnCodes::ABORTRUN;
        }
        m_b = eventhead->get_ImpactParameter();
        m_ep_angle = eventhead->get_EventPlaneAngle();
        m_ecc = eventhead->get_eccentricity();
        
        for ( int i = 0; i < 6; ++i )
        {
            m_psi_arr[i] = eventhead->get_FlowPsiN(i+1);
        }
        m_ncoll = eventhead->get_ncoll();
        m_npart = eventhead->get_npart();
        m_runnumber = eventhead->get_RunNumber();
        m_evtsequence = eventhead->get_EvtSequence();
  
        if ( Verbosity() > 1 ) 
        {
            std::cout << PHWHERE << " - b = " << m_b << ", ep_angle = " << m_ep_angle << ", ecc = " << m_ecc << ", psi2 = " << m_psi_arr[1] << ", ncoll = " << m_ncoll << ", npart = " << m_npart << std::endl;
        }

    }

    if ( !m_sub1jet_node.empty() ) 
    { 
        
        m_sub1_jet_E.clear();
        m_sub1_jet_phi.clear();
        m_sub1_jet_eta.clear();
        m_sub1_jet_pT.clear();
        m_sub1_jet_unsub_pT.clear();
        m_sub1_jet_unsub_E.clear();

        memset(m_sub2_towerbkgd_ue, 0, sizeof(m_sub2_towerbkgd_ue));
        m_sub2_v2 = 0.0;
        m_sub2_flowfaliure = 0;
        m_sub2_psi2 = 0.0;

        // get sub1 jet info
        auto * jets = findNode::getClass<JetContainer>( topNode, m_sub1jet_node );
        if ( !jets )
        {
            std::cout << PHWHERE << " Input node " << m_sub1jet_node << " Node missing, doing nothing." << std::endl;
            return Fun4AllReturnCodes::ABORTRUN; 
        }
        
        auto * tower_background_sub2 = findNode::getClass<TowerBackgroundv1>(topNode, "TowerInfoBackground_Sub2");
        if ( !tower_background_sub2 )
        {
            std::cout << PHWHERE << " TowerBackgroundv1 node is missing, skipping." << std::endl;
            return Fun4AllReturnCodes::ABORTRUN; // fatal error
        }
        else 
        {
            // read info
            for ( size_t ilay = 0 ; ilay < 3; ++ilay ) 
            {
            auto this_ue_sub2 = tower_background_sub2->get_UE(ilay);
            for ( size_t ieta = 0; ieta < k_ieta; ++ieta )
            {
                m_sub2_towerbkgd_ue[ilay][ieta] = this_ue_sub2[ieta];
            }
            }
            m_sub2_v2 = tower_background_sub2->get_v2();
            m_sub2_flowfaliure = tower_background_sub2->get_flow_failure_flag();
            m_sub2_psi2 = tower_background_sub2->get_Psi2();
        }

        for ( const auto & jet : * jets )
        {
            
            float unsub_pz = 0;
            float unsub_px = 0; 
            float unsub_py = 0;
            float unsub_E  = 0;
            for ( const auto & comp : jet -> get_comp_vec() )
            {
                double tower_r = 0.0;
                int layer_idx = -1;
                m_caloid = RawTowerDefs::CalorimeterId::NONE;
                m_towerinfos = nullptr;
                m_towergeom = nullptr;
                if( comp.first == Jet::SRC::HCALIN_TOWERINFO_SUB1 )
                {
                    m_towerinfos = LoadTowerInfoContainer( topNode, "TOWERINFO_CALIB_HCALIN_SUB1" );
                    m_towergeom = LoadTowerGeomContainer( topNode, "TOWERGEOM_HCALIN" );
                    tower_r = m_calo_r[RawTowerDefs::CalorimeterId::HCALIN-1];
                    m_caloid = RawTowerDefs::CalorimeterId::HCALIN;
                    layer_idx = 1;
                }
                else if ( comp.first == Jet::SRC::HCALOUT_TOWERINFO_SUB1 )
                {
                    m_towerinfos = LoadTowerInfoContainer( topNode, "TOWERINFO_CALIB_HCALOUT_SUB1" );
                    m_towergeom = LoadTowerGeomContainer( topNode, "TOWERGEOM_HCALOUT" );
                    tower_r = m_calo_r[RawTowerDefs::CalorimeterId::HCALOUT-1];
                    m_caloid = RawTowerDefs::CalorimeterId::HCALOUT;
                    layer_idx = 2;
                }
                else if ( comp.first == Jet::SRC::CEMC_TOWERINFO_SUB1 )
                {
                    m_towerinfos = LoadTowerInfoContainer( topNode, "TOWERINFO_CALIB_CEMC_RETOWER_SUB1" );
                    m_towergeom = LoadTowerGeomContainer( topNode, "TOWERGEOM_HCALIN" );
                    tower_r = m_calo_r[RawTowerDefs::CalorimeterId::CEMC-1];
                    layer_idx = 0;
                    m_caloid = RawTowerDefs::CalorimeterId::HCALIN; // use hcalin geometry for cemc towers since we just want eta/phi and the r is only used for calculating unsub pT which will be corrected by UE subtraction
                }
                else 
                {
                    if ( Verbosity() > 3 ) 
                    {
                        std::cout << PHWHERE << " Warning: jet constituent with unknown source " << comp.first << ", skipping." << std::endl;
                    }
                    continue;
                }
                
                auto * tower = m_towerinfos->get_tower_at_channel( comp.second );
                if ( !tower || !tower->get_isGood() ) 
                {
                    if ( Verbosity() > 3 ) 
                    {
                        std::cout << PHWHERE << " Warning: constituent tower with caloid " << comp.first << " and channel " << comp.second << " not found in towerinfo container, skipping." << std::endl;
                    }
                    continue;
                }

                const auto tkey = m_towerinfos->encode_key( comp.second );
                auto ieta = m_towerinfos->getTowerEtaBin(tkey);
                auto iphi = m_towerinfos->getTowerPhiBin(tkey);
                const auto key = RawTowerDefs::encode_towerid( m_caloid, ieta, iphi );
                
                auto * geom = m_towergeom->get_tower_geometry( key );
                if ( !geom )
                {
                    if ( Verbosity() > 3 ) 
                    {
                        std::cout << PHWHERE << " Warning: geometry for tower with caloid " << comp.first << " and channel " << comp.second << " not found, skipping." << std::endl;
                    }
                    continue;
                }

                double tower_z0   = sinh( geom -> get_eta() ) * tower_r;
                double comp_z     = tower_z0 - m_zvtx;
                double comp_eta   =  asinh( comp_z / tower_r );
                double comp_phi   = geom -> get_phi();
                double comp_E     = tower->get_energy();
                double ue         = m_sub2_towerbkgd_ue[layer_idx][ieta];
                double un_E       = comp_E + ue;

                float un_pT = un_E / cosh( comp_eta );
                float un_px = un_pT * cos( comp_phi );
                float un_py = un_pT * sin( comp_phi );
                float un_pz = un_pT * sinh( comp_eta );
                unsub_px += un_px;
                unsub_py += un_py;
                unsub_pz += un_pz;
                unsub_E  += un_E;

            } // end loop over constituents
            
            auto * unsub_jet = new Jetv2();
            unsub_jet->set_px(unsub_px);
            unsub_jet->set_py(unsub_py);
            unsub_jet->set_pz(unsub_pz);
            unsub_jet->set_e(unsub_E);
            
            m_sub1_jet_E.push_back(jet->get_e());
            m_sub1_jet_eta.push_back(jet->get_eta());
            m_sub1_jet_phi.push_back(jet->get_phi());
            m_sub1_jet_pT.push_back(jet->get_pt());
            m_sub1_jet_unsub_pT.push_back(unsub_jet->get_pt());
            m_sub1_jet_unsub_E.push_back(unsub_jet->get_e());

        } // end loop over jets
    } 
    else if ( m_do_towerbkgd && m_sub1jet_node.empty() )
    {
        // if not saving sub1 jets, then we wont have the tower bkgd info in the jet constituents, but we can still save the tower bkgd info for later use
        memset(m_sub2_towerbkgd_ue, 0, sizeof(m_sub2_towerbkgd_ue));
        m_sub2_v2 = 0.0;
        m_sub2_flowfaliure = 0;
        m_sub2_psi2 = 0.0;

        auto * tower_background_sub2 = findNode::getClass<TowerBackgroundv1>(topNode, "TowerInfoBackground_Sub2");
        if ( !tower_background_sub2 )
        {
            std::cout << PHWHERE << " TowerBackgroundv1 node is missing, skipping." << std::endl;
            return Fun4AllReturnCodes::ABORTRUN; // fatal error
        }
        else 
        {
            // read info
            for ( size_t ilay = 0 ; ilay < 3; ++ilay ) 
            {
            auto this_ue_sub2 = tower_background_sub2->get_UE(ilay);
            for ( size_t ieta = 0; ieta < k_ieta; ++ieta )
            {
                m_sub2_towerbkgd_ue[ilay][ieta] = this_ue_sub2[ieta];
            }
            }
            m_sub2_v2 = tower_background_sub2->get_v2();
            m_sub2_flowfaliure = tower_background_sub2->get_flow_failure_flag();
            m_sub2_psi2 = tower_background_sub2->get_Psi2();
        }

    }

    if ( !m_truthjet_node.empty() ) 
    { 
        // get truth jet info
        m_truth_jet_E.clear();
        m_truth_jet_phi.clear();
        m_truth_jet_eta.clear();
        m_truth_jet_pT.clear();
        auto * truth_jets = findNode::getClass<JetContainer>( topNode, m_truthjet_node );
        if ( !truth_jets )        
        {
            std::cout << PHWHERE << " Input node " << m_truthjet_node << " Node missing, doing nothing." << std::endl;
            return Fun4AllReturnCodes::ABORTRUN; 
        }
        for ( const auto & jet : * truth_jets )
        {
            m_truth_jet_E.push_back(jet->get_e());
            m_truth_jet_eta.push_back(jet->get_eta());
            m_truth_jet_phi.push_back(jet->get_phi());
            m_truth_jet_pT.push_back(jet->get_pt());
        } // end loop over truth jets
    }

    if ( m_save_full_calo )
    {
        memset(m_tower_E, 0, sizeof(m_tower_E));
        for ( size_t ilay = 0; ilay < 3; ++ilay )   
        {
            m_towerinfos = LoadTowerInfoContainer( topNode, m_fullcalo_nodes[ilay] );
            for ( unsigned int ich = 0; ich < m_towerinfos->size(); ich++ ) 
            {
                auto tower = m_towerinfos->get_tower_at_channel(ich);
                if ( !tower || ! tower->get_isGood() || std::isnan(tower->get_energy() ) )
                {
                    continue; // skip bad towers
                }
                unsigned int key = m_towerinfos -> encode_key(ich);
                int ieta = m_towerinfos -> getTowerEtaBin(key);
                int iphi = m_towerinfos -> getTowerPhiBin(key);
                m_tower_E[ilay][ieta][iphi] = tower->get_energy();
            }
        }
    }
    
    if ( m_save_sumeT )
    {
        memset(m_sumeT, 0, sizeof(m_sumeT));
        auto sumet_vec = SumCaloE( topNode, m_sumeT_nodes );
        for ( size_t ilay = 0; ilay < 3; ++ilay )
        {
            m_sumeT[ilay] = sumet_vec[ilay];
        }
    }
    if ( m_save_sub1_sumeT )
    {
        memset(m_sumeT_sub1, 0, sizeof(m_sumeT_sub1));
        auto sumet_vec = SumCaloE( topNode, m_sub1_sumeT_nodes );
        for ( size_t ilay = 0; ilay < 3; ++ilay )
        {
            m_sumeT_sub1[ilay] = sumet_vec[ilay];
        }
    }
    if ( m_save_org_sumeT )
    {
        memset(m_sumeT_org, 0, sizeof(m_sumeT_org));
        
        auto sumet_vec = SumCaloE( topNode, m_org_sumeT_nodes );
        for ( size_t ilay = 0; ilay < 3; ++ilay )
        {
            m_sumeT_org[ilay] = sumet_vec[ilay];
        }
    }

    // fill tree
    m_tree->Fill();
    
    return Fun4AllReturnCodes::EVENT_OK;

}

int AnaTree::End( PHCompositeNode * /*topNode*/ )
{
  
  if( Verbosity() > 0 ) 
  {
    std::cout << "AnaTree::EndRun - End run " << std::endl;
    std::cout << "AnaTree::EndRun - Writing to " << m_output_filename << std::endl;
  }

  PHTFileServer::get().cd(m_output_filename); 

  m_tree->Write();
  

  if ( Verbosity() > 0 ) 
  {
    std::cout << "AnaTree::EndRun - Writing run tree" << std::endl;
  }
  PHTFileServer::get().close();
 
  if ( Verbosity () > 0 ) 
  {
    std::cout << "AnaTree::EndRun - done" << std::endl;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

TowerInfoContainer * AnaTree::LoadTowerInfoContainer(PHCompositeNode *topNode, const std::string &tower_node_name)
{
  auto * tic = findNode::getClass<TowerInfoContainer>(topNode, tower_node_name.c_str());
  if (!tic)
  {
    std::cout << "AnaTree::LoadTowerInfoContainer - cannot find " << tower_node_name << ", exiting" << std::endl;
    exit(1);
  }
  return tic;
}

RawTowerGeomContainer * AnaTree::LoadTowerGeomContainer(PHCompositeNode *topNode, const std::string &tower_geom_node_name)
{
  auto * tg = findNode::getClass<RawTowerGeomContainer>(topNode, tower_geom_node_name.c_str());
  if (!tg)
  {
    std::cout << "AnaTree::LoadTowerGeomContainer - cannot find " << tower_geom_node_name << ", exiting" << std::endl;
    exit(1);
  }
  return tg;
}

std::vector<float> AnaTree::SumCaloE( PHCompositeNode *topNode,  const std::vector< std::string > &towerinfo_nodes )
{
    std::vector<float> sumeT(3, 0.0);

   
    for ( size_t ilay = 0; ilay < 3; ++ilay )   
    {
        double tower_r =0.0;
        m_caloid = RawTowerDefs::CalorimeterId::NONE;
        m_towerinfos = LoadTowerInfoContainer( topNode, towerinfo_nodes[ilay] );
        m_towergeom = nullptr;
        if ( ilay == 0 ) 
        {
            m_caloid = RawTowerDefs::CalorimeterId::HCALIN;
            tower_r = m_calo_r[RawTowerDefs::CalorimeterId::CEMC-1];
            m_towergeom = LoadTowerGeomContainer( topNode, "TOWERGEOM_HCALIN" );
        }
        else if ( ilay == 1 ) 
        {
            m_caloid = RawTowerDefs::CalorimeterId::HCALIN;
            tower_r = m_calo_r[RawTowerDefs::CalorimeterId::HCALIN-1];
            m_towergeom = LoadTowerGeomContainer( topNode, "TOWERGEOM_HCALIN" );
        }
        else if ( ilay == 2 ) 
        {
            m_caloid = RawTowerDefs::CalorimeterId::HCALOUT;
            tower_r = m_calo_r[RawTowerDefs::CalorimeterId::HCALOUT-1];
            m_towergeom = LoadTowerGeomContainer( topNode, "TOWERGEOM_HCALOUT" );
        }

        for ( unsigned int ich = 0; ich < m_towerinfos->size(); ich++ ) 
        {
            auto tower = m_towerinfos->get_tower_at_channel(ich);
            if ( !tower || ! tower->get_isGood() || std::isnan(tower->get_energy() ) )
            {
                continue; // skip bad towers
            }

            unsigned int key = m_towerinfos -> encode_key(ich);
            int ieta = m_towerinfos -> getTowerEtaBin(key);
            int iphi = m_towerinfos -> getTowerPhiBin(key);
            const RawTowerDefs::keytype geokey = RawTowerDefs::encode_towerid(m_caloid, ieta, iphi);

            auto tower_geom = m_towergeom -> get_tower_geometry( geokey );
            if ( !tower_geom )
            {
                std::cout << PHWHERE << " Warning: cannot find geometry for tower with calo id " << static_cast<int>(m_caloid) << " and channel " << ich << ", skipping." << std::endl;
                exit(1);
                continue;
            }
            
            double tower_z0   = sinh( tower_geom -> get_eta() ) * tower_r;
            double tower_z    = tower_z0 - m_zvtx;
            double tower_eta  =  asinh( tower_z / tower_r );
            sumeT[ilay] += tower->get_energy() / cosh( tower_eta );
        }
    }

    return sumeT;
}







   