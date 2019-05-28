$url = "https://developer.nvidia.com/compute/cuda/10.1/Prod/network_installers/cuda_10.1.168_win10_network.exe"
$output = "$PSScriptRoot\install-cuda.exe"

Write-Output "Downloading: $url"
(New-Object System.Net.WebClient).DownloadFile($url, $output)

Write-Output "Installing"
Start-Process -FilePath $output -ArgumentList "-s nvcc_10.1 visual_studio_integration_10.1" -Wait -NoNewWindow