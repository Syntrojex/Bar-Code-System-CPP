param(
    [string]$QtRoot,
    [string]$MinGWRoot,
    [int]$Jobs = 4,
    [switch]$SkipTests
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$deps = Join-Path $repo 'build/deps'
$build = Join-Path $repo 'build/local-release'
$package = Join-Path $repo 'build/portable/BarcodeHub'
function Run([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed with exit code $LASTEXITCODE" }
}

if (!$QtRoot) {
    $kits = @(Get-ChildItem 'C:/Qt/6.*/mingw_64','D:/Qt/6.*/mingw_64' -Directory -ErrorAction SilentlyContinue |
        Sort-Object { [version]$_.Parent.Name } -Descending)
    if (!$kits.Count) { throw 'Install Qt 6 for MinGW with its matching compiler, or pass -QtRoot and -MinGWRoot.' }
    $QtRoot = $kits[0].FullName
}
$qtBase = Split-Path (Split-Path $QtRoot -Parent) -Parent
$config = Get-Content (Join-Path $QtRoot 'mkspecs/qconfig.pri') -Raw
$gccMajor = [regex]::Match($config, 'QT_GCC_MAJOR_VERSION = (\d+)').Groups[1].Value
$gccMinor = [regex]::Match($config, 'QT_GCC_MINOR_VERSION = (\d+)').Groups[1].Value
if (!$MinGWRoot) {
    foreach ($candidate in Get-ChildItem (Join-Path $qtBase 'Tools/mingw*') -Directory) {
        $compiler = Join-Path $candidate.FullName 'bin/g++.exe'
        if (!(Test-Path $compiler)) { continue }
        $version = & $compiler -dumpfullversion
        if ($version -like "$gccMajor.$gccMinor.*") { $MinGWRoot = $candidate.FullName; break }
    }
}
if (!$MinGWRoot) { throw 'No matching Qt MinGW compiler found. Pass -MinGWRoot explicitly.' }
$cmake = Join-Path $qtBase 'Tools/CMake_64/bin/cmake.exe'
$ninja = Join-Path $qtBase 'Tools/Ninja/ninja.exe'
$deploy = Join-Path $QtRoot 'bin/windeployqt.exe'
foreach ($file in @($cmake,$ninja,$deploy,(Join-Path $MinGWRoot 'bin/g++.exe'))) {
    if (!(Test-Path $file)) { throw "Missing dependency: $file" }
}
$originalPath = $env:PATH
try {
    $env:PATH = "$MinGWRoot/bin;$QtRoot/bin;$(Split-Path $ninja -Parent);$originalPath"
    New-Item -ItemType Directory -Force $deps,$package | Out-Null
    $opencv = Join-Path $deps 'opencv'
    if (!(Test-Path (Join-Path $opencv 'OpenCVConfig.cmake'))) {
        $source = Join-Path $deps 'opencv-src'
        if (!(Test-Path (Join-Path $source 'CMakeLists.txt'))) {
            $archive = Join-Path $deps 'opencv-4.13.0.zip'
            $source = Join-Path $deps 'opencv-4.13.0'
            if (!(Test-Path (Join-Path $source 'CMakeLists.txt'))) {
                Write-Host 'Downloading official OpenCV 4.13.0 sources...'
                Invoke-WebRequest 'https://codeload.github.com/opencv/opencv/zip/refs/tags/4.13.0' -OutFile $archive
                Expand-Archive -LiteralPath $archive -DestinationPath $deps -Force
            }
        }
        # Some existing source caches contain only the modules used by this app.
        $dlpackLicense = Join-Path $source '3rdparty/dlpack/LICENSE'
        if (!(Test-Path $dlpackLicense)) {
            New-Item -ItemType Directory -Force (Split-Path $dlpackLicense -Parent) | Out-Null
            Invoke-WebRequest 'https://raw.githubusercontent.com/opencv/opencv/4.13.0/3rdparty/dlpack/LICENSE' -OutFile $dlpackLicense
        }
        $opencvBuild = Join-Path $deps 'opencv-build'
        # OpenCV 4.13 uses non-standard M_PI here, unavailable in strict MinGW C++.
        # Its own CV_PI constant is equivalent and portable.
        $ellipse = Join-Path $source 'modules/imgproc/src/shapedescr.cpp'
        $ellipseText = [IO.File]::ReadAllText($ellipse)
        [IO.File]::WriteAllText($ellipse, $ellipseText.Replace('angle_deg * M_PI', 'angle_deg * CV_PI'))
        Run $cmake @('-S',$source,'-B',$opencvBuild,'-G','Ninja',
            '-DCMAKE_BUILD_TYPE=Release',"-DCMAKE_C_COMPILER=$MinGWRoot/bin/gcc.exe",
            "-DCMAKE_CXX_COMPILER=$MinGWRoot/bin/g++.exe","-DCMAKE_MAKE_PROGRAM=$ninja",
            "-DCMAKE_INSTALL_PREFIX=$opencv",'-DBUILD_LIST=core,imgproc,imgcodecs,videoio',
            '-DBUILD_SHARED_LIBS=ON','-DBUILD_TESTS=OFF','-DBUILD_PERF_TESTS=OFF',
            '-DBUILD_EXAMPLES=OFF','-DBUILD_opencv_apps=OFF','-DBUILD_JAVA=OFF',
            '-DBUILD_opencv_python3=OFF','-DWITH_FFMPEG=OFF','-DWITH_MSMF=OFF',
            '-DWITH_DSHOW=ON','-DWITH_IPP=OFF','-DWITH_OPENCL=OFF','-DWITH_OPENEXR=OFF',
            '-DWITH_WEBP=OFF','-DWITH_AVIF=OFF','-DWITH_JASPER=OFF','-DWITH_TIFF=OFF',
            '-DWITH_OPENJPEG=OFF','-DWITH_PROTOBUF=OFF','-DWITH_FLATBUFFERS=OFF','-DCPU_DISPATCH=')
        Run $cmake @('--build',$opencvBuild,'--parallel',"$Jobs")
        Run $cmake @('--install',$opencvBuild)
    }
    Run $cmake @('-S',$repo,'-B',$build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release',
        "-DCMAKE_CXX_COMPILER=$MinGWRoot/bin/g++.exe","-DCMAKE_MAKE_PROGRAM=$ninja",
        "-DCMAKE_PREFIX_PATH=$QtRoot","-DOpenCV_DIR=$opencv",'-DBUILD_TESTING=ON')
    Run $cmake @('--build',$build,'--parallel',"$Jobs")
    Run $deploy @('--release','--no-translations','--no-system-d3d-compiler','--no-opengl-sw',
        '--compiler-runtime',"$build/BarcodeHub.exe")
    Copy-Item (Join-Path $QtRoot 'plugins/platforms/qoffscreen.dll') (Join-Path $build 'platforms/qoffscreen.dll') -Force
    if (!$SkipTests) {
        Run (Join-Path (Split-Path $cmake -Parent) 'ctest.exe') @('--test-dir',$build,'--output-on-failure')
    }
    $builtExe = Join-Path $build 'BarcodeHub.exe'
    $portableExe = Join-Path $package 'BarcodeHub.exe'
    try {
        Run $cmake @('-E','copy_if_different',$builtExe,$portableExe)
    } catch {
        # Windows permits renaming an executing image. Keep that session intact
        # and retain its executable while publishing the new build for next launch.
        if (!(Test-Path $portableExe)) { throw }
        $previousExe = Join-Path $package ("BarcodeHub.previous-" + [guid]::NewGuid().ToString('N') + '.exe')
        Move-Item -LiteralPath $portableExe -Destination $previousExe
        Copy-Item -LiteralPath $builtExe -Destination $portableExe
        Write-Host "Running session preserved at $previousExe; restart BarcodeHub to use the new build."
    }
    foreach ($dll in Get-ChildItem $build -Filter '*.dll') {
        Run $cmake @('-E','copy_if_different',$dll.FullName,(Join-Path $package $dll.Name))
    }
    Run $deploy @('--release','--no-translations','--no-system-d3d-compiler','--no-opengl-sw',
        '--compiler-runtime',"$package/BarcodeHub.exe")
    $licenses = Join-Path $package 'licenses'
    New-Item -ItemType Directory -Force $licenses | Out-Null
    Copy-Item (Join-Path $repo 'LICENSE') (Join-Path $licenses 'BarcodeHub-LICENSE.txt') -Force
    Copy-Item (Join-Path $opencv 'LICENSE') (Join-Path $licenses 'OpenCV-LICENSE.txt') -Force
    Get-ChildItem (Join-Path $opencv 'etc/licenses') -File -Recurse -ErrorAction SilentlyContinue |
        Copy-Item -Destination $licenses -Force
    $qtLicenses = Join-Path $qtBase 'Licenses'
    if (Test-Path $qtLicenses) {
        $qtLicenseTarget = Join-Path $licenses 'Qt'
        New-Item -ItemType Directory -Force $qtLicenseTarget | Out-Null
        Get-ChildItem $qtLicenses -File | Copy-Item -Destination $qtLicenseTarget -Force
    }
    Write-Host "Ready: $package/BarcodeHub.exe"
} finally {
    $env:PATH = $originalPath
}
