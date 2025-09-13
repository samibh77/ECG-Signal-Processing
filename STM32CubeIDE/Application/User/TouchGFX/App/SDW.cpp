/*
 * SDW.cpp
 *
 *  Created on: May 10, 2025
 *      Author: wiki
 */


#include "SDW.h"
#include "string.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"



extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern osMutexId_t spiMutexHandle;


/* Util Data ########################################################################################################### */
static int chipSelectPin;
uint16_t tf::currentSysId = SDW_SYS_ID_BASE;
uint16_t node::currentNodeId = SDW_NODE_ID_BASE;
uint16_t macro::currentMacroId = SDW_MACRO_ID_BASE;
uint16_t input::currentInputId = SDW_INPUT_ID_BASE;
uint16_t probe::currentProbeId = SDW_PROBE_ID_BASE;
uint8_t input::currentInputIdx = 0u;
uint8_t probe::currentProbeIdx = 0u;
static uint16_t SequenceNumber;
static bool DataExchangeStarted = false;
static SDW_DesignStatus currentDesignStatus = SDW_DesignStatus::DesignOk;
static SDW_ComStatus currentComStatus = SDW_ComStatus::ComOk;
static ErrorHandlerType ComErrorHandler = NULL;
static ErrorHandlerType DesignErrorHandler = NULL;
static uint8_t SerialData[SDW_CMD_MAX_SIZE];
static uint8_t Response[SDW_RESPONSE_SIZE];
static SDW_CommandInfo Command;
static uint8_t SDW_InputsBuffer[SDW_MAX_INPUTS_NUMBER * SDW_REAL_SIZE];
static uint8_t SDW_ProbesBuffer[SDW_MAX_PROBES_NUMBER * SDW_REAL_SIZE];
static bool SDW_TX_DONE = false ;
static bool SDW_RX_DONE = false ;

/* Util Functions ###################################################################################################### */
static void SendCommand(uint8_t *Cmd, uint16_t Size);
static void GetResponse(uint8_t *Cmd, uint8_t *Response);
static void CalculateAndsetCrc(uint8_t *Data, uint16_t DataLength);
static uint16_t CalculateCrc(uint8_t *Data, uint16_t DataLength);
static void SerializeResponseRequest(uint8_t *SerialData, SDW_CommandInfo *Command);
static void SerializeFullCommand(uint8_t *SerialData, SDW_CommandInfo *Command);
static void SerializeHeader(uint8_t *SerialData, SDW_CommandInfo *Command);
static void HandleCmd(SDW_CommandInfo *Command, uint16_t SerialDataSize);
static void PrepareCmdHeader(SDW_CommandInfo *Command,
        uint16_t SerialDataSize,
        SDW_FuncId FuncId,
        uint8_t NbrOfParams,
        SDW_SubFuncId SubFuncId,
        uint16_t BlockId);
static void PrepareParam(SDW_CommandInfo *Command,
        uint8_t ParamIndex,
        uint8_t SizeOfParam,
        SDW_FuncParamId FuncParamId,
        SDW_FuncParam *FuncParam);
static void LinkBlock2Block(uint16_t SrcId, uint8_t OutIdx, uint16_t DstId, uint8_t InIdx, SDW_SubFuncId SubFuncId);

static void SendCommand(uint8_t *Cmd, uint16_t Size)
{
    if (Cmd != NULL)
    {
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);
        //if (osMutexAcquire(spiMutexHandle, osWaitForever) == osOK)
        //{
        HAL_SPI_Transmit_DMA(&hspi2, (const uint8_t*)Cmd, Size);
        while (SDW_TX_DONE != true);
        SDW_TX_DONE = false ;
        //}
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    }
}

static void GetResponse(uint8_t *Cmd, uint8_t *Response)
{
    if ((Cmd != NULL) && (Response != NULL))
    {
        /* 1. Request Read */
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET); //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);
        //if (osMutexAcquire(spiMutexHandle, osWaitForever) == osOK)
       // {
        HAL_SPI_Transmit_DMA(&hspi2, (const uint8_t*)Cmd, SDW_RESPONSE_REQ_SIZE);
        while (SDW_TX_DONE != true);
        SDW_TX_DONE = false ;
        //}
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

        /* 2. Read Respone Code */
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET);//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);
        //if (osMutexAcquire(spiMutexHandle, osWaitForever) == osOK)
        //{
        HAL_SPI_Receive_DMA(&hspi2, Response, SDW_RESPONSE_SIZE);
        while (SDW_RX_DONE != true);
        SDW_RX_DONE = false ;
        //}
       // HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

        if (SequenceNumber != Response[SDW_RES_SEQ_NUMBER_IDX])
        {
            currentComStatus = WrongSeqNumber;
        }
        else if (CalculateCrc(&Response[0u], SDW_RESPONSE_SIZE - SDW_CMD_CRC_SIZE) !=
                ((uint16_t)(Response[SDW_RES_CRCL_IDX] | (Response[SDW_RES_CRCH_IDX] << 8u))))
        {
            currentComStatus = WrongCrc;
        }
        else
        {
            currentDesignStatus = static_cast<SDW_DesignStatus>(Response[SDW_RES_RESCODE_IDX]);
        }
    }
    else
    {
        currentDesignStatus = SDW_DesignStatus::BadParam;
    }
}

static uint16_t CalculateCrc(uint8_t *Data, uint16_t DataLength)
{
    uint16_t Crc = 0;
    uint16_t ByteIndex;
    uint8_t BitIndex;
    uint8_t CrcLsb = 0;

    for (ByteIndex = 0; ByteIndex < DataLength; ByteIndex++)
    {
        for (BitIndex = 0; BitIndex < 8; BitIndex++)
        {
            CrcLsb = Crc & 0x01;
            Crc >>= 1;

            if (((Data[ByteIndex] & (1u << BitIndex)) >> BitIndex) != CrcLsb)
            {
                Crc = Crc ^ 0x1021;
            }
        }
    }

    return Crc;
}

static void CalculateAndsetCrc(uint8_t *Data, uint16_t DataLength)
{
    uint16_t Crc = 0;
    Crc = CalculateCrc(Data, DataLength);
    Data[DataLength] = (uint8_t)Crc;
    Data[DataLength + 1u] = (uint8_t)(Crc >> 8u);
}

static void SerializeResponseRequest(uint8_t *SerialData, SDW_CommandInfo *Command)
{
    if ((SerialData != NULL) && (Command != NULL))
    {
        SerialData[SDW_CMD_SEQ_NUMBER_IDX] = Command->TransactionInfo.SeqNbr;
        SerialData[SDW_CMD_READ_WRITE_IDX] = SDW_CMD_READ;
        SerialData[SDW_CMD_SIZEL_IDX] = (uint8_t)(SDW_CMD_RESPONSE_ID & 0x00FFu);
        SerialData[SDW_CMD_SIZEH_IDX] = (uint8_t)((SDW_CMD_RESPONSE_ID & 0xFF00u) >> 8u);
        CalculateAndsetCrc(SerialData, SDW_RESPONSE_REQ_SIZE - SDW_CMD_CRC_SIZE);
    }
}

