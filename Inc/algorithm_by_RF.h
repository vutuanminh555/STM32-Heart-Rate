#ifndef ALGORITHM_BY_RF_H_
#define ALGORITHM_BY_RF_H_

#include <stdint.h>

#define ST 4      // Thời gian lấy mẫu (s).
#define FS 25     // Tần số lấy mẫu (Hz).
// Sum of squares of ST*FS numbers from -mean_X (see below) to +mean_X incremented be one. For example, given ST=4 and FS=25,
// the sum consists of 100 terms: (-49.5)^2 + (-48.5)^2 + (-47.5)^2 + ... + (47.5)^2 + (48.5)^2 + (49.5)^2
// The sum is symmetrc, so you can evaluate it by multiplying its positive half by 2.
#define sum_X2 83325.0f
#define MAX_HR 180  // Nhịp tim tối đa. Dùng để loại bỏ kết quả lỗi
#define MIN_HR 40   // Nhịp tim tối thiểu.
// Minimal ratio of two autocorrelation sequence elements: one at a considered lag to the one at lag 0.
// Good quality signals must have such ratio greater than this minimum.
#define min_autocorrelation_ratio 0.5f
// Pearson correlation between red and IR signals.
// Good quality signals must have their correlation coefficient greater than this minimum.
#define min_pearson_correlation 0.8f


#define BUFFER_SIZE (FS*ST) // Số mẫu trong 1 đợt
#define FS60 (FS*60)  // Chuyển đổi bps sang bpm
#define LOWEST_PERIOD (FS60/MAX_HR) // Khoảng cách tối thiểu giữa các đỉnh
#define HIGHEST_PERIOD  (FS60/MIN_HR) // Khoảng cách tối đa giữa các đỉnh
#define mean_X ((float)(BUFFER_SIZE-1)/2.0) // Giá trị trung bình từ 0 đến BUFFER_SIZE-1. For ST=4 and FS=25 it's equal to 49.5.

void rf_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer, int32_t n_ir_buffer_length, uint32_t *pun_red_buffer, float *pn_spo2, int8_t *pch_spo2_valid, int32_t *pn_heart_rate, 
                                        int8_t *pch_hr_valid, float *ratio, float *correl);
float rf_linear_regression_beta(float *pn_x, float xmean, float sum_x2);
float rf_autocorrelation(float *pn_x, int32_t n_size, int32_t n_lag);
float rf_rms(float *pn_x, int32_t n_size, float *sumsq);
float rf_Pcorrelation(float *pn_x, float *pn_y, int32_t n_size);
void rf_initialize_periodicity_search(float *pn_x, int32_t n_size, int32_t *p_last_periodicity, int32_t n_max_distance, float min_aut_ratio, float aut_lag0);
void rf_signal_periodicity(float *pn_x, int32_t n_size, int32_t *p_last_periodicity, int32_t n_min_distance, int32_t n_max_distance, float min_aut_ratio, float aut_lag0, float *ratio);

#endif /* ALGORITHM_BY_RF_H_ */

