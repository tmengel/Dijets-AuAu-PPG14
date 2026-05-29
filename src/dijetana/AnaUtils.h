#ifndef _DIJETANA_ANAUTILS_H_
#define _DIJETANA_ANAUTILS_H_

#include <vector>
#include <string>

namespace AnaUtils
{
    float get_dpsi2( const float psi2, const float phi );
    
    float deta_abs( const float eta1, const float eta2 ) ;
    float dphi_wrap( const float phi1, const float phi2 ) ;

    float calc_dr( const float eta1, const float phi1, const float eta2, const float phi2 ) ;

    bool accept_jet_eta( const float eta, const float zvrtx, const float jet_R  = 0.3 ) ;
    float correct_calo_eta( const float eta0, const float zvrtx, const float R ) ;

    double flow_func( double * x, double * par );

    bool phi_top ( const float phi );
    bool phi_bottom ( const float phi );
    bool phi_west ( const float phi );
    bool phi_east ( const float phi );
    bool phi_vert ( const float phi );
    bool phi_horz ( const float phi );
    bool eta_neg ( const float eta );
    bool eta_pos ( const float eta );

    bool in_plane( const float psi2, const float phi_jet ) ;
    bool mid_plane( const float psi2, const float phi_jet ) ;
    bool out_of_plane( const float psi2, const float phi_jet ) ;   

   std::vector< std::string > getFilelist( const std::string & inlist , const std::string & ext = ".root" );
   
} // namespace MyAna

#endif // _DIJETANA_ANAUTILS_H_