static void SerializeFullCommand(uint8_t *SerialData, SDW_CommandInfo *Command)
{
    if ((SerialData != NULL) && (Command != NULL))
    {
        uint32_t currentCmdInfoIndex = SDW_CMD_START_OF_PARAMS_IDX;

        SerialData[SDW_CMD_SEQ_NUMBER_IDX] = Command->TransactionInfo.SeqNbr;
        SerialData[SDW_CMD_READ_WRITE_IDX] = SDW_CMD_WRITE;
        SerialData[SDW_CMD_SIZEL_IDX] = (uint8_t)(SDW_CMD_FULL_CMD_ID & 0x00FFu);
        SerialData[SDW_CMD_SIZEH_IDX] = (uint8_t)((SDW_CMD_FULL_CMD_ID & 0xFF00u) >> 8u);
        SerialData[SDW_CMD_BLOCKIDL_IDX] = (uint8_t)(Command->FuncInfo.BlockId & 0x00FFu);
        SerialData[SDW_CMD_BLOCKIDH_IDX] = (uint8_t)((Command->FuncInfo.BlockId & 0xFF00u) >> 8u);
        SerialData[SDW_CMD_FUNC_ID_IDX] = (uint8_t)(Command->FuncInfo.FuncId);
        SerialData[SDW_CMD_SUB_FUNC_IDL_IDX] = (uint8_t)(Command->FuncInfo.SubFuncId & 0x00FFu);
        SerialData[SDW_CMD_SUB_FUNC_IDH_IDX] = (uint8_t)((Command->FuncInfo.SubFuncId & 0xFF00u) >> 8u);
        SerialData[SDW_CMD_NBR_OF_PARAMS_IDX] = Command->FuncInfo.NbrOfParams;

        for (uint8_t index = 0u; index < Command->FuncInfo.NbrOfParams; index++)
        {
            SDW_FuncParamId TmpFuncParamId = Command->FuncInfo.FuncParamIds[index];
            uint8_t TmpFuncParamSize = Command->FuncInfo.FuncParamSizes[index];
            SerialData[currentCmdInfoIndex] = (uint8_t)TmpFuncParamId;
            SerialData[currentCmdInfoIndex + 1u] = TmpFuncParamSize;
            currentCmdInfoIndex += 2u;

            switch (TmpFuncParamId)
            {
                case SDW_FuncParamId::TYPE_String:
                case SDW_FuncParamId::TYPE_RealArray:
                case SDW_FuncParamId::TYPE_Uint32Array:
                case SDW_FuncParamId::TYPE_Uint16Array:
                case SDW_FuncParamId::TYPE_Uint8Array:
                {
                    void *tmp;
                    memcpy(&tmp, (const void *)&Command->FuncInfo.FuncParams[index], sizeof(int));
                    memcpy((void *)&SerialData[currentCmdInfoIndex],
                            (const void *)tmp,
                            TmpFuncParamSize);
                    currentCmdInfoIndex += TmpFuncParamSize;
                }
                break;
                case SDW_FuncParamId::TYPE_Real:
                case SDW_FuncParamId::TYPE_Uint32:
                case SDW_FuncParamId::TYPE_Uint16:
                case SDW_FuncParamId::TYPE_Uint8:
                {
                    memcpy((void *)&SerialData[currentCmdInfoIndex],
                            (const void *)&Command->FuncInfo.FuncParams[index],
                            TmpFuncParamSize);
                    currentCmdInfoIndex += TmpFuncParamSize;
                }
                break;
                default:
                {
                    // Handle unknown type
                }
                break;
            }
        }

        CalculateAndsetCrc(SerialData, currentCmdInfoIndex);
    }
}

static void SerializeHeader(uint8_t *SerialData, SDW_CommandInfo *Command)
{
    if ((SerialData != NULL) && (Command != NULL))
    {
        SerialData[SDW_CMD_SEQ_NUMBER_IDX] = Command->TransactionInfo.SeqNbr;
        SerialData[SDW_CMD_READ_WRITE_IDX] = SDW_CMD_WRITE;
        SerialData[SDW_CMD_SIZEL_IDX] = (uint8_t)(Command->FuncInfo.DataSize & 0x00FFu);
        SerialData[SDW_CMD_SIZEH_IDX] = (uint8_t)((Command->FuncInfo.DataSize & 0xFF00u) >> 8u);
        CalculateAndsetCrc(SerialData, SDW_CMD_HEADER_SIZE - SDW_CMD_CRC_SIZE);
    }
}

static void HandleCmd(SDW_CommandInfo *Command, uint16_t SerialDataSize)
{
    if (SerialDataSize <= SDW_CMD_MAX_SIZE)
    {
        if ((SDW_ComStatus::ComOk == currentComStatus) &&
                (SDW_DesignStatus::DesignOk == currentDesignStatus))
        {
            /* Send a Write Request Command With the Header */
            SerializeHeader(&SerialData[0u], Command);
            SendCommand(&SerialData[0u], SDW_CMD_HEADER_SIZE);

            /* Send a Write Request Command With the full Command */
            SerializeFullCommand(&SerialData[0u], Command);
            SendCommand(&SerialData[0u], SerialDataSize);
            HAL_Delay(50); // Check if needed

            /* Send a Read Request and get Response Code */
            SerializeResponseRequest(&SerialData[0u], Command);
            GetResponse(SerialData, Response);

            bool com_error = false;
            bool design_error = false;

            if (currentComStatus != SDW_ComStatus::ComOk)
            {
                com_error = true;
            }

            if (currentDesignStatus != SDW_DesignStatus::DesignOk)
            {
                design_error = true;
            }

            if ((true == com_error) && (ComErrorHandler != NULL))
            {
                ComErrorHandler();
            }

            if ((true == design_error) && (DesignErrorHandler != NULL))
            {
                DesignErrorHandler();
            }

            if ((false == design_error) && (false == com_error))
            {
                SequenceNumber++;
                SequenceNumber %= 256u;
            }
        }
    }
    else
    {
        currentDesignStatus = SDW_DesignStatus::BadParam;
    }
}

static void PrepareCmdHeader(SDW_CommandInfo *Command,
        uint16_t SerialDataSize,
        SDW_FuncId FuncId,
        uint8_t NbrOfParams,
        SDW_SubFuncId SubFuncId,
        uint16_t BlockId)
{
    if (Command != NULL)
    {
        Command->TransactionInfo.SeqNbr = (uint8_t)SequenceNumber;
        Command->FuncInfo.DataSize = SerialDataSize;
        Command->FuncInfo.BlockId = BlockId;
        Command->FuncInfo.FuncId = FuncId;
        Command->FuncInfo.SubFuncId = SubFuncId;
        Command->FuncInfo.NbrOfParams = NbrOfParams;
    }
}

static void PrepareParam(SDW_CommandInfo *Command,
        uint8_t ParamIndex,
        uint8_t SizeOfParam,
        SDW_FuncParamId FuncParamId,
        SDW_FuncParam *FuncParam)
{
    if ((Command != NULL) && (FuncParam != NULL))
    {
        Command->FuncInfo.FuncParamIds[ParamIndex] = FuncParamId;
        Command->FuncInfo.FuncParamSizes[ParamIndex] = SizeOfParam;
        memcpy((void *)&Command->FuncInfo.FuncParams[ParamIndex], (const void *)FuncParam, sizeof(SDW_FuncParam));
    }
}

