/*
 * \file   hs_mmcsd_rw.c
 *
 * \brief  Sample application for HS MMCSD
 *
*/

/* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

// must include this file BEFORE ff.h
#include "integer.h"
#include "soc_AM335x.h"
#include "hs_mmcsd.h"
#include "edma.h"
#include "interrupt.h"
#include "uartStdio.h"
#include "mmcsd_proto.h"
#include "hs_mmcsdlib.h"
#include "beaglebone.h"
#include "string.h"
// must be included AFTER integer.h
#include "ff.h"


/******************************************************************************
**                      TYPE DEFINITIONS
*******************************************************************************/


/******************************************************************************
**                      INTERNAL MACRO DEFINITIONS
*******************************************************************************/

/* Frequency */
#define HSMMCSD_0_IN_FREQ    96000000 /* 96MHz */
#define HSMMCSD_0_INIT_FREQ  400000   /* 400kHz */

/* EDMA Events */
#define EDMA3_CHA_MMCSD_0_TX        24
#define EDMA3_CHA_MMCSD_0_RX        25

/* EDMA3 Event queue number. */
#define EVT_QUEUE_NUM             (0)

/* EDMA3 Region Number. */
#define REGION_NUMBER             (0)

/* EDMA callback function array */
static void (*cb_Fxn[EDMA3_NUM_TCC]) (unsigned int tcc, unsigned int status);

/* Block size config */
#define HSMMCSD_BLK_SIZE            512
#define HSMMCSD_RW_BLK                1

/* Global flags for interrupt handling */
volatile unsigned int callbackOccured = 0; 
volatile unsigned int xferCompFlag = 0; 
volatile unsigned int dataTimeout = 0;
volatile unsigned int cmdCompFlag = 0;
volatile unsigned int cmdTimeout = 0; 
volatile unsigned int errFlag = 0;
volatile unsigned int sdBlkSize = HSMMCSD_BLK_SIZE;

/* SD card info structure */
mmcsdCardInfo sdCard;

/* SD Controller info structure */
mmcsdCtrlInfo  ctrlInfo;

/* Global data pointers */
#define HSMMCSD_DATA_SIZE 512

#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=32
unsigned char data[HSMMCSD_DATA_SIZE];

#elif defined(__TMS470__)
#pragma DATA_ALIGN(data, 32);
unsigned char data[HSMMCSD_DATA_SIZE];
#else
unsigned char data[HSMMCSD_DATA_SIZE] __attribute__ ((aligned (32)))= {0};

#endif

extern void HSMMCSDFsMount(unsigned int driveNum, void *ptr);
extern void HSMMCSDFsProcessCmdLine(void);
/******************************************************************************
**                      INTERNAL FUNCTION PROTOTYPES
*******************************************************************************/

/******************************************************************************
**                      INTERNAL VARIABLE DEFINITIONS
*******************************************************************************/

/******************************************************************************
**                          FUNCTION DEFINITIONS
*******************************************************************************/

/*
 * Check command status
 */

static unsigned int HSMMCSDCmdStatusGet(mmcsdCtrlInfo *ctrl)
{
    unsigned int status = 0;

    while ((cmdCompFlag == 0) && (cmdTimeout == 0));
    
    if (cmdCompFlag)
    {
        status = 1;
        cmdCompFlag = 0;
    }

    if (cmdTimeout)
    {
        status = 0;
        cmdTimeout = 0;
    }

    return status;
}

static unsigned int HSMMCSDXferStatusGet(mmcsdCtrlInfo *ctrl)
{
    unsigned int status = 0;

    while ((xferCompFlag == 0) && (dataTimeout == 0));
    
    if (xferCompFlag)
    {
        status = 1;
        xferCompFlag = 0;
    }

    if (dataTimeout)
    {
        status = 0;
        dataTimeout = 0;
    }

    /* Also, poll for the callback */
    if (HWREG(ctrl->memBase + MMCHS_CMD) & MMCHS_CMD_DP)
    {
        while(callbackOccured == 0);
        callbackOccured = 0;
    }

    ctrlInfo.dmaEnable = 0;

    return status;
}

