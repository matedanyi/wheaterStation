Szia!

Szóval most olvashatod a kéretlen beszámolómat erről a kis projektről. Az alap koncepció egy ESP32-alapú hő- és páramérő, ami x-időnként mérés végez és kiírja a kijelzőre. Ez kész is volt, szerintem mutattam is. Nem nagy dolog, Ctrl+c, Ctrl+v és már kész is.

Persze, ahogy keresgél az ember szembe jön vele egy rakat hasznos vagy haszontalan projekt. Ezek összességéből alakult ki a fejemben, hogy kell egy időjárás logger, ami kijelzőre viszi az adatot, ezután menti adatbázisba is, ahonnan már egy weblapot lehet rá írni, grafikonokkal stb.

Funkciók:
-méri a benti hő- és páratartalmat DHT22 szenzor
-méri a kinti hőmérsékletet DS18B20 szenzor
-lekéri NTP szerverről az időt
-ezeket megjeleníti a hozzákapcsolt kijelzőn SSD1106 Oled 128x64
-fixen minden 10. percben végez mérést, mert a hivatalos napi átlag hőmérséklet mérés 7 és 19 óra között történik 10 percenként. Ezt elengedtem, jó lesz a 10 percenkénti is, nem kell percre pontosan reggel 7-kor méri, valószínű javítok majd rajta, mert ilyenkor csak az órát állítja be NTP szerverről. És a belső órája chip-enként eltérést mutat.Egy idő után elkezd késni újraindításig. Én viszont állandó üzemelésre tervezem a dolgot

-előző sor javítva
-wifi manager hozzáadva

Különböző problémákba ütköztem a prog nyelv ( C ) és a tapasztalat hiánya miatt.

Több library-t használtam a projektben, de mindegyik elég gyengén dokumentált, szóval órákat töltöttem az infók összeollózásával, ameddigre eredményre jutottam.

Például a NTPClient lib-ben volt getFormattedDate(), de nálam nem működött, 4 órámba telt, hogy kiderítsem, az a funkció kikerült belőle, vagy sosem létezett ezt nem tudom, de egy másik srác beletette, csak azt felejtette el közölni, hogy ő a módosított lib-et használja, szóval ami nyilvánosan elérhető abban nem jó. Kézzel kellett frissíteni az övére, aztán annak a verziószámát átírni, hogy az IDE-m ne akarja frissíteni az újabb, de eredeti könyvtárra. Igen ebbe is beleszaladtam egy gép restart alkalmával. Utána meg néztem, hogy miért nem jó.

Egy másik könyvtár (u8g2) a kijelző megjelenítéséért felel. Nagyon jó, a dokumentáció is tűrhető, csak épp a getting started hiányzik. Meg a kulcs mondat, hogy mindig u8g2.clearBuffer()-el kezdünk és u8g2.sendBuffer();-el zárunk! A kettő közé megy a kód. Ameddig erre rájöttem elszaladt 3 óra. Végre tudtam középre helyezni szöveget, hogy nem nekem kellett karaktert számolni pixel pontosan, hanem megcsinálja a függvény. A hátulütője, hogy minden szövegem egy button-be van zárva.

Az óra kiírása a kijelzőre is egy külön kihívás volt, mert egy stringet kapok az NTP szerverről. Ezt a u8g2 lib nem fogadja el, mert ő csak char[] -el hajlandó dolgozni. Külön nehézség volt, hogy az arduino-ban nem létezik typeof() függvény… Magadnak kell csinálni. Ezzel is elszórakoztam 2-3 órát, amíg rájöttem, hogy mi a hiba. Utána már egyszerű volt keresni rá megoldást.

Összességében egyáltalán nem volt haszontalan ez a projekt. Meggyőződtem arról, hogy nem akarok C-ben vagy más típusos nyelvben programozni ahol fordítani is kell a programokat. Maradok a JS és társainál, ahol egy F5 megoldja a tesztelést. Nem kell egy 300 soros kód lefordításra és flash-elésére 2-5 percet várni. És biztos vagyok benne, hogy ezt a projektet nem tudtam volna elkezdeni sem mondjuk 3 éve. Azóta megtanultam helyesen megfogalmazni a keresésem tárgyát, értelmezni az összefüggéseket az ismeretlen példakódokban legyen az akár egy teljesen idegen nyelv, mint például most a C.

Elég jó kihívás volt ez a feladat számomra, de még hátra van a honlap és adatbázis helyrerázása. Kell egy jobban átlátható, szervezettebb felület, szűréssel dátumra plusz normális grafikonok.

Kicsit el is gondolkodtatott ez a projekt, hogy a mindennapi fejlesztés is ilyen bonyolult? Mert ebben az esetben az effektív programozás a rá fordított idő 10%-át ha kitöltötte! Vagy ez pusztán a tapasztalatom hiánya és a programnyelv nem ismerése végett alakult így? Mondjuk JS-ben hamarabb megírtam volna. Már csak azért is mert ismerem a syntax-ot.

Update:

Ma újabb két könyvtárral bővítettem az programot. Van wifi manager, első indításnál vagy ha nincs elmentett hálózat, akkor létrehozza a saját hálózatát, amire bármilyen wifi képes eszközzel, (legjobb telefon, tablet, mert automatikusan a hitelesítési oldalra irányít) fel lehet csatlakozni és az adott config honlapon kell beírni az internettel rendelkező hálózat jelszavát. Utána AP kikapcs és indul a program.

Került bele egy task manager is, ami végre unix időből számolja a kezdeti 10 percet. Ezzel azt elérve, hogy a httprequest nem akkor kerül először elküldésre amikor bekapcsolom az eszközt. Hanem vár amíg x óra x perce 10-el nem osztható. Ezzel elérve, hogy az adatbázis mindig egész 10 perces adatot kapjon. Az első indítás után, ezt már a belső timer végze, ami korrigálja a minimális driftet. Így hosszabb használat után is marad a maximum 1 mp-es csúszás az exponenciálisan növekvő helyett
