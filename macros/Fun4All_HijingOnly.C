#ifndef _FUN4ALL_PPG14_C_
#define _FUN4ALL_PPG14_C_

#include <HIJetReco.C>

#include <Calo_Calib.C>

#include "PPG14.C"

#include <ffamodules/CDBInterface.h>

#include <fun4all/Fun4AllServer.h>

#include <phool/recoConsts.h>

#include <dijetana/EventSelector.h>
#include <dijetana/MinBiasCut.h>
#include <dijetana/TriggerSelect.h>
#include <dijetana/MissingSebFilter.h>
#include <dijetana/AnaTree.h>

#include <dijetana/CaloManip.h>

R__LOAD_LIBRARY( libfun4all.so )
R__LOAD_LIBRARY( libdijetana.so )


void Fun4All_HijingOnly ( 
    const std::string & conf_file = "config.txt" ,
    const int user_first_segment = -1,
    const int user_num_segments = -1,
    const std::string & user_output_file = "" 
)
{

    std::cout << "Starting Fun4All_PPG14()" << std::endl;

    ANA_SETTINGS::ANA_VERBOSITY = 1;

    Init_Ana_Settings( conf_file );

    // output settings
    std::string outfile                 = ANA_SETTINGS::conf -> GetString( "outfile", "myout.root" );
    if ( !user_output_file.empty() )
    {
        outfile = user_output_file;
    }
    std::cout << "Output file: " << outfile << std::endl;
    if ( user_first_segment >= 0 )
    {
        ANA_SETTINGS::first_segment = user_first_segment;
    }
    if ( user_num_segments >= 0 )
    {
        ANA_SETTINGS::num_segments = user_num_segments;
    }
    std::cout << "First segment: " << ANA_SETTINGS::first_segment << std::endl;
    std::cout << "Num segments: " << ANA_SETTINGS::num_segments << std::endl;

    // global verbosity
    Enable::VERBOSITY                   = ANA_SETTINGS::conf -> GetInt( "verbosity", 0 );
    Enable::HIJETS_VERBOSITY            = ANA_SETTINGS::conf -> GetInt( "hijets_verbosity", 0 );

    // jet settings
    HIJETS::do_flow                     = ANA_SETTINGS::conf -> GetInt( "ue_flow_flag", 0 );
    HIJETS::do_vertex_type              = true;
    HIJETS::is_pp                       = false;
    HIJETS::tower_prefix                = "TOWERINFO_CALIB";
    Enable::HIJETS_TOWER                = true;
    if ( Enable::HIJETS_VERBOSITY  > 0 ) 
    {
        std::cout << "\tHIJETS::do_flow = " << HIJETS::do_flow << std::endl;
        std::cout << "\tHIJETS::do_vertex_type = " << ( HIJETS::do_vertex_type ? "true" : "false" ) << std::endl;
        std::cout << "\tHIJETS::is_pp = " << ( HIJETS::is_pp ? "true" : "false" ) << std::endl;
        std::cout << "\tHIJETS::tower_prefix = " << HIJETS::tower_prefix << std::endl;
        std::cout << "\tEnable::HIJETS_TOWER = " << ( Enable::HIJETS_TOWER ? "true" : "false" ) << std::endl;
    }

    auto * se = Fun4AllServer::instance();
    se -> Verbosity( Enable::VERBOSITY  );

    auto * rc = recoConsts::instance();
    rc -> set_StringFlag( "CDB_GLOBALTAG", ANA_SETTINGS::cdbtag );
    rc -> set_uint64Flag( "TIMESTAMP", ANA_SETTINGS::run_number );
    CDBInterface::instance( ) -> Verbosity( Enable::VERBOSITY );

    Init_Ana_Inputs();

    if ( ANA_SETTINGS::CALIBRATE_CALO ) 
    { 
        Process_Calo_Calib(); 
    }

    if ( ANA_SETTINGS::RESCALE_CALOS )
    {
        for ( const auto & layer : {"CEMC", "HCALIN", "HCALOUT"} )
        {
            auto * cm = new CaloManip(Form("CaloRescale_%s", layer));
            auto input_node = HIJETS::tower_prefix + "_" + layer;
            auto org_node = HIJETS::tower_prefix + "_ORIGINAL_" + layer;
          
            std::cout << "CaloManip for " << layer << ": input node = " << input_node << ", original node = " << org_node << std::endl;
            
            cm -> setInput( input_node );
            cm -> scaleE(  ANA_SETTINGS::scale_factor );
            cm -> copyOriginal( true, org_node );
            cm -> Verbosity( Enable::VERBOSITY  );
            se -> registerSubsystem( cm );
        }
    } 

    Ana_Reco();

    if ( ANA_SETTINGS::EVENT_SELECT )
    {
    
        auto * es = new EventSelector( );
        es -> Verbosity( Enable::VERBOSITY );
        
        auto * mbc = new MinBiasCut( );
        mbc -> SetNodeName( "MinimumBiasInfo" );
        es  -> AddCut( mbc );

        if ( ANA_SETTINGS::IS_DATA ) 
        {
            auto * trigger = new TriggerSelect();
            trigger -> SetPacket( 14001 );
            trigger -> SelectTrigger( 10 );
            es -> AddCut( trigger );

            auto * msf = new MissingSebFilter( );
            msf -> SetNodeNames( {
                "TOWERINFO_CALIB_CEMC_RETOWER",
                "TOWERINFO_CALIB_HCALIN",
                "TOWERINFO_CALIB_HCALOUT"
                } );
            msf -> setThreshold( 1.0 / 16.0 ); // allow up to 90 dead channels in the ECal
            msf -> Verbosity( Enable::VERBOSITY );
            es  -> AddCut( msf );
        }
        es -> PrintCuts( );
        se -> registerSubsystem( es );
    }

    if ( ANA_SETTINGS::DO_JETS )
    {
        Ana_JetReco();
    }

    auto * atree = new AnaTree( outfile );
    if ( ANA_SETTINGS::IS_DATA )
    {
        atree -> add_gl1_node ( "14001" );
    }
    atree -> add_zvrtx_node ( "GlobalVertexMap" );
    atree -> add_cent_node ( "CentralityInfo" );
    if ( ANA_SETTINGS::DO_JETS )
    {
        atree -> add_sub1jet_node ( Form("AntiKt_Tower_r0%d_Sub1", static_cast<int>(ANA_SETTINGS::jet_R * 10) ) );
        atree -> save_towerbkgd ( true );
    }
    if ( ANA_SETTINGS::IS_SIM )
    {
        atree -> add_event_header ( "EventHeader" );
        if ( ANA_SETTINGS::jet_samp > 0 )
        {
            atree -> add_truthjet_node ( Form( "AntiKt_Truth_r0%d", static_cast<int>(ANA_SETTINGS::jet_R * 10) ) );
        }
    }
    if ( ANA_SETTINGS::SAVE_FULL_CALO )
    {
        atree -> save_full_calo( 
            "TOWERINFO_CALIB_CEMC_RETOWER", 
            "TOWERINFO_CALIB_HCALIN", 
            "TOWERINFO_CALIB_HCALOUT" 
        );
    }
    atree -> Verbosity( Enable::VERBOSITY  );
    se -> registerSubsystem( atree );

    se -> run( ANA_SETTINGS::num_events );
    se -> End( );   
    se -> PrintTimer( );

    CDBInterface::instance() -> Print();

    delete se;

    std::cout << "Done4All" << std::endl;

    gSystem -> Exit( 0 );

}

#endif // _FUN4ALL_PPG14_C_
