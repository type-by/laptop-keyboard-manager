@echo off
net session >nul 2>&1
if %0% neq 0 (
    echo [HATA] Lutfen bu dosyaya sag tiklayip "Yonetici Olarak Calistir" deyin.
    pause
    exit /b
)
cd /d "%%~dp0"
Interception surucusu kuruluyor...
"Interception\command line installer\install-interception.exe" /install

===================================================================
 KURULUM TAMAMLANDI!
 Surucunun devreye girmesi icin bilgisayarinizi 1 kez yeniden baslatin.
===================================================================
pause
