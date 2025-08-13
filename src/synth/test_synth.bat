@echo off
echo ~~ Compile test program ~~
call make_test.bat

echo ~~ Render track ~~
.\test_main.exe ..\..\dataloss\midi-music\eetteritekno.mid

echo ~~ Check SHA256 hash ~~
certutil -hashfile ..\..\dataloss\midi-music\eetteritekno.mid.wav SHA256 > check

node -e "sha=fs.readFileSync('check').toString().split(/\r?\n/)[1];console.log(sha === '058fd0b325f2697e104368cd5b9d0852789fa010c615424372927f82a231b56e' ? 'OK' : 'FAIL ' + sha)"
del check