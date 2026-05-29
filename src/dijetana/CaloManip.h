#ifndef _DIJETANA_CALOMANIP_H_
#define _DIJETANA_CALOMANIP_H_

#include <fun4all/SubsysReco.h>

#include <string>
#include <vector>
#include <limits>

class PHCompositeNode;
class TRandom3;
class TowerInfoContainer;

class CaloManip : public SubsysReco {
 
 public:

  CaloManip( const std::string & name = "CaloManip" ) : SubsysReco(name) {}
  ~CaloManip() override {}

  int InitRun( PHCompositeNode * topNode ) override;

  int process_event( PHCompositeNode * topNode ) override;

  void setInput ( const std::string & name ) 
  { m_input_node = name; }

  void scaleE ( const float factor ) 
  { m_scale_factor = factor; }

  void setMinE( const float min_e ) 
  { m_min_energy = min_e; }
  
  void setMaxE( const float max_e ) 
  { m_max_energy = max_e; }
  
  void setRangeE( const float min_e, const float max_e ) 
  { m_min_energy = min_e; m_max_energy = max_e; }

  void copyOriginal( const bool b = true, const std::string & name = "" ) 
  { m_save_original_towers = b; m_copy_node = name; }

  void randomizePos( const unsigned int seed  = 0 )
  { m_do_randomize_towers = true; m_random_seed = seed; } // kinda fun , 0 means use time-based seed

 private:

  std::string m_input_node { "" };
  
  float m_scale_factor { 1.0 };
  float m_min_energy { -std::numeric_limits<float>::max() };
  float m_max_energy {  std::numeric_limits<float>::max() };
  
  bool m_save_original_towers { false };
  std::string m_copy_node { "" };
  
  bool m_do_randomize_towers { false };
  unsigned int m_random_seed { 0 };
  TRandom3 * m_random { nullptr };

  TowerInfoContainer* m_srcti { nullptr };
  TowerInfoContainer* m_copyti { nullptr} ;

  int m_ntowers { -1 };
  std::vector< unsigned int  > m_unmasked_keys {};
  std::vector< unsigned int  > m_masked_keys {};
  std::vector< unsigned int  > randomize_keys( );

  int CreateNode( PHCompositeNode * topNode );
  int LoadNodes( PHCompositeNode * topNode );

  inline TowerInfoContainer * getTowerInfo( 
    PHCompositeNode * topNode, 
    const std::string & node_name , 
    bool abort_on_missing = true );

};

#endif  // CALOMANIP_CALOMANIP_H
