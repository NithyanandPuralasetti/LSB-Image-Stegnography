#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "common.h"
#include "encode.h"

/* Master controller function for decoding */
Status do_decoding(EncodeInfo *encInfo);

/* Reconstructs integer or character from LSBs of carrier buffer */
long int decode_data_from_lsb(char *buffer, int size);

#endif