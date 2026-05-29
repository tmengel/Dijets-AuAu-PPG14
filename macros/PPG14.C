#ifndef __PPG14_C__
#define __PPG14_C__

#include <GlobalVariables.C>

#include <G4_ActsGeom.C>
#include <G4_Centrality.C>
#include <G4_Global.C>
#include <G4_Magnet.C>

#include <Trkr_Reco.C>
#include <Trkr_RecoInit.C>
#include <Trkr_TpcReadoutInit.C>

#include <HIJetReco.C>

#include <Calo_Calib.C>

#include <ffamodules/CDBInterface.h>

#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>

#include <mbd/MbdReco.h>

#include <epd/EpdReco.h>

#include <zdcinfo/ZdcReco.h>

#include <globalvertex/GlobalVertexReco.h>

#include <centrality/CentralityReco.h>

#include <calotrigger/MinimumBiasClassifier.h>

#include <dijetana/AnaConf.h>

R__LOAD_LIBRARY( libfun4all.so )
R__LOAD_LIBRARY( libffamodules.so )

R__LOAD_LIBRARY( libcalotrigger.so )
R__LOAD_LIBRARY( libcentrality.so )
R__LOAD_LIBRARY( libmbd.so )
R__LOAD_LIBRARY( libepd.so )
R__LOAD_LIBRARY( libzdcinfo.so )

R__LOAD_LIBRARY( libjetbase.so )
R__LOAD_LIBRARY( libjetbackground.so )

R__LOAD_LIBRARY( libglobalvertex.so )

R__LOAD_LIBRARY( libdijetana.so )


namespace ANA_SETTINGS
{

    int run_number      = 31;
    int num_events      = -1;
    int first_segment   = 0;
    int num_segments    = 1;
    int jet_samp        = 0;

    int ANA_VERBOSITY   = 1;

    bool IS_SIM         = false;
    bool IS_DATA        = false;

    bool CALIBRATE_CALO = true; // always true

    bool RESCALE_CALOS  = false;
    float scale_factor  = 1.0f;

    bool SAVE_FULL_CALO = false;
    
    bool DO_JETS        = true; // default to true
    float jet_R         = 0.3f;

    bool EVENT_SELECT   = true; // default to true


    const std::string default_cdbtag_sim = "MDC2";
    const std::string default_cdbtag_data = "ProdA_2024";
    const std::string default_prodtag_sim = "sHijing_0_20fm";
    const std::string default_prodtag_data = "run2auau_ana509_2024p022_v001";
    
    std::string cdbtag {""};
    std::string prodtag {""};
    
    std::vector<std::string> in_dsts {};    

    AnaConf * conf { nullptr };

