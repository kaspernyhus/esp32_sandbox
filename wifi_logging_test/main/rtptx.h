/*
 *  B&O Classics Adaptor
 *
 *  RTP transmit module
 *
 *  LAPJ/KANK
 *  2022
 * 
*/

#pragma once

#include <stdint.h>
#include "esp_err.h"


#define RTP_HEADER_LEN 12

typedef struct {
  uint8_t ver   : 2;  // version
  uint8_t pad   : 1;  // padding
  uint8_t ext   : 1;  // extension bit
  uint8_t csrcc : 4;  // CSRC count
  uint8_t mark  : 1;  // marker bit
  uint8_t ptype : 7;  // payload type
  uint16_t seq;       // sequence number
  uint32_t timestamp; // timestamp
  uint32_t ssrc;      // synchronization source (SSRC) identifier
} __attribute__((packed)) rtp_header_t;


typedef struct {
  uint8_t initialized;
  rtp_header_t header;
} rtp_session_t;


// Payload type
// https://en.wikipedia.org/wiki/RTP_payload_formats
typedef enum {
  L16,
  L24,
} rtp_pt_t;


// RTP session number
typedef enum {
  RTP_SESSION_NUM_0 = 0,
  RTP_SESSION_NUM_1,
  RTP_SESSION_NUM_2,
  RTP_SESSION_NUM_3,
  RTP_SESSION_NUM_MAX
} rtp_session_num_t;


void rtp_to16BE(uint8_t *in_buf, uint8_t *out_buf, size_t samples_num);
esp_err_t rtp_session_init(rtp_session_num_t session, rtp_pt_t payload_type);
size_t rtp_create_packet(rtp_session_num_t session, uint8_t *payload, size_t length, uint8_t *output);