static void LinkBlock2Block(uint16_t SrcId, uint8_t OutIdx, uint16_t DstId, uint8_t InIdx, SDW_SubFuncId SubFuncId)
{
    uint8_t NbrOfParams = 0xff;
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            2u * (SDW_PARAM_INFO_SIZE + SDW_BLOCKID_SIZE);
    switch (SubFuncId)
    {
        case SDW_EDL_LinkSys2Sys:
        case SDW_EDL_LinkSys2Probe:
        case SDW_EDL_LinkIn2Sys:
        {
            NbrOfParams = 2u;

            /* Prepare Param_1 */
            FuncParam.ParamBlockId(SrcId);
            PrepareParam(&Command, 0u, SDW_BLOCKID_SIZE, SDW_BLOCK_ID_TYPE, &FuncParam);

            /* Prepare Param_2 */
            FuncParam.ParamBlockId(DstId);
            PrepareParam(&Command, 1u, SDW_BLOCKID_SIZE, SDW_BLOCK_ID_TYPE, &FuncParam);
        }
        break;
        case SDW_EDL_LinkSys2Node:
        case SDW_EDL_LinkSys2Macro:
        case SDW_EDL_LinkIn2Node:
        case SDW_EDL_LinkIn2Macro:
        {
            NbrOfParams = 3u;
            SerialDataSize += SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

            /* Prepare Param_1 */
            FuncParam.ParamBlockId(SrcId);
            PrepareParam(&Command, 0u, SDW_BLOCKID_SIZE, SDW_BLOCK_ID_TYPE, &FuncParam);

            /* Prepare Param_2 */
            FuncParam.ParamBlockId(DstId);
            PrepareParam(&Command, 1u, SDW_BLOCKID_SIZE, SDW_BLOCK_ID_TYPE, &FuncParam);

            /* Prepare Param_3 */
            FuncParam.ParamUint8 = InIdx;
            PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);
        }
        break;
        case SDW_EDL_LinkNode2Sys:
        case SDW_EDL_LinkNode2Probe:
        case SDW_EDL_LinkMacro2Sys:
        case SDW_EDL_LinkMacro2Probe:
        {
            NbrOfParams = 3u;
            SerialDataSize += SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

            /* Prepare Param_1 */
            FuncParam.ParamBlockId(SrcId);
            PrepareParam(&Command, 0u, SDW_BLOCKID_SIZE, SDW_BLOCK_ID_TYPE, &FuncParam);

            /* Prepare Param_2 */
            FuncParam.ParamUint8 = OutIdx;
            PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

            /* Prepare Param_3 */
            FuncParam.ParamBlockId(DstId);
            PrepareParam(&Command, 2u, SDW_BLOCKID_SIZE, SDW_BLOCK_ID_TYPE, &FuncParam);
        }
        break;
        case SDW_EDL_LinkNode2Node:
        case SDW_EDL_LinkNode2Macro:
        case SDW_EDL_LinkMacro2Node:
        case SDW_EDL_LinkMacro2Macro:
        {
            NbrOfParams = 4u;
            SerialDataSize += 2u * (SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE);

            /* Prepare Param_1 */
            FuncParam.ParamBlockId(SrcId);
            PrepareParam(&Command, 0u, SDW_BLOCKID_SIZE, SDW_BLOCK_ID_TYPE, &FuncParam);

            /* Prepare Param_2 */
            FuncParam.ParamUint8 = OutIdx;
            PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

            /* Prepare Param_3 */
            FuncParam.ParamBlockId(DstId);
            PrepareParam(&Command, 2u, SDW_BLOCKID_SIZE, SDW_BLOCK_ID_TYPE, &FuncParam);

            /* Prepare Param_4 */
            FuncParam.ParamUint8 = InIdx;
            PrepareParam(&Command, 3u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);
        }
        break;
        default:
        {
        }
        break;
    }

    PrepareCmdHeader(&Command, SerialDataSize, SDW_LinkBlock, NbrOfParams, SubFuncId, 0xFFFF);

    /* Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

/* Generic Functions ##################################################################################################### */
SDW::SDW(){};

SDW::SDW(int cs)
{
    /* Workaround  */
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET); // HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET);
    /* Workaround  */

    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

    /* Workaround  */
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET); // HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_Delay(10);
    /* Workaround  */
}

SDW::SDW(int cs, int bitrate)
{
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

SDW::SDW(int cs, int mosi, int miso, int clk)
{
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

SDW::SDW(int cs, int mosi, int miso, int clk, int bitrate)
{
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

SDW_DesignStatus SDW::GetcurrentDesignStatus()
{
    return currentDesignStatus;
}

SDW_ComStatus SDW::GetcurrentComStatus(void)
{
    return currentComStatus;
}

void SDW::setComErrorHandler(ErrorHandlerType UserHandler)
{
    ComErrorHandler = UserHandler;
}

void SDW::setDesignErrorHandler(ErrorHandlerType UserHandler)
{
    DesignErrorHandler = UserHandler;
}

void SDW::startDesign(float SamplingFreq, SDW_C2dAlgo Algo)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_C2D_ALGO_SIZE;

    /* 0. Set current Chip Select Pin */
    chipSelectPin = this->cs;

    /* 1. Prepare Configuration : Header + Function Info */
    PrepareCmdHeader(&Command, SerialDataSize, SDW_ManageBlock, 2u, SDW_EDL_DesignInit, 0xFFFF);

    /* 2. Prepare Param_1 */
    FuncParam.ParamReal = SamplingFreq;
    PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamC2dAlgo(Algo);
    PrepareParam(&Command, 1u, SDW_C2D_ALGO_SIZE, SDW_C2D_ALGO_TYPE, &FuncParam);

    /* 4. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

void SDW::stopDesign(void)
{
    uint16_t SerialDataSize = SDW_FIXED_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    PrepareCmdHeader(&Command, SerialDataSize, SDW_ManageBlock, 0u, SDW_EDL_StopDesign, 0xFFFF);

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

void SDW::startSampling(void)
{
    uint16_t SerialDataSize = SDW_FIXED_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    PrepareCmdHeader(&Command, SerialDataSize, SDW_ManageBlock, 0u, SDW_EDL_StartSampling, 0xFFFF);

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);

    if (false == DataExchangeStarted)
    {
        DataExchangeStarted = true;
    }
}

void SDW::stopSampling(void)
{
    uint16_t SerialDataSize = SDW_FIXED_SIZE;

    if (true == DataExchangeStarted)
    {
        /* Request Switch Back To CFG Mode */
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_Delay(1);
        uint8_t tmp = SDW_CMD_SWITCH_BACK_TO_CFG;
        //if (osMutexAcquire(spiMutexHandle, osWaitForever) == osOK)
        //{
        HAL_SPI_Transmit_DMA(&hspi2, (const uint8_t*)&tmp, 1);
        while (SDW_TX_DONE != true);
        SDW_TX_DONE = false ;
        //}
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        DataExchangeStarted = false;
    }

    /* 1. Prepare Configuration : Header + Function Info */
    PrepareCmdHeader(&Command, SerialDataSize, SDW_ManageBlock, 0u, SDW_EDL_StopSampling, 0xFFFF);

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

void SDW::setMonitoringInterface(SDW_MonInterface InterfaceId)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_MON_INTERFACE_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    PrepareCmdHeader(&Command, SerialDataSize, SDW_ManageBlock, 1u, SDW_SetMonInterface, 0xFFFF);

    /* 2. Prepare Param_1 */
    FuncParam.ParamMonInterface(InterfaceId);
    PrepareParam(&Command, 0u, SDW_MON_INTERFACE_SIZE, SDW_MON_INTERFACE_TYPE, &FuncParam);

    /* 4. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

void SDW::setInputs(void)
{
    if ((input::currentInputIdx > 0u) && (input::currentInputIdx < SDW_MAX_INPUTS_NUMBER))
    {
        /* Request Write */
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_Delay(1);
        uint8_t tmp = SDW_CMD_WRITE;
       // if (osMutexAcquire(spiMutexHandle, osWaitForever) == osOK)
        //{
        HAL_SPI_Transmit_DMA(&hspi2, (const uint8_t*)&tmp, 1);
        while (SDW_TX_DONE != true);
        SDW_TX_DONE = false ;
        //}
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

        /* Send Inputs */
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_Delay(1);
      //  if (osMutexAcquire(spiMutexHandle, osWaitForever) == osOK)
       // {
        HAL_SPI_Transmit_DMA(&hspi2, (const uint8_t*)&SDW_InputsBuffer[0u], input::currentInputIdx * SDW_REAL_SIZE);
        while (SDW_TX_DONE != true);
        SDW_TX_DONE = false ;
        //}
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); //  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    }
}

