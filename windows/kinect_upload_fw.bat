REM First of all, make sure the drivers from ''libfreenect'' are installed, 
REM they are in ''platform/windows/inf'' in the libfreenect code base..
REM 
REM Download the MSR Kinect SDK from:
REM http://download.microsoft.com/download/8/4/C/84C9EF40-EE49-42C2-AE26-C6E30921182F/KinectSDK32.msi
REM 
REM Explore ''KinectSDK32.msi'' using 7-zip from http://www.7-zip.org, extract the 
REM ''media1.cab'' file in it and get the firmware file, named ''UACFirmware.*''
REM 
REM Put ''kinect_upload_fw.exe'', the firmware file and ''kinect_upload_fw.bat'' 
REM file in the same directory and double_click on the ''kinect_upload_fw.bat'' 
REM file.

Set FIRMWARE="UACFirmware.C9C6E852_35A3_41DC_A57D_BDDEB43DFD04"

kinect_upload_fw.exe %FIRMWARE%
@PAUSE