void HSMMCSDRxDmaConfig(void *ptr, unsigned int blkSize, unsigned int nblks)
{
    EDMA3CCPaRAMEntry paramSet;

    paramSet.srcAddr    = ctrlInfo.memBase + MMCHS_DATA;
    paramSet.destAddr   = (unsigned int)ptr;
    paramSet.srcBIdx    = 0;
    paramSet.srcCIdx    = 0;
    paramSet.destBIdx   = 4;
    paramSet.destCIdx   = blkSize;
    paramSet.aCnt       = 0x4;
    paramSet.bCnt       = blkSize/4;              
    paramSet.cCnt       = nblks;
    paramSet.bCntReload = 0x0;
    paramSet.linkAddr   = 0xffff;
    paramSet.opt        = 0;

    /* Set OPT */
    paramSet.opt |= ((EDMA3_CHA_MMCSD_0_RX << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);

    /* 1. Transmission complition interrupt enable */
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* 2. Read FIFO : SRC Constant addr mode */
    paramSet.opt |= (1 << 0);

    /* 3. SRC FIFO width is 32 bit */
    paramSet.opt |= (2 << 8);

    /* 4.  AB-Sync mode */
    paramSet.opt |= (1 << 2);

    /* configure PaRAM Set */
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MMCSD_0_RX, &paramSet);

    /* Enable the transfer */
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MMCSD_0_RX, EDMA3_TRIG_MODE_EVENT);
}

void HSMMCSDTxDmaConfig(void *ptr, unsigned int blkSize, unsigned int blks)
{
    EDMA3CCPaRAMEntry paramSet;

    paramSet.srcAddr    = (unsigned int)ptr;
    paramSet.destAddr   = ctrlInfo.memBase + MMCHS_DATA;
    paramSet.srcBIdx    = 4;
    paramSet.srcCIdx    = blkSize;
    paramSet.destBIdx   = 0;
    paramSet.destCIdx   = 0;
    paramSet.aCnt       = 0x4;
    paramSet.bCnt       = blkSize/4;
    paramSet.cCnt       = blks;
    paramSet.bCntReload = 0x0;
    paramSet.linkAddr   = 0xffff;
    paramSet.opt        = 0;

    /* Set OPT */
    paramSet.opt |= ((EDMA3_CHA_MMCSD_0_TX << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);

    /* 1. Transmission complition interrupt enable */
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* 2. Read FIFO : DST Constant addr mode */
    paramSet.opt |= (1 << 1);

    /* 3. DST FIFO width is 32 bit */
    paramSet.opt |= (2 << 8);

    /* 4.  AB-Sync mode */
    paramSet.opt |= (1 << 2);

    /* configure PaRAM Set */
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MMCSD_0_TX, &paramSet);

    /* Enable the transfer */
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MMCSD_0_TX, EDMA3_TRIG_MODE_EVENT);
}

static void HSMMCSDXferSetup(mmcsdCtrlInfo *ctrl, unsigned char rwFlag, void *ptr,
                             unsigned int blkSize, unsigned int nBlks)
{
    if (rwFlag == 1)
    {
        HSMMCSDRxDmaConfig(ptr, blkSize, nBlks);
    }
    else
    {
        HSMMCSDTxDmaConfig(ptr, blkSize, nBlks);
    }

    ctrl->dmaEnable = 1;
    HSMMCSDBlkLenSet(ctrl->memBase, blkSize);

    xferCompFlag = 0;
    callbackOccured = 0;
}


/*
** This function is used as a callback from EDMA3 Completion Handler.
*/
static void callback(unsigned int tccNum, unsigned int status)
{
    callbackOccured = 1;
    EDMA3DisableTransfer(SOC_EDMA30CC_0_REGS, tccNum, EDMA3_TRIG_MODE_EVENT);
}

static void Edma3CompletionIsr(void)
{
    volatile unsigned int pendingIrqs;
    volatile unsigned int isIPR = 0;

    unsigned int indexl;
    unsigned int Cnt = 0;

    indexl = 1;

    isIPR = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_IPR(REGION_NUMBER));
    if(isIPR)
    {
        while ((Cnt < EDMA3CC_COMPL_HANDLER_RETRY_COUNT)&& (indexl != 0u))
        {
            indexl = 0u;
            pendingIrqs = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_IPR(REGION_NUMBER));
            while (pendingIrqs)
            {
                if((pendingIrqs & 1u) == TRUE)
                {
                    /**
                    * If the user has not given any callback function
                    * while requesting the TCC, its TCC specific bit
                    * in the IPR register will NOT be cleared.
                    */
                    /* here write to ICR to clear the corresponding IPR bits */
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_ICR(REGION_NUMBER)) = (1u << indexl);

                    if (cb_Fxn[indexl] != NULL)
                    {
                        (*cb_Fxn[indexl])(indexl, EDMA3_XFER_COMPLETE);
                    }
                }
                ++indexl;
                pendingIrqs >>= 1u;
            }
            Cnt++;
        }
    }
}

