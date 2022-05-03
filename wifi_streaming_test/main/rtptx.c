#include "rtptx.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "string.h"


rtp_session_t sessions[RTP_SESSION_NUM_MAX];

static const char *RTP_TAG = "rtptx";


esp_err_t rtp_session_init(rtp_session_num_t session, rtp_pt_t payload_type)
{
  if(session >= RTP_SESSION_NUM_MAX) {
    ESP_LOGE(RTP_TAG,"session error");
    return ESP_FAIL;
  }
  if(sessions[session].initialized == 1) {
    ESP_LOGE(RTP_TAG,"Session already initialized");
    return ESP_FAIL;
  }
  uint32_t ssrc_id = 33144096;  // synchronization source (SSRC) identifier
  uint8_t _pt = payload_type == L16 ? 96 : payload_type == L24 ? 100 : 99;
  rtp_header_t header = {
    .ver = 2,
    .pad = 0,
    .ext = 0,
    .csrcc = 0,
    .mark = 0,
    .ptype = _pt,
    .seq = 1000,
    .timestamp = (uint32_t)esp_timer_get_time(),
    .ssrc = ssrc_id
  };
  rtp_session_t ses = {
    .initialized = 1,
    .header = header
  };
  sessions[session] = ses;
  return ESP_OK;
}

size_t rtp_serialize(rtp_header_t *header, uint8_t *payload, size_t length, uint8_t *rtp_packet)
{
  if(length == 0) {
    return 0;
  }
  rtp_packet[0] = (header->ver << 6) | (0 << 5) | (header->ext << 4) | (header->csrcc); 
  rtp_packet[1] = (header->mark << 7) | (header->ptype);
  rtp_packet[2] = header->seq >> 8;
  rtp_packet[3] = header->seq;
  rtp_packet[4] = header->timestamp >> 24;
  rtp_packet[5] = header->timestamp >> 16;
  rtp_packet[6] = header->timestamp >> 8;
  rtp_packet[7] = header->timestamp;
  rtp_packet[8] = header->ssrc >> 24;
  rtp_packet[9] = header->ssrc >> 16;
  rtp_packet[10] = header->ssrc >> 8;
  rtp_packet[11] = header->ssrc;
  memcpy(rtp_packet+12,payload,length);
  return 12+length;
}

size_t rtp_create_packet(rtp_session_num_t session, uint8_t *payload, size_t length, uint8_t *output)
{
  rtp_session_t *ses = &sessions[session];
  
  if(ses->initialized == 1) {
    ses->header.mark = 1;
    ses->initialized = 2;
  }
  else {
    ses->header.mark = 0;
  }

  ses->header.seq++;
  ses->header.timestamp = (uint32_t)(esp_timer_get_time()/1000);
  // ses->header.timestamp += 1;  // hard coded timestamp increment
  size_t ret = rtp_serialize(&ses->header, payload, length, output);
  return ret;
}

void rtp_to16BE(uint8_t *in_buf, uint8_t *out_buf, size_t samples_num)
{
  uint16_t *in_sample = (uint16_t *)in_buf;

  uint32_t out_index = 0;
  for(int i=0; i<samples_num; i++) {
    out_buf[out_index++] = (in_sample[i] >> 8) & 0xff;
    out_buf[out_index++] = (in_sample[i]) & 0xff;
  }
}


