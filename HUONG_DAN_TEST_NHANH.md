# 🚀 HƯỚNG DẪN TEST NHANH - TUNING THAM SỐ

## 📁 **File cần chỉnh sửa:** `Lib/line_params.h`

### ⚡ **QUY TRÌNH TEST NHANH:**

1. **Mở file:** `Lib/line_params.h`
2. **Sửa tham số** theo bảng dưới đây
3. **Build project** (F7 trong Keil)
4. **Flash vào robot** và test
5. **Lặp lại** cho đến khi ổn

---

## 🎯 **CÁC THAM SỐ CHÍNH ĐỂ CHỈNH**

### **1. VSUM_THRESH_PARAM** (hiện tại: 1800)

**Làm gì:** Ngưỡng để coi là có line rõ ràng

- **Tăng lên** 2000-2200: Robot khó detect line hơn (ít false positive)
- **Giảm xuống** 1600-1400: Robot dễ detect line hơn (nhiều false positive)

### **2. MIN_CONTRAST_PARAM** (hiện tại: 500)

**Làm gì:** Độ chênh lệch tối thiểu giữa line và nền

- **Tăng lên** 700-800: Cần contrast cao mới detect line
- **Giảm xuống** 300-400: Chấp nhận contrast thấp

### **3. MAX_AVG_WHITE_PARAM** (hiện tại: 3000)

**Làm gì:** Ngưỡng coi là nền trắng (không có line)

- **Giảm xuống** 2800-2500: Nghiêm khắc hơn với nền sáng
- **Tăng lên** 3200-3500: Cho phép nền sáng hơn

### **4. LOST_LINE_TIME_MS_PARAM** (hiện tại: 50)

**Làm gì:** Thời gian tìm line khi mất (ms)

- **Tăng lên** 100-150: Robot tìm line lâu hơn
- **Giảm xuống** 30-20: Robot dừng nhanh hơn

---

## 🔧 **BẢNG TROUBLESHOOTING**

| Vấn đề                          | Sửa gì                                                                                   | Giá trị đề xuất |
| ------------------------------- | ---------------------------------------------------------------------------------------- | --------------- |
| 🚫 Robot chạy khi KHÔNG có line | `VSUM_THRESH_PARAM = 2000`<br>`MIN_CONTRAST_PARAM = 700`<br>`MAX_AVG_WHITE_PARAM = 2800` | Nghiêm khắc hơn |
| ❌ Robot KHÔNG chạy khi có line | `VSUM_THRESH_PARAM = 1600`<br>`MIN_CONTRAST_PARAM = 300`<br>`MAX_AVG_WHITE_PARAM = 3200` | Dễ dãi hơn      |
| ⚡ Robot dừng quá NHANH         | `LOST_LINE_TIME_MS_PARAM = 100`<br>`SEARCH_SPEED_PARAM = 1200`                           | Kiên nhẫn hơn   |
| 🐌 Robot tìm line quá LÂU       | `LOST_LINE_TIME_MS_PARAM = 30`<br>`SEARCH_SPEED_PARAM = 600`                             | Nhanh gọn hơn   |

---

## 📋 **TEST CHECKLIST**

### ✅ **Test 1: Không có line**

- [ ] Đặt robot trên nền trắng
- [ ] Nhấn button START
- [ ] **Kết quả mong đợi:** Robot KHÔNG chạy

### ✅ **Test 2: Có line đen**

- [ ] Đặt robot trên line đen rõ ràng
- [ ] Nhấn button START
- [ ] **Kết quả mong đợi:** Robot chạy và follow line

### ✅ **Test 3: Line yếu/mờ**

- [ ] Đặt robot trên line nhạt/mờ
- [ ] Nhấn button START
- [ ] **Kết quả mong đợi:** Tùy setting (có thể chạy hoặc không)

### ✅ **Test 4: Mất line**

- [ ] Robot đang chạy trên line
- [ ] Nhấc robot lên (mất line)
- [ ] **Kết quả mong đợi:** Robot dừng trong thời gian quy định

---

## 🎨 **PRESET THAM SỐ**

### **🔒 STRICT MODE (ít false positive)**

```c
#define VSUM_THRESH_PARAM           2000
#define MIN_CONTRAST_PARAM          700
#define MAX_AVG_WHITE_PARAM         2800
#define LOST_LINE_TIME_MS_PARAM     30
```

### **🔓 RELAXED MODE (dễ detect line)**

```c
#define VSUM_THRESH_PARAM           1600
#define MIN_CONTRAST_PARAM          300
#define MAX_AVG_WHITE_PARAM         3200
#define LOST_LINE_TIME_MS_PARAM     100
```

### **⚡ BALANCED MODE (cân bằng)**

```c
#define VSUM_THRESH_PARAM           1800
#define MIN_CONTRAST_PARAM          500
#define MAX_AVG_WHITE_PARAM         3000
#define LOST_LINE_TIME_MS_PARAM     50
```

---

## 💡 **MẸO TEST HIỆU QUẢ**

1. **Bắt đầu với STRICT MODE** → giảm dần cho đến khi robot chạy được
2. **Test trên nhiều loại line:** đen đậm, đen nhạt, băng keo, marker
3. **Test trên nhiều nền:** giấy trắng, gỗ, thảm, bàn
4. **Ghi lại giá trị hoạt động tốt** cho từng điều kiện
5. **Chọn giá trị trung bình** phù hợp nhất

**➡️ Chỉ cần sửa 1 file `line_params.h` là test được ngay!**
