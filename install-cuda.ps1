$url = "https://developer.nvidia.com/compute/cuda/10.1/Prod/network_installers/cuda_10.1.105_win10_network.exe"
$output = "$PSScriptRoot\install-cuda.exe"
(New-Object System.Net.WebClient).DownloadFile($url, $output)
.\install-cuda.exe -s nvcc_10.1 