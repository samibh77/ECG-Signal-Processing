/*
 * SDW.h
 *
 *  Created on: May 10, 2025
 *      Author: wiki
 */

#ifndef APPLICATION_USER_TOUCHGFX_APP_SDW_H_
#define APPLICATION_USER_TOUCHGFX_APP_SDW_H_


#include "stm32f4xx_hal.h"

#define SDW_CMD_WRITE 0x00u
#define SDW_CMD_READ 0x01u
#define SDW_CMD_SWITCH_BACK_TO_CFG 0x02u

#define SDW_NODE_ID_BASE 0x1000u
#define SDW_SYS_ID_BASE 0x2000u
#define SDW_MACRO_ID_BASE 0x3000u
#define SDW_INPUT_ID_BASE 0x4000u
#define SDW_PROBE_ID_BASE 0x5000u

#define SDW_REAL_SIZE 4u
#define SDW_UINT8_SIZE 1u
#define SDW_UINT16_SIZE 2u
#define SDW_UINT32_SIZE 4u
#define SDW_PARAM_ID_SIZE 1u
#define SDW_PARAM_SIZE_SIZE 1u
#define SDW_PARAM_INFO_SIZE (SDW_PARAM_ID_SIZE + SDW_PARAM_SIZE_SIZE)

#define SDW_ELLIP_CFG_SIZE SDW_UINT8_SIZE
#define SDW_CHEBYX_CFG_SIZE SDW_UINT8_SIZE
#define SDW_C2D_ALGO_SIZE SDW_UINT8_SIZE
#define SDW_NODE_TYPE_SIZE SDW_UINT8_SIZE
#define SDW_DESIGN_STATUS_SIZE SDW_UINT8_SIZE
#define SDW_COM_STATUS_SIZE SDW_UINT8_SIZE
#define SDW_HW_ROUTINE_ID_SIZE SDW_UINT16_SIZE
#define SDW_WAVE_FORM_SIZE SDW_UINT16_SIZE
#define SDW_SPECIAL_SYS_FUNC_SIZE SDW_UINT16_SIZE
#define SDW_ONE_BAND_FILTER_SIZE SDW_UINT32_SIZE
#define SDW_TWO_BANDS_FILTER_SIZE SDW_UINT32_SIZE
#define SDW_SPECIAL_FILTER_SIZE SDW_UINT16_SIZE
#define SDW_MON_INTERFACE_SIZE SDW_UINT8_SIZE
#define SDW_FUNC_PARAM_ID_SIZE SDW_UINT8_SIZE
#define SDW_BLOCKID_SIZE SDW_UINT16_SIZE
#define SDW_BAUDRATE_SIZE SDW_UINT32_SIZE

#define SDW_ELLIP_CFG_TYPE TYPE_Uint8
#define SDW_CHEBYX_CFG_TYPE TYPE_Uint8
#define SDW_C2D_ALGO_TYPE TYPE_Uint8
#define SDW_NODE_TYPE_TYPE TYPE_Uint8
#define SDW_DESIGN_STATUS_TYPE TYPE_Uint8
#define SDW_COM_STATUS_TYPE TYPE_Uint8
#define SDW_HW_ROUTINE_ID_TYPE TYPE_Uint16
#define SDW_WAVE_FORM_TYPE TYPE_Uint16
#define SDW_SPECIAL_SYS_FUNC_TYPE TYPE_Uint16
#define SDW_ONE_BAND_FILTER_TYPE TYPE_Uint32
#define SDW_TWO_BANDS_FILTER_TYPE TYPE_Uint32
#define SDW_SPECIAL_FILTER_TYPE TYPE_Uint16
#define SDW_MON_INTERFACE_TYPE TYPE_Uint8
#define SDW_BLOCK_ID_TYPE TYPE_Uint16
#define SDW_BAUDRATE_TYPE TYPE_Uint32

