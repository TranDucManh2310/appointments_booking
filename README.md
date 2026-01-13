# Appointments Booking

Ung dung client/server C dung TCP socket, luu tru tep trong `data/`. Ho tro vai tro patient/doctor/admin voi menu dong lenh.

## Yeu cau
- GCC ho tro pthread (Linux/macOS) hoac MinGW-w64 tren Windows.
- `make` de bien dich nhanh (co the bien dich thu cong neu khong co make).
- Cong 9000 trong (hoac cong khac neu chay server voi tham so khac).

## Cai dat nhanh tren Windows de co `make`/`gcc`
1) Cai MSYS2 (https://www.msys2.org/), mo terminal "MSYS2 MinGW UCRT x64".
2) Cai toolchain (gom `make`, `gcc`):
   ```
   pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
   ```
3) Vao thu muc du an:
   ```
   cd /c/Users/hp/Downloads/appointments_booking
   ```
4) Tu day co the dung `make` va chay binary trong cung terminal MSYS2.

## Bien dich thu cong neu khong co make
- Linux/WSL/Git Bash:
  ```
  gcc -Wall -g -pthread -Ishared -Iserver/db -Iserver/handlers \
    -o server/server \
    server/server.c server/db/db.c server/handlers/patient_handler.c \
    server/handlers/doctor_handler.c server/handlers/admin_handler.c \
    shared/socket.c shared/protocol.c

  gcc -Wall -g -pthread -Ishared -Iclient/menus \
    -o client/client \
    client/client.c client/menus/patient_menu.c client/menus/doctor_menu.c \
    client/menus/admin_menu.c shared/socket.c shared/protocol.c
  ```
- Windows (MinGW-w64 trong MSYS2 MinGW UCRT x64):
  ```
  gcc -Wall -g -pthread -Ishared -Iserver/db -Iserver/handlers ^
    -o server\server.exe ^
    server\server.c server\db\db.c server\handlers\patient_handler.c ^
    server\handlers\doctor_handler.c server\handlers\admin_handler.c ^
    shared\socket.c shared\protocol.c

  gcc -Wall -g -pthread -Ishared -Iclient\menus ^
    -o client\client.exe ^
    client\client.c client\menus\patient_menu.c client\menus\doctor_menu.c ^
    client\menus\admin_menu.c shared\socket.c shared\protocol.c
  ```
Sau khi biên dịch thủ công, chạy server/client như phần “Cach chay”.

## Cau truc nhanh
- `server/`: ma nguon server da luong doc/ghi file du lieu.
- `client/`: ung dung console cho benh nhan/bac si/admin.
- `shared/`: code socket va giao thuc text.
- `data/`: du lieu mau (users, doctors, doctor_accounts, appointments).

## Cach chay (tuan tu)
1) Tu thu muc goc `appointments_booking`, bien dich:
   - `make` (tao `server/server` va `client/client`; tren Windows thuong sinh `.exe`).
2) Mo terminal 1, khoi dong server (mac dinh port 9000):
   - Linux/macOS/Git Bash: `./server/server` hoac `./server/server 9000`
   - PowerShell/CMD: `./server/server.exe 9000` (su dung ten file thuc te neu khac).
3) Mo terminal 2, chay client ket noi ve server:
   - Linux/macOS/Git Bash: `./client/client 127.0.0.1 9000`
   - PowerShell/CMD: `./client/client.exe 127.0.0.1 9000`
   - Menu chinh se xuat hien de dang ky, dang nhap va su dung menu theo vai tro.
4) Tai khoan mau thu nhanh:
   - Admin: `admin` / `admin`
   - Bac si: `doctor1` / `doc123`, `doctor2` / `doc123`
   - Benh nhan: `tranmanh` / `Tranmanh`
5) Don dep (tuy chon): `make clean` de xoa binary.

Ghi chu:
- Server doc/ghi cac tep trong `data/`; co the sua noi dung de reset du lieu.
- Co the doi port khi chay server bang tham so, client phai dung cung dia chi va port.
