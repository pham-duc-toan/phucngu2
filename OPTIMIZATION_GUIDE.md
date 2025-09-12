# HƯỚNG DẪN TỐI ƯU LINE FOLLOWING ROBOT

## Những thay đổi đã thực hiện:

### 1. Cải thiện thuật toán Line Sensors:

- **Giảm ngưỡng phát hiện line**: Từ 2000 xuống 1200 để nhạy cảm hơn
- **Thêm ngưỡng line yếu**: 800 để xử lý line mờ/không rõ
- **Logic xử lý mất line thông minh**:
  - 100ms đầu: quay với tốc độ vừa (±1500)
  - 500ms tiếp: tăng dần tốc độ quay (±1800)
  - Sau 500ms: dừng lại tránh quay vòng vô tận

### 2. Tối ưu PID Controller:

- **Kp**: 1.8 → 1.5 (giảm dao động)
- **Ki**: 0.0 → 0.05 (thêm khả năng tự điều chỉnh lỗi tích lũy)
- **Kd**: 0.06 → 0.08 (tăng khả năng dự đoán)
- **d_alpha**: 0.88 → 0.85 (lọc nhiễu tốt hơn)
- **u_max/u_min**: ±800 → ±1500 (tăng dải điều khiển cho tốc độ cao)

### 3. **TĂNG TỐC ĐỘ LÊN GẦN MAXIMUM**:

- **Tốc độ cơ bản**: 1600 → 3200 (~80% max thay vì 40%)
- **Tốc độ theo lỗi**:
  - Lỗi < 300: tốc độ rất cao (3600 - 90% max) - đường thẳng
  - Lỗi 300-1200: tốc độ cao (2800 - 70% max)
  - Lỗi 1200-1800: tốc độ trung bình (2800 - 70% max) - cua nhẹ
  - Lỗi > 1800: tốc độ thấp (2400 - 60% max) - cua gắt
- **Chế độ test motor**: 1500 → 3000 (75% max)

### 4. Điều chỉnh cho tốc độ cao:

### 4. Điều chỉnh cho tốc độ cao:

- **Slew rate**: 60 → 100 để thay đổi PWM nhanh hơn
- **Dải PID**: ±1000 → ±1500 để đủ khả năng điều khiển

### 5. Cải thiện hiệu chuẩn:

- **Tăng thời gian hiệu chuẩn**: 5s → 8s
- **Kiểm tra chất lượng hiệu chuẩn**: Tự động kết thúc sớm nếu đủ tốt
- **Reset PID** khi dừng để tránh tích lũy lỗi

### 5. **HIỆU CHUẨN MOTOR** - Sửa vấn đề bánh chạy không cân bằng:

- **Thêm hệ số hiệu chuẩn motor**: Cho phép điều chỉnh tốc độ từng bánh
- **Chế độ test motor**: Nh?n gi? nút trong 3 giây khi khởi động
- **PWM có hiệu chuẩn**: Tự động áp dụng hệ số bù trừ

### 6. Giảm slew rate: 80 → 60 để chuyển đổi mượt mà hơn

## Hướng dẫn sử dụng:

### Bước 0: **HIỆU CHUẨN MOTOR** (Quan trọng nhất!)

1. **Vào chế độ test motor**: Nh?n giữ nút START trong 3 giây khi bật nguồn
2. **Kiểm tra**: Robot sẽ chạy thẳng với tốc độ cố định
3. **Quan sát**:
   - Nếu robot lệch trái → bánh trái nhanh hơn → giảm `left_factor`
   - Nếu robot lệch phải → bánh phải nhanh hơn → giảm `right_factor`
4. **Điều chỉnh** trong `Control_Init()`:

   ```c
   // Ví dụ: nếu bánh trái nhanh hơn 10%
   Motor_SetCalibration(0.9f, 1.0f);

   // Ví dụ: nếu bánh phải nhanh hơn 5%
   Motor_SetCalibration(1.0f, 0.95f);
   ```

5. **Test lại** cho đến khi robot chạy thẳng hoàn hảo

### Bước 1: Hiệu chuẩn Line Sensor

1. Đặt robot trên line, nhấn nút START
2. Di chuyển robot qua các vùng: line đen, nền trắng, line mờ
3. Làm trong 3-8 giây cho đến khi hiệu chuẩn hoàn tất

### Bước 2: Fine-tuning (nếu cần)

Nếu robot vẫn chưa hoạt động tốt, điều chỉnh:

**Trong line_sensors.c:**

- `VSUM_THRESH`: Tăng nếu robot nhạy cảm quá mức, giảm nếu không đủ nhạy
- `VSUM_WEAK_THRESH`: Điều chỉnh cho line yếu

**Trong control.c:**

- `Kp`: Tăng để phản ứng nhanh hơn, giảm để ít dao động
- `Ki`: Tăng để tự điều chỉnh lỗi offset, giảm nếu bị dao động
- `Kd`: Tăng để dự đoán tốt hơn, giảm nếu quá nhạy nhiễu

### Bước 3: Kiểm tra

- Robot phải chạy ổn định trên đường thẳng
- Không dao động qua lại
- Vào cua mượt mà không bị văng ra
- Khi mất line, tìm lại nhanh chóng và không quay vòng

## Troubleshooting:

**Robot chạy vòng tròn (vấn đề chính):**

- ✅ **HIỆU CHUẨN MOTOR TRƯỚC** - Đây là nguyên nhân chính!
- Sử dụng chế độ test motor để xác định bánh nào nhanh hơn
- Điều chỉnh hệ số hiệu chuẩn trong `Control_Init()`
- Test lại cho đến khi robot chạy thẳng hoàn hảo

**Robot không theo kịp line:**

- Tăng Kp
- Giảm BASE speed
- Kiểm tra tần số ADC/Control

**Robot dao động:**

- Giảm Kp
- Tăng Kd
- Kiểm tra slew rate
