# KRAA1-line-following-robot-analog

Robotek na experimentální DPS, který sleduje čáru a zastaví před překážkou. Postaven na Arduino Nano, řadiči motorů TB6612FNG, používá 5 IR čidel TCRT5000, 6V motorky N20, 2 trimry pro ladění parametrů a 2 LED (modrou a červenou) pro signalizaci různých stavů. Dále je připojeno tlačítko a 2 spínače. Jeden spínač pro napájení digitální části, druhý spínač pro napájení motorků (Toshiba v dokumentaci k TB6612 píše, že nikdy nemá být připojeno napájení motorků dříve, než je pod napětím digitální část). Tlačítko slouží pro spuštění/zastavení jízdy.

Čidla jsou čtena analogově, i když v reálu to moc rozdílné oproti digitálnímu čtení není, je ale tím pádem potřeba před jízdou dělat kalibraci senzorů. Ta naběhne automaticky po zapnutí, stačí autíčko postavit někam k čáře, případně - pokud není zapnuto napájení motorů - s ním ručně pohybovat senzory nad čarou.
