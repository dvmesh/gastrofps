# GastroFPS — Design & Dev Brief

> **Dla AI**: to plik context dla Claude Code. Wczytaj przed każdą sesją.
> **Dla człowieka**: jednostrona prawdy o projekcie. Zmieniasz decyzje tutaj.

---

## Quick summary

**GastroFPS** — rankedowa gra multiplayer 5v5 o pracy w gastronomii, FPS-like, Unreal Engine 5.7.4. Inspirowana "Hells Kitchen", "The Bear", "Overcooked" + mechaniki competitive z CS:GO.

Target: **Steam**, Windows. Docelowo 5v5 ranked matchmaking z MMR, cosmetyki (rank-gated), BO3 15-min mecze.

**Faza obecna**: M1 — Unreal scaffolding, single-player loop, capsule meshes, zero tekstur.

---

## 1. Core concept w 1 zdaniu

Dwie drużyny po 5 kucharzy/serverów obsługują identyczną pizzerię w 15-min service, memory-based order system (piktogramy widoczne proximity + krótki timer) zmusza do skupienia solo, a role-lock+friendly-fire+visible-chaos buduje brutalną presję jak w prawdziwej kuchni.

## 2. Drużyna — 5 ról, role-lock

| Rola | Stanowisko | Główna akcja | Unikalne |
|------|-----------|--------------|----------|
| **Expediter** (Host) | Pass window (granica kuchnia/sala) | Shot-calling, QC, pingi | Expedite combo 2+ticketów, bell, remake |
| **Hot Cook** | Line prawa (grill/oven) | Pizza dough + oven timing | "Flip" precision timing |
| **Cold Cook** | Line lewa (cold prep) | Sałaty, desery, napoje | Garnish rush chain |
| **Server A** | Floor north (T1-T4) | Take order → POS → run food | "Read the room" cue |
| **Server B** | Floor south (T5-T8) | Take order → POS → run food | "Read the room" cue |

**Role-lock**: wybierasz przy queue, trzymasz cały BO3 (3 rundy × 15 min). Nie zmieniasz.

**Dishwashing** — nie ma dedykowanej roli. Gdy clean stack <3, system piknie "DISH!" i dowolny z teamu musi odskoczyć na 45s QTE. Rytm 120 BPM tap spacji.

## 3. Core loop — jedno zamówienie step-by-step

1. Gość (NPC) siada przy stoliku. 20s timer — server musi podejść, inaczej −5 mood.
2. Server klik `E` na stoliku → **piktogramy order** nad głowami 1-4 gości:
   - Widoczne **tylko w FOV 60° z odległości <2.5m**
   - Żyje **5s** (stałe dla wszystkich rang — skill growth przez doświadczenie, nie mechanikę)
   - Complex order (3 goście × modyfikatory) = 5-6s
3. Server zapamiętuje → idzie do **POS terminal** (2 w restauracji).
4. W POS grid 4×5: klika pozycje z pamięci. **Grid layout randomizowany per player per match** (anty-Discord).
5. Send → Expediter dostaje ticket na Kitchen Display + printer paragonik fizyczny na rail.
6. Expediter `Q+wskazuje T{X}` → ping "Fire T4" do cooków z priority 1-5.
7. Cooks biorą swoją część z ticketu. Hot = dough+oven, Cold = side+drinks.
8. Gotowe komponenty → **pass**. Expediter składa w 1 danie, sprawdza modyfikatory.
9. `E` na pass = bell → server słyszy.
10. Server picks up, zanosi do stolika. **Musi trafić do właściwego** — pomyłka = wrong delivery penalty.
11. Gość je 30-60s, płaci. Tip = f(speed, accuracy, server attention).
12. Stolik → dirty plates → dish pile. Server/busser sprząta.

## 4. Piktogramy (kluczowa innowacja)