    // helper function to create tower jet input with correct settings
    TowerJetInput* GetTowerInput( Jet::SRC src )
    {   
        auto * input = new TowerJetInput(src, HIJETS::tower_prefix);
        if ( HIJETS::do_vertex_type )
        {
            input -> set_GlobalVertexType(HIJETS::vertex_type);
        }
        return input;
    };

} // namespace ANA_SETTINGS

 
void Init_Ana_Settings( const std::string & conf_file )
{
    if ( ANA_SETTINGS::ANA_VERBOSITY > 0 )
    {
        std::cout << "Initializing analysis settings from config file: " << conf_file << std::endl;
    }

    ANA_SETTINGS::conf = new AnaConf( );
    if ( !ANA_SETTINGS::conf -> Load( conf_file ) ) 
    { 
        std::cerr << "Failed to load config file: " << conf_file << std::endl; 
        exit(-1); 
    }
    

    ANA_SETTINGS::num_events              = ANA_SETTINGS::conf -> GetInt( "num_events",     ANA_SETTINGS::num_events  );
    ANA_SETTINGS::run_number              = ANA_SETTINGS::conf -> GetInt( "run_number",     ANA_SETTINGS::run_number );
    ANA_SETTINGS::jet_samp                = ANA_SETTINGS::conf -> GetInt( "jet_flag",       ANA_SETTINGS::jet_samp );
    ANA_SETTINGS::first_segment           = ANA_SETTINGS::conf -> GetInt( "first_segment",  ANA_SETTINGS::first_segment );
    ANA_SETTINGS::num_segments            = ANA_SETTINGS::conf -> GetInt( "num_segments",   ANA_SETTINGS::num_segments );

    ANA_SETTINGS::in_dsts                 = ANA_SETTINGS::conf -> string_vectors[ "dsts" ];

    ANA_SETTINGS::IS_SIM                  = ( ANA_SETTINGS::run_number < 1000 );
    ANA_SETTINGS::IS_DATA                 = !ANA_SETTINGS::IS_SIM;
    ANA_SETTINGS::cdbtag                  = ANA_SETTINGS::conf -> GetString( "cdbtag",  ANA_SETTINGS::cdbtag );
    ANA_SETTINGS::prodtag                 = ANA_SETTINGS::conf -> GetString( "prodtag", ANA_SETTINGS::prodtag );
    if ( ANA_SETTINGS::cdbtag.empty() ) 
    {
        ANA_SETTINGS::cdbtag = ( ANA_SETTINGS::IS_SIM ? ANA_SETTINGS::default_cdbtag_sim : ANA_SETTINGS::default_cdbtag_data );
    }
    if ( ANA_SETTINGS::prodtag.empty() )
    {
        ANA_SETTINGS::prodtag = ( ANA_SETTINGS::IS_SIM ? ANA_SETTINGS::default_prodtag_sim : ANA_SETTINGS::default_prodtag_data );
        if ( ANA_SETTINGS::IS_SIM && ANA_SETTINGS::jet_samp > 0 )
        {
            ANA_SETTINGS::prodtag = Form( "pythia8_Jet%d_%s",  ANA_SETTINGS::jet_samp, ANA_SETTINGS::default_prodtag_sim.c_str() );
        }
    }

    ANA_SETTINGS::RESCALE_CALOS           = ( ANA_SETTINGS::conf -> GetInt( "rescale_calo_flag", ANA_SETTINGS::RESCALE_CALOS) > 0 );
    ANA_SETTINGS::SAVE_FULL_CALO          = ( ANA_SETTINGS::conf -> GetInt( "save_full_calo", ANA_SETTINGS::SAVE_FULL_CALO ) > 0 );
    ANA_SETTINGS::scale_factor            = ANA_SETTINGS::conf -> GetFloat( "upscale_calo_factor", ANA_SETTINGS::scale_factor );
    ANA_SETTINGS::DO_JETS                 = ( ANA_SETTINGS::conf -> GetInt( "do_jets", ANA_SETTINGS::DO_JETS ) > 0 );
    ANA_SETTINGS::jet_R                   = ANA_SETTINGS::conf -> GetFloat( "jet_R", ANA_SETTINGS::jet_R );

    ANA_SETTINGS::EVENT_SELECT           = ( ANA_SETTINGS::conf -> GetInt( "event_select", ANA_SETTINGS::EVENT_SELECT ) > 0 );

    if ( ANA_SETTINGS::ANA_VERBOSITY > 0 )
    {
        std::cout << "Analysis settings initialized:" << std::endl;
    
        std::cout << "\trun_number = " << ANA_SETTINGS::run_number << std::endl;
        std::cout << "\tnum_events = " << ANA_SETTINGS::num_events << std::endl;
        std::cout << "\tjet_samp = " << ANA_SETTINGS::jet_samp << std::endl;

        // input settings
    
        std::cout << "\tANA_SETTINGS::IS_SIM = " << ( ANA_SETTINGS::IS_SIM ? "true" : "false" ) << std::endl;
        std::cout << "\tANA_SETTINGS::IS_DATA = " << ( ANA_SETTINGS::IS_DATA ? "true" : "false" ) << std::endl;
        
        // try to load cbd and prod
        
        std::cout << "\tCDB tag: " << ANA_SETTINGS::cdbtag << std::endl;
        std::cout << "\tProduction tag: " << ANA_SETTINGS::prodtag << std::endl;

        std::cout << "\tfirst_segment, num_segments = " << ANA_SETTINGS::first_segment << ", " << ANA_SETTINGS::num_segments << std::endl;
        std::cout << "\tdsts = [ ";
        for ( const auto & dst : ANA_SETTINGS::in_dsts ) std::cout << dst << " ";
        std::cout << "]\n";

        // calo settings
        std::cout << "\tANA_SETTINGS::CALIBRATE_CALO = " << ( ANA_SETTINGS::CALIBRATE_CALO ? "true" : "false" ) << std::endl;
        std::cout << "\tANA_SETTINGS::RESCALE_CALOS = " << ( ANA_SETTINGS::RESCALE_CALOS ? "true" : "false" ) << std::endl;
        if ( ANA_SETTINGS::RESCALE_CALOS ) std::cout << "\tupscale_calo_factor = " << ANA_SETTINGS::scale_factor << std::endl;
        std::cout << "\tANA_SETTINGS::SAVE_FULL_CALO = " << ( ANA_SETTINGS::SAVE_FULL_CALO ? "true" : "false" ) << std::endl;
        
        // jet settings
        std::cout << "\tANA_SETTINGS::DO_JETS = " << ( ANA_SETTINGS::DO_JETS ? "true" : "false" ) << std::endl;
        if ( ANA_SETTINGS::DO_JETS ) std::cout << "\tjet_R = " << ANA_SETTINGS::jet_R << std::endl;
        std::cout << "\tANA_SETTINGS::EVENT_SELECT = " << ( ANA_SETTINGS::EVENT_SELECT ? "true" : "false" ) << std::endl;
    }

    return;
}

