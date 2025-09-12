# TỔNG HỢP CÁC THAM SỐ NHẬN BIẾT LINE

## 📊 **CÁC THAM SỐ CHÍNH TRONG SYSTEM**

### 🎯 **1. NGƯỠNG PHÁT HIỆN LINE (line_sensors.c)**

```c
#define VSUM_THRESH 1800        // Ngưỡng chính để phát hiện line rõ ràng
#define VSUM_WEAK_THRESH 1000   // Ngưỡng phát hiện line yếu
```

**Ý nghĩa:**

- `VSUM_THRESH = 1800`: Tổng weighted sum > 1800 → có line rõ ràng
- `VSUM_WEAK_THRESH = 1000`: Tổng weighted sum > 1000 → có line yếu

### 🔍 **2. NGƯỠNG KIỂM TRA CONTRAST (control.c)**

```c
if (debug_info.contrast < 500) {
    line_detected = 0;  // Không có line
}
```

**Ý nghĩa:**

- `contrast = max_sensor - min_sensor`
- Nếu contrast < 500 → không đủ sự khác biệt giữa line và nền

### 📈 **3. NGƯỠNG KIỂM TRA NỀN TRẮNG (control.c)**

```c
if (debug_info.avg_all > 3000) {
    line_detected = 0;  // Nền trắng
}
```

**Ý nghĩa:**

- `avg_all = trung bình 8 sensors`
- Nếu avg_all > 3000 → tất cả sensors đều cao → nền trắng

### ⏱️ **4. THỜI GIAN TÌM LINE KHI MẤT (line_sensors.c)**

```c
if (lostLineCounter < 50) {         // 50ms đầu
    return ±800;                    // Quay tìm line
} else {
    return 0;                       // Dừng lại
}
```

**Ý nghĩa:**

- Khi mất line, robot chỉ tìm trong 50ms
- Sau đó dừng lại (không quay vô tận)

### 🏁 **5. NGƯỠNG HIỆU CHUẨN (line_sensors.c)**

```c
if ((s_max[i] - s_min[i]) > 500) {
    good_channels++;    // Sensor đã hiệu chuẩn tốt
}
```

**Ý nghĩa:**

- Mỗi sensor cần có độ chênh lệch > 500 giữa min/max để coi là hiệu chuẩn tốt
- Cần ít nhất 6/8 sensors hiệu chuẩn tốt

## 🎚️ **BẢNG THAM SỐ CHO TUNING**

| Tham số                | Giá trị hiện tại | Tăng lên nếu                            | Giảm xuống nếu                            |
| ---------------------- | ---------------- | --------------------------------------- | ----------------------------------------- |
| `VSUM_THRESH`          | 1800             | Robot quá nhạy (chạy khi không có line) | Robot không nhạy (không chạy khi có line) |
| `VSUM_WEAK_THRESH`     | 1000             | Robot phát hiện line yếu quá nhiều      | Robot bỏ lỡ line yếu                      |
| `contrast < 500`       | 500              | Robot nhạy với noise                    | Robot bỏ lỡ line có contrast thấp         |
| `avg_all > 3000`       | 3000             | Robot chạy trên nền sáng                | Robot không chạy trên nền tối             |
| `lostLineCounter < 50` | 50ms             | Robot quay tìm line quá lâu             | Robot dừng quá nhanh khi mất line         |
| `return ±800`          | 800              | Robot quay quá chậm                     | Robot quay quá nhanh                      |

## 🔧 **CÁCH TUNING THỰC TẾ**

### **Trường hợp 1: Robot chạy khi không có line**

```c
VSUM_THRESH: 1800 → 2000-2200
contrast: 500 → 700-800
avg_all: 3000 → 2800-2500
```

### **Trường hợp 2: Robot không chạy khi có line**

```c
VSUM_THRESH: 1800 → 1600-1400
contrast: 500 → 300-400
avg_all: 3000 → 3200-3500
```

### **Trường hợp 3: Robot dừng quá nhanh khi mất line**

```c
lostLineCounter: 50 → 100-150ms
return value: ±800 → ±1200-1500
```

### **Trường hợp 4: Robot quay tìm line quá lâu**

```c
lostLineCounter: 50 → 30-20ms
return value: ±800 → ±500-600
```

## 📋 **CÁC GIÁ TRỊ DEBUG QUAN TRỌNG**

Đặt breakpoint trong `Control_DebugInfo()` và check:

```c
debug_info.contrast        // Độ tương phản (nên > 500)
debug_info.avg_all         // Trung bình sensors (nên < 3000)
debug_info.is_valid        // Line hợp lệ (1=có, 0=không)
debug_info.computed_error  // Error tính được
line_detected              // Kết quả cuối cùng (1=có line, 0=không)
```

## ⚡ **QUICK REFERENCE**

**Line tốt:** `contrast > 500` + `avg_all < 3000` + `vsum > 1800`  
**Line yếu:** `contrast > 500` + `avg_all < 3000` + `1000 < vsum < 1800`  
**Không có line:** `contrast < 500` OR `avg_all > 3000` OR `vsum < 1000`