void SDW::getProbes(void)
{
    if ((probe::currentProbeIdx > 0u) && (probe::currentProbeIdx < SDW_MAX_PROBES_NUMBER))
    {
        /* Request Read */
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
        //HAL_Delay(1);
        for ( volatile int i = 0; i < 100; i++ );
        uint8_t tmp = SDW_CMD_READ;
      //  if (osMutexAcquire(spiMutexHandle, osWaitForever) == osOK)
        //{
        HAL_SPI_Transmit_DMA(&hspi2, (const uint8_t*)&tmp, 1);
        while (SDW_TX_DONE != true);
        SDW_TX_DONE = false ;
        //}
        //HAL_Delay(1);
        for ( volatile int i = 0; i < 100; i++ );
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);  // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

        /* Get Probes */
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
        //HAL_Delay(1);
        for ( volatile int i = 0; i < 100; i++ );
     //   if (osMutexAcquire(spiMutexHandle, osWaitForever) == osOK)
       // {
        HAL_SPI_Receive_DMA(&hspi2, &SDW_ProbesBuffer[0], probe::currentProbeIdx * SDW_REAL_SIZE);
        while (SDW_RX_DONE != true);
        SDW_RX_DONE = false ;
        //}
        //HAL_Delay(1);
        for ( volatile int i = 0; i < 100; i++ );
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET); // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    }
}

uint8_t SDW::getSequenceNumber(void)
{
    return SequenceNumber;
}

SDW_SubFuncId getButterSubFuncId(SDW_FilterType ftype)
{
    SDW_SubFuncId retval = NoSubFuncId;

    switch (ftype)
    {
        case passLow:
        {
            retval = SDW_EDL_NewButterLow;
        }
        break;
        case passHigh:
        {
            retval = SDW_EDL_NewButterHigh;
        }
        break;
        case passBand:
        {
            retval = SDW_EDL_NewButterBand;
        }
        break;
        case stopBand:
        {
            retval = SDW_EDL_NewButterStop;
        }
        break;
        default:
        {
        }
        break;
    };

    return retval;
}

SDW_SubFuncId getButterBqSubFuncId(SDW_FilterType ftype)
{
    SDW_SubFuncId retval = NoSubFuncId;

    switch (ftype)
    {
        case passLow:
        {
            retval = SDW_EDL_NewBiquadButterLow;
        }
        break;
        case passHigh:
        {
            retval = SDW_EDL_NewBiquadButterHigh;
        }
        break;
        case passBand:
        {
            retval = SDW_EDL_NewBiquadButterBand;
        }
        break;
        case stopBand:
        {
            retval = SDW_EDL_NewBiquadButterStop;
        }
        break;
        default:
        {
        }
        break;
    };

    return retval;
}

SDW_SubFuncId getCheby1SubFuncId(SDW_FilterType ftype)
{
    SDW_SubFuncId retval = NoSubFuncId;

    switch (ftype)
    {
        case passLow:
        {
            retval = SDW_EDL_NewChebyshev1Low;
        }
        break;
        case passHigh:
        {
            retval = SDW_EDL_NewChebyshev1High;
        }
        break;
        case passBand:
        {
            retval = SDW_EDL_NewChebyshev1Band;
        }
        break;
        case stopBand:
        {
            retval = SDW_EDL_NewChebyshev1Stop;
        }
        break;
        default:
        {
        }
        break;
    };

    return retval;
}

SDW_SubFuncId getCheby1BqSubFuncId(SDW_FilterType ftype)
{
    SDW_SubFuncId retval = NoSubFuncId;

    switch (ftype)
    {
        case passLow:
        {
            retval = SDW_EDL_NewBiquadCheby1Low;
        }
        break;
        case passHigh:
        {
            retval = SDW_EDL_NewBiquadCheby1High;
        }
        break;
        case passBand:
        {
            retval = SDW_EDL_NewBiquadCheby1Band;
        }
        break;
        case stopBand:
        {
            retval = SDW_EDL_NewBiquadCheby1Stop;
        }
        break;
        default:
        {
        }
        break;
    };

    return retval;
}

SDW_SubFuncId getCheby2SubFuncId(SDW_FilterType ftype)
{
    SDW_SubFuncId retval = NoSubFuncId;

    switch (ftype)
    {
        case passLow:
        {
            retval = SDW_EDL_NewChebyshev2Low;
        }
        break;
        case passHigh:
        {
            retval = SDW_EDL_NewChebyshev2High;
        }
        break;
        case passBand:
        {
            retval = SDW_EDL_NewChebyshev2Band;
        }
        break;
        case stopBand:
        {
            retval = SDW_EDL_NewChebyshev2Stop;
        }
        break;
        default:
        {
        }
        break;
    };

    return retval;
}

SDW_SubFuncId getCheby2BqSubFuncId(SDW_FilterType ftype)
{
    SDW_SubFuncId retval = NoSubFuncId;

    switch (ftype)
    {
        case passLow:
        {
            retval = SDW_EDL_NewBiquadCheby2Low;
        }
        break;
        case passHigh:
        {
            retval = SDW_EDL_NewBiquadCheby2High;
        }
        break;
        case passBand:
        {
            retval = SDW_EDL_NewBiquadCheby2Band;
        }
        break;
        case stopBand:
        {
            retval = SDW_EDL_NewBiquadCheby2Stop;
        }
        break;
        default:
        {
        }
        break;
    };

    return retval;
}

SDW_SubFuncId getEllipSubFuncId(SDW_FilterType ftype)
{
    SDW_SubFuncId retval = NoSubFuncId;

    switch (ftype)
    {
        case passLow:
        {
            retval = SDW_EDL_NewEllipLow;
        }
        break;
        case passHigh:
        {
            retval = SDW_EDL_NewEllipHigh;
        }
        break;
        case passBand:
        {
            retval = SDW_EDL_NewEllipBand;
        }
        break;
        case stopBand:
        {
            retval = SDW_EDL_NewEllipStop;
        }
        break;
        default:
        {
        }
        break;
    };

    return retval;
}

SDW_SubFuncId getEllipBqSubFuncId(SDW_FilterType ftype)
{
    SDW_SubFuncId retval = NoSubFuncId;

    switch (ftype)
    {
        case passLow:
        {
            retval = SDW_EDL_NewBiquadEllipLow;
        }
        break;
        case passHigh:
        {
            retval = SDW_EDL_NewBiquadEllipHigh;
        }
        break;
        case passBand:
        {
            retval = SDW_EDL_NewBiquadEllipBand;
        }
        break;
        case stopBand:
        {
            retval = SDW_EDL_NewBiquadEllipStop;
        }
        break;
        default:
        {
        }
        break;
    };

    return retval;
}

/* System Class ########################################################################################################## */
tf::tf(){};

uint16_t tf::getId()
{
    return this->id;
}

void tf::setId(uint16_t id)
{
    this->id = id;
}

void tf::operator>(tf Dst) // Sys2Sys
{
    LinkBlock2Block(this->id, 0xFF, Dst.getId(), 0xFF, SDW_EDL_LinkSys2Sys);
}

void tf::operator>(probe &Dst) // Sys2Probe
{
    LinkBlock2Block(this->id, 0xFF, Dst.getId(), 0xFF, SDW_EDL_LinkSys2Probe);
    Dst.addToQueue();
}

void tf::operator>(node Dst) // Sys2Node
{
    LinkBlock2Block(this->id, 0xFF, Dst.getId(), Dst.getActiveIoIdx(), SDW_EDL_LinkSys2Node);
}

void tf::operator>(macro Dst) // Sys2Macro
{
    LinkBlock2Block(this->id, 0xFF, Dst.getId(), Dst.getActiveIoIdx(), SDW_EDL_LinkSys2Macro);
}

