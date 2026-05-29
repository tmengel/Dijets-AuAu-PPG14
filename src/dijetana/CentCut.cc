#include "CentCut.h"

// phool includes
#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

// jet reco
#include <centrality/CentralityInfo.h>
#include <centrality/CentralityInfov1.h>
#include <centrality/CentralityInfov2.h>

#include <cstdlib>

void CentCut::identify(std::ostream &os) const
{
    os << Name() + "::identify: " << std::endl;
    os << "  Node name: " << GetNodeName() << std::endl;
    os << "  Cent range: " << "[" << GetRange().second << ", " << GetRange().first << "]" << std::endl;
    return;
}

bool CentCut::operator()(PHCompositeNode *topNode)
{


    // get centrality
    auto cent_node = findNode::getClass< CentralityInfo >(topNode, GetNodeName());
    if ( !cent_node ) {
        std::cerr << Name() + "::operator(PHCompositeNode *topNode) Could not find " + GetNodeName() << std::endl;
        exit(-1); // this is a fatal error
    }

    int cent = -1;
    if ( cent_node -> has_centrality_bin(CentralityInfo::PROP::mbd_NS) ) {
        cent = (int)(cent_node->get_centrality_bin(CentralityInfo::PROP::mbd_NS));
    } else if ( cent_node -> has_centile(CentralityInfo::PROP::mbd_NS) ){
        cent = (int)(cent_node->get_centile(CentralityInfo::PROP::mbd_NS));
    } 

    if ( cent < 0 ) {
        if(Verbosity()){
            std::cout << Name() + "::operator(PHCompositeNode *topNode) Could not find valid centrality in " + GetNodeName() << std::endl;
        }
        return false;
    }

    Passed((cent > GetRange().first) && (cent <= GetRange().second)); // check centrality
   
    if( !Passed() && Verbosity() ){
        std::cout << Name() + "::operator(PHCompositeNode *topNode) Event failed" + Name() + " centrality selection. Centrality: " + std::to_string(cent) << std::endl;
    }
    
    return Passed();
}