#define ParamC2dAlgo(X) ParamUint8 = (uint8_t)X
#define ParamNodeType(X) ParamUint8 = (uint8_t)X
#define ParamDesignStatus(X) ParamUint8 = (uint8_t)X
#define ParamComStatus(X) ParamUint8 = (uint8_t)X
#define ParamHwRoutineId(X) ParamUint16 = (uint16_t)X
#define ParamWaveForm(X) ParamUint16 = (uint16_t)X
#define ParamSpecialSysFunc(X) ParamUint16 = (uint16_t)X
#define ParamOneBandFilter(X) ParamUint32 = (uint32_t)X
#define ParamTwoBandsFilter(X) ParamUint32 = (uint32_t)X
#define ParamSpecialFilter(X) ParamUint16 = (uint16_t)X
#define ParamBlockId(X) ParamUint16 = (uint16_t)X
#define ParamMonInterface(X) ParamUint8 = (uint8_t)X
#define ParamChebyx(X) ParamUint8 = (uint8_t)X
#define ParamEllip(X) ParamUint8 = (uint8_t)X

#define SDW_FUNC_PARAM_MAX_NBR 20u
#define SDW_MAX_PROBES_NUMBER 16u
#define SDW_MAX_INPUTS_NUMBER 16u

#define SDW_CMD_SEQ_NUMBER_IDX 0x00u
#define SDW_CMD_READ_WRITE_IDX 0x01u
#define SDW_CMD_SIZEL_IDX 0x02u
#define SDW_CMD_SIZEH_IDX 0x03u
#define SDW_CMD_BLOCKIDL_IDX 0x04u
#define SDW_CMD_BLOCKIDH_IDX 0x05u
#define SDW_CMD_FUNC_ID_IDX 0x06u
#define SDW_CMD_SUB_FUNC_IDL_IDX 0x07u
#define SDW_CMD_SUB_FUNC_IDH_IDX 0x08u
#define SDW_CMD_NBR_OF_PARAMS_IDX 0x09u
#define SDW_CMD_START_OF_PARAMS_IDX 0x0Au
#define SDW_CMD_HEADER_SIZE 0x06u
#define SDW_CMD_CRC_SIZE 0x02u

#define SDW_RESPONSE_REQ_SIZE 0x06u
#define SDW_RESPONSE_SIZE 0x04u
#define SDW_CMD_MAX_SIZE 400u
#define SDW_RES_SEQ_NUMBER_IDX 0x00u
#define SDW_RES_RESCODE_IDX 0x01u
#define SDW_RES_CRCL_IDX 0x02u
#define SDW_RES_CRCH_IDX 0x03u
#define SDW_RES_REQ_CRCL_IDX 0x04u
#define SDW_RES_REQ_CRCH_IDX 0x05u

#define SDW_CMD_FULL_CMD_ID 0xEEEE
#define SDW_CMD_RESPONSE_ID 0xFFFF
#define SDW_FIXED_SIZE (SDW_CMD_START_OF_PARAMS_IDX + SDW_CMD_CRC_SIZE)

class node;
class tf;
class macro;
class input;
class probe;

typedef enum
{
    Zoh,
    Tustin,
    EulerBackward,
    EulerForward,

    NoC2dAlgo = 0xFF
} SDW_C2dAlgo;

typedef enum
{
    Sum,
    Mul,
    Abs1,
    Abs2,
    Abs3,
    Ident,

    NoNodeType = 0xFF
} SDW_NodeType;

typedef enum
{
    DesignOk,
    MemError,
    TfError,
    UnknownC2dAlgo,
    TextOk,
    TxtError,
    BadParam,
    BadConnection,
    BadRequest,
    SamplingFreqOutOfLimit,
    GPIO_Failure,
    TIM_Failure,
    ADC_Failure,
    DAC_Failure,
    COMP_Failure,
    DMA_Failure,
    OPAMP_Failure,
    SPI_Failure,
    UART_Failure,
    I2C_Failure,
    USB_Failure,
    NoDesignStatus = 0xFF
} SDW_DesignStatus;

typedef enum
{
    ComOk,
    WrongSeqNumber,
    WrongCrc,
    UnknownComError,

    NoComStatus = 0xFF
} SDW_ComStatus;

typedef enum
{
    SDW_CreateBlock = 0x00,
    SDW_LinkBlock = 0x01,
    SDW_ManageBlock = 0x02,

    NoFuncId = 0xFF
} SDW_FuncId;

