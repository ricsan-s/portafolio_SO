REM Integrantes : Ricardo Sanchez Aguilar, Hernández Bernal Angel Said, Lancón Barragán Erick Aarón
REM IPN ESCOM SO 4CV4
REM Sistema de Operaciones Matematicas 
@echo off
setlocal enabledelayedexpansion
title Sistema de Operaciones Matematicas - IPN ESCOM
cls

:: Verificar si el archivo de usuarios existe
if not exist usuarios.txt (
    echo Creando archivo de usuarios...
    echo. > usuarios.txt
    timeout /t 2 /nobreak >nul
)

:menuPrincipal 
cls
echo ====================================
echo    INSTITUTO POLITECNICO NACIONAL
echo    Escuela Superior de Computo
echo ====================================
echo.
echo        SISTEMA MATEMATICO
echo.
echo 1. Iniciar Sesion
echo 2. Registrarse
echo 3. Salir
echo.
set /p "opcion_principal=Seleccione una opcion: "

if "!opcion_principal!"=="1" goto inicioSesion
if "!opcion_principal!"=="2" goto registrarse
if "!opcion_principal!"=="3" goto salir_programa

echo.
echo Opcion no valida por favor intente nuevamente.
echo.
pause
cls
goto menuPrincipal 

:inicioSesion
cls
echo ================================
echo        INICIAR SESION
echo ================================
echo.
set /p "usuario_input=Usuario: "
set /p "password_input=Contraseña: "
:: Limpiar espacios en blanco
set "usuario_input=!usuario_input: =!"
set "password_input=!password_input: =!"

:: y si no se llenan los campos
if "!usuario_input!"=="" (
    echo ERROR: Debe ingresar un usuario
    pause
    goto menuPrincipal 

)

if "!password_input!"=="" (
    echo ERROR: Debe ingresar una contraseña
    pause
    goto menuPrincipal 

)

set "inicioExitoso=0"
::Lectura de usuario y contraseña almacenados en archivo de texto "usurios.txt" con formato usuario,contraseña
for /f "tokens=1,2 delims=:" %%a in (usuarios.txt) do (
    :: Limpiar espacios de los valores del archivo, no borrar por que el inicio de sesion se romperia
    set "usuarioEntrada=%%a"
    set "password=%%b"
    set "usuarioEntrada=!usuarioEntrada: =!"
    set "password=!password: =!"
    :: usuario y contraseña correctas 
    if "!usuarioEntrada!"=="!usuario_input!" (
        if "!password!"=="!password_input!" (
            set "inicioExitoso=1"
            set "usuario=!usuario_input!"
        )
    )
)

if !inicioExitoso! equ 1 (
    echo.
    echo ¡Bienvenido !usuario!!
    echo Fecha: %date%
    echo Hora: %time%
    echo.
    pause
    cls
    goto menu_matematico
) else (
    echo.
    echo ERROR: Usuario o contraseña incorrectos
    echo.
    pause
    cls
    goto menuPrincipal 

)
::opciones de resgistrar nuevo usuario
:registrarse
cls
echo ================================
echo         REGISTRARSE
echo ================================
echo.
set /p "nuevoUsuario=Nuevo usuario: "
set /p "nuevopassword=Nueva contraseña: "

:: Limpiar espacios en blanco
set "nuevoUsuario=!nuevoUsuario: =!"
set "nuevopassword=!nuevopassword: =!"

if "!nuevoUsuario!"=="" (
    echo.
    echo ERROR: Debe ingresar un usuario
    echo.
    pause
    cls
    goto registrarse
)

if "!nuevopassword!"=="" (
    echo.
    echo ERROR: Debe ingresar una contraseña
    echo.
    pause
    cls
    goto registrarse
)

:: Calcular longitud de la contraseña
set "longitud=0"
for /l %%i in (0,1,100) do (
    if not "!nuevopassword:~%%i,1!"=="" set /a longitud=%%i+1
)

:: Verificar que la contraseña tenga al menos 4 caracteres
if !longitud! lss 4 (
    echo.
    echo ERROR: La contraseña debe tener al menos 4 caracteres
    echo.
    pause
    cls
    goto registrarse
)

:: Verificar si el usuario ya existe 
set "ExisteUsuario=0"
if exist usuarios.txt (
    for /f "usebackq tokens=1 delims=:" %%a in ("usuarios.txt") do (
        set "temporalUsuario=%%a"
        set "temporalUsuario=!temporalUsuario: =!"
        if "!temporalUsuario!"=="!nuevoUsuario!" (
            set "ExisteUsuario=1"
        )
    )
)

if !ExisteUsuario! equ 1 (
    echo.
    echo ERROR: El usuario '!nuevoUsuario!' ya existe
    echo.
    pause
    cls
    goto registrarse
)

:: Guardar nuevo usuario en el archivo 
echo !nuevoUsuario!:!nuevopassword!>> usuarios.txt
echo.
echo ¡Usuario registrado exitosamente!
echo Se ha guardado: !nuevoUsuario!:!nuevopassword!
echo.
pause
goto menuPrincipal 

:menu_matematico
cls
echo ================================
echo        MENU MATEMATICO
echo ================================
echo Usuario: !usuario!
echo ================================
echo 1. Suma
echo 2. Resta
echo 3. Multiplicacion
echo 4. Division
echo 5. Cerrar Sesion
echo 6. Salir del programa
echo ================================
set /p "opcionMatematica=Seleccione una opcion: "

if "!opcionMatematica!"=="1" goto suma
if "!opcionMatematica!"=="2" goto resta
if "!opcionMatematica!"=="3" goto multiplicacion
if "!opcionMatematica!"=="4" goto division
if "!opcionMatematica!"=="5" goto cerrar_sesion
if "!opcionMatematica!"=="6" goto salir_programa

echo.
echo Opcion no valida. Intente nuevamente.
echo.
pause
cls
goto menu_matematico

:suma
echo.
set /p "numero1=Ingrese un numero: "
set /p "numero2=Ingrese el segundo numero: "
set /a "resultado=numero1+numero2"
echo.
echo Resultado: !numero1! + !numero2! = !resultado!
echo.
pause
cls
goto menu_matematico

:resta
echo.
set /p "numero1=Ingrese un numero: "
set /p "numero2=Ingrese el segundo numero: "
set /a "resultado=numero1-numero2"
echo.
echo Resultado: !numero1! - !numero2! = !resultado!
echo.
pause
cls
goto menu_matematico

:multiplicacion
echo.
set /p "numero1=Ingrese un numero: "
set /p "numero2=Ingrese el segundo numero: "
set /a "resultado=numero1*numero2"
echo.
echo Resultado: !numero1! x !numero2! = !resultado!
echo.
pause
cls
goto menu_matematico

:division
echo.
set /p "numero1=Ingrese un numero: "
set /p "numero2=Ingrese el segundo numero: "

if !numero2! equ 0 (
    echo.
    echo ERROR: No se puede dividir por cero
    echo.
    pause
    cls
    goto menu_matematico
)

set /a "resultado=numero1/numero2"
set /a "residuo=numero1%%numero2"
echo.
echo Resultado: !numero1! / !numero2! = !resultado!
if not !residuo! equ 0 echo Residuo: !residuo!
echo.
pause
cls
goto menu_matematico

:cerrar_sesion
echo.
echo Cerrando sesion de !usuario!
echo.
pause
cls
goto menuPrincipal 

:salir_programa
echo.
echo Gracias por usar el sistema !
echo Grupo 4CV4 2026/1 
echo Sistemas Operativos. 
echo.
pause
exit