# 🤖 LINE FOLLOWING ROBOT - THAM SỐ TUNING

## 📂 **CẤU TRÚC PROJECT**

```
ou-phuc-main/
├── 🎛️ Lib/line_params.h          # FILE THAM SỐ CHÍNH - SỬA ĐÂY!
├── 📖 HUONG_DAN_TEST_NHANH.md    # Hướng dẫn test từng bước
├── 📊 THAM_SO_NHAN_BIET_LINE.md  # Chi tiết kỹ thuật
├── 🔧 LINE_DETECTION_FIX.md      # Lịch sử sửa lỗi
└── Lib/                          # Code chính
    ├── line_sensors.c/h          # Thuật toán phát hiện line
    ├── control.c/h               # Logic điều khiển robot
    └── ...
```

## 🚀 **CÁCH SỬ DỤNG - 3 BƯỚC ĐỞN GIẢN**

### **Bước 1: Chỉnh tham số**

Mở file `Lib/line_params.h` và sửa các giá trị:

```c
#define VSUM_THRESH_PARAM           1800    // Ngưỡng line rõ ràng
#define MIN_CONTRAST_PARAM          500     // Contrast tối thiểu
#define MAX_AVG_WHITE_PARAM         3000    // Ngưỡng nền trắng
#define LOST_LINE_TIME_MS_PARAM     50      // Thời gian tìm line (ms)
```

### **Bước 2: Build & Flash**

- Build project trong Keil (F7)
- Flash vào robot

### **Bước 3: Test**

- Test không có line → robot KHÔNG chạy ✅
- Test có line → robot chạy ✅
- Điều chỉnh lại nếu cần

## 📋 **CÁC FILE HƯỚNG DẪN**

| File                           | Mục đích              | Khi nào dùng                  |
| ------------------------------ | --------------------- | ----------------------------- |
| 🎛️ `line_params.h`             | **Chỉnh tham số**     | Luôn luôn - đây là file chính |
| 📖 `HUONG_DAN_TEST_NHANH.md`   | **Test từng bước**    | Khi cần test có hệ thống      |
| 📊 `THAM_SO_NHAN_BIET_LINE.md` | **Hiểu sâu kỹ thuật** | Khi cần hiểu rõ thuật toán    |
| 🔧 `LINE_DETECTION_FIX.md`     | **Lịch sử sửa lỗi**   | Khi gặp vấn đề tương tự       |

## ⚡ **QUICK START - PRESET CÓ SẴN**

### 🔒 **Nghiêm khắc** (ít chạy nhầm):

```c
VSUM_THRESH_PARAM = 2000, MIN_CONTRAST_PARAM = 700, MAX_AVG_WHITE_PARAM = 2800
```

### 🔓 **Dễ dãi** (dễ detect line):

```c
VSUM_THRESH_PARAM = 1600, MIN_CONTRAST_PARAM = 300, MAX_AVG_WHITE_PARAM = 3200
```

### ⚖️ **Cân bằng** (mặc định):

```c
VSUM_THRESH_PARAM = 1800, MIN_CONTRAST_PARAM = 500, MAX_AVG_WHITE_PARAM = 3000
```

## 🎯 **MẸO TUNING HIỆU QUẢ**

1. **Bắt đầu nghiêm khắc** → nới lỏng dần
2. **Test nhiều điều kiện:** line đậm/nhạt, nền khác nhau
3. **Ghi lại giá trị tốt** cho từng trường hợp
4. **Chọn giá trị trung bình** phù hợp nhất

**➡️ Chỉ cần sửa 1 file `line_params.h` là có thể tuning toàn bộ hệ thống!**
