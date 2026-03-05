#ifndef HAL_ATOMIC_H
#define HAL_ATOMIC_H

/* * 抽象化 Memory Barrier
 * 在 RP2350 (ARM) 上將實作為 __DMB()，在 Host 測試時則作為 Mock 標的
 */
void hal_atomic_dmb(void);

#endif  // HAL_ATOMIC_H