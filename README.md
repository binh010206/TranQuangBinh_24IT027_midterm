# Midterm Project – Implement ls(1)

**Họ và tên:** Trần Quang Bình  
**MSSV:** 24IT027  
**Môn học:** Lập trình hệ thống (Midterm Project)  
**Repository:** [GitHub Link](https://github.com/binh010206/TranQuangBinh_24IT027_midterm.git)

---

## 1. Yêu cầu hệ thống

- **Hệ điều hành:** NetBSD, Linux (Ubuntu/WSL)
- **Trình biên dịch:** `gcc`
- **Công cụ build:** `make` (GNU Make)
- **Quản lý mã nguồn:** `git`

---

## 2. Cấu trúc thư mục

```text
TranQuangBinh_24IT027_midterm/
├── include/
│   ├── ls_args.h         # Định nghĩa cấu trúc cờ lệnh
│   ├── ls_entry.h        # Cấu trúc dữ liệu cho mỗi file entry
│   ├── ls_compare.h      # Giao diện sắp xếp
│   ├── ls_format.h       # Giao diện xuất kết quả
│   └── ls_walk.h         # Duyệt thư mục và xử lý tham số
├── src/
│   ├── main.c            # Hàm main(), điểm khởi đầu chương trình
│   ├── ls_args.c         # Phân tích cờ lệnh bằng getopt()
│   ├── ls_entry.c        # Tạo entry, đọc thư mục, quản lý bộ nhớ
│   ├── ls_compare.c      # Hàm so sánh cho qsort()
│   ├── ls_format.c       # In kết quả dạng ngắn và dạng dài (-l)
│   └── ls_walk.c         # Phân tách file/thư mục, đệ quy -R
├── .gitignore
├── LICENSE
├── Makefile
└── README.md
```

---

## 3. Hướng dẫn biên dịch và chạy

### Bước 1: Clone repository

```bash
git clone https://github.com/binh010206/TranQuangBinh_24IT027_midterm.git
cd TranQuangBinh_24IT027_midterm
```

### Bước 2: Biên dịch

```bash
make              # Tạo file thực thi ./myls
make clean        # Xóa file .o và file thực thi
```

### Bước 3: Chạy chương trình

```bash
./myls [cờ_lệnh] [file/thư_mục ...]
```

**Ví dụ:**

```bash
./myls                    # Liệt kê thư mục hiện tại
./myls -la                # Hiển thị tất cả file dạng chi tiết
./myls -lhS /tmp          # Dạng dài, kích thước dễ đọc, sắp theo size
./myls -Ri src include    # Đệ quy vào nhiều thư mục
./myls -d /usr /etc       # Xem thư mục như file thường (không liệt kê bên trong)
```

---

## 4. Bảng mô tả các cờ lệnh

| Cờ  | Chức năng |
|:---:|:---|
| `-A` | Hiển thị tất cả entry trừ `.` và `..`. Mặc định bật cho super-user. |
| `-a` | Hiển thị tất cả entry bao gồm cả `.` và `..` |
| `-c` | Dùng thời gian thay đổi trạng thái (ctime) thay cho mtime khi sắp xếp (`-t`) hoặc hiển thị (`-l`) |
| `-d` | Coi thư mục như file thường, không liệt kê nội dung bên trong |
| `-F` | Thêm ký tự loại file: `/` (thư mục), `*` (thực thi), `@` (symlink), `=` (socket), `\|` (FIFO), `%` (whiteout) |
| `-f` | Không sắp xếp, in theo thứ tự readdir (kèm hiệu ứng `-a`) |
| `-h` | Hiển thị kích thước dạng dễ đọc (B, K, M, G). Ghi đè `-k`. |
| `-i` | In số inode (file serial number) cho mỗi entry |
| `-k` | Hiển thị kích thước theo đơn vị kilobytes. Ghi đè `-h`. |
| `-l` | Hiển thị dạng chi tiết (quyền, số link, owner, group, size, time, tên) |
| `-n` | Giống `-l` nhưng hiển thị UID/GID bằng số thay vì tên |
| `-q` | Thay ký tự không in được thành `?`. Mặc định khi output ra terminal. |
| `-R` | Liệt kê đệ quy tất cả thư mục con |
| `-r` | Đảo ngược thứ tự sắp xếp |
| `-S` | Sắp xếp theo kích thước file (lớn nhất trước) |
| `-s` | Hiển thị số block đĩa thực tế (đơn vị 512 bytes hoặc BLOCKSIZE) |
| `-t` | Sắp xếp theo thời gian chỉnh sửa (mới nhất trước) |
| `-u` | Dùng thời gian truy cập cuối (atime) thay cho mtime |
| `-w` | In nguyên ký tự không in được (mặc định khi output không ra terminal) |

### Quy tắc ghi đè giữa các cặp cờ

Cờ xuất hiện **sau cùng** (bên phải nhất) sẽ có hiệu lực:

| Cặp đối lập | Ý nghĩa |
|:---:|:---|
| `-c` / `-u` | Chọn loại timestamp (ctime hoặc atime) |
| `-R` / `-d` | Đệ quy hoặc coi thư mục là file |
| `-k` / `-h` | Đơn vị hiển thị kích thước |
| `-l` / `-n` | Tên owner/group hoặc số UID/GID |
| `-q` / `-w` | Ẩn hoặc in nguyên ký tự đặc biệt |

---

## 5. Thiết kế chương trình

### 5.1. Luồng xử lý chính

```
main() → config_parse() → walk_args()
                              ├── Tách file operands / directory operands
                              ├── items_sort() + format_output() cho files
                              └── process_dir() cho mỗi directory
                                    ├── dir_read()       đọc entries
                                    ├── items_sort()      sắp xếp
                                    ├── format_output()   in kết quả
                                    └── process_dir()     đệ quy nếu -R
```

### 5.2. Phân chia module

| Module | File | Nhiệm vụ |
|:---|:---|:---|
| **Argument Parser** | `ls_args.c` / `ls_args.h` | Phân tích cờ lệnh bằng `getopt()`, xử lý các cặp ghi đè, thiết lập mặc định theo môi trường |
| **File Entry** | `ls_entry.c` / `ls_entry.h` | Tạo `ls_item_t` từ `lstat()`, đọc symlink target, quản lý bộ nhớ (malloc/free) |
| **Comparator** | `ls_compare.c` / `ls_compare.h` | Sắp xếp mảng entry bằng `qsort()` theo size/time/name, hỗ trợ reverse |
| **Formatter** | `ls_format.c` / `ls_format.h` | In kết quả dạng ngắn hoặc dạng dài, xây dựng chuỗi permission, format thời gian, human-readable size |
| **Walker** | `ls_walk.c` / `ls_walk.h` | Phân loại operands (file vs. directory), xử lý `-d`, đệ quy `-R` |

### 5.3. Cấu trúc dữ liệu chính

```c
/* Cấu hình cờ lệnh */
typedef struct {
    int flag_all;           /* -a */
    int flag_almost_all;    /* -A */
    int flag_ctime;         /* -c */
    /* ... 16 cờ khác ... */
} ls_config_t;

/* Thông tin một file entry */
typedef struct {
    char        *name;          /* tên file */
    char        *fullpath;      /* đường dẫn đầy đủ */
    struct stat  info;          /* kết quả lstat() */
    char        *symlink_dest;  /* đích symlink, NULL nếu không phải */
} ls_item_t;
```

---

## 6. Demo kết quả

### Biên dịch

```bash
$ make clean && make
rm -f src/*.o myls
gcc -Wall -Wextra -std=c99 -Iinclude -D_DEFAULT_SOURCE -c src/main.c -o src/main.o
gcc -Wall -Wextra -std=c99 -Iinclude -D_DEFAULT_SOURCE -c src/ls_args.c -o src/ls_args.o
gcc -Wall -Wextra -std=c99 -Iinclude -D_DEFAULT_SOURCE -c src/ls_entry.c -o src/ls_entry.o
gcc -Wall -Wextra -std=c99 -Iinclude -D_DEFAULT_SOURCE -c src/ls_compare.c -o src/ls_compare.o
gcc -Wall -Wextra -std=c99 -Iinclude -D_DEFAULT_SOURCE -c src/ls_format.c -o src/ls_format.o
gcc -Wall -Wextra -std=c99 -Iinclude -D_DEFAULT_SOURCE -c src/ls_walk.c -o src/ls_walk.o
gcc src/main.o src/ls_args.o src/ls_entry.o src/ls_compare.o src/ls_format.o src/ls_walk.o -o myls
```

### Chạy cơ bản

```bash
$ ./myls
Makefile
README.md
include
myls
src

$ ./myls -l
total 96
-rw-r--r-- 1 binh     binh       350 Oct 10 00:05 Makefile
-rw-r--r-- 1 binh     binh      5200 Oct 10 00:08 README.md
drwxr-xr-x 2 binh     binh      4096 Oct 10 00:04 include
-rwxr-xr-x 1 binh     binh     25600 Oct 10 00:05 myls
drwxr-xr-x 2 binh     binh      4096 Oct 10 00:07 src
```

### Các cờ nâng cao

```bash
$ ./myls -a
.
..
.gitignore
Makefile
README.md
include
myls
src

$ ./myls -lhS
total 96
-rwxr-xr-x 1 binh     binh       25K Oct 10 00:05 myls
-rw-r--r-- 1 binh     binh      5.1K Oct 10 00:08 README.md
drwxr-xr-x 2 binh     binh      4.0K Oct 10 00:04 include
drwxr-xr-x 2 binh     binh      4.0K Oct 10 00:07 src
-rw-r--r-- 1 binh     binh       350B Oct 10 00:05 Makefile

$ ./myls -R
Makefile
README.md
include
myls
src

./include:
ls_args.h
ls_compare.h
ls_entry.h
ls_format.h
ls_walk.h

./src:
ls_args.c
ls_compare.c
ls_entry.c
ls_format.c
ls_walk.c
main.c

$ ./myls -liF
55001 -rw-r--r-- 1 binh     binh       350 Oct 10 00:05 Makefile
55002 -rw-r--r-- 1 binh     binh      5200 Oct 10 00:08 README.md
 2001 drwxr-xr-x 2 binh     binh      4096 Oct 10 00:04 include/
57001 -rwxr-xr-x 1 binh     binh     25600 Oct 10 00:05 myls*
13001 drwxr-xr-x 2 binh     binh      4096 Oct 10 00:07 src/
```
