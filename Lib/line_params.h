#ifndef LINE_PARAMS_H
#define LINE_PARAMS_H

/* ============================================================================
 * FILE THAM SỐ NHẬN BIẾT LINE - DỄ DÀNG TUNING
 * ============================================================================
 * Chỉnh sửa các giá trị ở đây rồi build lại để test
 * Không cần sửa trong code chính
 */

// 🎯 NGƯỠNG PHÁT HIỆN LINE CHÍNH
#define VSUM_THRESH_PARAM 1800      // Ngưỡng line rõ ràng
#define VSUM_WEAK_THRESH_PARAM 1000 // Ngưỡng line yếu

// 🔍 NGƯỠNG KIỂM TRA CONTRAST
#define MIN_CONTRAST_PARAM 500 // Contrast tối thiểu để có line

// 📈 NGƯỠNG KIỂM TRA NỀN TRẮNG
#define MAX_AVG_WHITE_PARAM 3000 // Avg > này = nền trắng

// ⏱️ THỜI GIAN TÌM LINE KHI MẤT
#define LOST_LINE_TIME_MS_PARAM 50 // Thời gian tìm line (ms)
#define SEARCH_SPEED_PARAM 800     // Tốc độ quay tìm line (±)

// 🏁 NGƯỠNG HIỆU CHUẨN
#define MIN_CALIB_DIFF_PARAM 500 // Min-max diff cho hiệu chuẩn tốt

/* ============================================================================
 * HƯỚNG DẪN TUNING NHANH:
 * ============================================================================
 *
 * 🚫 Robot chạy khi KHÔNG có line:
 *    - Tăng VSUM_THRESH_PARAM: 1800 → 2000-2200
 *    - Tăng MIN_CONTRAST_PARAM: 500 → 700-800
 *    - Giảm MAX_AVG_WHITE_PARAM: 3000 → 2800-2500
 *
 * ❌ Robot KHÔNG chạy khi có line:
 *    - Giảm VSUM_THRESH_PARAM: 1800 → 1600-1400
 *    - Giảm MIN_CONTRAST_PARAM: 500 → 300-400
 *    - Tăng MAX_AVG_WHITE_PARAM: 3000 → 3200-3500
 *
 * ⚡ Robot dừng quá NHANH khi mất line:
 *    - Tăng LOST_LINE_TIME_MS_PARAM: 50 → 100-150
 *    - Tăng SEARCH_SPEED_PARAM: 800 → 1200-1500
 *
 * 🐌 Robot tìm line quá LÂU:
 *    - Giảm LOST_LINE_TIME_MS_PARAM: 50 → 30-20
 *    - Giảm SEARCH_SPEED_PARAM: 800 → 500-600
 *
 * ============================================================================
 */

#endif /* LINE_PARAMS_H */