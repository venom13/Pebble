# BN_WATCHFACE

Pebble BN_Watchface kodu

Kodun düzgün çalışması için PebbleCloud daki projenin ayarlarında "Uses Location" ve "Uses Health" seçili olmalıdır.

Ayrıca "Message Key Assignment Kind" --> "Manuel Assignment" olarak seçilmeli.
Message Key ler de
TEMPERATURE - 0
CONDITIONS - 1
LOCATION - 2
POSITION_X - 3
POSITION_Y - 4

Aynı Font lar kullanılacaksa Phenomena Bold ve ExtraBold fontları import edilmeli. Import edilecek font boyutları için main.c de kullanılan fontlar ve boyutları incelenebilir. Farklı bir font kullanılacaksa main.c deki font satırları değiştirilmelidir.