tf::tf(const char *numerator, const char *denominator)
{
    SDW_FuncParam FuncParam;
    uint8_t param1_size = strlen(numerator) + 1u;
    uint8_t param2_size = strlen(denominator) + 1u;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + param1_size +
            SDW_PARAM_INFO_SIZE + param2_size;

    /* 1. Prepare Configuration : Header + Function Info */
    this->id = currentSysId++;
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 2u, SDW_EDL_NewSys, this->id);

    /* 2. Prepare Param_1 */
    FuncParam.ParamString = numerator;
    PrepareParam(&Command, 0u, param1_size, TYPE_String, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamString = denominator;
    PrepareParam(&Command, 1u, param2_size, TYPE_String, &FuncParam);

    /* 4. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

tf::tf(float *numerator, uint8_t numSize, float *denominator, uint8_t denomSize)
{
    SDW_FuncParam FuncParam;
    uint8_t param1_size = numSize * SDW_REAL_SIZE;
    uint8_t param3_size = denomSize * SDW_REAL_SIZE;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + param1_size +
            SDW_PARAM_INFO_SIZE + param3_size +
            2u * (SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE);

    /* 1. Prepare Configuration : Header + Function Info */
    this->id = currentSysId++;
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 4u, SDW_EDL_NewSysLightC, this->id);

    /* 2. Prepare Param_1 */
    FuncParam.ParamRealArray = numerator;
    PrepareParam(&Command, 0u, param1_size, TYPE_RealArray, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamUint8 = numSize;
    PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

    /* 4. Prepare Param_3 */
    FuncParam.ParamRealArray = denominator;
    PrepareParam(&Command, 2u, param3_size, TYPE_RealArray, &FuncParam);

    /* 5. Prepare Param_4 */
    FuncParam.ParamUint8 = denomSize;
    PrepareParam(&Command, 3u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

    /* 6. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

tfd::tfd(float *numerator, float *denominator, uint8_t sysord)
{
    SDW_CommandInfo Command;
    SDW_FuncParam FuncParam;
    uint8_t param1_size = (sysord + 1) * SDW_REAL_SIZE;
    uint8_t param2_size = (sysord + 1) * SDW_REAL_SIZE;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + param1_size +
            SDW_PARAM_INFO_SIZE + param2_size +
            SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */

    this->setId(currentSysId++);
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SDW_EDL_NewSysLightD, this->getId());

    /* 2. Prepare Param_1 */
    FuncParam.ParamRealArray = numerator;
    PrepareParam(&Command, 0u, param1_size, TYPE_RealArray, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamRealArray = denominator;
    PrepareParam(&Command, 1u, param2_size, TYPE_RealArray, &FuncParam);

    /* 4. Prepare Param_3 */
    FuncParam.ParamUint8 = sysord;
    PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

    /* 5. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

wave::wave(SDW_WaveForm waveform, float freq, float offset, float phaseshift)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            3u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE);

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentSysId++);
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, (SDW_SubFuncId)waveform, this->getId());

    /* 2. Prepare Param_1 */
    FuncParam.ParamReal = freq;
    PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamReal = offset;
    PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 4. Prepare Param_3 */
    FuncParam.ParamReal = phaseshift;
    PrepareParam(&Command, 2u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 6. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

pid::pid(float ki, float kp, float kd, float tau)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            4u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE);

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentSysId++);
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 4u, (SDW_SubFuncId)SDW_EDL_NewPid, this->getId());

    /* 2. Prepare Param_1 */
    FuncParam.ParamReal = ki;
    PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamReal = kp;
    PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 4. Prepare Param_3 */
    FuncParam.ParamReal = kd;
    PrepareParam(&Command, 2u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 5. Prepare Param_4 */
    FuncParam.ParamReal = tau;
    PrepareParam(&Command, 3u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 6. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

/* Node Class ############################################################################################################ */
node::node(){};
node::node(const char *InCoef, const char *OutCoef, SDW_NodeType Type)
{
    SDW_FuncParam FuncParam;
    uint8_t param1_size = strlen(InCoef) + 1u;
    uint8_t param2_size = strlen(OutCoef) + 1u;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + param1_size +
            SDW_PARAM_INFO_SIZE + param2_size +
            SDW_PARAM_INFO_SIZE + SDW_NODE_TYPE_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    this->id = currentNodeId++;
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SDW_EDL_NewNode, this->id);

    /* 2. Prepare Param_1 */
    FuncParam.ParamString = InCoef;
    PrepareParam(&Command, 0u, param1_size, TYPE_String, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamString = OutCoef;
    PrepareParam(&Command, 1u, param2_size, TYPE_String, &FuncParam);

    /* 4. Prepare Param_3 */
    FuncParam.ParamNodeType(Type);
    PrepareParam(&Command, 2u, SDW_NODE_TYPE_SIZE, SDW_NODE_TYPE_TYPE, &FuncParam);

    /* 5. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

plant::plant(SDW_ActuatorSpec *Actuators,
        uint8_t NbrOfActuators,
        SDW_SensorSpec *Sensors,
        uint8_t NbrOfSensors)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            (3u * (SDW_REAL_SIZE + SDW_PARAM_INFO_SIZE) + (SDW_UINT16_SIZE + SDW_PARAM_INFO_SIZE)) * NbrOfActuators +
            (2u * (SDW_REAL_SIZE + SDW_PARAM_INFO_SIZE) + (SDW_UINT16_SIZE + SDW_PARAM_INFO_SIZE)) * NbrOfSensors +
            SDW_UINT8_SIZE + SDW_PARAM_INFO_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentNodeId++);
    PrepareCmdHeader(&Command,
            SerialDataSize,
            SDW_CreateBlock,
            4u * NbrOfActuators + 3u * NbrOfSensors + 1u,
            SDW_NewHwBlock,
            this->getId());

    /* 2. Prepare Param_1 */
    FuncParam.ParamUint8 = (NbrOfActuators & 0x0Fu) | ((NbrOfSensors << 4u) & 0xF0u);
    PrepareParam(&Command, 0u, 1u, TYPE_Uint8, &FuncParam);

    uint8_t Offset = 1u;

    for (uint8_t index = 0u; index < NbrOfActuators; index++)
    {
        /* HW ID */
        FuncParam.ParamHwRoutineId(Actuators[index].HwId);
        PrepareParam(&Command, 4u * index + Offset, SDW_HW_ROUTINE_ID_SIZE, SDW_HW_ROUTINE_ID_TYPE, &FuncParam);

        /* ConvConst1 */
        FuncParam.ParamReal = Actuators[index].ConvConst1;
        PrepareParam(&Command, 4u * index + Offset + 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* Lower Limit */
        FuncParam.ParamReal = Actuators[index].LowerLimit;
        PrepareParam(&Command, 4u * index + Offset + 2u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* Upper Limit */
        FuncParam.ParamReal = Actuators[index].UpperLimit;
        PrepareParam(&Command, 4u * index + Offset + 3u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);
    }

    Offset += NbrOfActuators * 4u;

    for (uint8_t index = 0u; index < NbrOfSensors; index++)
    {
        /* HW ID */
        FuncParam.ParamHwRoutineId(Sensors[index].HwId);
        PrepareParam(&Command, 3u * index + Offset, SDW_HW_ROUTINE_ID_SIZE, SDW_HW_ROUTINE_ID_TYPE, &FuncParam);

        /* ConvConst1 */
        FuncParam.ParamReal = Sensors[index].ConvConst1;
        PrepareParam(&Command, 3u * index + Offset + 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* ConvConst2 */
        FuncParam.ParamReal = Sensors[index].ConvConst2;
        PrepareParam(&Command, 3u * index + Offset + 2u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);
    }

    /* 5. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

actuator::actuator(SDW_ActuatorSpec *Actuator)
{
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            3u * (SDW_REAL_SIZE + SDW_PARAM_INFO_SIZE) +
            SDW_UINT16_SIZE + SDW_PARAM_INFO_SIZE;

    SDW_FuncParam FuncParam;

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentNodeId++);
    PrepareCmdHeader(&Command,
            SerialDataSize,
            SDW_CreateBlock,
            4u,
            SDW_EDL_NewHwOutput,
            this->getId());

    /* HW ID */
    FuncParam.ParamHwRoutineId(Actuator->HwId);
    PrepareParam(&Command, 0u, SDW_HW_ROUTINE_ID_SIZE, SDW_HW_ROUTINE_ID_TYPE, &FuncParam);

    /* ConvConst1 */
    FuncParam.ParamReal = Actuator->ConvConst1;
    PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* Lower Limit */
    FuncParam.ParamReal = Actuator->LowerLimit;
    PrepareParam(&Command, 2u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* Upper Limit */
    FuncParam.ParamReal = Actuator->UpperLimit;
    PrepareParam(&Command, 3u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 5. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

sensor::sensor(SDW_SensorSpec *Sensor)
{
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            2u * (SDW_REAL_SIZE + SDW_PARAM_INFO_SIZE) +
            SDW_UINT16_SIZE + SDW_PARAM_INFO_SIZE;
    SDW_FuncParam FuncParam;

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentNodeId++);
    PrepareCmdHeader(&Command,
            SerialDataSize,
            SDW_CreateBlock,
            3u,
            SDW_EDL_NewHwInput,
            this->getId());

    /* HW ID */
    FuncParam.ParamUint16 = (uint16_t)Sensor->HwId;
    PrepareParam(&Command, 0u, SDW_HW_ROUTINE_ID_SIZE, TYPE_Uint16, &FuncParam);

    /* ConvConst1 */
    FuncParam.ParamReal = Sensor->ConvConst1;
    PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* ConvConst2 */
    FuncParam.ParamReal = Sensor->ConvConst2;
    PrepareParam(&Command, 2u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 5. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

schmidt::schmidt()
{
    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentNodeId++);
    PrepareCmdHeader(&Command,
            SDW_FIXED_SIZE,
            SDW_CreateBlock,
            0,
            SDW_EDL_NewSchmidt,
            this->getId());

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SDW_FIXED_SIZE);
}

comp::comp()
{
    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentNodeId++);
    PrepareCmdHeader(&Command,
            SDW_FIXED_SIZE,
            SDW_CreateBlock,
            0,
            SDW_EDL_NewComp,
            this->getId());

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SDW_FIXED_SIZE);
}

sat::sat()
{
    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentNodeId++);
    PrepareCmdHeader(&Command,
            SDW_FIXED_SIZE,
            SDW_CreateBlock,
            0,
            SDW_EDL_NewSat,
            this->getId());

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SDW_FIXED_SIZE);
}

deadZone::deadZone()
{
    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentNodeId++);
    PrepareCmdHeader(&Command,
            SDW_FIXED_SIZE,
            SDW_CreateBlock,
            0,
            SDW_EDL_NewDeadZone,
            this->getId());

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SDW_FIXED_SIZE);
}

backlashDeadBand::backlashDeadBand()
{
    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentNodeId++);
    PrepareCmdHeader(&Command,
            SDW_FIXED_SIZE,
            SDW_CreateBlock,
            0,
            SDW_EDL_NewBacklashDeadBand,
            this->getId());

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SDW_FIXED_SIZE);
}

