#include <gui/screen4_screen/Screen4View.hpp>
#include <gui/screen4_screen/Screen4Presenter.hpp>

Screen4Presenter::Screen4Presenter(Screen4View& v)
    : view(v)
{

}

void Screen4Presenter::activate()
{

}

void Screen4Presenter::deactivate()
{

}

void Screen4Presenter::UpdateGraphBPM(float value)
{
	view.UpdateGraphBPM(value);

}

void Screen4Presenter::UpdateBPMText(int bpm)
{
    view.UpdateBPMText(bpm);
}

