#ifndef _DIJETANA_ANATREE_H_
#define _DIJETANA_ANATREE_H_

#include <fun4all/SubsysReco.h>

#include <calobase/RawTowerDefs.h>

#include <string>
#include <vector>
#include <cstdint>

class PHCompositeNode;

class TowerInfoContainer;
class RawTowerGeomContainer;

class TTree;

class AnaTree : public SubsysReco
{
 public:

  AnaTree( const std::string & outputfile = "output.root" );
  ~AnaTree() override {}

  int Init( PHCompositeNode */*topNode*/  ) override;
  int InitRun( PHCompositeNode *topNode ) override;
  int process_event( PHCompositeNode * topNode ) override;
  int End( PHCompositeNode * /*topNode*/ ) override;

  void add_gl1_node ( const std::string & name = "GL1Packet"){  m_gl1_node = name; }

  void add_zvrtx_node ( const std::string & name = "GlobalVertexMap" ){ m_zvrtx_node = name; }

  void add_cent_node ( const std::string & name = "CentralityInfo" ) { m_cent_node = name;  }

  void add_sub1jet_node ( const std::string & name ) { m_sub1jet_node = name; }

  void save_towerbkgd( const bool b = true ) { m_do_towerbkgd = b; }

  void add_truthjet_node ( const std::string & name ) { m_truthjet_node = name; }

  void add_event_header( const std::string & name =  "EventHeader"){ m_eventhead_node = name; }
 
  void save_full_calo( const std::string & cemcnode, const std::string & hcalincode, const std::string & hcaloutcode, const bool b = true ) 
  { 
    m_save_full_calo = b; 
    m_fullcalo_nodes = { cemcnode, hcalincode, hcaloutcode };
  }
  void save_sumeT( const std::string & cemcsumeTnode, const std::string & hcalinsumeTnode, const std::string & hcaloutsumeTnode, const bool b = true ) 
  { 
    m_save_sumeT = b; 
    m_sumeT_nodes = { cemcsumeTnode, hcalinsumeTnode, hcaloutsumeTnode };
  }
  void save_sub1_sumeT( const std::string & cemcsumeTnode, const std::string & hcalinsumeTnode, const std::string & hcaloutsumeTnode, const bool b = true ) 
  { 
    m_save_sub1_sumeT = b; 
    m_sub1_sumeT_nodes = { cemcsumeTnode, hcalinsumeTnode, hcaloutsumeTnode };
  }
  void save_org_sumeT( const std::string & cemcsumeTnode, const std::string & hcalinsumeTnode, const std::string & hcaloutsumeTnode, const bool b = true ) 
  { 
    m_save_org_sumeT = b; 
    m_org_sumeT_nodes = { cemcsumeTnode, hcalinsumeTnode, hcaloutsumeTnode };
  }

 private:
    
  // output file name
  std::string m_output_filename { "" };

  TTree * m_tree {nullptr};

  int m_event_id {-1};

  // gl1
  std::string m_gl1_node {""};
  uint64_t s_triggervec { 0 };
  uint64_t l_triggervec { 0 };

  // z vertex info
  std::string m_zvrtx_node { "" };
  float m_zvtx { 0.0 };

  // centrality info
  std::string m_cent_node { "" };
  int m_cent {-1};
  
  // event header info
  std::string m_eventhead_node { "" };
  float m_b { 0.0 };
  float m_ep_angle { 0.0 };
  float m_ecc { 0.0 };
  float m_psi_arr[6] {};
  float m_ncoll { 0.0 };
  float m_npart { 0.0 };
  int m_runnumber { 0 };
  int m_evtsequence { 0 };
 
  // calo info
  static const int k_ieta = 24;
  static const int k_iphi = 64;
  bool m_save_full_calo { false };
  std::vector< std::string > m_fullcalo_nodes {};
  float m_tower_E[3][k_ieta][k_iphi] {};
  
  bool m_save_sumeT { false };
  std::vector< std::string > m_sumeT_nodes {};
  bool m_save_sub1_sumeT { false };
  std::vector< std::string > m_sub1_sumeT_nodes {};
  bool m_save_org_sumeT { false };
  std::vector< std::string > m_org_sumeT_nodes {};
  float m_sumeT[3] { 0.0 }; // 0: cemc, 1: hcalin, 2: hcalout
  float m_sumeT_sub1[3] { 0.0 };
  float m_sumeT_org[3] { 0.0 };
  std::vector<float> SumCaloE( PHCompositeNode *topNode, const std::vector< std::string > &towerinfo_nodes );
  
  // sub1 jet info
  std::string m_sub1jet_node { "" };
  std::vector < float > m_sub1_jet_E {};
  std::vector < float > m_sub1_jet_phi {};
  std::vector < float > m_sub1_jet_eta {};
  std::vector < float > m_sub1_jet_pT {};
  std::vector < float > m_sub1_jet_unsub_pT {};
  std::vector < float > m_sub1_jet_unsub_E {};
  
  // truth jet info
  std::string m_truthjet_node { "" };
  std::vector < float > m_truth_jet_E {};
  std::vector < float > m_truth_jet_phi {};
  std::vector < float > m_truth_jet_eta {};
  std::vector < float > m_truth_jet_pT {};

  // tower background info
  bool m_do_towerbkgd = false;
  float m_sub2_v2 { 0.0 };
  int m_sub2_flowfaliure { 0 };
  float m_sub2_psi2 { 0.0 };
  float m_sub2_towerbkgd_ue[3][k_ieta] {}; // 0: cemc, 1: hcalin, 2: hcalout
 
  // helpers
  RawTowerDefs::CalorimeterId m_caloid = RawTowerDefs::CalorimeterId::NONE;
  TowerInfoContainer    * m_towerinfos = nullptr;
  RawTowerGeomContainer * m_towergeom  = nullptr;
  double m_calo_r[3] {}; // 0: cemc, 1: hcalin, 2: hcalout
  TowerInfoContainer * LoadTowerInfoContainer( PHCompositeNode *topNode, const std::string &name );
  RawTowerGeomContainer * LoadTowerGeomContainer( PHCompositeNode *topNode, const std::string &name );
};


#endif // _DIJETANA_ANATREE_H_