bridge::bridge(uint8_t nbrOdOutputs) //TODO
{

}


sign :: sign (void)
{
    /* 1. Prepare Configuration : Header + Function Info */
  this->setId(currentNodeId++);
  PrepareCmdHeader(&Command,
                   SDW_FIXED_SIZE,
                   SDW_CreateBlock,
                   0,
                   SDW_EDL_NewSign,
                   this->getId());

  /* 2. Send Configuration Request */
  HandleCmd(&Command, SDW_FIXED_SIZE);
}

node node::in(uint8_t InIdx)
{
    this->activeIoIdx = InIdx;
    return *this;
}

node node::out(uint8_t OutIdx)
{
    this->activeIoIdx = OutIdx;
    return *this;
}

uint16_t node::getId()
{
    return this->id;
}

void node::setId(uint16_t id)
{
    this->id = id;
}

uint8_t node::getActiveIoIdx()
{
    return this->activeIoIdx;
}

void node::operator>(tf Dst) // Node2Sys
{
    LinkBlock2Block(this->id, this->activeIoIdx, Dst.getId(), 0xFF, SDW_EDL_LinkNode2Sys);
}

void node::operator>(probe &Dst) // Node2Probe
{
    LinkBlock2Block(this->id, this->activeIoIdx, Dst.getId(), 0xFF, SDW_EDL_LinkNode2Probe);
    Dst.addToQueue();
}

void node::operator>(node Dst) // Node2Node
{
    LinkBlock2Block(this->id, this->activeIoIdx, Dst.getId(), Dst.getActiveIoIdx(), SDW_EDL_LinkNode2Node);
}

void node::operator>(macro Dst) // Node2Macro
{
    LinkBlock2Block(this->id, this->activeIoIdx, Dst.getId(), Dst.getActiveIoIdx(), SDW_EDL_LinkNode2Macro);
}

/* Macro Class ########################################################################################################### */
macro::macro(){};

uint16_t macro::getId()
{
    return this->id;
}

void macro::setId(uint16_t id)
{
    this->id = id;
}

macro macro::in(uint8_t InIdx)
{
    this->activeIoIdx = InIdx;
    return *this;
}

macro macro::out(uint8_t OutIdx)
{
    this->activeIoIdx = OutIdx;
    return *this;
}

void macro::operator>(tf Dst) // Macro2Sys
{
    LinkBlock2Block(this->id, this->activeIoIdx, Dst.getId(), 0xFF, SDW_EDL_LinkMacro2Sys);
}

void macro::operator>(probe &Dst) // Macro2Probe
{
    LinkBlock2Block(this->id, this->activeIoIdx, Dst.getId(), 0xFF, SDW_EDL_LinkMacro2Probe);
    Dst.addToQueue();
}

void macro::operator>(node Dst) // Macro2Node
{
    LinkBlock2Block(this->id, this->activeIoIdx, Dst.getId(), Dst.getActiveIoIdx(), SDW_EDL_LinkMacro2Node);
}

void macro::operator>(macro Dst) // Macro2Macro
{
    LinkBlock2Block(this->id, this->activeIoIdx, Dst.getId(), Dst.getActiveIoIdx(), SDW_EDL_LinkMacro2Macro);
}

uint8_t macro::getActiveIoIdx()
{
    return this->activeIoIdx;
}

void macro::start()
{
    /* 1. Prepare Configuration : Header + Function Info */
    PrepareCmdHeader(&Command, SDW_FIXED_SIZE, SDW_ManageBlock, 0u, SDW_EDL_StartMacro, this->id);

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SDW_FIXED_SIZE);
}

void macro::stop()
{
    /* 1. Prepare Configuration : Header + Function Info */
    PrepareCmdHeader(&Command, SDW_FIXED_SIZE, SDW_ManageBlock, 0u, SDW_EDL_StopMacro, this->id);

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SDW_FIXED_SIZE);
}

