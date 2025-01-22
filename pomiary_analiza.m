 close all; 

% Wczytaj dane z pliku CSV
data = readtable('data.csv');

% Przypisz dane do wektorów
Czas = data.Czas ;      % Kolumna "Czas", podzielona przez 10
Dystans = data.Dystans;     % Kolumna "Dystans"
Pozycja = data.Pozycja;     % Kolumna "Pozycja"
PWM = data.PWM;             % Kolumna "PWM"
Enkoder = data.Enkoder;     % Kolumna "Enkoder"

minLength = 40
Czas = Czas(1:minLength);
Dystans = Dystans(1:minLength);
Pozycja = Pozycja(1:minLength);
PWM = PWM(1:minLength);
Enkoder = Enkoder(1:minLength);

% Wykres danych
figure;

% Lewa oś Y
yyaxis left;
hold on;
plot(Czas, Dystans, '-', 'DisplayName', 'Dystans', 'Color', 'blue');   % Dystans vs Czas
plot(Czas, Pozycja, '-', 'DisplayName', 'Pozycja', 'Color', 'red');    % Pozycja vs Czas
%plot(Czas, Enkoder, '-', 'DisplayName', 'Enkoder', 'Color', 'green');  % Enkoder vs Czas
hold off;
ylabel('Wysokość [cm]');

% Prawa oś Y
yyaxis right;
plot(Czas, PWM, '-', 'DisplayName', 'PWM');        % PWM vs Czas
ylabel('Wypełnienie [%]');

% Dodanie legendy
legend('show', 'Location', 'best');

% Opis osi
xlabel('Czas [s]');

% Tytuł wykresu (opcjonalnie)
title('Wykres danych z pliku CSV');
grid on;