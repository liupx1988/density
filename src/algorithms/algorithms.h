/*
 * Centaurean Density
 *
 * Copyright (c) 2015, Guillaume Voirin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     1. Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *
 *     3. Neither the name of the copyright holder nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 3/02/15 19:51
 */

#ifndef DENSITY_ALGORITHMS_H
#define DENSITY_ALGORITHMS_H

#include "../globals.h"

#define DENSITY_ALGORITHMS_MULTIPLIER_64                0x88c65c9962e277c1llu
#define DENSITY_ALGORITHMS_INITIAL_DICTIONARY_KEY_BITS  8u
#define DENSITY_ALGORITHMS_MAX_DICTIONARY_KEY_BITS      16u

typedef enum {
    DENSITY_ALGORITHMS_EXIT_STATUS_FINISHED = 0,
    DENSITY_ALGORITHMS_EXIT_STATUS_ERROR_DURING_PROCESSING,
    DENSITY_ALGORITHMS_EXIT_STATUS_INPUT_STALL,
    DENSITY_ALGORITHMS_EXIT_STATUS_OUTPUT_STALL
} density_algorithm_exit_status;

typedef struct {
    void *dictionary;
    uint_fast8_t copy_penalty;
    uint_fast8_t copy_penalty_start;
    bool previous_incompressible;
    uint_fast64_t counter;
} density_algorithm_state;

#define DENSITY_ALGORITHM_FAST_COPY(NUMBER_OF_BYTES)\
    DENSITY_FAST_COPY_ARRAY_8(out_array, in_array, out_position, in_position, NUMBER_OF_BYTES);\
    out_position += (NUMBER_OF_BYTES);\
    in_position += (NUMBER_OF_BYTES);

#define DENSITY_ALGORITHM_COPY(NUMBER_OF_BYTES)\
    DENSITY_MEMCPY(*out, *in, NUMBER_OF_BYTES);\
    *in += (NUMBER_OF_BYTES);\
    *out += (NUMBER_OF_BYTES);

#define DENSITY_ALGORITHM_INCREASE_COPY_PENALTY_START\
    if(!(--state->copy_penalty))\
        state->copy_penalty_start++;

#define DENSITY_ALGORITHM_DECREASE_COPY_PENALTY_START\
    if (state->copy_penalty_start & ~0x1)\
        state->copy_penalty_start >>= 1;

#define DENSITY_ALGORITHM_TEST_INCOMPRESSIBILITY(SPAN, NUMBER_OF_BYTES)\
    if (DENSITY_UNLIKELY((SPAN) & ~((NUMBER_OF_BYTES) - 1))) {\
        if (state->previous_incompressible)\
            state->copy_penalty = state->copy_penalty_start;\
        state->previous_incompressible = true;\
    } else\
        state->previous_incompressible = false;

#define DENSITY_ALGORITHM_INCOMPRESSIBLE_PROTECTION_FUNCTION_START(POSITION, BYTES) \
    if (DENSITY_UNLIKELY(!(state->counter & 0x3))) {\
        DENSITY_ALGORITHM_DECREASE_COPY_PENALTY_START;\
    }\
    state->counter++;\
    if (/*DENSITY_UNLIKELY(*/state->copy_penalty/*)*/) {\
        DENSITY_ALGORITHM_FAST_COPY(BYTES);\
        DENSITY_ALGORITHM_INCREASE_COPY_PENALTY_START;\
    } else {\
        const uint_fast64_t marker = POSITION;

#define DENSITY_ALGORITHM_INCOMPRESSIBLE_PROTECTION_FUNCTION_END(POSITION, BYTES) \
        DENSITY_ALGORITHM_TEST_INCOMPRESSIBILITY(((POSITION) - marker), BYTES);\
    }

#define DENSITY_ALGORITHM_READ_PROTECTION(DISTANCE, EXIT_LABEL) \
    if(in_size < (DISTANCE)) {\
        goto DENSITY_EVAL(EXIT_LABEL);\
    } else {\
        in_limit = in_size - (DISTANCE);\
    }

#define DENSITY_ALGORITHMS_MULTIPLY_SHIFT_64(UNIT, HASH_BYTES)  (((UNIT) * DENSITY_ALGORITHMS_MULTIPLIER_64) >> (64 - ((HASH_BYTES) << 3)))
#define DENSITY_ALGORITHMS_EXTRACT_64(MEM_64, BYTE_GROUP_SIZE)  ((MEM_64) & (0xffffffffffffffffllu >> (64 - ((BYTE_GROUP_SIZE) << 3))))
#define DENSITY_ALGORITHMS_TRANSITION_UNROLL    4
#define DENSITY_ALGORITHMS_TRANSITION_ROUNDS(HASH_BYTES, NEXT_HASH_BYTES)   ((NEXT_HASH_BYTES) > (HASH_BYTES) ? (((1 << ((NEXT_HASH_BYTES) << 3)) - (1 << ((HASH_BYTES) << 3))) >> 6) : ((1 << ((HASH_BYTES) >> 3)) >> (1 << (HASH_BYTES))))

DENSITY_WINDOWS_EXPORT void density_algorithms_reset_state(density_algorithm_state *DENSITY_RESTRICT_DECLARE);
DENSITY_WINDOWS_EXPORT void density_algorithms_prepare_state(density_algorithm_state *DENSITY_RESTRICT_DECLARE, void *DENSITY_RESTRICT_DECLARE);

#endif
