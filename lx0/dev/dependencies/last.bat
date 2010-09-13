mkdir sdk
mkdir sdk\lib
mkdir sdk\include
mkdir sdk\include\v8
call copy v8.lib sdk\lib
call copy include\* sdk\include\v8