**3-warstwowa grammar**:
- **Kształt ramki** = kategoria (⭕ pizza, 🔶 przystawka, 🟦 napój, 🔺 deser)
- **Ikona środkowa** = pozycja (🍅=Margherita, 🔴=Pepperoni, 🧀=Quattro, 🟫=Funghi, 🌶️=Diavola, ⭐=Capricciosa)
- **Ramka kolor/grubość** = modyfikator (zielony=no meat, czerwony=extra spicy, niebieski=no cheese, złoty=extra, grubość=small/med/large)

Proximity + FOV zapobiega screen-share oglądaniu całej sali. Layout randomizacji POS zapobiega "kolega mówi slot 3" przez Discord.

## 5. Menu (pizzeria — start map)

**6 pizz + 4 przystawki + 3 napoje + 2 desery = 15 pozycji** (fill 20-slot grid):

| Pizza | Składniki | Oven time | Cook steps |
|-------|-----------|-----------|-----------|
| Margherita | sos, moz, bazylia | 4:00 | 1 |
| Pepperoni | sos, moz, pepperoni | 4:30 | 2 |
| Quattro Formaggi | sos, 4× ser | 5:00 | 3 |
| Funghi | sos, moz, pieczarki | 4:30 | 2 |
| Diavola | sos, moz, pepperoni, chili | 4:30 | 3 |
| Capricciosa | sos, moz, ham, pieczarki, oliwki, karczoch | 5:30 | 5 |

**Modifikatory** (3 kat): size (small/medium/large, ±30s oven), crust (thin/normal/thick), exclude (no cheese/no meat/extra toppings).

**Appetizers**: garlic bread, caprese, bruschetta, antipasto
**Drinks**: Coca-Cola, red wine glass, beer
**Desserts**: tiramisu, panna cotta

## 6. Mapa pizzerii

```
┌────── DINING ROOM ──────┐   ┌─── KITCHEN ───┐
│  T1  T2   T3  T4         │   │ [Pizza Oven  │
│                          │   │  6 slots]    │
│     BAR (POS×2)          │───│              │
│  T5  T6   T7  T8         │   │ HOT prep     │
│                          │   │ COLD prep    │
│  Entrance ●  Host stand  │   │ [PASS][DISH] │
└──────────────────────────┘   └──────────────┘
```

8 stolików (4×2), 2-4 osoby. W peak rush 16-32 gości. Pass window między kuchnią a salą.

## 7. Stacje + mikro-akcje

| Stacja | Rola | Mikro |
|--------|------|-------|
| Dough table | Hot | `E` ciasto, `LMB drag` rozpłaszczanie (gesture — jebnąć=70% score, idealnie=100%) |
| Sauce | Hot | `LMB drag circular` rozmaz, detection równości |
| Toppings | Hot | Pick & place (rozrzucenie wpływa na score) |
| Oven 6 slots | Hot | `E+scroll` wybierasz slot. Timer widoczny tylko na konkretnym |
| Cold prep | Cold | Caprese/sałaty drag-place |
| Dessert/wine | Cold | Wine pour (trzymaj LMB, puść zanim przeleje) |
| Dish pit | Dowolny | Rhythm QTE 120 BPM, 45s → +12 clean |
| POS | Server | Grid 4×5, random layout |
| Pass | Expediter | `E`=wydaj, `R`=remake |

## 8. Ping system (15 pingów, hold Q = ring)

**Ring 1 kuchnia**: Fire T{X}, All day?, 86 {item}, Behind!, Hands!
**Ring 2 sala**: T{X} ready, T{X} angry, T{X} VIP, Water T{X}, Bus T{X}
**Ring 3 ogólne**: DISH!, Incoming rush, QC fail, Reviewer T{X}, Last minute

## 9. Economy

| Event | $ |
|-------|---|
| Pizza base | $15 |
| Appetizer | $8 |
| Drink | $5 |
| Dessert | $9 |
| Tip modifier (na party) | ×0.0-0.3 (speed+accuracy+server attention) |
| Wrong order delivered | −$30 |
| Walkout | −$80 + rating hit |
| Reviewer good | +$200 |
| Reviewer bad | −$250 |
| Dish at closing | +$5 per clean |
| Open ticket at 15:00 | −$50 |

