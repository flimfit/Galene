$version = "10.1.105_418.96_win10"
$url = "https://developer.nvidia.com/compute/cuda/10.1/Prod/local_installers/cuda_$version.exe"
$output = "$PSScriptRoot\install-cuda.exe"
$cuda_root = "$PSScriptRoot\cuda\nvcc\"

Write-Output "Downloading: $url"
(New-Object System.Net.WebClient).DownloadFile($url, $output)

Write-Output "Expanding: $output"
&7z x $output -ocuda nvcc
Remove-Item $output

Write-Output "Setting environment variable: CUDA_PATH = $cuda_root"
[Environment]::SetEnvironmentVariable("CUDA_PATH", $cuda_root, "User")
[Environment]::SetEnvironmentVariable("CUDA_PATH_V10_1", $cuda_root, "User")
[Environment]::SetEnvironmentVariable("Path",
    [Environment]::GetEnvironmentVariable("Path", "User") + ";" + $cuda_root + "bin",
    "User")