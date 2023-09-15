:: Script to copy dependent DLLs to the named build directory and build the manual
::
:: Usage:
::
::    .\make-manual.bat x64\{Debug,Release} {htmldoc.exe,htmldocd.exe}
::

copy packages\libcups2_native.redist.2.4.3.4\build\native\bin\x64\Debug\*.dll %1
copy packages\libjpeg-turbo-v142.2.0.4.3\build\native\bin\x64\v142\Release\*.dll %1
copy packages\libpng_native.redist.1.6.30\build\native\bin\x64\Release\*.dll %1
copy packages\libressl_native.redist.3.7.3\build\native\bin\x64\Debug\*.dll %1
copy packages\zlib_native.redist.1.2.11\build\native\bin\x64\Release\*.dll %1

%1\%2 --verbose --datadir .. --path ../doc --batch ../doc/htmldoc.book -f ../doc/htmldoc.pdf
