#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "stm32f4xx_hal.h"
#include "SDW.h"
#include "APP.h"


extern  float ecg_filtered_val;
extern  float ecg_measure_val;
extern  float ecg_bpm_val;



Model::Model() : modelListener(0)
{

}

void Model::tick()
{
	modelListener->UpdateGraph(ecg_filtered_val);
	modelListener->UpdateGraphRaw(ecg_measure_val);
	modelListener->UpdateGraphBPM(ecg_bpm_val*100);
	modelListener->UpdateBPMText((int)ecg_bpm_val*100);



}



