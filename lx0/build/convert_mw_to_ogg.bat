@echo off
IF EXIST ffmpeg.exe (
    for /f "usebackq delims=|" %%f in (`dir /b /s "mwdata\*.mp3"`) do (
        title Processing %%f
        ffmpeg -i "%%f" -acodec libvorbis -aq 6 "%%f.ogg"
    )
    title Done
)