typedef enum
{
    /* SDW_Create */
    SDW_NewHwBlock = 0x0000,
    SDW_EDL_NewSys = 0x0001,
    SDW_EDL_NewNode = 0x0002,
    SDW_EDL_NewRLS = 0x0003,
    SDW_EDL_NewPhaseShiftObserver = 0x0004,
    SDW_EDL_NewSine = 0x0005,
    SDW_EDL_NewPid = 0x0006,
    SDW_EDL_NewProbe = 0x0007,
    SDW_EDL_NewInput = 0x0008,
    SDW_EDL_NewSaw = 0x0009,
    SDW_EDL_NewTri = 0x000A,
    SDW_EDL_NewSqr = 0x000B,
    SDW_EDL_NewButterStop = 0x000C,
    SDW_EDL_NewButterBand = 0x000D,
    SDW_EDL_NewButterHigh = 0x000E,
    SDW_EDL_NewButterLow = 0x000F,
    SDW_EDL_NewNotch = 0x0010,
    SDW_EDL_NewPassAll = 0x0011,
    SDW_EDL_NewMacro = 0x0012,
    SDW_EDL_New1stOrdSysObserver = 0x0013,
    SDW_EDL_New2ndOrdSysObserver = 0x0014,
    SDW_EDL_GetRLSParam = 0x0015,
    SDW_EDL_NewPeak = 0x0016,
    SDW_EDL_NewBiquadButterLow = 0x0017,
    SDW_EDL_NewBiquadButterHigh = 0x0018,
    SDW_EDL_NewBiquadButterBand = 0x0019,
    SDW_EDL_NewBiquadButterStop = 0x001A,
    SDW_EDL_NewParamEQLow = 0x001B,
    SDW_EDL_NewParamEQHigh = 0x001C,
    SDW_EDL_NewParamEQBand = 0x001D,
    SDW_EDL_NewParamEQPeak = 0x001E,
    SDW_EDL_NewInterpolation = 0x001F,
    SDW_EDL_NewChebyshev1Band = 0x0020,
    SDW_EDL_NewChebyshev1Stop = 0x0021,
    SDW_EDL_NewChebyshev1Low = 0x0022,
    SDW_EDL_NewChebyshev1High = 0x0023,
    SDW_EDL_NewChebyshev2Band = 0x0024,
    SDW_EDL_NewChebyshev2Stop = 0x0025,
    SDW_EDL_NewChebyshev2Low = 0x0026,
    SDW_EDL_NewChebyshev2High = 0x0027,
    SDW_EDL_NewBiquadCheby1Band = 0x0028,
    SDW_EDL_NewBiquadCheby1Stop = 0x0029,
    SDW_EDL_NewBiquadCheby1Low = 0x002A,
    SDW_EDL_NewBiquadCheby1High = 0x002B,
    SDW_EDL_NewBiquadCheby2Band = 0x002C,
    SDW_EDL_NewBiquadCheby2Stop = 0x002D,
    SDW_EDL_NewBiquadCheby2Low = 0x002E,
    SDW_EDL_NewBiquadCheby2High = 0x002F,
    SDW_EDL_NewRLS1 = 0x0030,
    SDW_EDL_NewSysRep = 0x0031,
    SDW_EDL_NewCustSysRep = 0x0032,
    SDW_EDL_NewBatchFusion = 0x0033,
    SDW_EDL_NewSQ_CKF = 0x0034,
    SDW_EDL_NewUKF = 0x0035,
    SDW_EDL_NewAdaptiveKF = 0x0036,
    SDW_EDL_NewIdealKF = 0x0037,
    SDW_EDL_NewAbsMax = 0x0038,
    SDW_EDL_NewAbsMin = 0x0039,
    SDW_EDL_NewEntropy = 0x003A,
    SDW_EDL_NewPower = 0x003B,
    SDW_EDL_NewMaximum = 0x003C,
    SDW_EDL_NewMean = 0x003D,
    SDW_EDL_NewMinimum = 0x003E,
    SDW_EDL_NewRootMeanSquare = 0x003F,
    SDW_EDL_NewStandardDeviation = 0x0040,
    SDW_EDL_NewVariance = 0x0041,
    SDW_EDL_NewBridge = 0x0042,
    SDW_EDL_Delay = 0x0043,
    SDW_EDL_Compressor = 0x0044,
    SDW_EDL_DecibelsConverter = 0x0045,
    SDW_EDL_NewFftResolver = 0x0046,
    SDW_EDL_NewAllPass = 0x0047,
    SDW_EDL_NewCombFeedForward = 0x0048,
    SDW_EDL_NewCombFeedBack = 0x0049,
    SDW_EDL_NewBiquadEllipLow = 0x004A,
    SDW_EDL_NewBiquadEllipHigh = 0x004B,
    SDW_EDL_NewBiquadEllipBand = 0x004C,
    SDW_EDL_NewBiquadEllipStop = 0x004D,
    SDW_EDL_NewEllipLow = 0x004E,
    SDW_EDL_NewEllipHigh = 0x004F,
    SDW_EDL_NewEllipBand = 0x0050,
    SDW_EDL_NewEllipStop = 0x0051,
    SDW_EDL_NewHwInput = 0x0052,
    SDW_EDL_NewHwOutput = 0x0053,
    SDW_EDL_NewMaxObserver = 0x0054,
    SDW_EDL_NewMinObserver = 0x0055,
    SDW_EDL_NewSchmidt = 0x0056,
    SDW_EDL_NewPeriodMeter = 0x0057,
    SDW_EDL_NewComp = 0x0058,
    SDW_EDL_NewSysLightC = 0x0059,
    SDW_EDL_NewSysLightD = 0x005A,
    SDW_EDL_NewSat = 0x005B,
    SDW_EDL_NewDeadZone = 0x005C,
    SDW_EDL_NewBacklashDeadBand = 0x005D,
	SDW_EDL_NewSign = 0x005E ,

    /* SDW_Link */
    SDW_EDL_LinkSys2Sys = 0x03E8,
    SDW_EDL_LinkNode2Node = 0x03E9,
    SDW_EDL_LinkSys2Node = 0x03EA,
    SDW_EDL_LinkIn2Node = 0x03EB,
    SDW_EDL_LinkNode2Probe = 0x03EC,
    SDW_EDL_LinkSys2Probe = 0x03ED,
    SDW_EDL_LinkIn2Sys = 0x03EE,
    SDW_EDL_LinkNode2Sys = 0x03EF,
    SDW_EDL_LinkMacro2Macro = 0x03F0,
    SDW_EDL_LinkMacro2Sys = 0x03F1,
    SDW_EDL_LinkSys2Macro = 0x03F2,
    SDW_EDL_LinkMacro2Node = 0x03F3,
    SDW_EDL_LinkNode2Macro = 0x03F4,
    SDW_EDL_LinkIn2Macro = 0x03F5,
    SDW_EDL_LinkMacro2Probe = 0x03F6,

    /* SDW_Manage */
    SDW_EDL_DesignInit = 0x07D0,
    SDW_EDL_SetInput = 0x07D1,
    SDW_EDL_StopDesign = 0x07D2,
    SDW_EDL_StartMacro = 0x07D3,
    SDW_EDL_StopMacro = 0x07D4,
    SDW_EDL_StartSampling = 0x07D5,
    SDW_EDL_StopSampling = 0x07D6,
    SDW_EDL_AddSensor2Fusion = 0x07D7,
    SDW_EnableRemoteInput = 0x07D8,
    SDW_SetMonInterface = 0x07D9,

    NoSubFuncId = 0xFFFF
} SDW_SubFuncId;

