/*
 * random.h
 *
 *  Created on: Dec 1, 2013
 *      Author: mmm
 */

#pragma once

#include <stdint.h>

enum randGenerators {XORShift=1};

class RandomGen
{
    private:
        uint32_t current_seed_value;
        uint32_t randGenerator;
    public:
        RandomGen(uint32_t seed, uint32_t randGenerator);
        ~RandomGen();

        void set_seed(uint32_t seed);
        uint32_t get_next_rand();


};

