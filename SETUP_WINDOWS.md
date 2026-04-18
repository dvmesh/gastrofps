# Setup Windows — krok po kroku

Ten plik = dokładna instrukcja gdy siadasz na Windows PC i chcesz zacząć pracować nad GastroFPS.

## 1. Epic Games Launcher

- https://www.epicgames.com/store/pl/download
- Download → install → zaloguj/załóż konto Epic
- Nie musisz kupować nic — Unreal jest darmowy

## 2. Unreal Engine 5.7.4

W Launcher → zakładka "Unreal Engine" (po lewej) → "Library" → "+" obok "ENGINE VERSIONS":
- Wybierz **5.7.4** (najnowszy stable)
- Rozmiar: ~45GB (zajmie 20-90 min zależnie od internetu)
- **Options** → zaznacz:
  - ✅ Engine Source (pomaga gdy Unreal wywali)
  - ✅ Editor symbols for debugging (debugging C++)
  - ✅ Templates and Feature Packs
  - ❌ Target Platforms: Android, iOS, Linux (oszczędź miejsce, tylko Windows)
  - ❌ Starter Content (nie potrzebujemy gotowych assetów)

Kliknij Install.

## 3. Visual Studio 2022 Community

- https://visualstudio.microsoft.com/downloads/ → Community (free)
- Uruchom installer
- Zakładka **Workloads** → zaznacz:
  - ✅ **Game development with C++** (cały workload, automatycznie dopina wszystko potrzebne)
  - ✅ Desktop development with C++
- Individual components (sprawdź czy zaznaczone):
  - ✅ Windows 10/11 SDK (najnowszy)
  - ✅ MSVC v143 toolset
  - ✅ Unreal Engine installer (!)

Install (~10GB).

Po instalacji **uruchom Visual Studio raz** i zaloguj się (free konto Microsoft).

## 4. Git + Git LFS

- https://git-scm.com/download/win → install (all defaults)
- Otwórz **PowerShell** jako admin, wpisz:
  ```
  git lfs install
  ```

## 5. GitHub

- https://github.com → załóż konto (jeśli nie masz)
- Powiedz mi swój username → utworzę repo dla projektu i dam ci dostęp

## 6. Discord

- Pobierz Discord, dołącz do serwera projektu (stworzysz jak będzie gotowe) albo zrób dla team playtestingowego

## 7. Opcjonalnie: Port forwarding dla multiplayer testów

Na M2 stage, dla testów z kolegami, potrzebujesz albo:
- **Router port forward** port 7777 UDP (trudniej, ale lepsza jakość)
- **Hamachi** (https://vpn.net) lub **ZeroTier** (darmowe, VPN prywatna) — każdy z kumpli instaluje, joinuje do twojej sieci. Łatwiejsze.

Zaczniesz od opcji drugiej.

## 8. Specyfikacja

Twoja: 16GB RAM, najnowszy Unreal, Windows. OK.

**Zalecenia dla komfortu pracy**:
- Zamknij Chrome/Edge i inne RAM hogs podczas pracy w Unreal
- Włącz "Game Mode" w Windows (Settings → Gaming → Game Mode ON)
- Unreal w **Project Settings** → Rendering → wyłączamy Lumen, Nanite (później)
- Editor → Preferences → General → Performance → Tick Editor Viewport "Realtime" OFF gdy nie testujesz — oszczędza RAM

## 9. Test install

Po instalacji wszystkiego:
- Otwórz Epic Launcher → UE 5.6 → Launch
- New Project → Games → **Third Person** (tylko do testu install) → C++ → Project Name `TestInstall` → Create
- Unreal kompiluje ~5-10 min pierwszy raz (tworzy VS solution)
- Jak się uruchomi i widzisz Third Person template z bohaterem który biega → **install OK**
- Zamknij, usuń TestInstall folder. Mamy gotowy dev env.

## 10. Następny krok

Daj mi znać gdy wszystkie kroki zielone ✅ — wtedy:
1. Utworzymy real Unreal project `GastroFPS`
2. Push na GitHub
3. Zaczniemy M1 scaffolding

## Known gotchas

- **"Cannot compile project"** — uruchom Visual Studio Installer → repair VS
- **Unreal crash at startup** — disable antivirus real-time protection na folder UE
- **Very slow compile** — exclude Unreal project folder z Windows Defender scanning
- **Out of memory compile** — reduce MSBuild parallel jobs (Edit > Preferences w VS > Projects > Build and Run > Maximum number of parallel project builds = 2)
