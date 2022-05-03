#include "pcm1863.h"
#include "driver/i2c.h"
#include "string.h"


static const char *PCM_TAG = "pcm1863";


void pcm1863_reset(pcm1863_t *pcm1863)
{
    pcm1863_register_write_byte(pcm1863,0x00,0xFE);
    ESP_LOGI(PCM_TAG,"pcm1863 reset!");
}


esp_err_t pcm1863_init(pcm1863_t *pcm1863, uint8_t device_address, uint8_t i2c_channel, uint32_t i2c_trans_buf_sz, uint16_t timeout)
{
    if(device_address != 0x94) { // There can max be two devices on the same bus.
        ESP_LOGE(PCM_TAG,"Invalid devide address - must be either 0x94 or 0x96");
        return -1;
    }
    pcm1863->device_address = device_address;
    pcm1863->i2c_channel = i2c_channel;
    pcm1863->i2c_trans_buf_sz = i2c_trans_buf_sz;
    pcm1863->i2c_timeout = timeout;

    pcm1863_reset(pcm1863);

    /* set GPIO2 to output, GPIO3 input */
    pcm1863_register_write_byte(pcm1863,PCM1863_GPIO3_2_CTRL,0x00);
    pcm1863_register_write_byte(pcm1863,PCM1863_GPIO3_2_DIR_CTRL,0x04);
    pcm1863_register_update_bits(pcm1863,PCM1863_GPIO_IN_OUT,0x40,0x40);

    // uint8_t reg;
    // pcm1863_register_read(pcm1863,PCM1863_GPIO3_2_DIR_CTRL,&reg,1);
    // ESP_LOGI(PCM_TAG,"reg %.2X",reg);

    pcm1863_get_power_status(pcm1863);
    pcm1863_get_mode(pcm1863);

    return 0;
}


esp_err_t i2c_master_w_r_device(pcm1863_t *pcm1863, const uint8_t* write_buffer, size_t write_size, uint8_t* read_buffer, size_t read_size)
{
    esp_err_t err = ESP_OK;
    uint8_t buffer[(pcm1863->i2c_trans_buf_sz)];
    memset(buffer,0,sizeof(buffer));
    TickType_t ticks_to_wait = pcm1863->i2c_timeout / portTICK_RATE_MS;

    i2c_cmd_handle_t handle = i2c_cmd_link_create_static(buffer, sizeof(buffer));
    assert (handle != NULL);

    err = i2c_master_start(handle);
    if (err != ESP_OK)
        goto end;
    err = i2c_master_write_byte(handle, pcm1863->device_address | I2C_MASTER_WRITE, true);
    if (err != ESP_OK)
        goto end;
    err = i2c_master_write(handle, write_buffer, write_size, true);
    if (err != ESP_OK)
        goto end;
    err = i2c_master_start(handle);
    if (err != ESP_OK)
        goto end;
    err = i2c_master_write_byte(handle, pcm1863->device_address | I2C_MASTER_READ, true);
    if (err != ESP_OK)
        goto end;
    err = i2c_master_read(handle, read_buffer, read_size, I2C_MASTER_LAST_NACK);
    if (err != ESP_OK)
        goto end;
    i2c_master_stop(handle);
    err = i2c_master_cmd_begin(pcm1863->i2c_channel, handle, ticks_to_wait);
end:
    i2c_cmd_link_delete_static(handle);
    return err;
}


esp_err_t i2c_master_w_device(pcm1863_t *pcm1863, const uint8_t* write_buffer, size_t write_size)
{
    esp_err_t err = ESP_OK;
    uint8_t buffer[pcm1863->i2c_trans_buf_sz];
    memset(buffer,0,sizeof(buffer));
    TickType_t ticks_to_wait = pcm1863->i2c_timeout / portTICK_RATE_MS;

    i2c_cmd_handle_t handle = i2c_cmd_link_create_static(buffer, sizeof(buffer));
    assert (handle != NULL);

    err = i2c_master_start(handle);
    if (err != ESP_OK)
        goto end;
    err = i2c_master_write_byte(handle, pcm1863->device_address | I2C_MASTER_WRITE, true);
    if (err != ESP_OK)
        goto end;
    err = i2c_master_write(handle, write_buffer, write_size, true);
    if (err != ESP_OK)
        goto end;
    i2c_master_stop(handle);
    err = i2c_master_cmd_begin(pcm1863->i2c_channel, handle, ticks_to_wait);
end:
    i2c_cmd_link_delete_static(handle);
    return err;
}

