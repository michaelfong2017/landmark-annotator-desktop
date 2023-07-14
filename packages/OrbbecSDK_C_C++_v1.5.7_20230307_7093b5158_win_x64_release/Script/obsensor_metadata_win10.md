# obsensor_metadata_win10

Due to Windows system limitation, under default configuration, UVC protocal cannot access timestamp from the device, please follow the instructions below to change the registry if needed

1. Make sure camera is securely connected to the Windows system；
2. Open powershell as administrator, `cd` to `scripts` directory；
3. Excecute `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser` and press `Y` to confirm;
4. Excecute `.\obsensor_metadata_win10.ps1 -op install_all` to finish registration
