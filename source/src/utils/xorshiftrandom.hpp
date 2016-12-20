/*
 * xorshiftrandom.h
 *
 *  Created on: Dec 1, 2013
 *      Author: mmm
 */

/**
 * @file xorshiftrandom.h
 *
 * The public interface of the XOR shift pseudo-random number generator
 */


#ifndef OPENER_XORSHIFTRANDOM_H_
#define OPENER_XORSHIFTRANDOM_H_

#include <stdint.h>
/**
 * @brief Sets the initial seed for the XOR shift pseudo-random algorithm
 * @param pa_nSeed The initial seed value
 */
void SetXorShiftSeed(uint32_t seed);

/**
 * @brief Returns the next generated pseudo-random number
 * @return The next pseudo-random number
 */
uint32_t NextXorShiftUint32(void);

#endif /* OPENER__XORSHIFTRANDOM_H_ */
