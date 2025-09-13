#include <gui/screen4_screen/Screen4View.hpp>

Screen4View::Screen4View()
{

}

void Screen4View::setupScreen()
{
    Screen4ViewBase::setupScreen();
    Unicode::snprintf(textArea1Buffer, sizeof(textArea1Buffer), "%d", 0); // Initialize with 0
    textArea1.setWildcard(textArea1Buffer);
    textArea1.invalidate();
    //textArea1.setWildcard(textArea1Buffer);
   // UpdateBPMText(60);
}

void Screen4View::tearDownScreen()
{
    Screen4ViewBase::tearDownScreen();
}

void  Screen4View:: UpdateGraphBPM(float value){
	dynamicGraph1.addDataPoint(value);
	dynamicGraph1.invalidate();
}


void Screen4View::UpdateBPMText(int bpm)
{
    Unicode::snprintf(textArea1Buffer, sizeof(textArea1Buffer), "%d", bpm);

    textArea1.setWildcard(textArea1Buffer);
    textArea1.invalidate();
}