void Init_Ana_Inputs()
{
    int verbosity = std::max( Enable::VERBOSITY, ANA_SETTINGS::ANA_VERBOSITY );
    if ( verbosity > 0 )
    {
        std::cout << "Initializing analysis inputs..." << std::endl;
    }

    auto * se = Fun4AllServer::instance();

    for ( const auto & DSTTPYE : ANA_SETTINGS::in_dsts )
    {
        std::cout << "\tAdding input files: " << DSTTPYE << std::endl;
        auto * input = new Fun4AllDstInputManager( Form( "DSTINPUT_%s", DSTTPYE.c_str() ) );
        for ( int ifile = ANA_SETTINGS::first_segment; 
            ifile < ANA_SETTINGS::first_segment + ANA_SETTINGS::num_segments; 
            ++ifile )
        {
            std::string infile = "";
            if ( ANA_SETTINGS::IS_DATA )
            {
                infile = Form( "%s_%s-%08d-%05d.root", DSTTPYE.c_str(), ANA_SETTINGS::prodtag.c_str(), ANA_SETTINGS::run_number, ifile );
            }
            if ( ANA_SETTINGS::IS_SIM )
            {
                infile = Form( "%s_%s-%010d-%06d.root", DSTTPYE.c_str(), ANA_SETTINGS::prodtag.c_str(), ANA_SETTINGS::run_number, ifile );
            }
            if ( infile.empty() ) 
            {
                continue;
            }
            std::cout << "\t\tAdding file: " << infile << std::endl;
            input -> AddFile( infile );
        }
        input -> Verbosity( verbosity );
        se -> registerInputManager( input );
    }

    CaloTowerDefs::BuilderType buildertype = CaloTowerDefs::kPRDFTowerv4;
    auto * ingeom = new Fun4AllRunNodeInputManager( "DST_GEO" );
    auto geoLocation = CDBInterface::instance() -> getUrl( "calo_geo" );
    ingeom -> AddFile( geoLocation );
    se -> registerInputManager( ingeom );

    if ( verbosity > 0 )
    {
        std::cout << "Analysis inputs initialized." << std::endl;
    }
    return;
}

void Ana_Reco()
{
    int verbosity = std::max( Enable::VERBOSITY, ANA_SETTINGS::ANA_VERBOSITY );
    if ( verbosity > 0 )
    {
        std::cout << "Starting analysis reconstruction..." << std::endl;
    }

    auto * se = Fun4AllServer::instance();

    if ( ANA_SETTINGS::IS_DATA )  
    {
        auto * mbdreco = new MbdReco();
        se -> registerSubsystem( mbdreco );
      
        auto * epdreco = new EpdReco();
        epdreco -> Verbosity( Enable::VERBOSITY );
        se -> registerSubsystem( epdreco );
      
        auto * gvertex = new GlobalVertexReco();
        se -> registerSubsystem( gvertex );
      
        auto * zdcreco = new ZdcReco();
        zdcreco -> set_zdc1_cut(0.0);
        zdcreco -> set_zdc2_cut(0.0);
        se -> registerSubsystem( zdcreco );
    }
    
    auto * rcemc = new RetowerCEMC(); 
    rcemc -> set_towerinfo( true );
    rcemc -> set_frac_cut( 0.5 ); 
    rcemc -> set_towerNodePrefix( HIJETS::tower_prefix );
    rcemc -> Verbosity( Enable::VERBOSITY );
    se -> registerSubsystem( rcemc );

    if ( HIJETS::do_flow  > 0 )
    {
        auto * ep = new EventPlaneReco();
        if ( ANA_SETTINGS::IS_SIM ) 
        {
            ep -> set_inputNode( "TOWERINFO_CALIB_EPD" );
        }
        ep -> Verbosity( Enable::VERBOSITY );
        se -> registerSubsystem( ep );    
    }

    auto * mb = new MinimumBiasClassifier();
    if ( ANA_SETTINGS::IS_SIM )
    {
        mb -> setIsSim( true );
        mb -> setOverwriteScale( "/sphenix/user/dlis/Projects/centrality/cdb/calibrations/scales/cdb_centrality_scale_1.root" );
        mb -> setOverwriteVtx( "/sphenix/user/dlis/Projects/centrality/cdb/calibrations/vertexscales/cdb_centrality_vertex_scale_1.root" );
    }
    mb -> Verbosity( Enable::VERBOSITY );
    se -> registerSubsystem( mb );

    auto * cr = new CentralityReco();
    if ( ANA_SETTINGS::IS_SIM )
    {
        cr -> setOverwriteScale( "/sphenix/user/dlis/Projects/centrality/cdb/calibrations/scales/cdb_centrality_scale_1.root" );
        cr -> setOverwriteVtx( "/sphenix/user/dlis/Projects/centrality/cdb/calibrations/vertexscales/cdb_centrality_vertex_scale_1.root" );
        cr -> setOverwriteDivs( "/sphenix/user/dlis/Projects/centrality/cdb/calibrations/divs/cdb_centrality_1.root" );
    }
    cr -> Verbosity( Enable::VERBOSITY );
    se -> registerSubsystem( cr );

    return;
   
}


