@rem vsvars32.bat does not check if it has already been run and can overflow
@rem the PATH variable if it is run continually
@rem ----
@if "%GEN_VS10_RUN%" == "" (
   call "%VS100COMNTOOLS%vsvars32.bat"
   set "GEN_VS10_RUN=1"
)