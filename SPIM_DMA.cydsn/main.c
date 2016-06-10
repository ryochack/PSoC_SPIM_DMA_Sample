/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>

#define BURST_COUNT   (1u)
#define REQ_PER_BURST (1u)
#define STORE_TD_CFG_ONCMPLT (1u)

static uint16 xfer_cnt = 1;

CY_ISR(xfer_done_interrupt_handler)
{
    if (++xfer_cnt > 8) xfer_cnt = 1;
}

int main()
{
    uint8 TD_tx, channel_tx;
    uint8 TD_rx, channel_rx;
    const uint8 txbuf[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint8 rxbuf[8];
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    // Configure Tx DMA
    channel_tx = DMA_MOSI_DmaInitialize(BURST_COUNT, REQ_PER_BURST, HI16((uint32)txbuf), HI16((uint32)SPIM_TXDATA_PTR));
    TD_tx = CyDmaTdAllocate();
    CyDmaTdSetAddress(TD_tx, (uint16)((uint32)txbuf), (uint16)((uint32)SPIM_TXDATA_PTR));
    CyDmaTdSetConfiguration(TD_tx, sizeof(txbuf), DMA_DISABLE_TD, DMA_MOSI__TD_TERMOUT_EN|TD_INC_SRC_ADR);
    CyDmaChSetInitialTd(channel_tx, TD_tx);

    // Configure Rx DMA
    channel_rx = DMA_MISO_DmaInitialize(BURST_COUNT, REQ_PER_BURST, HI16((uint32)SPIM_RXDATA_PTR), HI16((uint32)rxbuf));
    TD_rx = CyDmaTdAllocate();
    CyDmaTdSetAddress(TD_rx, (uint16)((uint32)SPIM_RXDATA_PTR), (uint16)((uint32)rxbuf));
    CyDmaTdSetConfiguration(TD_rx, sizeof(rxbuf), DMA_DISABLE_TD, TD_INC_DST_ADR);
    CyDmaChSetInitialTd(channel_rx, TD_rx);
    
    SPIM_Start();
    SPIM_ClearTxBuffer();
    isr_xfer_done_StartEx(xfer_done_interrupt_handler);
    
    for(;;)
    {
        /* Clear the Tx buffer to reinitialize the pointer */
        SPIM_ClearTxBuffer();
        memset(rxbuf, 0, sizeof(rxbuf));

        // Change DMA transfer size
        CyDmaTdSetConfiguration(TD_tx, xfer_cnt, DMA_DISABLE_TD, DMA_MOSI__TD_TERMOUT_EN|TD_INC_SRC_ADR);
        CyDmaTdSetConfiguration(TD_rx, xfer_cnt, DMA_DISABLE_TD, TD_INC_DST_ADR);

        CyDmaChEnable(channel_tx, STORE_TD_CFG_ONCMPLT);
        CyDmaChEnable(channel_rx, STORE_TD_CFG_ONCMPLT);
        // Kick DMA
        CyDmaChSetRequest(channel_tx, CPU_REQ);
        CyDelay(1000);
    }
}

/* [] END OF FILE */
