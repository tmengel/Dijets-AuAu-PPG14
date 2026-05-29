#include "CaloManip.h"

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNode.h>
#include <phool/PHObject.h>
#include <phool/getClass.h>
#include <phool/phool.h> // for PHWHERE

#include <TRandom3.h>
#include <TTimeStamp.h>

#include <calobase/TowerInfo.h> 
#include <calobase/TowerInfoContainer.h>

#include <fun4all/Fun4AllReturnCodes.h>

#include <iostream>
#include <algorithm>
#include <cstdlib>



int CaloManip::InitRun( PHCompositeNode * topNode )
{

  if ( m_do_randomize_towers ) 
  {
    if ( m_random_seed == 0 ) 
    {
      auto ts = new TTimeStamp();
      m_random_seed = ts -> GetNanoSec();
      delete ts;
    }

    m_random = new TRandom3( m_random_seed );
  }


  if ( Verbosity( ) > 0 ) 
  {
    std::cout << PHWHERE << " Initialized CaloManip with input node: " << m_input_node 
        << " scale factor: " << m_scale_factor 
        << " min energy: " << m_min_energy 
        << " max energy: " << m_max_energy 
        << " output node: " << m_copy_node 
        << " save original towers: " << m_save_original_towers 
        << " randomize towers: " << m_do_randomize_towers 
        << " random seed: " << m_random_seed 
      << std::endl;
  }

  return CreateNode( topNode );

}

int CaloManip::process_event( PHCompositeNode * topNode )
{

  if ( Verbosity( ) > 1 )
  {
    std::cout << PHWHERE << " Processing event in CaloManip" << std::endl;
  }


  // load nodes
  int ret = LoadNodes( topNode );
  if ( ret != Fun4AllReturnCodes::EVENT_OK )
  {
    std::cout << PHWHERE << " Failed to load nodes in CaloManip" << std::endl;  
    return ret;
  }

  auto manip_towerinfo = m_srcti; // towers to be manipulated
  auto unmanip_towerinfo = m_copyti; // original towers ( copied over or from node )

  if ( m_do_randomize_towers ) 
  { 
    auto dj_khalid = randomize_keys( );
    for ( auto ich = 0; ich < m_ntowers; ich++ ) 
    {
      auto manip_tower = manip_towerinfo -> get_tower_at_channel( ich );  // original tower
      auto unmanip_tower = unmanip_towerinfo -> get_tower_at_channel( dj_khalid.at( ich ) ); // should be random if randomize is on
      manip_tower -> copy_tower( unmanip_tower );    
    } // end loop over channels
  }

  // the towers are now randomized in manip_towerinfo if randomization is on 
  // apply remainder of manipulations
  for ( auto ich = 0; ich < m_ntowers; ich++ )
  {
    auto t = manip_towerinfo -> get_tower_at_channel( ich );
    if ( !t )
    {
      if ( Verbosity( ) > 0 )
      {
        std::cout << PHWHERE << " Tower at channel " << ich << " not found, skipping. " << std::endl;
      }
      continue;
    }

    if ( t -> get_isHot() 
        || t -> get_isNoCalib() 
        || t -> get_isNotInstr() 
        || t -> get_isBadChi2() 
        || std::isnan( t -> get_energy() ) )
    { // tower is masked, skip
      continue;
    }

    float this_E = t -> get_energy();
  
    // apply energy scale
    this_E *= m_scale_factor; // will be 1.0 if no scaling set
    // this_E = std::clamp( this_E, m_min_energy, m_max_energy ); // will be -inf to +inf if no min/max set
    if ( Verbosity() > 3 )
    {
      std::cout << PHWHERE << " Tower channel " << ich 
        << " new energy: " << this_E 
        << " ( original energy: " << t -> get_energy() << " ) " 
        << std::endl;
    }
    t -> set_energy( this_E );
  } // end loop over channels

  // all done

  return Fun4AllReturnCodes::EVENT_OK;
}

int CaloManip::CreateNode( PHCompositeNode * topNode )
{
  
  std::cout << PHWHERE << " Creating nodes in CaloManip" << std::endl;
  auto src_towerinfo = getTowerInfo( topNode, m_input_node );

  if ( m_save_original_towers )
  {
    if ( m_copy_node.empty() )  // this is the default behavior
    {
      m_copy_node = m_input_node + "_ORIGINAL";
      if ( Verbosity() > 0 ) 
      {
        std::cout << PHWHERE \
          <<  " Copy node name not set, using default name: " << m_copy_node \
          << std::endl;
      }
    }

    std::cout << PHWHERE \
      <<  " Saving original towers from input node " << m_input_node \
      <<  " to copy node " << m_copy_node \
      << std::endl;
    
    auto m_copy_towerinfo = getTowerInfo( topNode, m_copy_node, false ); // don't abort on missing
    if ( !m_copy_towerinfo )
    {
      std::cout << PHWHERE \
        <<  " Creating copy node " << m_copy_node << " of input node " << m_input_node \
        << std::endl;

      PHNodeIterator iter(topNode);
      // auto dstNode = dynamic_cast< PHCompositeNode * >( iter.findFirst( "PHCompositeNode", "DST" ) );
      // if ( !dstNode ) 
      // {
      //   std::cout << PHWHERE << "DST Node missing, doing nothing." << std::endl;
      //   return Fun4AllReturnCodes::ABORTRUN;
      // }

      // PHNodeIterator dstiter( dstNode );
      auto src_node = dynamic_cast< PHNode* > (iter.findFirst( "PHIODataNode", m_input_node ) );
      if ( !src_node ) 
      { 
        std::cout << PHWHERE << m_input_node << " Node missing, doing nothing." << std::endl;
        return Fun4AllReturnCodes::ABORTRUN;
      }

      auto parentNode = dynamic_cast< PHCompositeNode * >( src_node->getParent() );
      if (! parentNode ) 
      {
        std::cout << PHWHERE << "parentNode is missing, doing nothing." << std::endl;
        return Fun4AllReturnCodes::ABORTRUN;
      }

      m_copy_towerinfo = dynamic_cast< TowerInfoContainer* >( src_towerinfo->CloneMe() );
      auto newNode = new PHIODataNode< PHObject >( m_copy_towerinfo, m_copy_node, "PHObject" );
      parentNode->addNode( newNode );

    }// end of create copy node

    // make sure it exists now
    m_copy_towerinfo = getTowerInfo( topNode, m_copy_node, true ); // abort on missing
    
    if ( Verbosity() > 0 ) 
    {
      std::cout << PHWHERE \
        <<  " Copy node " << m_copy_node << " exists." \
        << std::endl;
      m_copy_towerinfo->identify();
      std::cout << std::endl;
      std::cout << PHWHERE \
        <<  " Input node " << m_input_node << " exists." \
        << std::endl;
      src_towerinfo->identify();
      std::cout << std::endl;
    }

  } // end of m_save_original_towers
  
  return Fun4AllReturnCodes::EVENT_OK;
} 