typedef enum
{
    EncPos_1 = 0x0000,
    EncPos_2,

    EncVel_1 = 0x0100,
    EncVel_2,

    FreqIn_Pulse_1 = 0x0200,
    FreqIn_Pulse_2,
    FreqIn_Pulse_3,
    FreqIn_Pulse_4,
    FreqIn_Pulse_5,

    FreqIn_Period_1 = 0x0300,
    FreqIn_Period_2,
    FreqIn_Period_3,
    FreqIn_Period_4,
    FreqIn_Period_5,

    FreqIn_Duty_1 = 0x0400,
    FreqIn_Duty_2,
    FreqIn_Duty_3,
    FreqIn_Duty_4,
    FreqIn_Duty_5,

    FreqIn_Phase_1 = 0x0500,
    FreqIn_Phase_2,
    FreqIn_Phase_3,
    FreqIn_Phase_4,
    FreqIn_Phase_5,

    FreqOut_1 = 0x0600,
    FreqOut_2,
    FreqOut_3,
    FreqOut_4,
    FreqOut_5,
    FreqOut_6,
    FreqOut_7,
    FreqOut_8,

    AnOut_1 = 0x0700,
    AnOut_2,
    AnOut_3,
    AnOut_4,
    AnOut_5,

    AnIn_1 = 0x0800,
    AnIn_2,
    AnIn_3,
    AnIn_4,
    AnIn_5,
    AnIn_6,
    AnIn_7,
    AnIn_8,
    AnIn_9,
    AnIn_10,

    PowerOut_1 = 0x0900,
    PowerOut_2,

    NoHwRoutineId = 0xFFFF
} SDW_HwRoutineId;

