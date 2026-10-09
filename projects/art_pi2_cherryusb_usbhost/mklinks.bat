@echo off
cd /d "%~dp0"
@echo on
mklink /D rt-thread ..\..\rt-thread
mklink /D libraries ..\..\libraries