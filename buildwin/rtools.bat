@echo off
cd..
cd bin_tools
@rem zetools.exe
@rem zetools.exe packer -pack base pak01
zetools.exe packer -scan base.dat
cd..
cd buildwin
@echo on