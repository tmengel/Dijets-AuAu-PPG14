#include "PlotUtils.h"
#include "sphenix_style.h"

#include <iostream>

#include <TLatex.h>
#include <TROOT.h>
#include <TError.h>

void PlotUtils::SetStyle()
{
    set_sphenix_style();
    gErrorIgnoreLevel = kWarning;
    std::cout << "PlotUtils: Style set to sPHENIX." << std::endl;
    return;
}

void PlotUtils::myText( double x, double y, int color, const char * text, const float size )
{
    TLatex * t = new TLatex();
    t -> SetNDC();
    t -> SetTextSize(size);
    t -> SetTextColor(color);
    t -> DrawLatex(x, y, text);
}

