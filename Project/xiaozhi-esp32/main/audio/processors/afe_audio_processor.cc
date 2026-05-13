#include "afe_audio_processor.h"

#include <esp_heap_caps.h>
#include <esp_log.h>

#define PROCESSOR_RUNNING 0x01

#define TAG "AfeAudioProcessor"

namespace {
constexpr int kAfeFetchTaskStackSize = 4096;
constexpr int kAfeWorkerCore = 1;
constexpr int kAfeWorkerPriority = 5;
constexpr int kAfeRingBufferFrames = 50;
}

AfeAudioProcessor::AfeAudioProcessor()
    : afe_data_(nullptr) {
    event_group_ = xEventGroupCreate();
}

AfeAudioProcessor::~AfeAudioProcessor() {
    if (afe_data_ != nullptr) {
        afe_iface_->destroy(afe_data_);
    }

    if (audio_processor_task_buffer_ != nullptr) {
        heap_caps_free(audio_processor_task_buffer_);
    }

    if (audio_processor_task_stack_ != nullptr) {
        heap_caps_free(audio_processor_task_stack_);
    }

    vEventGroupDelete(event_group_);
}

void AfeAudioProcessor::Initialize(AudioCodec* codec, int frame_duration_ms, srmodel_list_t* models_list) {
    codec_ = codec;
    frame_samples_ = frame_duration_ms * 16000 / 1000;

    // Pre-allocate output buffer capacity
    output_buffer_.reserve(frame_samples_);

    int ref_num = codec_->input_reference() ? 1 : 0;

    std::string input_format;
    for (int i = 0; i < codec_->input_channels() - ref_num; i++) {
        input_format.push_back('M');
    }
    for (int i = 0; i < ref_num; i++) {
        input_format.push_back('R');
    }

    srmodel_list_t *models;
    if (models_list == nullptr) {
        models = esp_srmodel_init("model");
    } else {
        models = models_list;
    }

    char* ns_model_name = esp_srmodel_filter(models, ESP_NSNET_PREFIX, NULL);
    char* vad_model_name = esp_srmodel_filter(models, ESP_VADN_PREFIX, NULL);
    
    afe_config_t* afe_config = afe_config_init(input_format.c_str(), NULL, AFE_TYPE_VC, AFE_MODE_HIGH_PERF);
    afe_config->aec_mode = AEC_MODE_VOIP_HIGH_PERF;
    afe_config->vad_mode = VAD_MODE_0;
    afe_config->vad_min_noise_ms = 100;
    if (vad_model_name != nullptr) {
        afe_config->vad_model_name = vad_model_name;
    }

    if (ns_model_name != nullptr) {
        afe_config->ns_init = true;
        afe_config->ns_model_name = ns_model_name;
        afe_config->afe_ns_mode = AFE_NS_MODE_NET;
    } else {
        afe_config->ns_init = false;
    }

    afe_config->agc_init = false;
    afe_config->afe_perferred_core = kAfeWorkerCore;
    afe_config->afe_perferred_priority = kAfeWorkerPriority;
    afe_config->afe_ringbuf_size = kAfeRingBufferFrames;
    afe_config->memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM;

#ifdef CONFIG_USE_DEVICE_AEC
    afe_config->aec_init = true;
    afe_config->vad_init = false;
#else
    afe_config->aec_init = false;
    afe_config->vad_init = true;
#endif

    afe_iface_ = esp_afe_handle_from_config(afe_config);
    afe_data_ = afe_iface_->create_from_config(afe_config);
    afe_config_free(afe_config);

    if ((afe_iface_ == nullptr) || (afe_data_ == nullptr)) {
        ESP_LOGE(TAG, "Failed to create AFE audio processor instance");
        afe_iface_ = nullptr;
        afe_data_ = nullptr;
        return;
    }

    if (audio_processor_task_stack_ == nullptr) {
        audio_processor_task_stack_ = static_cast<StackType_t*>(
            heap_caps_malloc(kAfeFetchTaskStackSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    }

    if (audio_processor_task_buffer_ == nullptr) {
        audio_processor_task_buffer_ = static_cast<StaticTask_t*>(
            heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    }

    if ((audio_processor_task_stack_ == nullptr) || (audio_processor_task_buffer_ == nullptr)) {
        ESP_LOGE(TAG,
                 "Failed to allocate AFE fetch task stack/buffer (free_sram=%u largest_internal=%u)",
                 static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)),
                 static_cast<unsigned int>(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)));
        if (audio_processor_task_buffer_ != nullptr) {
            heap_caps_free(audio_processor_task_buffer_);
            audio_processor_task_buffer_ = nullptr;
        }
        if (audio_processor_task_stack_ != nullptr) {
            heap_caps_free(audio_processor_task_stack_);
            audio_processor_task_stack_ = nullptr;
        }
        afe_iface_->destroy(afe_data_);
        afe_data_ = nullptr;
        afe_iface_ = nullptr;
        return;
    }

#if CONFIG_FREERTOS_NUMBER_OF_CORES > 1
    audio_processor_task_ = xTaskCreateStaticPinnedToCore([](void* arg) {
        auto this_ = (AfeAudioProcessor*)arg;
        this_->AudioProcessorTask();
        vTaskDelete(NULL);
    }, "audio_communication", kAfeFetchTaskStackSize, this, kAfeWorkerPriority,
       audio_processor_task_stack_, audio_processor_task_buffer_, kAfeWorkerCore);
#else
    audio_processor_task_ = xTaskCreateStatic([](void* arg) {
        auto this_ = (AfeAudioProcessor*)arg;
        this_->AudioProcessorTask();
        vTaskDelete(NULL);
    }, "audio_communication", kAfeFetchTaskStackSize, this, kAfeWorkerPriority,
       audio_processor_task_stack_, audio_processor_task_buffer_);
#endif

    if (audio_processor_task_ == nullptr) {
        ESP_LOGE(TAG,
                 "Failed to create AFE fetch task (free_sram=%u largest_internal=%u)",
                 static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)),
                 static_cast<unsigned int>(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)));
        heap_caps_free(audio_processor_task_buffer_);
        heap_caps_free(audio_processor_task_stack_);
        audio_processor_task_buffer_ = nullptr;
        audio_processor_task_stack_ = nullptr;
        afe_iface_->destroy(afe_data_);
        afe_data_ = nullptr;
        afe_iface_ = nullptr;
        return;
    }

    ESP_LOGI(TAG, "AFE fetch task created with PSRAM stack, size=%d", kAfeFetchTaskStackSize);
}