esp_err_t pcm1863_register_update_bits(pcm1863_t *pcm1863, uint8_t reg_addr, uint8_t mask, uint8_t val)
{
    uint8_t reg;
    pcm1863_register_read(pcm1863,reg_addr,&reg,1);
    return pcm1863_register_write_byte(pcm1863,reg_addr,reg|(val&mask));
}

void pcm1863_change_page(pcm1863_t *pcm1863, size_t page)
{
    pcm1863_register_write_byte(pcm1863,0x00,page);
    ESP_LOGI(PCM_TAG,"pcm1863 page:%d", page);
}


/**
 * @brief Read a sequence of bytes from PCM1863 registers
 */
esp_err_t pcm1863_register_read(pcm1863_t *pcm1863, uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_w_r_device(pcm1863, &reg_addr, 1, data, len);
}

/**
 * @brief Write a byte to PCM1863
 */
esp_err_t pcm1863_register_write_byte(pcm1863_t *pcm1863, uint8_t reg_addr, uint8_t data)
{
    int ret = ESP_OK;
    uint8_t write_buf[2] = {reg_addr, data};
    ret = i2c_master_w_device(pcm1863, write_buf, sizeof(write_buf));
    return ret;
}

esp_err_t pcm1863_set_bit_depth(pcm1863_t *pcm1863, pcm1863_bit_depth_t bit_depth)
{
    return pcm1863_register_update_bits(pcm1863,PCM1863_FORMAT_REG,0x0C,(bit_depth << 2));
}

esp_err_t pcm1863_set_clk_source(pcm1863_t *pcm1863, pcm1863_clk_source clk_source)
{
    return pcm1863_register_update_bits(pcm1863,PCM1863_CLK_SOURCE,0xC0,(clk_source << 6));
}

esp_err_t pcm1863_set_mode(pcm1863_t *pcm1863, size_t mode)
{
    esp_err_t ret = pcm1863_register_update_bits(pcm1863,PCM1863_CLK_SOURCE,0x10,(mode << 4));
    pcm1863_get_mode(pcm1863);
    return ret;
}

void pcm1863_get_power_status(pcm1863_t *pcm1863)
{
    uint8_t reg_data;
    pcm1863_register_read(pcm1863,PCM1863_POWER_STATUS_REG,&reg_data,1);
    if(reg_data == 0x07)
        ESP_LOGI(PCM_TAG,"Power OK!");
    else
        ESP_LOGE(PCM_TAG,"Power NOT ok!");
}

void pcm1863_get_samplerate(pcm1863_t *pcm1863)
{
    uint8_t reg_data;
    pcm1863_register_read(pcm1863,PCM1863_INFO_REG,&reg_data,1);
    switch (reg_data) {
    case 0x00:
        ESP_LOGI(PCM_TAG,"Sample rate: Out of range (Low) or LRCK Halt");
        break;
    case 0x01:
        ESP_LOGI(PCM_TAG,"Sample rate: 8kHz");
        break;
    case 0x02:
        ESP_LOGI(PCM_TAG,"Sample rate: 16kHz");
        break;
    case 0x03:
        ESP_LOGI(PCM_TAG,"Sample rate: 32kHz to 48kHz");
        break;
    case 0x04:
        ESP_LOGI(PCM_TAG,"Sample rate: 88.2kHz to 96kHz");
        break;
    case 0x05:
        ESP_LOGI(PCM_TAG,"Sample rate: 176.4kHz to 192kHz");
        break;
    case 0x06:
        ESP_LOGI(PCM_TAG,"Sample rate: Out of range (High)");
        break;
    default:
        ESP_LOGI(PCM_TAG,"Sample rate: Invalid");
        break;
    }
}

void pcm1863_get_bitdepth(pcm1863_t *pcm1863)
{
    uint8_t reg_data;
    pcm1863_register_read(pcm1863,PCM1863_FORMAT_REG,&reg_data,1);
    switch ((reg_data>>2)&0x03) {
    case 0x00:
        ESP_LOGI(PCM_TAG,"Stereo PCM Word Length: 32bit");
        break;
    case 0x01:
        ESP_LOGI(PCM_TAG,"Stereo PCM Word Length: 24bit");
        break;
    case 0x02:
        ESP_LOGI(PCM_TAG,"Stereo PCM Word Length: 20bit");
        break;
    case 0x03:
        ESP_LOGI(PCM_TAG,"Stereo PCM Word Length: 16bit");
        break;
    default:
        ESP_LOGI(PCM_TAG,"Stereo PCM Word Length: error reading");
        break;
    }
}

