/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "linux/string.h"
#include "match_table.h"

int reg2type(struct match_reg_type *table, int length, int reg, int def)
{
    while (length-- > 0) {
        if (table->reg == reg)
            return table->type;
        table++;
    }
    return def;
}

int type2reg(struct match_reg_type *table, int length, int type, int def)
{
    while (length-- > 0) {
        if (table->type == type)
            return table->reg;
        table++;
    }
    return def;
}

int str2type(struct match_type_str *table, int length, const char *str,
         int size, int def)
{
    while (length-- > 0) {
        if (!strncmp(table->str, str, size))
            return table->type;
        table++;
    }
    return def;
}

const char *type2str(struct match_type_str *table, int length, int type,
             const char *def)
{
    while (length-- > 0) {
        if (table->type == type)
            return table->str;
        table++;
    }
    return def;
}

int match_reg_to_type(struct match_t *table, int nr_table, int reg, int def)
{
    while (nr_table-- > 0) {
        if (table->reg == reg)
            return table->type;
        table++;
    }
    return def;
}

int match_type_to_reg(struct match_t *table, int nr_table, int type, int def)
{
    while (nr_table-- > 0) {
        if (table->type == type)
            return table->reg;
        table++;
    }
    return def;
}

int match_data_to_type(struct match_t *table, int nr_table, char *data,
        int size, int def)
{
    while (nr_table-- > 0) {
        if (!memcmp(table->data, data, size))
            return table->type;
        table++;
    }
    return def;
}

void *match_type_to_data(struct match_t *table, int nr_table, int type,
             void *def)
{
    while (nr_table-- > 0) {
        if (table->type == type)
            return table->data;
        table++;
    }
    return def;
}