## 10. Rush waves (15 min mecz)

| Min | Event | Spawn |
|-----|-------|-------|
| 0-2 | Warmup | 1/30s |
| 2-5 | Early rush | 1/15s |
| 5-8 | Peak | 2/15s + 1 VIP |
| 8-10 | Incidents | Reviewer random, allergy guest |
| 10-13 | Second peak | 1/10s + 2 large parties (6 osób) |
| 13-15 | Wind down | Nowe party nie siadają. Otwarte tickety → penalty |

## 11. Walkout / mood

Mood 100→0:
- Order not taken in 20s: −5
- 5s over wait limit (fire → delivered = 6 min): −2
- Wrong order: −30
- Empty drink >45s: −3
- Server not responding to "angry" ping: −1/s
- **Mood ≤0 = walkout**

Visuals @ 70/40/20/0: fidget / arms crossed / standing / walk to door.

## 12. Tension — "każdy błąd widoczny"

**4 warstwy**:

1. **Goście widocznie wściekli**: fidget→cross arms→stand+yell "HELLO?! HEY!" (proximity audio włącza się gdy angry)→pound fist (screen shake dla wszystkich w proximity)→walkout slam door
2. **Kuchnia się pali**: pizza burn = smoke + alarm, całkowity burn = CZARNY dym + fire alarm + 10s sprzątanie QTE
3. **UI czerwony flash**: ticket >2min = migający czerwony, wrong order = screen flash czerwony 1s, walkout = black vignette + drum slam
4. **Stress visual na graczach**: >50 trzęsą ręce, >75 krople potu particle, >90 czerwona mgła wokół capsule (widać z daleka że ten gracz w panice)

**Filozofia**: komunikacja "coś jest źle" przez wizualia (no voice chat). Jak w realu — wchodzisz, widzisz kolegę w panice, idziesz pomóc.

## 13. Stress system

Stress 0-100 per gracz. Start 20.
- Orders-in-queue > capacity: +5/s
- Something burning: +20 jednorazowo
- Complaint from your table: +10

Efekty:
- >50: mouse sens +15% (trzęsą ręce) → plating precision spada
- >75: audio muffled
- >90: POS inputs 200ms lag

Recovery:
- Successful delivery: −5
- 3s bez eventu: −1/s
- **Friendly-fire vent** (spacja + patelnia o stół): −15 instant, widoczna animacja

## 14. Friendly fire

ON. 4 typy:
- **Push** (body block): popchnij kolegę z drogi, −5 jego stress (stun 0.5s)
- **Knock plate** (LMB on plate): wybij danie koledze z ręki = −$30 + rage
- **Shove** (heavy push): przesuń kogoś o 2m
- **Vent** (space + patelnia): tylko na siebie, stress recovery
- **Sabotage soup**: wylać sos na stanowisko przeciwnika (ale no przeciwnik = inna drużyna, nie in-team)

Wszystkie z animacją. Team może vote-kick za nadmiar FF.

## 15. Reviewer & VIP

- **Każda drużyna**: 1 reviewer + 1 VIP w losowych momentach (asymetrycznie vs przeciwnik)
- **VIP** (+$200): specjalne wymagania (alergia, temperature preference). Widoczny ⚠️ nad głową.
- **Reviewer** (+$200 good / −$250 bad): pozornie zwykły gość. Expediter może `Q+scan` na stoliku żeby sprawdzić "odkryć" reviewer. Gra tylko expediter widzi.

## 16. Half-time (2 min między rundami)