void Ana_JetReco()
{

    if ( !ANA_SETTINGS::DO_JETS )
    {
        return;
    }
    
    std::cout << "Starting jet reconstruction..." << std::endl;

    auto * se = Fun4AllServer::instance();

    auto * towerjetreco = new JetReco();
    for ( const auto & src : {
            Jet::CEMC_TOWERINFO_RETOWER,
            Jet::HCALIN_TOWERINFO,
            Jet::HCALOUT_TOWERINFO
        } 
    ) 
    { 
        towerjetreco -> add_input( ANA_SETTINGS::GetTowerInput( src ) ); 
    }
    towerjetreco -> add_algo( HIJETS::GetFJAlgo(0.2), "AntiKt_TowerInfo_HIRecoSeedsRaw_r02" );
    towerjetreco -> set_algo_node( HIJETS::jet_node );
    towerjetreco -> set_input_node( "TOWER" );
    towerjetreco -> Verbosity( Enable::HIJETS_VERBOSITY );
    se -> registerSubsystem( towerjetreco );

    auto * dtb = new DetermineTowerBackground( );
    dtb -> SetBackgroundOutputName( "TowerInfoBackground_Sub1" );
    dtb -> SetFlow( HIJETS::do_flow  );
    dtb -> SetSeedType( 0 );
    dtb -> SetSeedJetPt( 5.0 );
    dtb -> SetSeedJetD( 3.0 );
    dtb -> SetSeedMaxConst( 3.0 );
    dtb -> Verbosity( 0 );
    dtb -> UseReweighting( true );
    dtb -> set_towerNodePrefix( HIJETS::tower_prefix );
    se   -> registerSubsystem( dtb );

    auto * casj = new CopyAndSubtractJets( );
    casj -> SetFlowModulation( HIJETS::do_flow  );
    casj -> Verbosity( Enable::HIJETS_VERBOSITY  ); 
    casj -> set_towerinfo( true );
    casj -> set_towerNodePrefix( HIJETS::tower_prefix );
    se   -> registerSubsystem( casj );

    auto * dtb2 = new DetermineTowerBackground();
    dtb2 -> SetBackgroundOutputName( "TowerInfoBackground_Sub2" );
    dtb2 -> SetFlow( HIJETS::do_flow );
    dtb2 -> SetSeedType( 1 );
    dtb2 -> SetSeedJetPt( 7.0 );
    dtb2 -> Verbosity( 0 );
    dtb2 -> UseReweighting( true );
    dtb2 -> set_towerNodePrefix( HIJETS::tower_prefix );
    se   -> registerSubsystem( dtb2 );

    auto * st = new SubtractTowers();
    st -> SetFlowModulation( HIJETS::do_flow  );
    st -> Verbosity( 0 );
    st -> set_towerinfo( true );
    st -> set_towerNodePrefix( HIJETS::tower_prefix );
    se -> registerSubsystem( st );

    towerjetreco = new JetReco();
    for ( const auto & src : {
            Jet::CEMC_TOWERINFO_SUB1,
            Jet::HCALIN_TOWERINFO_SUB1,
            Jet::HCALOUT_TOWERINFO_SUB1
        }
    )        
    {
        towerjetreco -> add_input( ANA_SETTINGS::GetTowerInput( src ) ); 
    }
    towerjetreco -> add_algo( HIJETS::GetFJAlgo(ANA_SETTINGS::jet_R), Form("AntiKt_Tower_r0%d_Sub1", static_cast<int>(ANA_SETTINGS::jet_R * 10) ) );
    towerjetreco -> set_algo_node( HIJETS::jet_node );
    towerjetreco -> set_input_node( "TOWER" );
    towerjetreco -> Verbosity( Enable::HIJETS_VERBOSITY );
    se -> registerSubsystem( towerjetreco );

    return;
}

void PPG14()
{
    return;
}


#endif // __PPG14_C__