void pcm1863_get_device_state(pcm1863_t *pcm1863)
{
    uint8_t reg_data;
    pcm1863_register_read(pcm1863,PCM1863_STATE,&reg_data,1);
    switch (reg_data&0xFF) {
    case 0x00:
        ESP_LOGI(PCM_TAG,"Device State: Power down (pp. 126)");
        break;
    case 0x01:
        ESP_LOGE(PCM_TAG,"Device State: Wait clock stable (pp. 126)");
        break;
    case 0x02:
        ESP_LOGI(PCM_TAG,"Device State: Release reset (pp. 126)");
        break;
    case 0x03:
        ESP_LOGI(PCM_TAG,"Device State: Standby (pp. 126)");
        break;      
    case 0x09:
        ESP_LOGI(PCM_TAG,"Device State: Sleep (pp. 126)");
        break;
    case 0x0F:
        ESP_LOGI(PCM_TAG,"Device State: Run");
        break;  
    default:
        break;
    }
}

void pcm1863_get_clk_state(pcm1863_t *pcm1863)
{
    uint8_t reg_data;
    pcm1863_register_read(pcm1863,PCM1863_CLK_ERR_STAT,&reg_data,1);
    if(reg_data&0x01) 
        ESP_LOGE(PCM_TAG,"SCK error (pp. 128)");
    if(reg_data&0x02) 
        ESP_LOGE(PCM_TAG,"BCK error (pp. 128)");
    if(reg_data&0x04)
        ESP_LOGE(PCM_TAG,"LRCK error (pp. 128)");
    if(reg_data&0x10) 
        ESP_LOGI(PCM_TAG,"SCK halt (pp. 128)");
    if(reg_data&0x20) 
        ESP_LOGI(PCM_TAG,"BCK halt (pp. 128)");
    if(reg_data&0x40)
        ESP_LOGI(PCM_TAG,"LRCK halt (pp. 128)");
    if(reg_data == 0x00)
        ESP_LOGI(PCM_TAG,"All clocks OK!");
}

void pcm1863_get_clk_ratio(pcm1863_t *pcm1863)
{
    uint8_t reg;
    pcm1863_register_read(pcm1863,PCM1863_CLK_RATIO,&reg,1);
    switch (reg&0x07) {
        case 0x000:
            ESP_LOGE(PCM_TAG,"Current SCK Ratio: Out of range (L) or BCK Halt"); 
            break;
        case 0x001:
            ESP_LOGI(PCM_TAG,"Current SCK Ratio: 128");
            break;
        case 0x002:
            ESP_LOGI(PCM_TAG,"Current SCK Ratio: 256");
            break;
        case 0x003:
            ESP_LOGI(PCM_TAG,"Current SCK Ratio: 384");
            break;
        case 0x100:
            ESP_LOGI(PCM_TAG,"Current SCK Ratio: 512");
            break;
        case 0x101:
            ESP_LOGI(PCM_TAG,"Current SCK Ratio: 768");
            break;
        case 0x110:
            ESP_LOGE(PCM_TAG,"Current SCK Ratio: Out of range (H)");
            break;
        case 0x111:
            ESP_LOGE(PCM_TAG,"Current SCK Ratio: Invalid SCK or LRCK Halt");
            break;
        default:
            break;
    }

    switch ((reg>>4)&0x07) {
        case 0x000:
            ESP_LOGE(PCM_TAG,"Current Receiving BCK Ratio: Out of range (L) or BCK Halt"); 
            break;
        case 0x001:
            ESP_LOGI(PCM_TAG,"Current Receiving BCK Ratio: 32");
            break;
        case 0x002:
            ESP_LOGI(PCM_TAG,"Current Receiving BCK Ratio: 48");
            break;
        case 0x003:
            ESP_LOGI(PCM_TAG,"Current Receiving BCK Ratio: 64");
            break;
        case 0x100:
            ESP_LOGI(PCM_TAG,"Current Receiving BCK Ratio: 256");
            break;
        case 0x101:
            ESP_LOGI(PCM_TAG,"Current Receiving BCK Ratio: Not assigned");
            break;
        case 0x110:
            ESP_LOGE(PCM_TAG,"Current Receiving BCK Ratio: Out of range (H)");
            break;
        case 0x111:
            ESP_LOGE(PCM_TAG,"Current Receiving BCK Ratio: Invalid BCK or LRCK Halt");
            break;
        default:
            break;
    }
}

void pcm1863_get_mode(pcm1863_t *pcm1863)
{
    uint8_t reg_data;
    pcm1863_register_read(pcm1863,PCM1863_CLK_SOURCE,&reg_data,1);
    if((reg_data & 0x10) == 0 )
        ESP_LOGI(PCM_TAG, "Mode: Slave");
    else
        ESP_LOGI(PCM_TAG, "Mode: Master");
}