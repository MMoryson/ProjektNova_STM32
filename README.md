# ProjektNova_STM32
Jest to projekt na zajęcia z Systemów Mikroprocesorowych w ramach studiów Automatyka i Robotyka na Politechnice Poznańskiej

Projekt ma na celu stowrzenia układu sterowania wysokością piłeczki ping pongowej w tubie za pomocą wiatraka.

Projekt jest stworzony na płytce NUCLEO-F746ZG (z mikrokontrolerem STM32F746ZG)
Użyte elementy:
- enkoder inkrementalny (20 impulsów na obrót)
- wyświetlacz ssd1306 oled komunikujący się za pomocą I2C 
- ultradźwiękowy hc-sr04
- tranzystor n mosfet tht irl540npbf (100 Vds)
- wiatrak (50x50mm, Napięcie: DC 5-12V, Prąd: 0.3A, RPM(11.1V): 3500, 50x50x10mm, 20g)
- zasilacz (9V/0.6A)
- kondensatory 10nF
- reyzstory 1.1 kOhmów i 10 kOhmów
- przewody