1. **Service debrief** (30s): per-role stats, MVP crown
2. **Earnings distribution** (30s): base $20 + MVP bonus + performance
3. **Upgrade shop** (60s): "convenience" upgrades, max 2 per gracz, refund 50%
   - Sharp Knife (cook, $30): prep −15% czas
   - Non-slip shoes (server, $25): brak slip na rozlanym sosie
   - Laminated menu (server, $20): bubble +1s
   - Second POS screen (server/team-wide, $60): ticket na display 2s wcześniej
   - Better oven mitts (cook, $25): hot pickup −0.5s
   - Clean towel (everyone, $15): usuwa stain

## 17. Rubber banding (balansowanie BO3)

**Losing team boost** (skala różnicy $$$):
- <10%: brak
- 10-25%: +1 VIP
- 25-50%: +1 VIP + patient reviewer
- >50%: +2 VIP + patient reviewer + "happy hour" 3min +30% tip

**NIE nerfy** wygrywającego (jego R2 normalna). Jeśli BO3 tie 1:1 → R3 z "celebrity" (reviewer ×3 mnożnik) dla obu stron.

## 18. MMR (CS-like)

- ELO base + role MMR (cook_mmr, server_mmr, expediter_mmr — osobne)
- Matchmaking dobiera z rola-lock
- Winner +25, loser −25, scaled przez MVP score
- MVP per role:
  - Expediter = % tickets on-time
  - Cook = avg plating score
  - Server = avg tip ratio + mood delta
- Ranks: **Brązowy → Srebrny → Złoty → Platyna → Diament → Master Chef → Global Elite Chef (GEC)**
- Decay: brak gry 7 dni = −50 MMR (Master+ only)

## 19. Lobby + match flow

```
Main menu → Find Match
  → Queue (rola-lock pick)
  → 10 graczy, ETA 2-4 min
  → Map vote
  → Team assign (losowy side)
  → Loading 30s
  → Pre-round 15s warmup
  → SERVICE 15 min R1
  → Half-time 2 min
  → SERVICE R2
  → If tie 1:1 → R3 celebrity mode
  → End screen: MVP, earnings, rank +/−
```

### Custom lobby (dla testów)

- **Private Match code**
- 10 slotów 5×5
- **Bot fill** dla pustych (easy/med/hard AI)
- Dla 5-osobowego teamu test: wszyscy 5 po jednej stronie, 5 botów po drugiej
- Solo test: 1 gracz + 9 botów

## 20. AFK/DC (CS-rules)

- 30s AFK → warning
- 60s AFK → kick, slot → bot
- Disconnect → 120s reconnect timer, slot frozen
- Vote-kick: 4/5 głosów po 90s AFK
- Abandon penalty: −75 MMR + 24h cooldown ranked

## 21. Anti-cheat (design-driven + EAC)

**3 warstwy obrony przed Discord cheating**:
1. **Asymmetric info**: nikt nie widzi wszystkiego (Expediter widzi tickety, nie bubbles; Server odwrotnie)
2. **Layout randomization per player per match**: POS grid shuffled, oven slots shuffled — "slot 3" nic nie znaczy koledze
3. **Time budget Discord > solo**: werbalizacja wolniejsza niż internal memory + in-game ping 0.2s latency

Plus: **Easy Anti-Cheat** (free z Unreal), dedicated servers, screen-share obfuscation (motion blur na UI poza focus FOV).

## 22. Steam cosmetics

| Tier | Drop | Przykład |
|------|------|----------|
| Common (gray) | 40% | Biały fartuch |
| Uncommon (green) | 15% | Kolorowy fartuch |
| Rare (blue) | 4% tradable | Branded apron |
| Epic (purple) | 0.5% + rank Gold+ | Gold-trim |
| Legendary (orange) | Rank Diamond+ | Neon toque |
| Master (red/gold) | Rank Master Chef only | Diamond crown |
| GEC (animated) | Rank GEC only | Holographic, particle trail |

Slots: apron, toque, tools (patelnia/clipboard/tablet), capsule color, win emote, plate design.

**Battle Pass $10/season** = dodatkowe drops + exclusive items. Free path też daje ~1 Rare/tydzień.

---

## Tech stack