macro::macro(uint8_t NbrOfInputs, uint8_t NbrOfOutputs)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    this->id = currentMacroId++;
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 2u, SDW_EDL_NewMacro, this->id);

    /* 2. Prepare Param_1 */
    FuncParam.ParamUint8 = NbrOfInputs;
    PrepareParam(&Command, 0u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamUint8 = NbrOfOutputs;
    PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

    /* 4. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

butterBq::butterBq(uint8_t n, float fc, SDW_FilterType ftype)
{
    SDW_SubFuncId SubFuncId = getButterBqSubFuncId(ftype);

    if ((SDW_EDL_NewBiquadButterLow == SubFuncId) || (SDW_EDL_NewBiquadButterHigh == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentMacroId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 2u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 4. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

butter::butter(uint8_t n, float fc, SDW_FilterType ftype)
{
    SDW_SubFuncId SubFuncId = getButterSubFuncId(ftype);

    if ((SDW_EDL_NewButterLow == SubFuncId) || (SDW_EDL_NewButterHigh == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;

        SerialDataSize = SDW_FIXED_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentSysId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 2u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 4. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

butterBq::butterBq(uint8_t n, float fc1, float fc2, SDW_FilterType ftype)
{
    SDW_SubFuncId SubFuncId = getButterBqSubFuncId(ftype);

    if ((SDW_EDL_NewBiquadButterBand == SubFuncId) || (SDW_EDL_NewBiquadButterStop == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                2u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE) +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentMacroId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc1;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamReal = fc2;
        PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 5. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

butter::butter(uint8_t n, float fc1, float fc2, SDW_FilterType ftype)
{
    SDW_SubFuncId SubFuncId = getButterSubFuncId(ftype);

    if ((SDW_EDL_NewButterBand == SubFuncId) || (SDW_EDL_NewButterStop == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                2u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE) +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentSysId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc1;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamReal = fc2;
        PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 5. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

period::period(float Max, float Min, float Epsilon)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            3u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE);

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentSysId++);
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SDW_EDL_NewPeriodMeter, this->getId());

    /* 2. Prepare Param_1 */
    FuncParam.ParamReal = Max;
    PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamReal = Min;
    PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 4. Prepare Param_3 */
    FuncParam.ParamReal = Epsilon;
    PrepareParam(&Command, 2u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 6. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

maxObserve::maxObserve(uint8_t NbrOfSamples)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentSysId++);
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 1u, SDW_EDL_NewMaxObserver, this->getId());

    /* 2. Prepare Param_1 */
    FuncParam.ParamUint8 = NbrOfSamples;
    PrepareParam(&Command, 0u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

    /* 3. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

cheby1Bq::cheby1Bq(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_Cheby1Cfg chebyCfg)
{
    SDW_SubFuncId SubFuncId = getCheby1BqSubFuncId(ftype);

    if ((SDW_EDL_NewBiquadCheby1Band == SubFuncId) || (SDW_EDL_NewBiquadCheby1Stop == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                2u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE) +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_CHEBYX_CFG_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentMacroId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 4u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc1;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamReal = fc2;
        PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 5. Prepare Param_4 */
        FuncParam.ParamChebyx(chebyCfg);
        PrepareParam(&Command, 3u, SDW_CHEBYX_CFG_SIZE, SDW_CHEBYX_CFG_TYPE, &FuncParam);

        /* 6. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

cheby1Bq::cheby1Bq(uint8_t n, float fc, SDW_FilterType ftype, SDW_Cheby1Cfg chebyCfg)
{
    SDW_SubFuncId SubFuncId = getCheby1BqSubFuncId(ftype);

    if ((SDW_EDL_NewBiquadCheby1Low == SubFuncId) || (SDW_EDL_NewBiquadCheby1High == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_CHEBYX_CFG_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentMacroId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamChebyx(chebyCfg);
        PrepareParam(&Command, 2u, SDW_CHEBYX_CFG_SIZE, SDW_CHEBYX_CFG_TYPE, &FuncParam);

        /* 5. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

cheby1::cheby1(uint8_t n, float fc, SDW_FilterType ftype, SDW_Cheby1Cfg chebyCfg)
{
    SDW_SubFuncId SubFuncId = getCheby1SubFuncId(ftype);

    if ((SDW_EDL_NewChebyshev1Low == SubFuncId) || (SDW_EDL_NewChebyshev1High == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_CHEBYX_CFG_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentSysId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamChebyx(chebyCfg);
        PrepareParam(&Command, 2u, SDW_CHEBYX_CFG_SIZE, SDW_CHEBYX_CFG_TYPE, &FuncParam);

        /* 5. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

cheby1::cheby1(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_Cheby1Cfg chebyCfg)
{
    SDW_SubFuncId SubFuncId = getCheby1SubFuncId(ftype);

    if ((SDW_EDL_NewChebyshev1Band == SubFuncId) || (SDW_EDL_NewChebyshev1Stop == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                2u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE) +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentSysId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc1;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamReal = fc2;
        PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 5. Prepare Param_4 */
        FuncParam.ParamChebyx(chebyCfg);
        PrepareParam(&Command, 3u, SDW_CHEBYX_CFG_SIZE, SDW_CHEBYX_CFG_TYPE, &FuncParam);

        /* 6. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

cheby2Bq::cheby2Bq(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_Cheby2Cfg chebyCfg)
{
    SDW_SubFuncId SubFuncId = getCheby2BqSubFuncId(ftype);

    if ((SDW_EDL_NewBiquadCheby2Band == SubFuncId) || (SDW_EDL_NewBiquadCheby2Stop == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                2u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE) +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_CHEBYX_CFG_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentMacroId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 4u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc1;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamReal = fc2;
        PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 5. Prepare Param_4 */
        FuncParam.ParamChebyx(chebyCfg);
        PrepareParam(&Command, 3u, SDW_CHEBYX_CFG_SIZE, SDW_CHEBYX_CFG_TYPE, &FuncParam);

        /* 6. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

cheby2Bq::cheby2Bq(uint8_t n, float fc, SDW_FilterType ftype, SDW_Cheby2Cfg chebyCfg)
{
    SDW_SubFuncId SubFuncId = getCheby2BqSubFuncId(ftype);

    if ((SDW_EDL_NewBiquadCheby2Low == SubFuncId) || (SDW_EDL_NewBiquadCheby2High == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_CHEBYX_CFG_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentMacroId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamChebyx(chebyCfg);
        PrepareParam(&Command, 2u, SDW_CHEBYX_CFG_SIZE, SDW_CHEBYX_CFG_TYPE, &FuncParam);

        /* 5. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

cheby2::cheby2(uint8_t n, float fc, SDW_FilterType ftype, SDW_Cheby2Cfg chebyCfg)
{
    SDW_SubFuncId SubFuncId = getCheby2SubFuncId(ftype);

    if ((SDW_EDL_NewChebyshev2Low == SubFuncId) || (SDW_EDL_NewChebyshev2High == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_CHEBYX_CFG_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentSysId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamChebyx(chebyCfg);
        PrepareParam(&Command, 2u, SDW_CHEBYX_CFG_SIZE, SDW_CHEBYX_CFG_TYPE, &FuncParam);

        /* 5. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

cheby2::cheby2(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_Cheby2Cfg chebyCfg)
{
    SDW_SubFuncId SubFuncId = getCheby2SubFuncId(ftype);

    if ((SDW_EDL_NewChebyshev2Band == SubFuncId) || (SDW_EDL_NewChebyshev2Stop == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                2u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE) +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentSysId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc1;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamReal = fc2;
        PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 5. Prepare Param_4 */
        FuncParam.ParamChebyx(chebyCfg);
        PrepareParam(&Command, 3u, SDW_CHEBYX_CFG_SIZE, SDW_CHEBYX_CFG_TYPE, &FuncParam);

        /* 6. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

ellipBq::ellipBq(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_EllipCfg ellipCfg)
{
    SDW_SubFuncId SubFuncId = getEllipBqSubFuncId(ftype);

    if ((SDW_EDL_NewBiquadEllipBand == SubFuncId) || (SDW_EDL_NewBiquadEllipStop == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                2u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE) +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_ELLIP_CFG_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentMacroId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 4u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc1;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamReal = fc2;
        PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 5. Prepare Param_4 */
        FuncParam.ParamEllip(ellipCfg);
        PrepareParam(&Command, 3u, SDW_ELLIP_CFG_SIZE, SDW_ELLIP_CFG_TYPE, &FuncParam);

        /* 6. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

ellipBq::ellipBq(uint8_t n, float fc, SDW_FilterType ftype, SDW_EllipCfg ellipCfg)
{
    SDW_SubFuncId SubFuncId = getEllipBqSubFuncId(ftype);

    if ((SDW_EDL_NewBiquadEllipLow == SubFuncId) || (SDW_EDL_NewBiquadEllipHigh == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_ELLIP_CFG_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentMacroId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamEllip(ellipCfg);
        PrepareParam(&Command, 2u, SDW_ELLIP_CFG_SIZE, SDW_ELLIP_CFG_TYPE, &FuncParam);

        /* 5. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

ellip::ellip(uint8_t n, float fc, SDW_FilterType ftype, SDW_EllipCfg ellipCfg)
{
    SDW_SubFuncId SubFuncId = getEllipSubFuncId(ftype);

    if ((SDW_EDL_NewEllipLow == SubFuncId) || (SDW_EDL_NewEllipHigh == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_ELLIP_CFG_SIZE;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentSysId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 1u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamEllip(ellipCfg);
        PrepareParam(&Command, 2u, SDW_ELLIP_CFG_SIZE, SDW_ELLIP_CFG_TYPE, &FuncParam);

        /* 5. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

ellip::ellip(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_EllipCfg ellipCfg)
{
    SDW_SubFuncId SubFuncId = getEllipSubFuncId(ftype);

    if ((SDW_EDL_NewEllipBand == SubFuncId) || (SDW_EDL_NewEllipStop == SubFuncId))
    {
        SDW_FuncParam FuncParam;
        uint16_t SerialDataSize;
        SerialDataSize = SDW_FIXED_SIZE +
                2u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE) +
                SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
                SDW_PARAM_INFO_SIZE + SDW_ELLIP_CFG_SIZE;
        ;

        /* 1. Prepare Configuration : Header + Function Info */
        this->setId(currentSysId++);
        PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 4u, SubFuncId, this->getId());

        /* 2. Prepare Param_1 */
        FuncParam.ParamReal = fc1;
        PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 3. Prepare Param_2 */
        FuncParam.ParamReal = fc2;
        PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

        /* 4. Prepare Param_3 */
        FuncParam.ParamUint8 = n;
        PrepareParam(&Command, 2u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

        /* 5. Prepare Param_4 */
        FuncParam.ParamEllip(ellipCfg);
        PrepareParam(&Command, 3u, SDW_ELLIP_CFG_SIZE, SDW_ELLIP_CFG_TYPE, &FuncParam);

        /* 6. Send Configuration Request */
        HandleCmd(&Command, SerialDataSize);
    }
}

