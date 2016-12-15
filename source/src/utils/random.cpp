/*
 * random.cpp
 *
 *  Created on: Dec 16, 2013
 *      Author: mmm
 */

#include "random.h"

RandomGen::RandomGen(uint32_t seed, uint32_t randGenerator)
{
    this->current_seed_value = seed;
    this->randGenerator = randGenerator;
}

void RandomGen::set_seed(uint32_t seed)
{
    this->current_seed_value = seed;
}

uint32_t RandomGen::get_next_rand()
{
    switch(this->randGenerator)
    {
        case (XORShift):
            break;
    }
}