size_t AfeAudioProcessor::GetFeedSize() {
    if (afe_data_ == nullptr) {
        return 0;
    }
    return afe_iface_->get_feed_chunksize(afe_data_);
}

void AfeAudioProcessor::Feed(std::vector<int16_t>&& data) {
    if (afe_data_ == nullptr) {
        return;
    }

    /* Ignore late feed blocks after Stop() so the fetch task does not wake up to a refilled
     * FEED ringbuffer from the previous voice-processing session. */
    if (!IsRunning()) {
        return;
    }

    afe_iface_->feed(afe_data_, data.data());
}

void AfeAudioProcessor::Start() {
    xEventGroupSetBits(event_group_, PROCESSOR_RUNNING);
}

void AfeAudioProcessor::Stop() {
    xEventGroupClearBits(event_group_, PROCESSOR_RUNNING);
    output_buffer_.clear();
    is_speaking_ = false;

    if (afe_data_ != nullptr) {
        /* Drop any buffered PCM immediately so the next session does not inherit stale AFE data. */
        afe_iface_->reset_buffer(afe_data_);
    }
}

bool AfeAudioProcessor::IsRunning() {
    return xEventGroupGetBits(event_group_) & PROCESSOR_RUNNING;
}

void AfeAudioProcessor::OnOutput(std::function<void(std::vector<int16_t>&& data)> callback) {
    output_callback_ = callback;
}

void AfeAudioProcessor::OnVadStateChange(std::function<void(bool speaking)> callback) {
    vad_state_change_callback_ = callback;
}

void AfeAudioProcessor::AudioProcessorTask() {
    auto fetch_size = afe_iface_->get_fetch_chunksize(afe_data_);
    auto feed_size = afe_iface_->get_feed_chunksize(afe_data_);
    ESP_LOGI(TAG, "Audio communication task started, feed size: %d fetch size: %d",
        feed_size, fetch_size);

    while (true) {
        xEventGroupWaitBits(event_group_, PROCESSOR_RUNNING, pdFALSE, pdTRUE, portMAX_DELAY);

        auto res = afe_iface_->fetch_with_delay(afe_data_, portMAX_DELAY);
        if ((xEventGroupGetBits(event_group_) & PROCESSOR_RUNNING) == 0) {
            continue;
        }
        if (res == nullptr || res->ret_value == ESP_FAIL) {
            if (res != nullptr) {
                ESP_LOGI(TAG, "Error code: %d", res->ret_value);
            }
            continue;
        }

        // VAD state change
        if (vad_state_change_callback_) {
            if (res->vad_state == VAD_SPEECH && !is_speaking_) {
                is_speaking_ = true;
                vad_state_change_callback_(true);
            } else if (res->vad_state == VAD_SILENCE && is_speaking_) {
                is_speaking_ = false;
                vad_state_change_callback_(false);
            }
        }

        if (output_callback_) {
            size_t samples = res->data_size / sizeof(int16_t);
            
            // Add data to buffer
            output_buffer_.insert(output_buffer_.end(), res->data, res->data + samples);
            
            // Output complete frames when buffer has enough data
            while (output_buffer_.size() >= frame_samples_) {
                if (output_buffer_.size() == frame_samples_) {
                    // If buffer size equals frame size, move the entire buffer
                    output_callback_(std::move(output_buffer_));
                    output_buffer_.clear();
                    output_buffer_.reserve(frame_samples_);
                } else {
                    // If buffer size exceeds frame size, copy one frame and remove it
                    output_callback_(std::vector<int16_t>(output_buffer_.begin(), output_buffer_.begin() + frame_samples_));
                    output_buffer_.erase(output_buffer_.begin(), output_buffer_.begin() + frame_samples_);
                }
            }
        }
    }
}

void AfeAudioProcessor::EnableDeviceAec(bool enable) {
    if (enable) {
#if CONFIG_USE_DEVICE_AEC
        afe_iface_->disable_vad(afe_data_);
        afe_iface_->enable_aec(afe_data_);
#else
        ESP_LOGE(TAG, "Device AEC is not supported");
#endif
    } else {
        afe_iface_->disable_aec(afe_data_);
        afe_iface_->enable_vad(afe_data_);
    }
}
