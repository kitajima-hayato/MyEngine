# convert.ps1
# 目的:
#  - resources 配下の *.png を再帰検索して TextureConverter.exe で .dds へ変換
#  - 出力は png と同じフォルダ（同階層に .dds）
#  - UIフォルダ配下の png は -nc（無圧縮）で変換してぼけを防ぐ
#  - ログは resources から下の相対パスで記録（例: _Common\UI\a.png）

param(
  [string]$ResourcesRoot = "",
  [int]$MipLevels = -1,
  [switch]$SkipIfExists = $true
)
if ([string]::IsNullOrWhiteSpace($ResourcesRoot)) {
  # 右クリック実行でもわかることを期待 $PSCommandPath を優先
  $scriptPath = $PSCommandPath
  if ([string]::IsNullOrWhiteSpace($scriptPath)) {
    # 最終手段：カレント
    $ResourcesRoot = (Get-Location).Path
  } else {
    $scriptDir = Split-Path -Parent $scriptPath
    $ResourcesRoot = Split-Path -Parent $scriptDir  # resources
  }
}
$ErrorActionPreference = "Stop"

# スクリプトと同じ場所にある exe を使う（カレント依存を排除）
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ExePath   = Join-Path $ScriptDir "TextureConverter.exe"

if (!(Test-Path $ExePath)) {
  throw "TextureConverter.exe が見つかりません: $ExePath"
}

if (!(Test-Path $ResourcesRoot)) {
  throw "ResourcesRoot が見つかりません: $ResourcesRoot"
}

# ResourcesRoot を正規化（末尾の \ を除去）
$ResourcesRoot = (Resolve-Path $ResourcesRoot).Path.TrimEnd('\','/')

# ログは resources フォルダ直下に出す
$LogPath = Join-Path $ResourcesRoot "convert_log.csv"
if (!(Test-Path $LogPath)) {
  "time,status,src_png,dst_dds,message" | Out-File -Encoding UTF8 $LogPath
}

Write-Host "ResourcesRoot: $ResourcesRoot"
Write-Host "Exe         : $ExePath"
Write-Host "Log         : $LogPath"

# png を再帰検索
$pngFiles = Get-ChildItem -Path $ResourcesRoot -Recurse -File -Include *.png

$converted = 0
$skipped   = 0
$failed    = 0

foreach ($f in $pngFiles) {

  # resources配下の相対パス（ログ用）
  $relPng = $f.FullName.Substring($ResourcesRoot.Length).TrimStart('\','/')
  $relPng = $relPng -replace '/', '\'  # 表記統一

  # 出力先は同じフォルダ
  $dstDdsFull = [System.IO.Path]::ChangeExtension($f.FullName, ".dds")

  # dds側の相対パス（ログ用）
  $relDds = $dstDdsFull.Substring($ResourcesRoot.Length).TrimStart('\','/')
  $relDds = $relDds -replace '/', '\'

  if ($SkipIfExists -and (Test-Path $dstDdsFull)) {
    $skipped++
    $line = "{0},SKIP,{1},{2},{3}" -f (Get-Date -Format "s"), $relPng, $relDds, "already exists"
    Add-Content -Encoding UTF8 -Path $LogPath -Value $line
    continue
  }

  try {
    $exeArgs = @($f.FullName)
    if ($MipLevels -ge 0) {
      $exeArgs += @("-ml", "$MipLevels")
    }

    # UIフォルダ配下は無圧縮(-nc)で変換（BC7圧縮によるぼけを防ぐ）
    if ($relPng -match '(^|\\)UI(\\|$)') {
      $exeArgs += "-nc"
    }

    # 実行
    $p = Start-Process -FilePath $ExePath -ArgumentList $exeArgs -NoNewWindow -Wait -PassThru

    # 変換結果確認
    if (Test-Path $dstDdsFull) {
      $converted++
      $status = if ($relPng -match '(^|\\)UI(\\|$)') { "converted (no compress)" } else { "converted" }
      $line = "{0},OK,{1},{2},{3}" -f (Get-Date -Format "s"), $relPng, $relDds, $status
      Add-Content -Encoding UTF8 -Path $LogPath -Value $line
    } else {
      $failed++
      $line = "{0},NG,{1},{2},{3}" -f (Get-Date -Format "s"), $relPng, $relDds, "dds not found after conversion"
      Add-Content -Encoding UTF8 -Path $LogPath -Value $line
    }
  }
  catch {
    $failed++
    $msg = ($_ | Out-String).Trim().Replace("`r"," ").Replace("`n"," ").Replace(","," ")
    $line = "{0},NG,{1},{2},{3}" -f (Get-Date -Format "s"), $relPng, $relDds, $msg
    Add-Content -Encoding UTF8 -Path $LogPath -Value $line
  }
}

Write-Host "Done. OK=$converted SKIP=$skipped NG=$failed"
Write-Host "Log: $LogPath"