typedef struct //rawData * ConvConst1 + ConvConst2
{
    float ConvConst1;
    float ConvConst2;
    SDW_HwRoutineId HwId;
} SDW_SensorSpec;

typedef struct
{
    float ConvConst1;
    float LowerLimit;
    float UpperLimit;
    SDW_HwRoutineId HwId;
} SDW_ActuatorSpec;

typedef enum
{
    passLow,
    passHigh,
    passBand,
    stopBand,

    NoFilterType = 0xFF
} SDW_FilterType;

typedef enum
{
    Cheby1Cfg1, // 0.1dB cheby1     //0.25 cheby1
    Cheby1Cfg2, // 0.5dB
    Cheby1Cfg3, // 1dB
    Cheby1Cfg4, // 2dB
    Cheby1Cfg5, // 3dB

    NoCheby1Cfg = 0xFF
} SDW_Cheby1Cfg;

typedef enum
{
    Cheby2Cfg1, // 30dB
    Cheby2Cfg2, // 40dB
    Cheby2Cfg3, // 50dB
    Cheby2Cfg4, // 60dB
    Cheby2Cfg5, // 70dB

    NoCheby2Cfg = 0xFF
} SDW_Cheby2Cfg;

typedef enum
{
    EllipCfg1, // rp = 1db rs = 30dB
    EllipCfg2, // rp = 1db rs = 40dB
    EllipCfg3, // rp = 1db rs = 50dB
    EllipCfg4, // rp = 1db rs = 60dB

    NoEllipCfg = 0xFF
} SDW_EllipCfg;

typedef enum
{
    Sine = SDW_EDL_NewSine,
    Tri = SDW_EDL_NewTri,
    Sqr = SDW_EDL_NewSqr,
    Saw = SDW_EDL_NewSaw,

    NoWaveForm = 0xFFFF
} SDW_WaveForm;

typedef enum
{
    PhaseShiftObserver = SDW_EDL_NewPhaseShiftObserver,
    FirstOrderObserver = SDW_EDL_New1stOrdSysObserver,
    SecondOrderObserver = SDW_EDL_New1stOrdSysObserver,
} SDW_SpecialMacroFunc;

typedef enum
{
    MonitoringOverUsb,
    MonitoringOverSpi,

    NoMonInterface = 0xFF
} SDW_MonInterface;

typedef enum
{
    TYPE_String,
    TYPE_RealArray,
    TYPE_Uint32Array,
    TYPE_Uint16Array,
    TYPE_Uint8Array,
    TYPE_Uint32,
    TYPE_Real,
    TYPE_Uint16,
    TYPE_Uint8,

    TYPE_NoType = 0xFF
} SDW_FuncParamId;

typedef union
{
    const char *ParamString;
    float *ParamRealArray;
    uint32_t *ParamUint32Array;
    uint16_t *ParamUint16Array;
    uint8_t *ParamUint8Array;
    uint32_t ParamUint32;
    float ParamReal;
    uint16_t ParamUint16;
    uint8_t ParamUint8;
} SDW_FuncParam;

typedef struct
{
    SDW_FuncParam FuncParams[SDW_FUNC_PARAM_MAX_NBR];
    SDW_FuncParamId FuncParamIds[SDW_FUNC_PARAM_MAX_NBR];
    uint8_t FuncParamSizes[SDW_FUNC_PARAM_MAX_NBR];
    uint16_t DataSize;
    uint16_t BlockId;
    SDW_SubFuncId SubFuncId;
    SDW_FuncId FuncId;
    uint8_t NbrOfParams;
} SDW_FuncInfo;

typedef struct
{
    uint16_t ReceivedCrc;
    uint16_t CalculatedCrc;
    uint8_t SeqNbr;
    uint8_t ReadWrite;
} SDW_TransactionInfo;

typedef struct
{
    SDW_FuncInfo FuncInfo;
    SDW_TransactionInfo TransactionInfo;
} SDW_CommandInfo;

typedef void (*ErrorHandlerType)(void);

