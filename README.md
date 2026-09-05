\# ⌨️ Laptop Internal Keyboard Manager (Hardware-Level Filter)



A lightweight Windows GUI utility designed to lock the laptop's built-in PS/2 keyboard at the hardware/driver level while keeping external USB and wireless HID keyboards fully functional.



Ideal for users who want to place an external mechanical keyboard directly on top of their laptop, or bypass a malfunctioning built-in keyboard with phantom keystrokes.



\---



\## ❓ Why This Project?



Windows classifies the internal keyboard as a \*\*Critical System Device\*\*. Standard API hooks (`WH\_KEYBOARD\_LL`) intercept keys globally without reliable device isolation, while device manager / PnP disable commands require a full system restart. 



This project solves the problem by integrating a kernel-level keyboard filter driver (\*\*Interception\*\*), isolating the internal PS/2 device ID from external HID devices without requiring reboots after the initial driver setup.



\---



\## ✨ Features



\- \*\*Hardware-Level Filtering:\*\* Intercepts only internal PS/2 keystrokes; external USB/wireless keyboards remain unaffected.

\- \*\*Instant Control:\*\* One-click Lock and Unlock toggling without restarting Windows.

\- \*\*Background Daemon:\*\* Runs silently in the background even after closing the GUI window.

\- \*\*Zero Heavy Dependencies:\*\* Written entirely in pure C and native Win32 API (no heavy runtimes like Python or .NET).

\- \*\*Clean Unicode GUI:\*\* Native Windows interface built with Segoe UI typography.



\---



\## 🚀 Installation \& Usage



1\. Download or clone this repository.

2\. Right-click on `Kurulum.bat` and select \*\*Run as Administrator\*\* to register the kernel filter driver.

3\. \*\*Restart your computer once\*\* to enable the driver in the Windows kernel.

4\. Run `LaptopKlavyeYonetici.exe`:

&#x20;  - Click \*\*Klavyeyi Kilitle (Lock)\*\* to suppress built-in keyboard input.

&#x20;  - Click \*\*Klavyeyi Aç (Unlock)\*\* to restore normal keyboard behavior.



\---



\## 🛠️ Building from Source (MinGW / GCC)



If you wish to compile the project yourself:



```cmd

cd src

gcc -mwindows -municode laptop\_lock.c -o LaptopKlavyeYonetici.exe interception.lib