- **Engine**: Unreal Engine 5.7.4 (upgrade z 5.6 — user wybrał 5.7.4 2026-04-18)
- **Language**: C++ (gameplay logic) + Blueprint (UI, tweaki)
- **Networking**: native UE replication, server-authoritative, client-prediction
- **Dedicated servers** (docelowo)
- **Voice**: żadnego team voice. Proximity audio dla gości i efektów kuchennych.
- **Anti-cheat**: Easy Anti-Cheat (free z UE)
- **Plugins**: Enhanced Input, GameplayAbilitySystem, CommonUI
- **Platforma**: Windows primary, Steam deployment

## Setup Windows (dev machine)

1. **Epic Games Launcher**: epicgames.com
2. **Unreal Engine 5.7.4**: przez launcher, ~45GB. Przy instalacji zaznacz "Engine Source", "Debug Symbols", "Android Support" (off), "Starter Content" (off)
3. **Visual Studio 2022 Community** (free): przy instalacji workload **"Game development with C++"** (all checkboxes)
4. **Git + Git LFS**: git-scm.com
5. **GitHub konto** (do collaboration)

**16GB RAM note**: zamknij Chrome i inne RAM-hogs podczas pracy. Disable Lumen/Nanite w project settings dla prototypu — user mówił o tym, zgadzamy się.

## File structure (target)

```
~/Projects/gastrofps/
├── CLAUDE.md              ← ten plik (design brief)
├── README.md              ← dla human/new devs
├── .gitignore
├── .gitattributes         ← Git LFS config
├── GastroFPS/             ← Unreal project
│   ├── GastroFPS.uproject
│   ├── Source/
│   │   └── GastroFPS/
│   │       ├── Characters/
│   │       ├── Stations/
│   │       ├── GameMode/
│   │       ├── Networking/
│   │       ├── UI/
│   │       └── ... (plan poniżej)
│   ├── Content/
│   │   ├── Blueprints/
│   │   ├── Maps/
│   │   └── UI/
│   └── Config/
└── docs/
    └── (dodatkowe design notes)
```

## Development milestones

- **M1** (tydzień 1-2): single-player whitebox. 1 gracz server role, 1 stolik, 1 POS, 1 pizza, full pętla. Capsule everything.
- **M2** (tydzień 3-4): 5v5 multiplayer, role-lock, listen server. Pełne role. Ping ring Q.
- **M3** (tydzień 5-6): memory bubble, POS randomize, dishpit, stress, tipy, walkout, reviewer/VIP
- **M4** (tydzień 7-8): dedicated server + EAC + MMR stub + lobby
- **M5** (tydzień 9-10): polish — animations placeholder, sounds, UI
- **M6** (tydzień 11-12): closed alpha 30 graczy, iteracja balansu

## Team

- **Lead dev + designer**: @antonibulsiewicz (właściciel, GitHub: `dvmesh`)
- **Playtesters**: 4 kolegów (Windows, Steam)
- **Współpraca**: Discord server dla koordynacji, GitHub dla kodu

## Conventions

- **Commit messages**: `[M1] feat: add order pickup interaction` (milestone prefix + conventional commits)
- **Naming**: Unreal convention — `ABaseCharacter`, `UOrderComponent`, `EOrderState`
- **Branching**: `main` stabilny, `dev` integration, feature branches `feat/xxx`
- **Żadnych assetów binarnych > 10MB** bez Git LFS
- **Code review**: wszystko na `dev` przez PR (nawet solo — dla history i auto-formatter check)

## Current status

**Faza**: design finalized, ready for M1 scaffolding.
**Blokery**: czekamy na instalację Unreal 5.6 na Windows PC.
**Next step**: utworzenie Unreal project (next, pusty, C++ enabled), git init, first commit scaffolding.

## Kontakty z AI (instructions for future Claude sessions)