static void Edma3CCErrorIsr(void)
{
    volatile unsigned int pendingIrqs = 0;
    unsigned int evtqueNum = 0;
    unsigned int index = 1;
    unsigned int Cnt = 0;

    if((HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_EMR) != 0 ) || \
       (HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_QEMR) != 0) || \
       (HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_CCERR) != 0))
    {
        /* Loop for EDMA3CC_ERR_HANDLER_RETRY_COUNT number of time, breaks
           when no pending interrupt is found */
        while ((Cnt < EDMA3CC_ERR_HANDLER_RETRY_COUNT) && (index != 0u))
        {
            index = 0u;
            pendingIrqs = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_EMR);
            while (pendingIrqs)
            {
                /*Process all the pending interrupts*/
                if((pendingIrqs & 1u)==TRUE)
                {
                    /* Write to EMCR to clear the corresponding EMR bits.*/
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_EMCR) = (1u<<index);
                    /*Clear any SER*/
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_SECR(REGION_NUMBER)) = (1u << index);
                }
                ++index;
                pendingIrqs >>= 1u;
            }
            index = 0u;
            pendingIrqs = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_QEMR);
            while (pendingIrqs)
            {
                /*Process all the pending interrupts*/
                if((pendingIrqs & 1u)==TRUE)
                {
                    /* Here write to QEMCR to clear the corresponding QEMR bits*/
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_QEMCR) = (1u<<index);
                    /*Clear any QSER*/
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_S_QSECR(0)) = (1u<<index);
                }
                ++index;
                pendingIrqs >>= 1u;
            }
            index = 0u;
            pendingIrqs = HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_CCERR);
            if (pendingIrqs != 0u)
            {
                /* Process all the pending CC error interrupts. */
                  /* Queue threshold error for different event queues.*/
                for (evtqueNum = 0u; evtqueNum < EDMA3_0_NUM_EVTQUE; evtqueNum++)
                {
                    if((pendingIrqs & (1u << evtqueNum)) != 0u)
                    {
                        /* Clear the error interrupt. */
                        HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_CCERRCLR) = (1u << evtqueNum);
                    }
                 }

                 /* Transfer completion code error. */
                 if ((pendingIrqs & (1 << EDMA3CC_CCERR_TCCERR_SHIFT)) != 0u)
                 {
                    HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_CCERRCLR) = \
                    (0x01u << EDMA3CC_CCERR_TCCERR_SHIFT);
                 }
                 ++index;
            }

            Cnt++;
        }
    }
}

static void HSMMCSDIsr(void)
{
    volatile unsigned int status = 0;

    status = HSMMCSDIntrStatusGet(ctrlInfo.memBase, 0xFFFFFFFF);
    
    HSMMCSDIntrStatusClear(ctrlInfo.memBase, status);

    if (status & HS_MMCSD_STAT_CMDCOMP)
    {
        cmdCompFlag = 1;
    }

    if (status & HS_MMCSD_STAT_ERR)
    {
        errFlag = status & 0xFFFF0000;

        if (status & HS_MMCSD_STAT_CMDTIMEOUT)
        {
            cmdTimeout = 1;
        }

        if (status & HS_MMCSD_STAT_DATATIMEOUT)
        {
            dataTimeout = 1;
        }
    }

    if (status & HS_MMCSD_STAT_TRNFCOMP)
    {
        xferCompFlag = 1;
    }
}


/*
** This function configures the AINTC to receive EDMA3 interrupts.
*/
#define SYS_INT_HSMMC_0             64