allpass::allpass(float Val1, float Val2)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            2u * (SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE);

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentSysId++);
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 2u, SDW_EDL_NewPassAll, this->getId());

    /* 2. Prepare Param_1 */
    FuncParam.ParamReal = Val1;
    PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamReal = Val2;
    PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 4. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

phiObserver::phiObserver(float freq)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize;

    SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentMacroId++);
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 1u, SDW_EDL_NewPhaseShiftObserver, this->getId());

    /* 2. Prepare Param_1 */
    FuncParam.ParamReal = freq;
    PrepareParam(&Command, 0u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 3. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

rls::rls(uint8_t modelOrd, float gain, macro system)
{
    SDW_FuncParam FuncParam;
    uint16_t SerialDataSize;

    SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_UINT8_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE +
            SDW_PARAM_INFO_SIZE + SDW_BLOCKID_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    this->setId(currentMacroId++);
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 3u, SDW_EDL_NewRLS, this->getId());

    /* 2. Prepare Param_1 */
    FuncParam.ParamUint8 = modelOrd;
    PrepareParam(&Command, 0u, SDW_UINT8_SIZE, TYPE_Uint8, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamReal = gain;
    PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 4. Prepare Param_3 */
    FuncParam.ParamBlockId(system.getId());
    PrepareParam(&Command, 2u, SDW_BLOCKID_SIZE, SDW_BLOCK_ID_TYPE, &FuncParam);

    /* 5. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

/* Probe Class ########################################################################################################### */
probe::probe(){};

probe::probe(const char *label)
{
    SDW_FuncParam FuncParam;
    uint8_t labelLen = strlen(label) + 1;
    uint16_t SerialDataSize;

    SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + labelLen;

    /* 1. Prepare Configuration : Header + Function Info */
    this->label = label;
    this->id = currentProbeId++;
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 1u, SDW_EDL_NewProbe, this->id);

    /* 2. Prepare Param_1 */
    FuncParam.ParamString = label;
    PrepareParam(&Command, 0u, labelLen, TYPE_String, &FuncParam);

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

uint16_t probe::getId()
{
    return this->id;
}

void probe::setId(uint16_t id)
{
    this->id = id;
}

void probe::addToQueue()
{
    this->idx = currentProbeIdx++;
}

float probe::getVal()
{
    float retval;
    memcpy((void *)&retval, (const void *)&SDW_ProbesBuffer[this->idx * SDW_REAL_SIZE], SDW_REAL_SIZE);
    return retval;
}

const char *probe::getLabel()
{
    return this->label;
}

void probe::show(void)
{
}

/* Input Class ########################################################################################################### */
input::input(){};

input::input(const char *label, float val)
{
    SDW_FuncParam FuncParam;
    uint8_t labelLen = strlen(label) + 1;
    uint16_t SerialDataSize = SDW_FIXED_SIZE +
            SDW_PARAM_INFO_SIZE + labelLen +
            SDW_PARAM_INFO_SIZE + SDW_REAL_SIZE;

    /* 1. Prepare Configuration : Header + Function Info */
    this->id = currentInputId++;
    this->val = val;
    this->label = label;
    PrepareCmdHeader(&Command, SerialDataSize, SDW_CreateBlock, 2u, SDW_EDL_NewInput, this->id);

    /* 2. Prepare Param_1 */
    FuncParam.ParamString = this->label;
    PrepareParam(&Command, 0u, labelLen, TYPE_String, &FuncParam);

    /* 3. Prepare Param_2 */
    FuncParam.ParamReal = val;
    PrepareParam(&Command, 1u, SDW_REAL_SIZE, TYPE_Real, &FuncParam);

    /* 4. Send Configuration Request */
    HandleCmd(&Command, SerialDataSize);
}

uint16_t input::getId()
{
    return this->id;
}

void input::setId(uint16_t id)
{
    this->id = id;
}

void input::enableRemote(void)
{
    this->idx = currentInputIdx++;
    this->addToQueue(this->val);

    /* 1. Prepare Configuration : Header + Function Info */
    PrepareCmdHeader(&Command, SDW_FIXED_SIZE, SDW_ManageBlock, 0u, SDW_EnableRemoteInput, this->id);

    /* 2. Send Configuration Request */
    HandleCmd(&Command, SDW_FIXED_SIZE);
}

void input::addToQueue(float InputValue)
{
    memcpy((void *)&SDW_InputsBuffer[this->idx * SDW_REAL_SIZE], (const void *)&InputValue, SDW_REAL_SIZE);
}

void input::operator>(tf Dst)
{
    LinkBlock2Block(this->id, 0xFF, Dst.getId(), 0xFF, SDW_EDL_LinkIn2Sys);
}

void input::operator>(node Dst)
{
    LinkBlock2Block(this->id, 0xFF, Dst.getId(), Dst.getActiveIoIdx(), SDW_EDL_LinkIn2Node);
}

void input::operator>(macro Dst)
{
    LinkBlock2Block(this->id, 0xFF, Dst.getId(), Dst.getActiveIoIdx(), SDW_EDL_LinkIn2Macro);
}

const char *input::getLabel(void)
{
    return this->label;
}



 void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  SDW_TX_DONE = true ;
}

 void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	 SDW_RX_DONE = true ;
}



