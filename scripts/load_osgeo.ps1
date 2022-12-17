# from https://gist.github.com/Guts/6303dc5eb941eb24be3e27609cd46985

$starter_path = Get-Location

md $env:OSGEO4W_ROOT

# Download installer if not exists
if (-Not (Test-Path "$env:OSGEO4W_ROOT\osgeo4w-setup.exe" -PathType leaf )) {
   Write-Host "= Start downloading the OSGeo4W installer"
   Invoke-WebRequest -Uri "https://download.osgeo.org/osgeo4w/v2/osgeo4w-setup.exe" -OutFile "$env:OSGEO4W_ROOT\osgeo4w-setup.exe"
   if (Test-Path "$env:OSGEO4W_ROOT\osgeo4w-setup.exe" -PathType leaf)
   {
       Write-Host "== Installer downloaded"
   }
   else
   {
       Write-Host "== Installer not downloaded"
   }
}
else
{ Write-Host "= OSGeo4W installer already exists. Let's use it!" -ForegroundColor Blue }

Set-Location $env:OSGEO4W_ROOT

# Install OSGEodep
Write-Host "=== Start installing OSGEO dependencies..."
Write-Host "=== Destination:"
$env:OSGEO4W_ROOT
Write-Host "================================================="

.\osgeo4w-setup.exe `
    --advanced `
    --arch x86_64 `
    --autoaccept `
    --delete-orphans `
    --local-package-dir "$env:APPDATA/OSGeo4W-Packages" `
    --menu-name "QGIS LTR" `
    --no-desktop `
    --packages qgis-deps `
    --root $env:OSGEO4W_ROOT `
    --quiet-mode `
    --site "http://download.osgeo.org/osgeo4w/v2" `
    --upgrade-also `
| out-null

Write-Host "=== Finished installing OSGEO dependencies..."

Set-Location $starter_path