class probe
{
private:
    uint16_t id;
    uint8_t idx;
    const char *label;

public:
    static uint16_t currentProbeId;
    static uint8_t currentProbeIdx;
    probe();
    probe(const char *label);
    uint16_t getId();
    void show();
    void setId(uint16_t id);
    float getVal();
    const char *getLabel();
    void addToQueue();
};

class node
{
private:
    uint16_t id;
    uint8_t activeIoIdx;

public:
    static uint16_t currentNodeId;
    node();
    node(const char *inCoef, const char *outCoef, SDW_NodeType Type);
    node(const float *inCoef, uint8_t inSize, const float *outCoef, uint8_t outSize, SDW_NodeType Type);
    uint16_t getId();
    void setId(uint16_t id);
    uint8_t getActiveIoIdx();
    node in(uint8_t inIdx);
    node out(uint8_t outIdx);
    void operator>(tf tfDst);
    void operator>(probe &probeDst);
    void operator>(node nodeDst);
    void operator>(macro macroDst);
};

class macro
{
private:
    uint16_t id;
    uint8_t activeIoIdx;

public:
    static uint16_t currentMacroId;
    macro();
    macro(uint8_t inCoef, uint8_t outCoef);
    uint16_t getId();
    void setId(uint16_t id);
    uint8_t getActiveIoIdx();
    macro in(uint8_t inIdx);
    macro out(uint8_t outIdx);
    void operator>(tf tfDst);
    void operator>(probe &probeDst);
    void operator>(node nodeDst);
    void operator>(macro macroDst);
    void start();
    void stop();
};

class input
{
private:
    uint16_t id;
    float val;
    uint8_t idx;
    const char *label;

public:
    static uint16_t currentInputId;
    static uint8_t currentInputIdx;
    input();
    input(const char *label, float val);
    uint16_t getId();
    void setId(uint16_t id);
    void addToQueue(float inputValue);
    void enableRemote();
    const char *getLabel();
    void operator>(tf tfDst);
    void operator>(node nodeDst);
    void operator>(macro macroDst);
};

class tf
{
private:
    uint16_t id;

public:
    static uint16_t currentSysId;
    tf();
    uint16_t getId();
    void setId(uint16_t id);
    tf(const char *numerator, const char *denominator);
    tf(float *numerator, uint8_t numSize, float *denominator, uint8_t denomSize);
    void operator>(tf srcTf);
    void operator>(probe &dstProbe);
    void operator>(node nodeDst);
    void operator>(macro macroDst);
};

class tfd : public tf
{
public:
    tfd(float *numerator, float *denominator, uint8_t sysord);
};

class fir : public tf
{
public:
    fir(float* coefficients, uint16_t size);
};

class butter : public tf
{
public:
    butter(uint8_t n, float fc, SDW_FilterType ftype);
    butter(uint8_t n, float fc1, float fc2, SDW_FilterType ftype);
};

class period : public tf
{
public:
    period(float Max, float Min, float Epsilon);
};

class maxObserve : public tf
{
public:
    maxObserve(uint8_t NbrOfSamples);
};

class cheby1 : public tf
{
public:
    cheby1(uint8_t n, float fc, SDW_FilterType ftype, SDW_Cheby1Cfg chebyCfg);
    cheby1(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_Cheby1Cfg chebyCfg);
};

class cheby2 : public tf
{
public:
    cheby2(uint8_t n, float fc, SDW_FilterType ftype, SDW_Cheby2Cfg chebyCfg);
    cheby2(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_Cheby2Cfg chebyCfg);
};

class ellip : public tf
{
public:
    ellip(uint8_t n, float fc, SDW_FilterType ftype, SDW_EllipCfg ellipCfg);
    ellip(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_EllipCfg ellipCfg);
};

class butterBq : public macro
{
public:
    butterBq(uint8_t n, float fc, SDW_FilterType ftype);
    butterBq(uint8_t n, float fc1, float fc2, SDW_FilterType ftype);
};

class cheby1Bq : public macro
{
public:
    cheby1Bq(uint8_t n, float fc, SDW_FilterType ftype, SDW_Cheby1Cfg chebyCfg);
    cheby1Bq(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_Cheby1Cfg chebyCfg);
};

class cheby2Bq : public macro
{
public:
    cheby2Bq(uint8_t n, float fc, SDW_FilterType ftype, SDW_Cheby2Cfg chebyCfg);
    cheby2Bq(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_Cheby2Cfg chebyCfg);
};

