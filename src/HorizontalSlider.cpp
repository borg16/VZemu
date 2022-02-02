#include "HorizontalSlider.hpp"

#include <widget/SvgWidget.hpp>
#include <app/SvgSlider.hpp>
#include <asset.hpp>

#include "VZemu.hpp"

using namespace rack;

namespace VZemu
{
    HorizontalSlider::HorizontalSlider()
    {
        math::Vec margin = math::Vec(2, 0);
        maxHandlePos = math::Vec(45, 1).plus(margin);
        minHandlePos = math::Vec(0, 1).plus(margin);
        setBackgroundSvg(window::Svg::load(asset::plugin(pluginInstance, "res/VCVSliderHorizontal.svg")));
        setHandleSvg(window::Svg::load(asset::plugin(pluginInstance, "res/VCVSliderHandleHorizontal.svg")));
        horizontal = true;
    }
}