int CaloManip::LoadNodes( PHCompositeNode * topNode )
{
  if ( Verbosity( ) > 0 )
  {
    std::cout << PHWHERE << " Loading nodes in CaloManip" << std::endl;
  }

  m_srcti = getTowerInfo( topNode, m_input_node );
  if ( m_save_original_towers )
  {
    m_copyti = getTowerInfo( topNode, m_copy_node );
  }
  else
  {
    m_copyti = (TowerInfoContainer *) m_srcti -> CloneMe();
  }  
  m_copyti -> Reset();

  m_ntowers = m_srcti -> size( );
  m_unmasked_keys.clear();
  m_masked_keys.clear();
  for ( auto ich = 0; ich < m_ntowers; ich++ )
  {
    auto st = m_srcti -> get_tower_at_channel( ich );
    auto ct = m_copyti -> get_tower_at_channel( ich );
    ct -> copy_tower( st ); // copy original tower info
    
    if ( ! ( st -> get_isHot() 
        || st -> get_isNoCalib() 
        || st -> get_isNotInstr() 
        || st -> get_isBadChi2() 
        || std::isnan( st -> get_energy() ) ) )
    {
      m_unmasked_keys.push_back( ich );
    }
    else 
    {
      m_masked_keys.push_back( ich );
    }
  } // end loop over channels

  if ( Verbosity( ) > 1 )
  {
    std::cout << PHWHERE << " Loaded nodes in CaloManip" << std::endl;
    std::cout << "Source TowerInfoContainer: " << std::endl;
    m_srcti -> identify();
    std::cout << "Copy TowerInfoContainer: " << std::endl;
    m_copyti -> identify();
    
    
    // individual tower info (alot of output)
    if ( Verbosity( ) > 4 )
    {
      std::cout << "(E_org, E_copy), (t_org, t_copy) for all towers: " << std::endl;
      for ( auto ich = 0; ich < m_ntowers; ich++ )
      {
        auto st = m_srcti -> get_tower_at_channel( ich );
        auto ct = m_copyti -> get_tower_at_channel( ich );
        std::cout << "Channel " << ich << ": ( " \
          << st -> get_energy() << ", " \
          << ct -> get_energy() << " ), (" \
          << st -> get_time() << ", " \
          << ct -> get_time() << " ) " \
          << std::endl;
      } // end loop over channels
      std::cout << std::endl;
    } // end Verbosity > 3



  } // end Verbosity > 0

  return Fun4AllReturnCodes::EVENT_OK;
}

inline TowerInfoContainer * CaloManip::getTowerInfo( PHCompositeNode * topNode, 
  const std::string & node_name , bool abort_on_missing 
)
{
  auto towerinfo = findNode::getClass<TowerInfoContainer>( topNode, node_name );
  if ( ! towerinfo )
  {
    if ( abort_on_missing )
    {
      std::cout << PHWHERE \
        <<  " Input node " << node_name << " missing, aborting." \
        << std::endl;
      exit(1);
    }
    else
    {
      std::cout << PHWHERE \
        <<  " Input node " << node_name << " missing, returning nullptr." \
        << std::endl;
      return nullptr;
    }
  }
  return towerinfo;
}

std::vector< unsigned int > CaloManip::randomize_keys()
{
  
  std::vector<unsigned int>  tower_keys = m_unmasked_keys; // copy of unmasked keys
  // fisher-yates shuffle
  for ( unsigned int iidx = tower_keys.size() - 1; iidx > 0; --iidx ) 
  {
    unsigned int jidx = m_random -> Integer( iidx + 1 );
    std::swap( tower_keys[iidx], tower_keys[jidx] );
  }
  // now tower_keys is shuffled version of m_unmasked_keys
  // insert masked keys back to their original position
  for ( auto masked_key : m_masked_keys ) 
  {
    tower_keys.insert( tower_keys.begin() + masked_key, masked_key );
  }

  // tower_keys is now a complete list of all keys from 0, nchannels -1
  // the unmasked keys are randomly shuffled to another unmasked key position
  // the masked keys are fixed to their original position 
  // ( so they aren't randomized and remain at starting position )
  return tower_keys;

}  