class ellipBq : public macro
{
public:
    ellipBq(uint8_t n, float fc, SDW_FilterType ftype, SDW_EllipCfg ellipCfg);
    ellipBq(uint8_t n, float fc1, float fc2, SDW_FilterType ftype, SDW_EllipCfg ellipCfg);
};

class pid : public tf
{
public:
    pid(float ki, float kp);                      // PI
    pid(float kp, float kd, float tau);           // PD
    pid(float ki, float kp, float kd, float tau); // PID
};

class allpass : public tf
{
public:
    allpass(float freq, float phi);
};

class notch : public tf
{
public:
    notch(float centernotch, float qualcoef);
};

class peak : public tf
{
public:
    peak(float centernotch, float qualcoef);
};

class compressor : public tf
{
public:
    compressor(float Attack, float Release, float Threshold_dB, float Ratio);
};

class absMax : public tf
{
public:
    absMax(uint8_t Length);
};

class absMin : public tf
{
public:
    absMin(uint8_t Length);
};

class entropy : public tf
{
public:
    entropy(uint8_t Length);
};

class power : public tf
{
public:
    power(uint8_t Length);
};

class mean : public tf
{
public:
    mean(uint8_t Length);
};

class rootMeanSquare : public tf
{
public:
    rootMeanSquare(uint8_t Length);
};

class standardDeviation : public tf
{
public:
    standardDeviation(uint8_t Length);
};

class variance : public tf
{
public:
    variance(uint8_t Length);
};

class interpolation : public tf
{
public:
    interpolation(uint32_t M_u32);
};


class plant : public node
{
public:
    plant(SDW_ActuatorSpec *Actuators, uint8_t NbrOfActuators, SDW_SensorSpec *Sensors, uint8_t NbrOfSensors);
};

class actuator : public node
{
public:
    actuator(SDW_ActuatorSpec *Actuator);
};

class sensor : public node
{
public:
    sensor(SDW_SensorSpec *Sensor);
};

class schmidt : public node
{
public:
    schmidt();
};

class comp : public node
{
public:
    comp();
};

class sat : public node
{
public:
    sat();
};

class deadZone : public node
{
public:
    deadZone();
};

class backlashDeadBand : public node
{
public:
    backlashDeadBand();
};

class bridge : public node
{
public:
    bridge(uint8_t nbrOdOutputs);
};

class wave : public tf
{
public:
    wave(SDW_WaveForm waveform, float freq, float offset, float phaseshift);
};

class phiObserver : public macro
{
public:
    phiObserver(float freq);
};

class rls : public macro
{
public:
    rls(uint8_t modelOrd, float gain, macro system);
};

class decimation : public macro
{
public:
    decimation(float *Buffer);
};

class paramEQLow : public macro
{
public:
    paramEQLow(float Fc, uint8_t FilterOrder, float Gain);
};

class ParamEQHigh : public macro
{
public:
    ParamEQHigh(float Fc, uint8_t FilterOrder, float Gain);
};

class ParamEQBand : public macro
{
public:
    ParamEQBand(float Fc1, float Fc2, uint8_t FilterOrder, float Gain);
};

class ParamEQPeak : public macro
{
public:
    ParamEQPeak(float Fc, float Gain);
};


class SDW
{
private:
    int cs;

public:
    SDW();
    SDW(int cs);
    SDW(int cs, int bitrate);
    SDW(int cs, int mosi, int miso, int clk);
    SDW(int cs, int mosi, int miso, int clk, int bitrate);
    void setComErrorHandler(ErrorHandlerType UserHandler);
    void setDesignErrorHandler(ErrorHandlerType UserHandler);
    void stopDesign(void);
    void startDesign(float SamplingFreq, SDW_C2dAlgo Algo);
    void startSampling(void);
    void stopSampling(void);
    void setMonitoringInterface(SDW_MonInterface InterfaceId);
    SDW_DesignStatus GetcurrentDesignStatus(void);
    SDW_ComStatus GetcurrentComStatus(void);
    uint8_t getSequenceNumber(void);
    void setInputs(void);
    void getProbes(void);
};


class sign : public node
{
  public :
    sign();
};





#endif /* APPLICATION_USER_TOUCHGFX_APP_SDW_H_ */
