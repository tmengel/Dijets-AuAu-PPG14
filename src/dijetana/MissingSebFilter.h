/*!
 * \file MissingSebFilter.h
 * \brief Filters events based on the presence of a all SEBs 
 * \author Tanner Mengel <tmengel@bnl.gov>
 * \version $Verison: 1.0.1 $
 * \date $Date: 09/10/2024 $
 */

#ifndef _DIJETANA_MISSINGSEBFILETER_H_
#define _DIJETANA_MISSINGSEBFILETER_H_

#include "EventCut.h"

class PHCompositeNode;

class MissingSebFilter  : public EventCut
{
  public:

    MissingSebFilter();
    ~MissingSebFilter() override {}

    void identify(std::ostream &os = std::cout) const override;
    bool operator()(PHCompositeNode* topNode) override;
    void setThreshold(float threshold) { m_threshold = threshold; }
  private:
    float m_threshold = 1.0/16.0;

};

#endif // EVENTSELECTION_MISSINGSEBFILETER_H