# Display pro meteostanici

Display pro meteostanici - ESP32 s ePaper driverem a ePaper displej.

Zobrazuje:
- východ a západ slunce
- fázi, východ a západ měsíce
- lidsky čitelnout předpověď Alojz.cz
- varování ČHMÚ
- předpověď po hodinách z yr.no
- stav lokální meteostanice

![Ukázka displeje](/doc/display.jpg "Ukázka displeje")

Jednotlivé vykreslované prvky jsou jako samostatné objekty; je tedy možné je snadno vytáhnout a použít ve vlastní aplikaci.

Defaultní konfigurace předpokládá tříbarevný (černá/bílá/červená) 4" ePaper displej 400x300 bodů. Displej se ovládá přes GxEPD2. Konfigurace displeje je v **ePaperConfig.h.** Konfigurace připojení displeje (pinů) je v **AppPins.h.**  Mělo by to fungovat s každým displejem podporovaným v GxEPD2; samozřejmě ale pro displeje jiných rozměrů je třeba změnit rozložení prvků na ploše. Pokud máte jen černobílý displej, pak nastavení barev je v DataAplikace.h.

Aplikace není optimalizovaná na paměť, není tedy možné ji provozovat na ESP8266. 

Není také optimalizovaná na spotřebu - počítá s tím, že procesor spí maximálně light sleepem (paměť zůstává zachovaná) a aplikace je tak odolná situaci, kdy nějakou dobu není dostupný nějaký online zdroj. Nicméně to lze snadno změnit a uspávat do deep sleep, pokud by spotřeba byla prioritní.

Při prvním startu se aplikace přepne do konfiguračního portálu - připojte se z telefonu na AP "RA_<nějaké číslo>" s heslem "aaaaaaaa" a z webového prohlížeče nakonfigurujte připojení na wifi.
Jak získáte potřebné konfigurační hodnoty [čtěte zde](https://pebrou.wordpress.com/2021/01/15/kostra-hotove-iot-aplikace-pro-esp32-esp8266-a-k-tomu-nejaky-server-3-n/#a-jak-to-spustit). Aplikace je postavená na IoT knihovně [RatatoskrIoT](https://github.com/petrbrouzda/RatatoskrIoT), proto je zde potřeba vložit přístupové údaje k serveru. Knihovna zajišťuje vzdálenou konfiguraci, OTA updaty, telemetrii a odesílání log souborů

Konfiguraci zobrazovaných dat, tedy zejména lokalitu, pro kterou se dává předpověď a vyhledávají varování ČHMÚ, můžete buď změnit nastavením defaultů v DataAplikace::DataAplikace(), nebo následně zaslat jako nastavení konfigurace ze serveru.

Konfigurační blok pro poslání ze serveru vypadá takto:
```
lat=49.8921
lon=14.5716
alt=430
idorp=2122
name=Teptin
alojz=jablonec-nad-nisou
```
- IDORP je číslo obce s rozšíženou působností - viz http://apl.czso.cz/iSMS/cisdet.jsp?kodcis=65 
- ALOJZ je jméno lokality, jaké najdete v URL na alojz.cz poté, co kliknete na jméno města
- NAME je zobrazované jméno místa
- LAT/LON/ALT je zeměpisná souřadnice a nadmořská výška

České fonty vznikly postupem popsaným zde https://github.com/petrbrouzda/fontconvert8-iso8859-2 

---


# Knihovny a kód třetích stran

## Nutné knihovny v Arduino IDE
V library manageru je nutné mít nainstalováno:
- Tasker 2.0.0
- MoonRise 2.0.1
- MoonPhase 1.0.3
- sunset 1.1.6
- GxEPD2 1.3.1 
- Adafruit_GFX_Library 1.10.0
- Adafruit_BusIO 1.4.1
- ArduinoJson 6.18.0

## Použité fonty
- Meteocons https://www.alessioatzeni.com/meteocons/
- moon-phases https://www.dafont.com/moon-phases.font
- Yanone Kaffeesatz https://fonts.google.com/specimen/Yanone+Kaffeesatz

## Knihovny a kód třetích stran 

Aplikace obsahují následující kód třetích stran ve formě zdrojových kódů distribuovaných přímo s aplikací (= nepoužívají se z library manageru):

### gfxlatin
- src\gfxlatin2\
- zdroj: https://www.sigmdel.ca/michel/program/misc/gfxfont_8bit_en.html
- licence: BSD 2-Clause License
- upraveno pro ISO-8859-2 viz https://github.com/petrbrouzda/fontconvert8-iso8859-2

### Tiny AES
- src\aes-sha\aes*
- zdroj: https://github.com/kokke/tiny-AES-c
- licence: public domain
- použito bez úprav

### CRC32
- src\aes-sha\CRC32*
- zdroj: https://github.com/bakercp/CRC32
- licence: MIT
- použito bez úprav

### SHA-256
- src\aes-sha\sha256*
- zdroj: https://github.com/CSSHL/ESP8266-Arduino-cryptolibs
- licence: public domain (dle https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c)
- použito bez úprav

### dtostrg
- src\math\
- zdroj: https://github.com/tmrttmrt/dtostrg
- licence: MIT
- použito bez úprav

### tzapu/WiFiManager
- src\wifiman\
- zdroj: https://github.com/tzapu/WiFiManager
- licence: MIT
- provedeny úpravy (např. možnost načtení SSID a hesla)

### kmackay/micro-ecc
- src\micro_ecc\
- zdroj: https://github.com/kmackay/micro-ecc
- licence: BSD-2-Clause License
- přejmenovány .inc -> .h, jinak žádné úpravy

