#include <gui/screen3_screen/Screen3View.hpp>
#include "stm32f4xx_hal.h"


Screen3View::Screen3View()
{

}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();

}

void Screen3View::tearDownScreen()
{

	Screen3ViewBase::tearDownScreen();

}


void  Screen3View:: UpdateGraphRaw(float value){



	dynamicGraph1.addDataPoint(value);
}
