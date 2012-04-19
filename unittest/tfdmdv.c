/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: tfdmdv.c
  AUTHOR......: David Rowe  
  DATE CREATED: April 16 2012
                                                                             
  Unit tests for FDMDV modem.  Combination of unit tests perfromed
  entirely by this program and comparisons with reference Octave
  version of the modem that require running an Octave script
  ../octave/tfdmdv.m.
                                                                             
\*---------------------------------------------------------------------------*/


/*
  Copyright (C) 2009 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fdmdv_internal.h"
#include "fdmdv.h"

#define FRAMES 25

void octave_save_int(FILE *f, char name[], int data[], int rows, int cols);
void octave_save_complex(FILE *f, char name[], COMP data[], int rows, int cols);

int main(int argc, char *argv[])
{
    struct FDMDV *fdmdv;
    int           tx_bits[FDMDV_BITS_PER_FRAME];
    COMP          tx_symbols[(NC+1)];
    COMP          tx_baseband[(NC+1)][M];
    COMP          tx_fdm[M];

    int           tx_bits_log[FDMDV_BITS_PER_FRAME*FRAMES];
    COMP          tx_symbols_log[(NC+1)*FRAMES];
    COMP          tx_baseband_log[(NC+1)][M*FRAMES];
    COMP          tx_fdm_log[M*FRAMES];

    FILE         *fout;
    int           f,c,i;

    fdmdv = fdmdv_create();

    for(f=0; f<FRAMES; f++) {
	fdmdv_get_test_bits(fdmdv, tx_bits);
	bits_to_dqpsk_symbols(tx_symbols, fdmdv->prev_tx_symbols, tx_bits, &fdmdv->tx_pilot_bit);
	memcpy(fdmdv->prev_tx_symbols, tx_symbols, sizeof(COMP)*(NC+1));
	tx_filter(tx_baseband, tx_symbols, fdmdv->tx_filter_memory);
	fdm_upconvert(tx_fdm, tx_baseband, fdmdv->phase_tx, fdmdv->freq);
  
	/* save log of outputs */

	memcpy(&tx_bits_log[FDMDV_BITS_PER_FRAME*f], tx_bits, sizeof(int)*FDMDV_BITS_PER_FRAME);
	memcpy(&tx_symbols_log[(NC+1)*f], tx_symbols, sizeof(COMP)*(NC+1));
	for(c=0; c<NC+1; c++)
	    for(i=0; i<M; i++)
		tx_baseband_log[c][f*M+i] = tx_baseband[c][i]; 
	memcpy(&tx_fdm_log[M*f], tx_fdm, sizeof(COMP)*M);
    }

    codec2_destroy(fdmdv);

    /* dump logs to Octave file for evaluation by tfdmdv.m Octave script */

    fout = fopen("tfdmdv_out.txt","wt");
    assert(fout != NULL);
    fprintf(fout, "# Created by tfdmdv.c\n");
    octave_save_int(fout, "tx_bits_log_c", tx_bits_log, 1, FDMDV_BITS_PER_FRAME*FRAMES);
    octave_save_complex(fout, "tx_symbols_log_c", tx_symbols_log, 1, (NC+1)*FRAMES);  
    octave_save_complex(fout, "tx_baseband_log_c", (COMP*)tx_baseband_log, (NC+1), M*FRAMES);  
    octave_save_complex(fout, "tx_fdm_log_c", (COMP*)tx_fdm_log, 1, M*FRAMES);  
    fclose(fout);

    return 0;
}

void octave_save_int(FILE *f, char name[], int data[], int rows, int cols)
{
    int r,c;

    fprintf(f, "# name: %s\n", name);
    fprintf(f, "# type: matrix\n");
    fprintf(f, "# rows: %d\n", rows);
    fprintf(f, "# columns: %d\n", cols);
    
    for(r=0; r<rows; r++) {
	for(c=0; c<cols; c++)
	    fprintf(f, " %d", data[r*cols+c]);
	fprintf(f, "\n");
    }

    fprintf(f, "\n\n");
}

void octave_save_complex(FILE *f, char name[], COMP data[], int rows, int cols)
{
    int r,c;

    fprintf(f, "# name: %s\n", name);
    fprintf(f, "# type: complex matrix\n");
    fprintf(f, "# rows: %d\n", rows);
    fprintf(f, "# columns: %d\n", cols);
    
    for(r=0; r<rows; r++) {
	for(c=0; c<cols; c++)
	    fprintf(f, " (%f,%f)", data[r*cols+c].real, data[r*cols+c].imag);
	fprintf(f, "\n");
    }

    fprintf(f, "\n\n");
}