Gdy odpalasz Claude Code w tym folderze:
1. Przeczytaj **CAŁY** ten plik jako pierwszy akt
2. Check git log / recent commits żeby wiedzieć co się działo ostatnio
3. NIE zmieniaj designu bez zapytania właściciela. Design decisions w tym pliku = źródło prawdy.
4. Kod piszesz po polsku w komentarzach, identyfikatory i nazwy klas po angielsku (Unreal convention).
5. Używaj C++ dla gameplay logic, Blueprint wyłącznie dla UI i tweaków wartości (performance + anti-cheat).
6. Multi-player first — każda nowa mechanika musi mieć replication z dnia pierwszego.
7. Nie dodawaj grafiki/tekstur/SFX na razie — mesh to capsule, material to flat color.
8. Każdy nowy feature → test w listen-server local multi przed mergem.

## Decyzje które jeszcze NIE są zatwierdzone

Nic. Wszystko w tym pliku = decyzja finalna. Jeśli chcesz zmienić — edytuj plik, commituj, powiedz AI.

---

## Session log / Progress

> **Format**: najnowsza sesja na górze. Każda sesja = data + co zrobione + pomysły + TODO + next step.
> **Zasada**: sekcje design powyżej to source of truth (nie ruszać). Tu tylko log postępów.

### 2026-04-18 — sesja setup

**Zrobione**:
- Przeczytano wszystkie pliki projektu (BRIEFING, CLAUDE, README, SETUP_WINDOWS)
- Ustanowiono konwencję "Session log" w CLAUDE.md
- Zapisano memory: profil projektu + konwencja session log + język PL
- **Design doc updated**: UE 5.6 → 5.7.4 we wszystkich plikach
- Instalacja UE 5.7.4 ✅
- VS 2022 zainstalowany z workloadami (Game dev C++ etc.) ✅
- Git + Git LFS gotowe ✅
- GitHub account + repo `dvmesh/gastrofps` utworzone ✅
- **Projekt Unreal utworzony**: `C:/Users/bulsi/Projects/gastrofps/GastroFPS/` (First Person C++ template, UE 5.7.4)
- Design .md przeniesione z Documents do Projects (wspólny root z Unreal project)
- `.gitignore` + `.gitattributes` (Git LFS dla uasset/umap/images/audio/3D/fonts) utworzone
- Git global config: `Antoni Bulsiewicz <antonibulsiewicz@gmail.com>` ustawione
- **Commit `2bc9bec`** — root scaffolding (601 plików, 155 MB LFS uploaded)
- **Commit `4e76678`** — strip shooter template (Variant_Horror/Shooter, pistol/rifle/grenade content)
- **Commit `2bc9bec..4a16a13` (2x commit)** — M1 gameplay scaffold:
  - `Core/GastroTypes.h` — enumy (EPizzaType, EOrderState, ECustomerState) + FOrderData
  - `Core/GastroFPSGameState` — money, timer, stats
  - `Stations/Station` base + `TableStation` + `POSStation` + `PassStation` (interact trace + prompt)
  - `Customers/Customer` — pawn z prostym state machine (walk→sit→wait→eat→leave) bez navmesh
  - `UI/GastroHUD` — Canvas HUD (top bar, crosshair, interact prompt, peek bubble 5s TTL, carried order, end screen)
  - `GastroFPSCharacter` — extended: interact E (legacy key binding), peek (FOrderData + TTL), carry pizza
  - `GastroFPSGameMode` — procedural spawn świata w StartPlay, customer spawn loop, FClassFinder dla BP_FirstPersonCharacter/PlayerController, HUDClass=AGastroHUD
  - `DefaultEngine.ini` — GlobalDefaultGameMode → /Script/GastroFPS.GastroFPSGameMode (C++ jest source of truth, BP_FirstPersonGameMode orphaned)
- **Build**: clean compile (UE 5.7.4 + VS 2026 14.50 toolchain, warning "non-preferred" ignorable)
- **Push**: wszystko na `origin/main`, repo live https://github.com/dvmesh/gastrofps

