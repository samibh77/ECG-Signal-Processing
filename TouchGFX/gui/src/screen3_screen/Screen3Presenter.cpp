#include <gui/screen3_screen/Screen3View.hpp>
#include <gui/screen3_screen/Screen3Presenter.hpp>
#include <gui/model/Model.hpp>
#include <SDW.h>

extern input buzzer_enable;
extern SDW sdw1;

Screen3Presenter::Screen3Presenter(Screen3View& v)
    : view(v)
{

}

void Screen3Presenter::activate()
{
    Presenter::activate();
    activateBuzzer();
}

void Screen3Presenter::deactivate()
{
    deactivateBuzzer();
    Presenter::deactivate();
}


void Screen3Presenter::UpdateGraphRaw(float value)
{
	view.UpdateGraphRaw(value);

}


void Screen3Presenter::activateBuzzer()
{

    buzzer_enable.addToQueue(1.0);
    sdw1.setInputs();


}

void Screen3Presenter::deactivateBuzzer()
{

	   buzzer_enable.addToQueue(0.0);
	   sdw1.setInputs();

}




