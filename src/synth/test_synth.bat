@echo off
echo ~~ Compile test program ~~
call make_test.bat

echo ~~ Render track ~~
.\test_main.exe ..\..\dataloss\midi-music\eetteritekno.mid

echo ~~ Check SHA256 hash ~~
certutil -hashfile ..\..\dataloss\midi-music\eetteritekno.mid.wav SHA256 > check

node -e "sha=fs.readFileSync('check').toString().split(/\r?\n/)[1];console.log(sha === '03e0b818b54921c04acb09247a52974a5932e239fa4de8d8057c048527707a75' ? 'OK' : 'FAIL ' + sha)"
del check