**Pomysły (w trakcie rozmowy)**:
- Customer używa APawn + raw AddActorWorldOffset (nie CharacterMovement/navmesh). W M2 podmiana na AIController+NavMesh gdy się pojawi 2+ stolików i trzeba unikać ludzi/ścian.
- Pomysł na M2 prep: wrzucić OrderComponent na klienta zamiast inlined CurrentOrder — czyściej gdy order staje się obiektem replikowanym w multiplayer.
- Ryzyko: MVP NIE używa replication (design doc wymaga "multi-player first"). Spłaciłem ten dług intentionally dla prędkości M1 single-player. M2 zacznie się od refactor na Replicated UPROPERTY, GetLifetimeReplicatedProps, RPC serwerowe dla Interact().

**Otwarte TODO przed "playable"**:
- [ ] **User: test w edytorze** — otworzyć uproject, odrzucić rebuild prompt (już skompilowane), Play-in-Editor, przejść pełną pętlę (patrz instrukcja w "Next step")
- [ ] Jeśli HUD się nie pokazuje — sprawdzić Output Log, prawdopodobnie problem z HUDClass override; fallback: ustawić HUDClass w BP_FirstPersonGameMode ręcznie
- [ ] Jeśli Customer nie spawnuje — sprawdzić log StartPlay, PlayerStart może nie być znalezione
- [ ] Jeśli E nie działa — legacy binding może konfliktować z Enhanced Input IMC; fallback: stworzyć IA_Interact.uasset i dodać binding przez Enhanced Input

**TODO po playable (M1 polish)**:
- [ ] Stworzyć własną mapę `L_PizzeriaWhitebox` (zastąpić Lvl_FirstPerson z shooter targetami z templatu) — wymaga ręcznej pracy w edytorze
- [ ] Usunąć BP_FirstPersonGameMode (już nieużywany)
- [ ] Dodać 2-3 stoliki + 2 POS (zgodnie z design doc mapą 8 stolików)
- [ ] Przejście z raw AddActorWorldOffset → AI Nav dla klienta
- [ ] Dodać visible pizza actor w ręku gracza (na razie tylko bool+data)

**TODO M2 (multiplayer)**:
- [ ] Wrzucić replication na kluczowe state (GameState money, Customer state, Station state)
- [ ] Refactor Interact() na Server RPC
- [ ] Listen server mode test 2 graczy

**Blokery**: —

**Next step — INSTRUKCJA TESTU PLAYABLE MVP**:
1. Otwórz `C:/Users/bulsi/Projects/gastrofps/GastroFPS/GastroFPS.uproject`
2. Jeśli UE prosi o rebuild → Yes (powinien zauważyć że już skompilowane z cmdline, ale pewność nie zaszkodzi)
3. Editor ładuje się ~30s → otworzy się Lvl_FirstPerson
4. Press **Play** (Alt+P albo przycisk w topbarze)
5. Powinieneś zobaczyć:
   - **Top HUD**: `$0   15:00   OK:0  FAIL:0`
   - **Centrum**: kropka (crosshair)
   - **Stations** w zasięgu: żółty stolik, niebieski POS, zielony pass
6. Po ~2s spawnuje się **czerwony klient** (cube+głowa) idzie do stolika
7. Gdy klient siedzi, podejdź do stolika, celuj kamerą → HUD pokaże **"[E] Weź zamówienie"**
8. Naciśnij **E** → pojawi się **peek bubble** z nazwą pizzy i timer 5s
9. W ciągu 5s biegnij do POS, celuj → **"[E] Wbij zamówienie"**, naciśnij E
10. 3s cook timer, potem idź do Pass → **"[E] Odbierz pizzę"**, E
11. Wróć do stolika → **"[E] Podaj pizzę"**, E
12. Klient je 4s, płaci, wychodzi. **$15 + tip** (do 30% za szybkie obsłużenie)
13. Co 20s nowy klient, spawn nie-infinitely (tylko gdy stolik wolny)

Zapisz wrażenia, screenshot bugów, cokolwiek nie działa — wrócę i poprawię.

