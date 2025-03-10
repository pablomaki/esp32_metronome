#include "shared_variables.h"
#include "settings.h"
#include "resources.h"

esp_system_state_t system_state = SYSTEM_ON; // System ON/OFF state
uint16_t bpm_selected = BPM_START;           // Baseline bpm
uint16_t bpm_candidate = BPM_START;          // Baseline bpm
uint16_t signature_mode = SIGNATURE_START;   // Baseline bpm
uint8_t current_beat = 1;                    // Starting beat
SemaphoreHandle_t selected_bpm_semaphore = NULL;
SemaphoreHandle_t candidate_bpm_semaphore = NULL;
SemaphoreHandle_t signature_semaphore = NULL;
SemaphoreHandle_t beat_semaphore = NULL;
SemaphoreHandle_t system_state_semaphore = NULL;

esp_err_t init_semaphores(void)
{
    selected_bpm_semaphore = xSemaphoreCreateMutex();
    candidate_bpm_semaphore = xSemaphoreCreateMutex();
    signature_semaphore = xSemaphoreCreateMutex();
    beat_semaphore = xSemaphoreCreateMutex();
    system_state_semaphore = xSemaphoreCreateMutex();

    // Check that semaphore creation succeeded
    if (selected_bpm_semaphore == NULL || candidate_bpm_semaphore == NULL || signature_semaphore == NULL || beat_semaphore == NULL || system_state_semaphore == NULL)
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}

uint8_t get_beat()
{
    uint16_t beat = 1;
    if (xSemaphoreTake(beat_semaphore, portMAX_DELAY) == pdTRUE)
    {
        beat = current_beat;
        xSemaphoreGive(beat_semaphore); // Release the mutex
    }
    return beat;
}

void increment_beat()
{
    if (xSemaphoreTake(beat_semaphore, portMAX_DELAY) == pdTRUE)
    {
        if (current_beat >= signature_modes[get_signature_mode()])
        {
            current_beat = 1;
        }
        else
        {
            current_beat++;
        }
        xSemaphoreGive(beat_semaphore); // Release the mutex
    }
}

void change_signature_mode(void)
{
    if (xSemaphoreTake(signature_semaphore, portMAX_DELAY) == pdTRUE)
    {
        if (signature_mode < SIGNATURE_IMAGES - 1)
        {
            signature_mode++;
        }
        else
        {
            signature_mode = 0;
        }
        xSemaphoreGive(signature_semaphore); // Release the mutex
    }
}

uint16_t get_signature_mode(void)
{
    uint16_t mode = SIGNATURE_START;
    if (xSemaphoreTake(signature_semaphore, portMAX_DELAY) == pdTRUE)
    {
        mode = signature_mode;
        xSemaphoreGive(signature_semaphore); // Release the mutex
    }
    return mode;
}

void change_bpm(uint16_t bpm_delta)
{
    if (xSemaphoreTake(candidate_bpm_semaphore, portMAX_DELAY) == pdTRUE)
    {
        uint16_t new_bpm = bpm_candidate + bpm_delta;
        bpm_candidate = (new_bpm > 999) ? 999 : (new_bpm < 1 ? 1 : new_bpm);
        xSemaphoreGive(candidate_bpm_semaphore); // Release the mutex
    }
}

void select_bpm(void)
{
    if (xSemaphoreTake(selected_bpm_semaphore, portMAX_DELAY) == pdTRUE &&
        xSemaphoreTake(candidate_bpm_semaphore, portMAX_DELAY) == pdTRUE)
    {
        bpm_selected = bpm_candidate;
        xSemaphoreGive(candidate_bpm_semaphore); // Release the mutex
        xSemaphoreGive(selected_bpm_semaphore);  // Release the mutex
    }
}

uint16_t get_selected_bpm(void)
{
    uint16_t bpm = BPM_START;
    if (xSemaphoreTake(selected_bpm_semaphore, portMAX_DELAY) == pdTRUE)
    {
        bpm = bpm_selected;
        xSemaphoreGive(selected_bpm_semaphore); // Release the mutex
    }
    return bpm;
}

uint16_t get_candidate_bpm(void)
{
    uint16_t bpm = BPM_START;
    if (xSemaphoreTake(candidate_bpm_semaphore, portMAX_DELAY) == pdTRUE)
    {
        bpm = bpm_candidate;
        xSemaphoreGive(candidate_bpm_semaphore); // Release the mutex
    }
    return bpm;
}

void reset_candidate_bpm(void)
{
    if (xSemaphoreTake(selected_bpm_semaphore, portMAX_DELAY) == pdTRUE &&
        xSemaphoreTake(candidate_bpm_semaphore, portMAX_DELAY) == pdTRUE)
    {
        bpm_candidate = bpm_selected;
        xSemaphoreGive(candidate_bpm_semaphore); // Release the mutex
        xSemaphoreGive(selected_bpm_semaphore);  // Release the mutex
    }
}

bool bpm_selcted(void)
{
    bool equal = false;
    if (xSemaphoreTake(selected_bpm_semaphore, portMAX_DELAY) == pdTRUE &&
        xSemaphoreTake(candidate_bpm_semaphore, portMAX_DELAY) == pdTRUE)
    {
        equal = (bpm_selected == bpm_candidate);
        xSemaphoreGive(candidate_bpm_semaphore); // Release the mutex
        xSemaphoreGive(selected_bpm_semaphore);  // Release the mutex
    }
    return equal;
}

esp_system_state_t get_system_state(void)
{
    esp_system_state_t state = SYSTEM_ON;
    if (xSemaphoreTake(system_state_semaphore, portMAX_DELAY) == pdTRUE)
    {
        state = system_state;
        xSemaphoreGive(system_state_semaphore);  // Release the mutex
    }
    return state;
}

void switch_system_off(void)
{
    if (xSemaphoreTake(system_state_semaphore, portMAX_DELAY) == pdTRUE)
    {
        system_state = SYSTEM_OFF;
        xSemaphoreGive(system_state_semaphore);  // Release the mutex
    }
}

void switch_system_on(void)
{    
    if (xSemaphoreTake(system_state_semaphore, portMAX_DELAY) == pdTRUE)
    {
        system_state = SYSTEM_ON;
        xSemaphoreGive(system_state_semaphore);  // Release the mutex
    }
}