static void EDMA3AINTCConfigure(void)
{
    /* Initializing the ARM Interrupt Controller. */
    IntAINTCInit();

    /* Registering EDMA3 Channel Controller 0 transfer completion interrupt.  */
    IntRegister(SYS_INT_EDMACOMPINT, Edma3CompletionIsr);

    /* Setting the priority for EDMA3CC0 completion interrupt in AINTC. */
    IntPrioritySet(SYS_INT_EDMACOMPINT, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Registering EDMA3 Channel Controller 0 Error Interrupt. */
    IntRegister(SYS_INT_EDMAERRINT, Edma3CCErrorIsr);

    /* Setting the priority for EDMA3CC0 Error interrupt in AINTC. */
    IntPrioritySet(SYS_INT_EDMAERRINT, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enabling the EDMA3CC0 completion interrupt in AINTC. */
    IntSystemEnable(SYS_INT_EDMACOMPINT);
    
    /* Enabling the EDMA3CC0 Error interrupt in AINTC. */
    IntSystemEnable(SYS_INT_EDMAERRINT);

    /* Registering HSMMC 0 Interrupt handler */
    IntRegister(SYS_INT_HSMMC_0, HSMMCSDIsr);

    /* Setting the priority for EDMA3CC0 completion interrupt in AINTC. */
    IntPrioritySet(SYS_INT_HSMMC_0, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enabling the HSMMC 0 interrupt in AINTC. */
    IntSystemEnable(SYS_INT_HSMMC_0);

    /* Enabling IRQ in CPSR of ARM processor. */
    IntMasterIRQEnable();
}


/* 
** Powering up, initializing and registering interrupts for EDMA.
*/

static void EDMA3Initialize(void)
{
    /* Initialization of EDMA3 */
    EDMA3Init(SOC_EDMA30CC_0_REGS, EVT_QUEUE_NUM);

    /* Configuring the AINTC to receive EDMA3 interrupts. */ 
    EDMA3AINTCConfigure();
}

static void HSMMCSDEdmaInit(void)
{
    /* Initializing the EDMA. */
    EDMA3Initialize();

    /* Request DMA Channel and TCC for MMCSD Transmit*/
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
                        EDMA3_CHA_MMCSD_0_TX, EDMA3_CHA_MMCSD_0_TX,
                        EVT_QUEUE_NUM);

    /* Registering Callback Function for TX*/
    cb_Fxn[EDMA3_CHA_MMCSD_0_TX] = &callback;

    /* Request DMA Channel and TCC for UART Receive */
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
                        EDMA3_CHA_MMCSD_0_RX, EDMA3_CHA_MMCSD_0_RX,
                        EVT_QUEUE_NUM);

    /* Registering Callback Function for RX*/
    cb_Fxn[EDMA3_CHA_MMCSD_0_RX] = &callback;
}


/*
** Initialize the MMCSD controller structure for use
*/
static void HSMMCSDControllerSetup(void)
{
    ctrlInfo.memBase = SOC_MMCHS_0_REGS;
    ctrlInfo.ctrlInit = HSMMCSDControllerInit;
    ctrlInfo.xferSetup = HSMMCSDXferSetup;
    ctrlInfo.cmdStatusGet = HSMMCSDCmdStatusGet;
    ctrlInfo.xferStatusGet = HSMMCSDXferStatusGet;
    ctrlInfo.cardPresent = HSMMCSDCardPresent;
    ctrlInfo.cmdSend = HSMMCSDCmdSend;
    ctrlInfo.busWidthConfig = HSMMCSDBusWidthConfig;
    ctrlInfo.busFreqConfig = HSMMCSDBusFreqConfig;
    ctrlInfo.intrMask = (HS_MMCSD_INTR_CMDCOMP | HS_MMCSD_INTR_CMDTIMEOUT |
                            HS_MMCSD_INTR_DATATIMEOUT | HS_MMCSD_INTR_TRNFCOMP);
    ctrlInfo.intrEnable = HSMMCSDIntEnable;
    ctrlInfo.busWidth = (SD_BUS_WIDTH_1BIT | SD_BUS_WIDTH_4BIT);
    ctrlInfo.highspeed = 1;
    ctrlInfo.ocr = (SD_OCR_VDD_3P0_3P1 | SD_OCR_VDD_3P1_3P2);
    ctrlInfo.card = &sdCard;
    ctrlInfo.ipClk = HSMMCSD_0_IN_FREQ;
    ctrlInfo.opClk = HSMMCSD_0_INIT_FREQ;
    sdCard.ctrl = &ctrlInfo;

    callbackOccured = 0;
    xferCompFlag = 0;
    dataTimeout = 0;
    cmdCompFlag = 0;
    cmdTimeout = 0;
}


int main(void)
{
    UARTStdioInit();

    EDMAModuleClkConfig();

    HSMMCSDEdmaInit();

    HSMMCSDPinMuxSetup();

    HSMMCSDModuleClkConfig();

    /* Basic controller initializations */
    HSMMCSDControllerSetup();

    /* First check, if card is inserted */
    while(1)
    {
        if (MMCSDCardPresent(&ctrlInfo) == 0)
        {
            volatile unsigned int i = 0xFFFFFF;
            UARTPuts("Please insert the card \n\r", -1);
            while(i--);
        }
        else
        {
            break;
        }
    }

    /* Initialize the MMCSD controller */
    MMCSDCtrlInit(&ctrlInfo);

    MMCSDIntEnable(&ctrlInfo);

    HSMMCSDFsMount(0, &sdCard);

    char flag = 0;

    // Create the file
    FIL logFile;

    // open the logfile
    // fopen(ptr to
    if(f_open(&logFile, "/log.txt", FA_READ | FA_WRITE | FA_OPEN_ALWAYS) != FR_OK)
    {
    	// fucking error!
    	flag = 1;
    }

    WORD bytesWritten;

    if (flag == 0)
    {
    	f_write(&logFile, "New log opened!\n", 16, &bytesWritten);
    }

    // close the file
    f_close(&logFile);

    // unmount the SD card
//    f_mount(0,NULL);

    HSMMCSDFsProcessCmdLine();

    while(1);
}

