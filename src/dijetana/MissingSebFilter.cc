#include "MissingSebFilter.h"

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

// tower info
#include <calobase/TowerInfo.h>
#include <calobase/TowerInfoContainer.h>

#include <cstdlib>

MissingSebFilter::MissingSebFilter() : EventCut("MissingSebFilter") 
{ 
    SetNodeNames({ "TOWERINFO_CALIB_CEMC", "TOWERINFO_CALIB_HCALIN", "TOWERINFO_CALIB_HCALOUT" }); // Set the default node names
}


void MissingSebFilter::identify(std::ostream &os) const
{
    os << Name() + "::identify: " << std::endl;
    for (const auto &node_name : GetNodeNames())
    {
        os << "  Node name: " << node_name << std::endl;
    }
    return;
}

bool MissingSebFilter::operator()(PHCompositeNode *topNode)
{

    for (const auto &node_name : GetNodeNames())
    {
        auto towers = findNode::getClass<TowerInfoContainer>(topNode, node_name);
        if (!towers) {
            std::cerr << Name() + "::operator(PHCompositeNode *topNode) Could not find " << node_name << " node" << std::endl;
            exit(-1); // this is a fatal error
        }

        // std::vector<int> dead_channels = ; 
        unsigned int nChannels = towers->size();
        unsigned int nDead = 0;
        for(unsigned int channel = 0; channel < nChannels; channel++){
           
            auto tower = towers->get_tower_at_channel(channel);
            int status = tower->get_status();

            if (tower->get_isHot() )
            {
                nDead++;
                if ( Verbosity() ) {
                    std::cout << Name() + "::operator(PHCompositeNode *topNode) Checking channel " << channel  << " with status " << status << std::endl;
                }
                continue;
            }
            if ( tower->get_isNoCalib() )
            {
                nDead++;
                if ( Verbosity() ) {
                    std::cout << Name() + "::operator(PHCompositeNode *topNode) Checking channel " << channel  << " with status " << status << std::endl;
                }
                continue;
            }
            if ( tower->get_isNotInstr() )
            {
                nDead++;
                if ( Verbosity() ) {
                    std::cout << Name() + "::operator(PHCompositeNode *topNode) Checking channel " << channel  << " with status " << status << std::endl;
                }
                continue;
            }
            if ( tower->get_isBadChi2() )
            {
                nDead++;
                if ( Verbosity() ) {
                    std::cout << Name() + "::operator(PHCompositeNode *topNode) Checking channel " << channel  << " with status " << status << std::endl;
                }
                continue;
            }
            if (std::isnan(tower->get_energy())) {
                nDead++;
                if ( Verbosity() ) {
                    std::cout << Name() + "::operator(PHCompositeNode *topNode) Checking channel " << channel  << " with status " << status << std::endl;
                }
                continue;
            }
        }
        
        float frac_dead = static_cast<float>(nDead) / static_cast<float>(nChannels);
        float threshold = m_threshold;
        bool accepted = (frac_dead < threshold);
        if(Verbosity()){
            std::cout << Name() + "::operator(PHCompositeNode *topNode) Node " << node_name << ": " 
                      << nDead << " dead channels out of " << nChannels 
                      << " (" << frac_dead*100.0 << "%), threshold is " << threshold*100.0 << "% --> "
                      << (accepted ? "ACCEPTED" : "REJECTED") << std::endl;
        }

        Passed(accepted);
        if (!Passed()){
            if(Verbosity()){
                std::cout << Name() + "::operator(PHCompositeNode *topNode) Node " << node_name << " failed, " 
                          << nDead << " dead channels out of " << nChannels << std::endl;
            }
            return Passed();
        } 
    }

    return Passed();
    
}
