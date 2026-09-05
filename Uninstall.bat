@echo off
net session >nul 2>&1
if %0% neq 0 (
    echo [HATA] Lutfen bu dosyaya sag tiklayip "Yonetici Olarak Calistir" deyin.
    pause
    exit /b
)
cd /d "%%~dp0"
Interception surucusu kaldiriliyor...
"Interception\command line installer\install-interception.exe" /uninstall

Surucu kaldirildi. Bilgisayarinizi yeniden baslatin.
pause
