#ifndef SCREEN4VIEW_HPP
#define SCREEN4VIEW_HPP

#include <gui_generated/screen4_screen/Screen4ViewBase.hpp>
#include <gui/screen4_screen/Screen4Presenter.hpp>

class Screen4View : public Screen4ViewBase
{
public:
    Screen4View();
    virtual ~Screen4View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void UpdateGraphBPM(float value);
    virtual void UpdateBPMText(int bpm);
protected:
    Unicode::UnicodeChar textArea1Buffer[200];
};

#endif // SCREEN4VIEW_HPP
