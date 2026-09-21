# Kusznik — zastępstwo chłopa

Kusznik ma własną grafikę (niebieski strój, hełm z piórem, kusza): 48 klatek
walki, portret, miniatury w paskach armii i animację na mapie przygody.

Statystyki: atak 7, obrona 7, obrażenia 4–8, zdrowie 20, szybkość 4,
12 strzałów, przyrost podstawowy 5, koszt 250 złota, brak zdolności specjalnych.
Dźwięki ataku wręcz, strzału, ruchu, obrażeń i śmierci są dźwiękami łucznika.

## Zakres zamiany

Zachowany jest identyfikator `Monster::PEASANT`, więc **wszyscy chłopi**, także
w starych mapach, armiach i zapisach, stają się Kusznikami. Liczebność oddziałów
już zapisanych w mapach/zapisach nie zmienia się. Nowi bohaterowie otrzymują
3–5 Kuszników; nowe losowe grupy mają 12–25 sztuk i kategorię siły 3.

Jednostka zajmuje pierwsze siedlisko Rycerza (Strzelnica kuszników). Numer
„Level 4” z H3SW nie przenosi jej do czwartego siedliska Heroes II.
Przyrost jak u Pikiniera: 5 tygodniowo, +2 ze Studnią.

Zmiany zależne od siły Kusznika (chłop był ~35× słabszy):

- **Farma Rycerza** daje +200 złota dziennie zamiast +8 do przyrostu
  (`ProfitConditions::FromBuilding`, `Castle::GetGrownWel2`). Inne rasy bez zmian.
- **Chata kuszników** (dawna Chłopska chata) na mapie: 3–8 Kuszników na start,
  +1–2 tygodniowo, za darmo — siłą jak Dom łuczników; w generatorze map
  wyceniona jak Dom łuczników (2000).
- **Neutralne zamki Rycerza**: najsłabsze posiłki garnizonu to 1–2 Kuszników
  (zamiast 8–15), czyli nadal mniej niż 5–7 Łuczników.
- **AI Rycerza** buduje Strzelnicę kuszników wcześnie, a Farmę jak inne
  budynki dochodowe.

Wybrano dopuszczony wariant jednego poziomu. Silnik nie ma `DWELLING_UPGRADE1`,
a maska budynków wykorzystuje wszystkie 32 bity. Strzelec wymaga osobnego
rozszerzenia systemu budynków, interfejsu, AI i zapisu gry — jest technicznie
możliwy, ale nie został dodany w tej wersji.

## Grafika

Gra wczytuje pakiet z `files/crossbowman/` **obok uruchamianego pliku gry**
(build Visual Studio kopiuje go tam sam):

- `CROSSBOW.ICN` — 48 klatek walki (klatka 0 zarezerwowana).
- `XBOWFRM.BIN` — sekwencje animacji tych klatek (`source/sequences.json`).
- `CROSSBOWH.ICN` — portret jednostki.

`CROSSBOW.ICN` i `XBOWFRM.BIN` działają tylko razem; bez nich gra używa
kompletu grafiki łucznika. Zwykły łucznik pozostaje bez zmian. Miniatury
(`MONS32`, `MINIMON`) są skalowane w grze z klatek postoju.

Pakiet powstaje z atlasu `source/blue-crossbowman-atlas.png` (siatka 8×6):

```powershell
python script/build_crossbowman_assets.py --resources build/crossbow-assets
```

Katalog `--resources` zawiera `KB.PAL` i `ARCHRFRM.BIN` z własnych danych gry.
Klatki marszu (9–16) są narysowane w miejscu; skrypt dodaje do nich postęp
łucznika z `ARCHRFRM.BIN`, bo w walce postać przesuwa się o własne `x` klatki.
Grafika budynku pozostaje oryginalna.

## Kontrola ręczna

1. Nowa gra → Bitwa: pierwsza pozycja listy jednostek to Kusznik.
2. Sprawdź 7/7, 4–8, 20 HP, szybkość 4, 12 strzałów; bohater może podnosić 7/7.
3. Oddaj strzał: amunicja maleje o jeden, cel otrzymuje obrażenia, gra nie zawiesza się.
4. Nowa rozgrywka Rycerzem: pierwsze siedlisko rekrutuje Kuszników po 250 złota.
5. Łucznik i Łowca zachowują swoje statystyki i animacje.

Test integracyjny (Developer PowerShell / MSBuild, katalog główny repozytorium):

```powershell
MSBuild fheroes2-vs2019.vcxproj /p:Configuration=Release-SDL2 /p:Platform=x64 /p:PlatformToolset=v143
MSBuild crossbowman-test.vcxproj /p:Configuration=Release-SDL2 /p:Platform=x64 /p:PlatformToolset=v143
& ./build/x64/Release-SDL2/crossbowman-test.exe
```

Test wymaga lokalnych zasobów gry, tak jak samo `fheroes2.exe`. Sprawdza
rzeczywiste API jednostki, koszt/przyrost, brak ulepszenia, dźwięki i komplet
sekwencji ruchu/strzału oraz zgodność przesunięć z łucznikiem. Asercje testu
są aktywne także w Release. Przed testem zawsze przebuduj grę.
