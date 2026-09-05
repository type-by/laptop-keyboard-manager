# ⌨️ Laptop Internal Keyboard Manager (Hardware-Level Filter)

A lightweight Windows GUI utility designed to isolate and lock a laptop's built-in PS/2 keyboard at the kernel/driver level while keeping external USB and wireless keyboards fully functional.

Perfect for placing external mechanical keyboards directly on top of your laptop or resolving unwanted inputs from broken built-in keys.

---

## 🚀 Step-by-Step User Guide (For New Users)

Since Windows protects the internal keyboard as a **Critical System Device**, a one-time kernel driver registration is required. Follow these steps on any fresh computer:

### 1. Initial Setup (One-time only)
1. Download or clone this repository to a folder on your computer.
2. Right-click on **`Kurulum.bat`** and select **Run as Administrator**.
3. A command prompt will confirm that the driver has been installed.
4. **Restart your PC once.** (Crucial: Windows kernel must load the filter driver upon boot).

### 2. Daily Usage
- Run **`LaptopKlavyeYonetici.exe`**.
- Click **"Klavyeyi Kilitle" (Lock Keyboard)**: The internal keyboard will stop responding immediately. Your external keyboard will work normally.
- You can freely close the application window (`X`); the locking process remains active as a background daemon.
- To re-enable the internal keyboard, simply run the application again and click **"Klavyeyi Aç" (Unlock Keyboard)**.

### 3. Uninstallation
If you ever want to completely remove the driver from your system:
- Right-click on **`Kaldir.bat`** and select **Run as Administrator**.
- Restart your computer.

---

## ❓ Frequently Asked Questions & Troubleshooting

- **The app closes immediately upon launching:** You forgot to restart your PC after running `Kurulum.bat`, or `interception.dll` is not in the same directory as the executable. Restart your PC and make sure the `.dll` sits alongside the `.exe`.

- **Will this disable my external USB keyboard?** No. The filter is bound specifically to the internal PS/2 port (`device == 1`), leaving standard USB and wireless HID devices completely untouched.

---

## 🛠️ Build from Source (MinGW / GCC)

```cmd
cd src
gcc -mwindows -municode laptop_lock.c -o LaptopKlavyeYonetici.exe interception.lib
