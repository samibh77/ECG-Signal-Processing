#include <gui/screen2_screen/Screen2View.hpp>

Screen2View::Screen2View()
  //  : startButtonCallback(this, &Screen2View::onStartButtonPressed)
{

}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();
    //startButton.setAction(startButtonCallback);
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}


/*void Screen2View::onStartButtonPressed(const touchgfx::AbstractButton& button)
{
    // Make sure it was our startButton
    if (&button == &startButton)
    {
        // Tell the presenter
        presenter->startButtonClicked();
    }
}*/
