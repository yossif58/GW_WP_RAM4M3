/*
 * kk_srec.h: A simple library for reading Motorola SREC hex files.
 * Intended mainly for embedded systems, and thus somewhat optimised
 * for size at the expense of error handling and elegance.
 *
 *
 *      USAGE
 *      -----
 * 
 * To read an SREC ASCII file into binary data, the library is used
 * as follows:
 *
 *      struct srec_state srec;
 *      srec_begin_read(srec);
 *      srec_read_bytes(&srec, my_input_bytes, length_of_my_input_bytes);
 *      srec_end_read(&srec);
 *
 * The function `srec_read_bytes` may be called any number of times with
 * any amount of data at a time (i.e., it does not need to be called with
 * full lines at a time). At any time during a call to `srec_read_bytes`
 * or `srec_end_read`, the callback function `srec_data_read` may be
 * called by the library.
 *
 * The callback `srec_data_read` must be provided by the user, e.g.,
 * as follows:
 *
 *      void srec_data_read (struct srec_state *srec,
 *                           srec_record_number_t record_type,
 *                           srec_address_t address,
 *                           uint8_t *data, srec_count_t length,
 *                           srec_bool_t checksum_error) {
 *          if (!checksum_error && SREC_IS_DATA(record_type)) {
 *              (void) fseek(outfile, address, SEEK_SET);
 *              (void) fwrite(data, 1, length, outfile);
 *          } else if (SREC_IS_TERMINATION(record_type)) {
 *              (void) fclose(outfile);
 *          }
 *      }
 *
 * For a more complete implementation, see `srec2bin.c`.
 * 
 *
 * Copyright (c) 2015 Kimmo Kulovesi, http://arkku.com/
 * Provided with absolutely no warranty, use at your own risk only.
 * Use and distribute freely, mark modified copies as such.
 */

#ifndef KK_SREC_H
#define KK_SREC_H
#include <stdint.h>

void PrepareFlashProgramming(void);
void RM_MCUBOOT_PORT_BootApp (void);
void ClearflashBlock_Data(void);
fsp_err_t ReadBlockOtap(int argc, uint8_t *buf_in);

#endif // !KK_SREC_H
