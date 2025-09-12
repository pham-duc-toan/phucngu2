# HƯỚNG DẪN SỬA LỖI LINE DETECTION

## Vấn đề đã sửa:

- Robot chạy dù không có line

## Các thay đổi đã thực hiện:

### 1. Tăng ngưỡng phát hiện line (line_sensors.c):

- `VSUM_THRESH`: 1200 → 1800 (chỉ chấp nhận line rõ ràng)
- `VSUM_WEAK_THRESH`: 800 → 1000 (tăng ngưỡng line yếu)

### 2. Thêm logic kiểm tra nghiêm khắc trong control.c:

- Kiểm tra contrast < 500 → không có line
- Kiểm tra avg_all > 3000 → nền trắng, không có line
- Kiểm tra error = 0 → chắc chắn không có line
- Thêm check `!line_detected` trước khi cho phép chạy

### 3. Giảm thời gian tìm line khi mất:

- Từ 100ms xuống 50ms
- Giảm tốc độ quay từ ±1500 xuống ±800

## Cách build và test:

### Build:

1. Mở Keil uVision 5
2. Open Project: `c:\phucngu\ou-phuc-main\ou-phuc-main\Project\Example.uvprojx`
3. Build (F7) hoặc Project → Build Target
4. Kiểm tra có lỗi không trong Build Output

### Test:

1. **Test không có line:**

   - Đặt robot trên nền trắng/bàn trống
   - Nhấn button → robot KHÔNG nên chạy
   - Nếu vẫn chạy → cần tăng thêm ngưỡng

2. **Test có line:**

   - Đặt robot trên line đen
   - Nhấn button → robot nên chạy và follow line

3. **Test mất line:**
   - Robot đang chạy trên line
   - Nhấc robot lên → robot nên dừng trong 50ms

### Debug nếu cần:

1. Đặt breakpoint trong `Control_DebugInfo()`
2. Check các giá trị:
   - `debug_info.contrast` (nên > 500 khi có line)
   - `debug_info.avg_all` (nên < 3000 khi có line)
   - `debug_info.is_valid` (nên = 1 khi có line)
   - `line_detected` (nên = 1 khi có line)

### Tuning nếu cần:

- Nếu robot quá nhạy (chạy khi không có line):

  - Tăng `VSUM_THRESH` lên 2000-2200
  - Tăng contrast threshold lên 700-800
  - Giảm `avg_all` threshold xuống 2500-2800

- Nếu robot không nhạy (không chạy khi có line):
  - Giảm `VSUM_THRESH` xuống 1600-1400
  - Giảm contrast threshold xuống 300-400
  - Tăng `avg_all` threshold lên 3200-3500

## Kết quả mong đợi:

- Robot CHỈ chạy khi thực sự có line rõ ràng
- Robot dừng ngay khi không có line
- Giảm false positive (chạy khi không có line)
